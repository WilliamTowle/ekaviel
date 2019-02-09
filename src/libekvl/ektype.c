#include <stdio.h>	/* NULL definition */
#include "ektype.h"

EkErrFlag	EK_ERR_NONE= NULL;
EkErrFlag	EK_ERR_NOT_SUPPORTED= "Not supported";
EkErrFlag	EK_ERR_FILENOTFOUND= "File not found";
EkErrFlag	EK_ERR_FILEINCOMPLETE= "File is incomplete/corrupt";
EkErrFlag	EK_ERR_FILECORRUPT= "File is corrupt";
EkErrFlag	EK_ERR_MALLOC_FAILURE= "Memory allocation failed";
EkErrFlag	EK_ERR_INSTCORRUPT= "Instrument(s) is/are corrupt";
EkErrFlag	EK_ERR_NO_AUDIO= "Couldn't open audio channel(s)";
EkErrFlag	EK_ERR_NO_FXTABLE= "No effects table for song format!";
EkErrFlag	EK_ERR_SAVEFAIL= "Couldn't write whole file";
EkErrFlag	EK_ERR_MISSING_INSTRS= "Couldn't load external samples";
#ifdef DEVEL
EkErrFlag	EK_ERR_DEVEL= "Still under development";
#endif

/*****
 *	Song types:
 *	EK_SONG_PARTIAL is an error type in the ekaviel player, but
 *	doubles as the dummy song-type used in eksfplay.
 *****/

EkSongType	EK_SONG_PARTIAL= "Partially loaded song";
#ifdef DEVEL
EkSongType	EK_SONG_SCREAM= "ScreamTracker module";
#endif
EkSongType	EK_SONG_MMD= "OctaMED song";
EkSongType	EK_SONG_MOD31= "31-instrument SoundTracker module or ST-32 lookalike";
EkSongType	EK_SONG_MOD15= "15-instrument [ST-16] SoundTracker module";

EkInstTuning	EK_INSTUNE_UNKNOWN= "?type?";
EkInstTuning	EK_INSTUNE_SB_PERFECT= "SBperf";
EkInstTuning	EK_INSTUNE_SB_COARSE= "SBcrse";
EkInstTuning	EK_INSTUNE_SB_FINE= "SBfine";
