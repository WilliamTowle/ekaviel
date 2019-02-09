#ifndef __IO_H__
#define __IO_H__

#include "ektype.h"

#include "ekinstr.h"

#define VBSF1_PLAY	1
#define VBSF1_STAT	2
#define VBSF1_WARN	4
#define VBSF1_ALL	VBSF1_PLAY | VBSF1_STAT | VBSF1_WARN

/* Commands */
#define EKVL_C_NONE	0
#define EKVL_C_QUIT	1
#define EKVL_C_NEXT	2
#define EKVL_C_REW	3
#define EKVL_C_FWD	4
#define EKVL_C_NEWPOS	5
#define EKVL_C_SAMNXT	6
#define EKVL_C_SAMPRV	7
#define EKVL_C_SAMFRST	8
#define EKVL_C_SAMLAST	9

#define GUIFN_PARMS			void
#define GUIFN_DECLARE_MEMBER(name)	EkErrFlag (*name)(GUIFN_PARMS)
#define GUIFN_ARGS			GUIFN_PARMS
#define GUIFN_HEADER(func)		EkErrFlag func(GUIFN_ARGS)

void error1(const char*);
void warn1(const char*);
void warn2(const char*, const char*);
void warn3(const char*, const char*, const char*);
void stat1(const char*);
void stat2(const char*, const char*);

#ifdef HAS_EKGUI
EkErrFlag io_init(GUIFN_DECLARE_MEMBER(gui_init));
EkErrFlag io_restore(GUIFN_DECLARE_MEMBER(gui_restore));
#else
EkErrFlag io_init();
EkErrFlag io_restore();
#endif

int io_keycmd();
void set_menu(int which);
void set_verbosity(int v);

void disp_songtitle(char *title);
void disp_sampdata(int num, EkInstr *samp);
void disp_blockhdr(int spos, int slen, int bnum, int blks, char *name);
void dbuff_blockpos(int pos);
#ifdef SLOW_DISPLAY
void dbuff_channel(int pitch, U_CHR instr, U_CHR fx, U_CHR fxdat);
#else
void dbuff_channel(const char *data);
#endif
void dbuff_flush();

#endif	/* __IO_H__ */
