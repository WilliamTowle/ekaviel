#include <string.h>	/* memcpy */
#include "ekprefs.h"

#include "ekaudio.h"	/* DEFAULT_OUTRATE */
#include "ekmalloc.h"
#include "vchannel.h"

/*****
 *	Preferences class
 *****/

EkErrFlag ekprefs_construct(EkPrefs *prefs)
{
	if (prefs == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	    EKPREFS_FIELD(*prefs, outrate)= DEFAULT_OUTRATE;
	    EKPREFS_FIELD(*prefs, stereo)= FL_FALSE;
	    EKPREFS_FIELD(*prefs, interact)= 1;
	    EKPREFS_FIELD(*prefs, volume)= 64;
	    EKPREFS_FIELD(*prefs, verbose)= 0;
	    EKPREFS_FIELD(*prefs, bleed)= 0;

#ifdef SLOWCPU
	    EKPREFS_FIELD(*prefs, periodfn)= period_l;
#else
	    EKPREFS_FIELD(*prefs, periodfn)= period_u;
#endif

	    EKPREFS_FIELD(*prefs, synchro)= FL_FALSE;
	    EKPREFS_FIELD(*prefs, loop)= FL_FALSE;
	    EKPREFS_FIELD(*prefs, shuffle)= FL_FALSE;
#ifdef HAS_EKGUI
#ifdef DEVEL
	    EKPREFS_FIELD(*prefs, hasgui)= FL_FALSE;
#else
	    EKPREFS_FIELD(*prefs, hasgui)= FL_TRUE;
#endif
#endif	/* HAS_EKGUI */
	    return EK_ERR_NONE;
	}
}

EkErrFlag ekprefs_destroy(EkPrefs *prefs)
{
	if (prefs == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	    return EK_ERR_NONE;
}

/*****
 *	Setup class
 *****/

EkErrFlag eksetup_construct(EkSetup *sett, EkPrefs *prefs)
{
	if (sett == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksetup_construct() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    EKSETUP_FIELD(*sett, outrate)= EKPREFS_FIELD(*prefs, outrate);
	    EKSETUP_FIELD(*sett, stereo)= EKPREFS_FIELD(*prefs, stereo);
	    EKSETUP_FIELD(*sett, volume)= EKPREFS_FIELD(*prefs, volume);
	    EKSETUP_FIELD(*sett, verbose)= EKPREFS_FIELD(*prefs, verbose);
	    EKSETUP_FIELD(*sett, bleed)= EKPREFS_FIELD(*prefs, bleed);

	    EKSETUP_FIELD(*sett, songType)= EK_SONG_PARTIAL;
	    EKSETUP_FIELD(*sett, ptempo)= 33;
	    EKSETUP_FIELD(*sett, stempo)= 6;
	    EKSETUP_FIELD(*sett, transpose)= 0;

	    EKSETUP_FIELD(*sett, periodfn)= EKPREFS_FIELD(*prefs, periodfn);
	    EKSETUP_FIELD(*sett, repeatTimes)= 1;
	    return EK_ERR_NONE;
	}
}

EkErrFlag eksetup_destroy(EkSetup *sett)
{
	if (sett == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksetup_destroy() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	    return EK_ERR_NONE;
}
