#ifndef __EKSFPLAY_H__
#define __EKSFPLAY_H__

#include "eklist.h"
#include "ekprefs.h"
#include "ektype.h"

typedef struct ekPlayList {
	const char	*filename;
	EkList		notelist;
	EkSetup		settings;
	} EkPlayList;

EkErrFlag ekplaylist_new(EkPlayList **list, int reqd);
EkErrFlag ekplaylist_construct(EkPlayList *plist, EkPrefs *prefs);
#define EKPLAYLIST_FIELD(name, field)	\
	(name).field
EkErrFlag ekplaylist_destroy(EkPlayList *list);
EkErrFlag ekplayist_delete(EkPlayList **list, int made);

#endif	/* __EKSFPLAY_H__ */
