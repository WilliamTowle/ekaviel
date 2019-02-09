#ifndef __EKNOTE_H__
#define __EKNOTE_H__

#include "ektype.h"

extern U_SHT *perTable;
extern int perTabSz;
extern const char *instName[];
extern char **ptchName;
extern const U_SHT octaves;

#ifndef SLOW_DISPLAY
#define	EKNOTE_TEXT_BUFFLEN	12
#define EKNOTE_TEXT_FMT		"%3s %2s %02X%02X",		\
				ptchName			\
				[EKNOTE_FIELD(*note, pitch)],	\
				instName			\
				[EKNOTE_FIELD(*note, instNum)],	\
				EKNOTE_FIELD(*note, fx),	\
				EKNOTE_FIELD(*note, fxdat)
#endif

typedef struct ekNote {
	U_CHR	instNum;
	int	pitch;
	U_CHR	fx, fxdat;
#ifndef SLOW_DISPLAY
	char	text[EKNOTE_TEXT_BUFFLEN];
#endif
	} EkNote;

EkErrFlag eknote_new(EkNote **notePtr, int reqd);
EkErrFlag eknote_construct(EkNote *note);
#define EKNOTE_FIELD(name, value)	\
	(name).value
#ifndef SLOW_DISPLAY
EkErrFlag eknote_set_text(EkNote *note);
#endif
EkErrFlag eknote_destroy(EkNote *note);
EkErrFlag eknote_delete(EkNote **notePtr);

/*****
 *
 *****/

EkErrFlag init_notes();
int getnote(int pitch);
int new_periodit(int pitch);
U_SHT c_ftune(S_CHR transp, S_CHR ftu);
void zap_notes();

#endif	/* __EKNOTE_H__ */
