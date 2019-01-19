#ifndef __EKAVIEL_H__
#define __EKAVIEL_H__

#include "ekprefs.h"
#include "ektype.h"

typedef struct ekPlayList {
	const char	*filename;
	EkSetup		settings;
	} EkPlayList;

EkErrFlag ekplaylist_new(EkPlayList **listPtr);
EkErrFlag ekplaylist_construct(EkPlayList *list, EkPrefs *prefs);
#define EKPLAYLIST_FIELD(name, field)	\
	(name).field
EkErrFlag ekplaylist_destroy(EkPlayList *list);
EkErrFlag ekplaylist_delete(EkPlayList **listPtr);

#if 0	/* load_s3m.h */
#define PLAYSEQ_DIVIDER	-1
#endif

#endif	/* __EKAVIEL_H__ */
