#ifndef __EKSFDUMP_H__
#define __EKSFDUMP_H__

#include "eklist.h"
#include "ektype.h"

typedef struct ekSongList {
	const char	*filename;
	EkList		samplist;
	} EkSongList;

EkErrFlag eksonglist_new(EkSongList **listPtr);
EkErrFlag eksonglist_construct(EkSongList *list);
#define EKSONGLIST_FIELD(name, field)	\
	(*name).field
EkErrFlag eksonglist_destroy(EkSongList *list);
EkErrFlag eksonglist_delete(EkSongList **listPtr);

#endif	/* __EKSFDUMP_H__ */
