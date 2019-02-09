#include <stdio.h>
#include <string.h>
    #ifdef OS_Linux
    #include <sys/termios.h>
    #define TTY struct termios
    #else
    #ifdef __hpux
    #include <sys/bsdtty.h>
    #endif
    #include <sys/termio.h>
    #define TTY struct termio
    #endif
#include <unistd.h>	/* read() */
#include "ekio.h"

#include "ekaudio.h"
#include "eknote.h"
#include "vchannel.h"

#define EKCTRL(x) (x-'A'+1)

#ifndef IO_BUFFER_SIZE
#define	IO_BUFFER_SIZE	80
#endif
char		io_dbuff[IO_BUFFER_SIZE];
static	Flag	io_is_init= FL_FALSE;
#ifdef HAS_EKGUI
static	Flag	io_is_ekgui= FL_FALSE;
#endif
U_CHR		verbose= VBSF1_ALL,
		menunum= 0;

static TTY	sanitty, quietty;

void	(*ext_gui_error1)(const char *)= NULL;
void	(*ext_gui_warn1)(const char *)= NULL;
void	(*ext_gui_warn2)(const char *, const char *)= NULL;
void	(*ext_gui_warn3)(const char *, const char *, const char *)= NULL;
void	(*ext_gui_stat1)(const char *)= NULL;
void	(*ext_gui_stat2)(const char *, const char *)= NULL;
int	(*ext_gui_kbread)()= NULL;
Flag	(*ext_gui_special)(int, int *)= NULL;
void	(*ext_gui_writemenu)(char *)= NULL;
void	(*ext_gui_disp_songtitle)(char *)= NULL;
void	(*ext_gui_disp_sampdata)(int, EkInstr *)= NULL;
void	(*ext_gui_disp_blockhdr)(int, int, int, int, char *);
void	(*ext_gui_dbuff_blockpos)(int);
#ifdef SLOW_DISPLAY
void	(*ext_gui_dbuff_channel)(int, U_CHR, U_CHR, U_CHR);
#else
void	(*ext_gui_dbuff_channel)(const char *);
#endif
void	(*ext_gui_dbuff_flush)();

void set_verbosity(int v)
{
        verbose= v;
}

void error1(const char *e1)
{
#ifdef HAS_EKGUI
	if (io_is_init && io_is_ekgui)
	    ext_gui_error1(e1);
	else
#endif
	    fprintf(stderr, "%s\n", e1);
}

void warn1(const char *w1)
{
	if (verbose & VBSF1_WARN)
	{
#ifdef HAS_EKGUI
	    if (io_is_init && io_is_ekgui)
		ext_gui_warn1(w1);
	    else
#endif
		fprintf(stderr, "%s\n", w1);
	}
}

void warn2(const char *w1, const char *w2)
{
	if (verbose & VBSF1_WARN)
	{
#ifdef HAS_EKGUI
	    if (io_is_init && io_is_ekgui)
		ext_gui_warn2(w1, w2);
	    else
#endif
		fprintf(stderr, "%s %s\n", w1, w2);
	}
}

void warn3(const char *w1, const char *w2, const char *w3)
{
	if (verbose & VBSF1_WARN)
	{
#ifdef HAS_EKGUI
	    if (io_is_init && io_is_ekgui)
		ext_gui_warn3(w1, w2, w3);
	    else
#endif
		fprintf(stderr, "%s %s %s\n", w1, w2, w3);
	}
}

void stat1(const char *s1)
{
	if (verbose & VBSF1_STAT)
	{
#ifdef HAS_EKGUI
	    if (io_is_init && io_is_ekgui)
		ext_gui_stat1(s1);
	    else
#endif
		printf("%s\n", s1);
	}
}

void stat2(const char *s1, const char *s2)
{
	if (verbose & VBSF1_STAT)
	{
#ifdef HAS_EKGUI
	    if (io_is_init && io_is_ekgui)
		ext_gui_stat2(s1, s2);
	    else
#endif
		printf("%s %s\n", s1, s2);
	}
}

#ifdef HAS_EKGUI
EkErrFlag io_init(GUIFN_DECLARE_MEMBER(gui_init))
#else
EkErrFlag io_init()
#endif
{
  EkErrFlag	error= EK_ERR_NONE;

#ifdef HAS_EKGUI
	if (gui_init)
	{
	    if ( (error= gui_init()) == EK_ERR_NONE )
		io_is_ekgui= FL_TRUE;
	}
	else
	{
#endif
#ifdef OS_Linux
	    tcgetattr(fileno(stdin), &sanitty);
	    tcgetattr(fileno(stdin), &quietty);
	    quietty.c_cc[VMIN]= 0;  /* Put 1 if you want it to wait */
	    quietty.c_cc[VTIME]= 0;
	    quietty.c_lflag &= ~(ICANON|ECHO|ECHONL);
	    tcsetattr(fileno(stdin), TCSADRAIN, &quietty);
#else
	    ioctl(fileno(stdin), TCGETA, &sanitty);
	    ioctl(fileno(stdin), TCGETA, &quietty);
	    quietty.c_cc[VMIN]= 0;  /* Put 1 if you want it to wait */
	    quietty.c_cc[VTIME]= 0;
	    quietty.c_lflag &= ~(ICANON | ECHO);
	    ioctl(fileno(stdin), TCSETA, &quietty);
#endif
#ifdef HAS_EKGUI
	}
#endif
	io_is_init= FL_TRUE;

  return error;
}

#ifdef HAS_EKGUI
EkErrFlag io_restore(GUIFN_DECLARE_MEMBER(gui_restore))
#else
EkErrFlag io_restore()
#endif
{
  EkErrFlag	error= EK_ERR_NONE;

	if (io_is_init)
	{
#ifdef HAS_EKGUI
	    if (io_is_ekgui)
		error= gui_restore();
	    else
	    {
#endif
#ifdef OS_Linux
		tcsetattr(fileno(stdin), TCSADRAIN, &sanitty);
#else
		ioctl(fileno(stdin), TCSETA, &sanitty);
#endif
#ifdef HAS_EKGUI
	    }
#endif
	}

  return error;
}

static
int io_kbread()
{
#ifdef HAS_EKGUI
	if (io_is_ekgui)
	    return ext_gui_kbread();
	else
	{
#endif
	  char   key;
	    if (read(fileno(stdin), &key, 1) == 0) return EOF;
	  return (int) key;
#ifdef HAS_EKGUI
	}
#endif
}

void set_menu(int which)
{
	switch(menunum= which)
	{
	case 0:
	    strcpy(io_dbuff, "-EKAVIEL- ^Tune ^Instr ^Display");
	    break;
	case 1:
	    strcpy(io_dbuff, "[Tune] (M)ain");
	    break;
	case 2:
	    strcpy(io_dbuff, "[Instr] (M)ain (N)ext (P)rev (F)irst (L)ast");
	    break;
	case 3:
	    strcpy(io_dbuff, "[Display] (M)ain (P)lay (R)eport (W)arn (S)ync");
	    break;
	default:
	    error1("Code calls undefined menu!");
	}

#ifdef HAS_EKGUI
	if (io_is_ekgui)
	    ext_gui_writemenu(io_dbuff);
	else
#endif
	    fprintf(stderr,"%s\n", io_dbuff);
}

static
int io_keycmd_0(int key)
{
  int	command= EKVL_C_NONE;
  Flag	special=
#ifdef HAS_EKGUI
		io_is_ekgui? ext_gui_special(key, &command) :
#endif
		FL_FALSE;

	if (!special)
	    switch(key)
	    {
	    case EKCTRL('D'):
		set_menu(3);
		break;
	    case EKCTRL('I'):
		set_menu(2);
		break;
	    case EKCTRL('T'):
		set_menu(1);
		break;
	    }
  return command;
}

static
int io_keycmd_1(int key)
{	/* (T)une menu */
  int	command= EKVL_C_NONE;
	switch(key)
	{
	case 'm':
	    set_menu(0);
	    break;
	}
  return command;
}

static
int io_keycmd_2(int key)
{	/* (I)nstr menu */
  int	command= EKVL_C_NONE;
	switch(key)
	{
	case 'f':
	    command= EKVL_C_SAMFRST;
	    break;
	case 'l':
	    command= EKVL_C_SAMLAST;
	    break;
	case 'm':
	    set_menu(0);
	    break;
	case 'n':
	    command= EKVL_C_SAMNXT;
	    break;
	case 'p':
	    command= EKVL_C_SAMPRV;
	    break;
	}
  return command;
}

static
int io_keycmd_3(int key)
{	/* (D)isplay menu */
  int	command= EKVL_C_NONE;
	switch(key)
	{
	case 'm':
	    set_menu(0);
	    break;
	case 'p':
	    set_verbosity(verbose^VBSF1_PLAY);
	    break;
	case 'r':
	    set_verbosity(verbose^VBSF1_STAT);
	    break;
	case 'w':
	    set_verbosity(verbose^VBSF1_WARN);
	    break;
	case 's':
	    if (toggle_sync())
		stat1("[Synchronised display]");
	    else
		stat1("[Unsynchronised display]");
	    break;
	}
  return command;
}

int io_keycmd()
{
  int	key= io_kbread(),
	command= EKVL_C_NONE;
  Flag	special=
#ifdef HAS_EKGUI
		io_is_ekgui? ext_gui_special(key, NULL) :
#endif
		FL_FALSE;

	if (!special)
	    switch(key= io_kbread())
	    {
	    case EKCTRL('N'):
		command= EKVL_C_NEXT;
		break;
	    case EKCTRL('V'):
		if ( verbose & VBSF1_PLAY )
		    stat1("** Display frozen **");
		set_verbosity(verbose ^ VBSF1_PLAY);
		break;
	    case 27:	/* ESCape */
		stat1("ESCape keycode detected!");
		audio_discardBuffers();
		command= EKVL_C_QUIT;
		break;

	    case '<':
		command= EKVL_C_REW;
		break;
	    case '>':
		command= EKVL_C_FWD;
		break;
	    default:
		if (key!= EOF)
		{
		    switch (menunum)
		    {
		    case 0:
			command= io_keycmd_0(key);
			break;
		    case 1:
			command= io_keycmd_1(key);
			break;
		    case 2:
			command= io_keycmd_2(key);
			break;
		    case 3:
			command= io_keycmd_3(key);
			break;
		    }
#ifdef DEVEL
		    if (command == EKVL_C_NONE)
		    {
			sprintf(io_dbuff, "KEY ASCII %d", key);
			stat1(io_dbuff);
		    }
#endif
		}
	    }
  return command;
}

void disp_songtitle(char *title)
{
#ifdef HAS_EKGUI
	if (io_is_ekgui)
	    ext_gui_disp_songtitle(title);
	else
	{
#endif
	    printf("Song title: %s\n", title);
#ifdef HAS_EKGUI
	}
#endif
}

void disp_sampdata(int num, EkInstr *samp)
{
#ifdef HAS_EKGUI
	if (io_is_ekgui)
	    ext_gui_disp_sampdata(num, samp);
	else
	{
#endif
	    if ( EKINSTR_FIELD(*samp, name)
			&& strlen(EKINSTR_FIELD(*samp, name)) )
	    {
		sprintf(io_dbuff, "Instrument %2s: %-40s",
			instName[num+1], EKINSTR_FIELD(*samp, name));
	    }
	    else if (EKINSTR_FIELD(*samp, length))
	    {
		sprintf(io_dbuff, "Instrument %2s: %-40s",
					instName[num+1], "<Unnamed>");
	    }
	    else
	    {
		sprintf(io_dbuff, "Instrument %2s: %-40s",
					instName[num+1], "<Unused>");
	    }

	    printf("%s\n", io_dbuff);

	    sprintf(io_dbuff, "%20s Transp %+d,%d Vol %2d Len %6li",
			"", EKINSTR_FIELD(*samp, transpose),
			EKINSTR_FIELD(*samp, finetune),
			EKINSTR_FIELD(*samp, volume),
			EKINSTR_FIELD(*samp, length) );
	    if (EKINSTR_FIELD(*samp, rpt_len))
	    {
		sprintf(io_dbuff, "%s::%li/%li", io_dbuff,
			EKINSTR_FIELD(*samp, rpt_bgn),
			EKINSTR_FIELD(*samp, rpt_len));
	    }
	    else
	    {
		sprintf(io_dbuff, "%s%20s", io_dbuff, "");
	    }

	    printf("%s\n", io_dbuff);
#ifdef HAS_EKGUI
	}
#endif
}

void disp_blockhdr(int spos, int slen, int bnum, int blks, char *name)
{
#ifdef HAS_EKGUI
	if (io_is_ekgui)
	    ext_gui_disp_blockhdr(spos, slen, bnum, blks, name);
	else
	{
#endif
	    sprintf(io_dbuff, "--- Seq: %2d/%2d Blk: %2d/%2d --- ",
		spos, slen, bnum, blks);
	    if (name) strcat(io_dbuff, name);
	    printf("%s\n", io_dbuff);
#ifdef HAS_EKGUI
	}
#endif
}

void dbuff_blockpos(int pos)
{
	if (verbose & VBSF1_PLAY)
	{
#ifdef HAS_EKGUI
	    if (io_is_ekgui)
		ext_gui_dbuff_blockpos(pos);
	    else
	    {
#endif
		if (pos == 0)
		    sprintf(io_dbuff, "\n%03d !", pos);
		else
		    sprintf(io_dbuff, "%03d !", pos);
#ifdef HAS_EKGUI
	    }
#endif
	}
}

#ifdef SLOW_DISPLAY
void dbuff_channel(int pitch, U_CHR instr, U_CHR fx, U_CHR fxdat)
{
	if (verbose & VBSF1_PLAY)
	{
#ifdef HAS_EKGUI
	    if (io_is_ekgui)
		ext_gui_dbuff_channel(pitch, instr, fx, fxdat);
	    else
	    {
#endif
	      int	p= pitch;
	      char	temp[13];

		for (; p >= perTabSz; p-= 12);
		sprintf(temp, " %3s %2s %02X%02X !",
			ptchName[p], instName[instr], fx, fxdat);
		strcat(io_dbuff, temp);
#ifdef HAS_EKGUI
	    }
#endif
	}
}
#else		/* ifndef SLOW_DISPLAY */
void dbuff_channel(const char *data)
{
	if (verbose & VBSF1_PLAY)
	{
#ifdef HAS_EKGUI
	    if (io_is_ekgui)
		ext_gui_dbuff_channel(data);
	    else
	    {
#endif
		strcat(io_dbuff, " ");
		strcat(io_dbuff, data);
		strcat(io_dbuff, " !");
#ifdef HAS_EKGUI
	    }
#endif
	}
}
#endif

void dbuff_flush()
{
	if (verbose & VBSF1_PLAY)
	{
#ifdef HAS_EKGUI
	    if (io_is_ekgui)
		ext_gui_dbuff_flush();
	    else
	    {
#endif
		printf("%s\n", io_dbuff);
#ifdef HAS_EKGUI
	    }
#endif
	}
}
