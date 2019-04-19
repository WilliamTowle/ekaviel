#ifndef __EKPREFS_H__
#define __EKPREFS_H__

#include "eksong.h"
#include "ektype.h"

typedef struct ekPrefs {
	int	outrate;
	Flag	stereo;
	int	interact,
		volume,
		verbose;
	U_CHR	bleed;
	Flag	synchro,
		loop,
		shuffle;
#ifdef HAVE_EKGUI
	Flag	hasgui;
#endif
	PERIODFN_DECLARE_MEMBER(periodfn)
	} EkPrefs;

EkErrFlag ekprefs_construct(EkPrefs *prefs);
#define EKPREFS_FIELD(name, field)	\
	(name).field
EkErrFlag ekprefs_destroy(EkPrefs *prefs);


typedef struct ekSetup {
/* PLAYBACK INFORMATION */
	int	interact,
		outrate,
		volume,		/* absolute master volume! */
		verbose;
	Flag	stereo,
		synchro,
		filter;
	U_CHR	bleed;
	int	pbFlags;

/* SONG INFORMATION */
	EkSongType	songType;
	U_SHT	ptempo;	/* primary tempo */
	U_CHR	stempo,	/* secondary tempo ("ticks" per line) */
		ttempo;	/* "tertiary tempo" (additional # ticks) */
	S_CHR	transpose;
	int	flags1, flags2;
	U_CHR	seqSkip;

/* PLAYER INFORMATION */
	int	*trkvol;	/* track volumes */
	int	*pitch;		/* last played note */
	int	*ptch_bnd;
	int	seqPos, blkNum, tracks,	/* position in song */
		blkPos, blkLen;		/* and sizes */
	int	(*periodfn) (PERIODFN_PARMS);
	int	repeatTimes;
	} EkSetup;

EkErrFlag eksetup_construct(EkSetup *sett, EkPrefs *prefs);
#define EKSETUP_FIELD(name, field)	\
	(name).field
EkErrFlag eksetup_destroy(EkSetup *sett);

#endif	/* __EKPREFS_H__ */
