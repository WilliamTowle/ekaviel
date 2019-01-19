#ifdef __cplusplus	/* tut tut - missing from curses.h */
extern "C" {
#include <curses.h>
}
#else
#include <curses.h>
#endif
#include "ekgui.h"
#include "ekinstr.h"
#include "eknote.h"


static	WINDOW	*wmain,
		*wplay,
		*wstat;

extern void	(*ext_gui_error1)(const char *);
extern void	(*ext_gui_warn1)(const char *);
extern void	(*ext_gui_warn2)(const char *, const char *);
extern void	(*ext_gui_warn3)(const char *, const char *, const char *);
extern void	(*ext_gui_stat1)(const char *);
extern void	(*ext_gui_stat2)(const char *, const char *);

extern int	(*ext_gui_kbread)();
extern Flag	(*ext_gui_special)(int, int *);
extern void	(*ext_gui_writemenu)(char *);
extern void	(*ext_gui_disp_songtitle)(char *);
extern void	(*ext_gui_disp_sampdata)(int, EkInstr *);
extern void	(*ext_gui_disp_blockhdr)(int, int, int, int, char *);
extern void	(*ext_gui_dbuff_blockpos)(int);
#ifdef SLOW_DISPLAY
extern void	(*ext_gui_dbuff_channel)(int, U_CHR, U_CHR, U_CHR);
#else
extern void	(*ext_gui_dbuff_channel)(const char *);
#endif
extern void	(*ext_gui_dbuff_flush)();

extern	char	io_dbuff[];

void gui_error1(const char *e1)
{
	sprintf(io_dbuff, "ekaviel: error - %s\n", e1);
	waddstr(wstat, io_dbuff);
}

void gui_warn1(const char *w1)
{
	sprintf(io_dbuff, "ekaviel: warning - %s\n", w1);
	waddstr(wstat, io_dbuff);
}

void gui_warn2(const char *w1, const char *w2)
{
	sprintf(io_dbuff, "ekaviel: warning - %s %s\n", w1, w2);
	waddstr(wstat, io_dbuff);
}

void gui_warn3(const char *w1, const char *w2, const char *w3)
{
	sprintf(io_dbuff, "ekaviel: warning - %s %s %s\n", w1, w2, w3);
	waddstr(wstat, io_dbuff);
}

void gui_stat1(const char *s1)
{
	sprintf(io_dbuff, "%s\n", s1);
	waddstr(wstat, io_dbuff);
	touchwin(wmain);
}

void gui_stat2(const char *s1, const char *s2)
{
	sprintf(io_dbuff, "%s %s\n", s1, s2);
	waddstr(wstat, io_dbuff);
	touchwin(wmain);
}

int gui_kbread()
{
  return getch();
}

Flag gui_special(int keyval, int *command)
{
	if (keyval == KEY_REFRESH)
	{
	    clearok(wmain, FL_TRUE);
	    return FL_TRUE;
	}
	else if (keyval == KEY_SLEFT)
	{
	    if (command) *command= EKVL_C_SAMPRV;
	    return FL_TRUE;
	}
	else if (keyval == KEY_SRIGHT)
	{
	    if (command) *command= EKVL_C_SAMNXT;
	    return FL_TRUE;
	}

  return FL_FALSE;
}

void gui_writemenu(char *menutext)
{
	move(0,0); clrtoeol();
	addstr(menutext);
	wrefresh(wmain);
}

void gui_disp_songtitle(char *title)
{
	touchwin(wmain);
	wmove(wmain, 2,0); clrtoeol();
	sprintf(io_dbuff, "Song title: %s\n", title);
	waddstr(wmain, io_dbuff);
	wrefresh(wmain);
}

void gui_disp_sampdata(int num, EkInstr *samp)
{
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
					instName[num+1],"<Unused>");
	}
	mvwaddstr(wmain, 3,0, io_dbuff);
	
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
	mvwaddstr(wmain, 4,0, io_dbuff);

	touchwin(wmain);
	wrefresh(wmain);
}

void gui_disp_blockhdr(int spos, int slen, int bnum,
			int blks, char *name)
{
	sprintf(io_dbuff, "--- Seq: %2d/%2d Blk: %2d/%2d --- ",
		spos, slen, bnum, blks);
	if (name) strcat(io_dbuff, name);
	mvwaddstr(wmain, 6,6, io_dbuff);
	clrtoeol();
	wrefresh(wmain);
}

void gui_dbuff_blockpos(int pos)
{
	if (pos == 0)
	    sprintf(io_dbuff, "\n%03d !", pos);
	else
	    sprintf(io_dbuff, "%03d !", pos);
}

#ifdef SLOW_DISPLAY
void gui_dbuff_channel(int pitch, U_CHR instr, U_CHR fx, U_CHR fxdat)
{
  int	p= pitch;
  char	temp[13];

	if (p >= (octaves+2)*12)
	    p= octaves*12 + (p%12);
	sprintf(temp, " %3s %2s %02X%02X !",
		ptchName[p], instName[instr], fx, fxdat);
	strcat(io_dbuff, temp);
}
#else
void gui_dbuff_channel(const char *data)
{
	strcat(io_dbuff, " ");
	strcat(io_dbuff, data);
	strcat(io_dbuff, " !");
}
#endif

void gui_dbuff_flush()
{
	strcat(io_dbuff, "\n");
	waddstr(wplay, io_dbuff);
	touchwin(wmain);
	wrefresh(wmain);
}

GUIFN_HEADER(gui_init)
{
	wmain= initscr();		/* wmain == stdscr */
	cbreak(); noecho();		/* output mode */
	nodelay(wmain, FL_TRUE);	/* input mode */
	keypad(wmain, FL_TRUE);		/* cursors != ESCAPE */

	wplay= subwin(wmain, 12,64,7,7);
	wstat= subwin(wmain, 4,80,20,0);
	scrollok(wplay, FL_TRUE);
	scrollok(wstat, FL_TRUE);

	ext_gui_error1= gui_error1;
	ext_gui_warn1= gui_warn1;
	ext_gui_warn2= gui_warn2;
	ext_gui_warn3= gui_warn3;
	ext_gui_stat1= gui_stat1;
	ext_gui_stat2= gui_stat2;
	ext_gui_kbread= gui_kbread;
	ext_gui_special= gui_special;
	ext_gui_writemenu= gui_writemenu;
	ext_gui_disp_songtitle= gui_disp_songtitle;
	ext_gui_disp_sampdata= gui_disp_sampdata;
	ext_gui_disp_blockhdr= gui_disp_blockhdr;
	ext_gui_dbuff_blockpos= gui_dbuff_blockpos;
	ext_gui_dbuff_channel= gui_dbuff_channel;
	ext_gui_dbuff_flush= gui_dbuff_flush;

  return EK_ERR_NONE;
}

GUIFN_HEADER(gui_restore)
{
	clearok(wmain, FL_TRUE);
	touchwin(wmain);
	wrefresh(wmain);
	endwin();

  return EK_ERR_NONE;
}

