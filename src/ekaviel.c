#ident "$ ekaviel v1.53b 16/02/96 Wm.Towle $"

/*****
 *	 Easther, Ke'lle', Vernie, Louise:
 *	 	" No matter what they say,
 *	 	No words I know can keep me away from your love;
 *	 	They say I'm crazy, when it comes to you. "
 *	 You will forever remain a great influence to me.
 *	 May you live long and prosper. Stay gorgeous,
 *	 						Wills.
 *****/

#include <math.h>	/* srandom() */
#include <stdlib.h>	/* atoi(), exit() */
#include <string.h>
#include <time.h>	/* time() in random number generation */
#include <unistd.h>	/* unlink() */
#include "ekaviel.h"

#include "argcv.h"
#include "ekmalloc.h"
#include "eklist.h"
#ifdef HAS_EKGUI
#include "ekgui.h"
#endif
#include "ekio.h"
#include "eksong.h"
#include "loadbase.h"	/* find_file(), compression(), uncompress() */
#include "player.h"	/* play_song() */
#include "vchannel.h"	/* period_l() / period_u() */

typedef enum ekParseState {
	parseState_initial, parseState_midoptions,
	parseState_postsong, parseState_error
	} EkParseState;

EkErrFlag ekplaylist_new(EkPlayList **listPtr)
{
	if (listPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekplaylist_new() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkPlayList	*list= (EkPlayList *)
				ALLOCATE_MEMORY(1, EkPlayList,
					"ekaviel::ekplaylist_new()");

	    if ( (*listPtr= list) == NULL )
		return EK_ERR_MALLOC_FAILURE;
	    else
		return EK_ERR_NONE;
	}
}

EkErrFlag ekplaylist_construct(EkPlayList *list, EkPrefs *prefs)
{
	if (list == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekplaylist_construct() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    EKPLAYLIST_FIELD(*list, filename)= NULL;
	    return eksetup_construct(
			&EKPLAYLIST_FIELD(*list, settings), prefs);
	}
}

EkErrFlag ekplaylist_destroy(EkPlayList *list)
{
	if (list == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekplaylist_destroy() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	    return eksetup_destroy(&EKPLAYLIST_FIELD(*list, settings));
}

EkErrFlag ekplaylist_delete(EkPlayList **listPtr)
{
	if (listPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekplaylist_delete() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    FREE_MEMORY(*listPtr, "ekaviel::ekplaylist_delete()");
	    *listPtr= NULL;
	    return EK_ERR_NONE;
	}
}

#ifndef DEVEL
static
#endif
EkErrFlag playlist_clear(EkList *plist)
{
  EkErrFlag	error= EK_ERR_NONE;

	while ( eklist_next(plist) && (error == EK_ERR_NONE) )
	{
	  EkPlayList	*listent= (EkPlayList *)eklist_current(plist);

	    if ( (error= ekplaylist_destroy(listent)) == EK_ERR_NONE )
		error= ekplaylist_delete(&listent);
	    if (error == EK_ERR_NONE)
		error= eklist_unlink(plist);
	}

  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag play_playlist(EkList *unplayed,
#ifdef HAS_EKGUI
			Flag ekgui,
#endif
			Flag shuffle, Flag loopOption)
{
  EkErrFlag	error;
  EkList	played;
  int		randmax= 0;
  EkPlayList	*lastSong;
  Flag		firstSong;
  EkSong	song;

#ifdef HAS_EKGUI
	error= io_init(ekgui? gui_init : NULL);
#else
	error= io_init();
#endif
	if ( (error != EK_ERR_NONE)
		|| ((error= eklist_construct(&played)) != EK_ERR_NONE) )
	{
	    fprintf(stderr, "Initialise failure -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	    return error;
	}

	if (shuffle)
	{
	    while ( eklist_next(unplayed) )
		randmax++;
	    eklist_set_circular(unplayed, FL_TRUE);
	    srandom((unsigned int) time(NULL));
	}

	firstSong= FL_TRUE;
	lastSong= NULL;	/* either last song played or load-attempted */
	do
	{
	  Flag quitNow= FL_FALSE;

	    while ( eklist_next(unplayed)
			&& (quitNow == FL_FALSE)
			&& (error == EK_ERR_NONE) )
	    {
	      EkPlayList	*listent;
	      int		command;
	      char		*tempfile;
	      Flag		doLoad;

		if (shuffle)
		{
		  int i;
		    for (i=0; i<(random() % randmax); i++)
			eklist_next(unplayed);
		}
		listent= (EkPlayList *)eklist_current(unplayed);
		doLoad= ( (lastSong == NULL)
			|| (strcmp(
				EKPLAYLIST_FIELD(*listent, filename),
				EKPLAYLIST_FIELD(*lastSong, filename))
				!= 0) )
			? FL_TRUE : FL_FALSE;

		error= EK_ERR_NONE;	/* hmmm, if doLoad "corrupt"? */
		tempfile= NULL;
		if (doLoad)
		{
		  int	compressID;
		    if ((compressID= compression(
			EKPLAYLIST_FIELD(*listent, filename))) != -1)
		    {
			stat2("Uncompressing",
				EKPLAYLIST_FIELD(*listent, filename));
			tempfile= uncompress(
				EKPLAYLIST_FIELD(*listent, filename),
				compressID);
		    }

		    if (firstSong)
			firstSong= FL_FALSE;
		    else
			error= eksong_destroy(&song);

		    error= eksong_construct(&song);
		}

		if (error == EK_ERR_NONE)
		{
		  EkErrFlag serror= EK_ERR_NONE;

		    if (tempfile)
		    {
			serror= eksong_load_bestguess(&song,
							tempfile);
			stat1("Cleaning temporary files...");
			unlink(tempfile);
			FREE_MEMORY(tempfile, "ekaviel::play_playlist()");
		    }
		    else if (doLoad)
			serror= eksong_load_bestguess(&song,
				EKPLAYLIST_FIELD(*listent, filename));

		    lastSong= listent;

		    if (serror != EK_ERR_NONE)
		    {
			warn3(EKPLAYLIST_FIELD(*listent, filename),
				"is unplayable -",
				EKERRFLAG_STRING_VALUE(serror));
		    }
		    else
		    {
		      int repeatTimes= EKSETUP_FIELD(
					EKPLAYLIST_FIELD(
						*listent, settings),
						repeatTimes);
					
			if ((eksong_instrs_playable(&song) == 0)
				&& (eksong_instrs_unplayable(&song) > 0)
				)
			{
			    warn2("All instruments unplayable in",
				EKPLAYLIST_FIELD(*listent, filename));
			}

			while ( !quitNow && (repeatTimes > 0) )
			{
			    stat2("Playing",
				EKPLAYLIST_FIELD(*listent, filename));

			    error= play_song(&song, &EKPLAYLIST_FIELD(
						*listent, settings),
						&command);
			    if ( error != EK_ERR_NONE )
			    {
				quitNow= FL_TRUE;
				stat2("Playback error:",
					EKERRFLAG_STRING_VALUE(error));
			    }
			    else if (command != EKVL_C_QUIT)
				repeatTimes--;
			    else
				quitNow= FL_TRUE;
			}
		    }
		}
		else if (tempfile)
		{	/* Clean up, no memory to do anything. */
		    stat1("Cleaning temporary files...");
		    unlink(tempfile);
		    FREE_MEMORY(tempfile, "ekaviel::play_playlist()");
		}

		eklist_add(&played, listent);
		eklist_unlink(unplayed);
	    }

	    if (loopOption)
	    {
		/* put "played" songs back on "unplayed" list */
		while (eklist_next(&played))
		{
		    eklist_add(unplayed, eklist_current(&played));
		    eklist_unlink(&played);
		}

		if (quitNow) loopOption= FL_FALSE;
	    }
	}
	while (loopOption);

	error= eksong_destroy(&song);


	/* clear "unplayed" in case quit with songs yet to play */
	if ((error= playlist_clear(unplayed)) != EK_ERR_NONE)
	{
	    fprintf(stderr, "Terminate failure -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	    return error;
	}

	/* clear "played" in case songs as yet unplayed */
	if ( ((error= playlist_clear(&played)) != EK_ERR_NONE)
		|| ((error= eklist_destroy(&played)) != EK_ERR_NONE) )
	{
	    fprintf(stderr, "Terminate failure -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	    return error;
	}

#ifdef HAS_EKGUI
	return io_restore(ekgui? gui_restore : NULL);
#else
	return io_restore();
#endif
}

int main(int argc, const char **argv)
{
  Flag		done;
  const	char	*argText, *parText;
  EkErrFlag	error;
  EkParseState	pState;
  EkList	playList;
  EkPrefs	defPrefs;

	if ( ((error= eklist_construct(&playList)) != EK_ERR_NONE)
		|| ((error= ekprefs_construct(&defPrefs))
						!= EK_ERR_NONE) )
	{
	    fprintf(stderr, "Initialise failure: %s\n",
					EKERRFLAG_STRING_VALUE(error));
	    exit(1);
	}

	initArg(argc, argv);

	done= FL_FALSE;
	pState= parseState_initial;
	while ( done == FL_FALSE )
	{
	  EkPlayList	*listent= NULL;

	    if ( ((error= ekplaylist_new(&listent)) != EK_ERR_NONE)
		|| ((error= ekplaylist_construct(listent, &defPrefs))
						!= EK_ERR_NONE) )
	    {
		fprintf(stderr, "Initialise failure -%s\n",
					EKERRFLAG_STRING_VALUE(error));
		pState= parseState_error; done= FL_TRUE;
	    }

	    EKPREFS_FIELD(defPrefs, interact)=
			EKSETUP_FIELD( EKPLAYLIST_FIELD(
				*listent, settings), interact)= 2;
	    EKPREFS_FIELD(defPrefs, verbose)=
			EKSETUP_FIELD( EKPLAYLIST_FIELD(
				*listent, settings), verbose)= 7;

	    while (nextOpt(&argText, &parText))
	    {
	      Flag	missing= FL_FALSE;

		if (strcmp(argText, "bleed") == 0)
		{
		    if (parText)
			defPrefs.bleed= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					bleed)= atoi(parText);
		    else
			missing= FL_TRUE;
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "gui") == 0)
		{
#ifdef HAS_EKGUI
		    defPrefs.hasgui= FL_TRUE;
#endif
		    if (parText) rewindArg();
		}
		else if (strcmp(argText, "nogui") == 0)
		{
#ifdef HAS_EKGUI
		    defPrefs.hasgui= FL_FALSE;
#endif
		    if (parText) rewindArg();
		}

		else if (strcmp(argText, "interact") == 0)
		{
		    if (parText)
		    {
			if ((defPrefs.interact= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					interact)= atoi(parText)) == 2)
			{
#ifdef I2RATE
			    defPrefs.outrate= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					outrate)= I2RATE;
#else
			    defPrefs.outrate= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					outrate)= 8000;
#endif
			}
		    }
		    else
			missing= FL_TRUE;
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "limit") == 0)
		{
		    defPrefs.periodfn= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					periodfn)= period_l;
		    if (parText) rewindArg();
		    pState= parseState_midoptions;
		}
		else if (strcmp(argText, "unlimit") == 0)
		{
		    defPrefs.periodfn= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					periodfn)= period_u;
		    if (parText) rewindArg();
		}

		else if (strcmp(argText, "loop") == 0)
		{
		    if (pState != parseState_initial)
		    {
			fprintf(stderr,
				"Expected '-loop' before playlist\n");
			pState= parseState_error; done= FL_TRUE;
		    }
		    else
		    {
			defPrefs.loop= FL_TRUE;
			if (parText) rewindArg();
		    }
		}

		else if (strcmp(argText, "outrate") == 0)
		{
		    if (parText)
			defPrefs.outrate= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					outrate)= atoi(parText);
		    else missing= FL_TRUE;
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "repeat") == 0)
		{
		    if (parText)
			EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					repeatTimes)= atoi(parText);
		    else missing= FL_TRUE;
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "seq") == 0)
		{
		    if (parText)
			EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					seqSkip)= atoi(parText);
		    else missing= FL_TRUE;
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "shuffle") == 0)
		{
		    if (pState != parseState_initial)
		    {
			fprintf(stderr, "Expected '-shuffle' before playlist\n");
			pState= parseState_error; done= FL_TRUE;
		    }
		    else
		    {
			defPrefs.shuffle= FL_TRUE;
			if (parText) rewindArg();
		    }
		}

		else if (strcmp(argText, "stereo") == 0)
		{
		    defPrefs.stereo= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					stereo)= FL_TRUE;
		    if (parText) rewindArg();
		    pState= parseState_midoptions;
		}
		else if (strcmp(argText, "mono") == 0)
		{
		    defPrefs.stereo= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					stereo)= FL_FALSE;
		    if (parText) rewindArg();
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "sync") == 0)
		{
		    defPrefs.synchro= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					synchro)= FL_TRUE;
		    if (parText) rewindArg();
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "transpose") == 0)
		{
		    if (parText)
			EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					transpose)= atoi(parText);
		    else missing= FL_TRUE;
		    pState= parseState_midoptions;
		}
		else if (strcmp(argText, "downtranspose") == 0)
		{
		    if (parText)
			EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					transpose)= -atoi(parText);
		    else missing= FL_TRUE;
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "verbose") == 0)
		{
		    if (parText)
			defPrefs.verbose= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					verbose)= atoi(parText);
		    else
			missing= FL_TRUE;
		    pState= parseState_midoptions;
		}

		else if (strcmp(argText, "volume") == 0)
		{
		    if (parText)
			defPrefs.volume= EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
					volume)= atoi(parText);
		    else
			missing= FL_TRUE;
		    pState= parseState_midoptions;
		}

		else
		{
		    fprintf(stderr, "Bad argument '%s'\n", argText);
		    pState= parseState_error; done= FL_TRUE;
		}

		if (missing)
		{
		    fprintf(stderr, "Missing argument to option %s\n",
					argText);
		    pState= parseState_error; done= FL_TRUE;
		}
	    }

	    if (! nextArg(&argText) )
	    {
		if ( (pState == parseState_initial)
			|| (pState == parseState_midoptions) )
		{
		    fprintf(stderr, "Expected song file\n");
		    pState= parseState_error; done= FL_TRUE;
		}

		done= FL_TRUE;
	    }
	    else
	    {
		if ( (EKPLAYLIST_FIELD(*listent, filename)=
					find_file(argText)) == NULL )
		{
		    fprintf(stderr, "Couldn't open %s\n", argText);
		    if ( ((error= ekplaylist_destroy(listent))
						!= EK_ERR_NONE)
			|| ((error= ekplaylist_delete(&listent))
						!= EK_ERR_NONE) )
		    {
			fprintf(stderr, "Initialise failure -%s\n",
					EKERRFLAG_STRING_VALUE(error));
			pState= parseState_error; done= FL_TRUE;
		    }
		}
		else
		{
		    eklist_add(&playList, listent);
		    pState= parseState_postsong;
		}
	    }
	}

	switch (pState)
	{
	case parseState_postsong:
	    if (play_playlist(&playList,
#ifdef HAS_EKGUI
				defPrefs.hasgui,
#endif
				defPrefs.shuffle, defPrefs.loop)
			!= EK_ERR_NONE)
	    {
		exit(1);	/* some failure (already reported) */
	    }
	    break;
	case parseState_error:
	    fprintf(stderr, "\n");
	default:
	    fprintf(stderr,
		"Usage:\n\t%s [options] < [prefs] song >[...]\n\n",
			argv[0]);
	    fprintf(stderr, "%s\n\t%s\t%s\n\t%s\t%s\n",
			"Options:",
			"-loop\t", "Cycle infinitely through songs",
			"-shuffle", "Play songs in random order");
	    fprintf(stderr, "%s\n\t%s\t%s\n\t%s\t%s\n\t%s\t%s\n",
			"Prefs:",
			"-bleed <n>", "balance control",
			"-gui/-nogui", "with/without 'cute' front-end",
			"-interact <n>", "keyboard sensitivity");
	    fprintf(stderr, "\t%s\t%s\n\t%s\t%s\n\t%s\t%s\n",
			"-limit/-unlimit", "note accuracy",
			"-outrate", "output quality",
			"-repeat <n>", "individual song repetition");
	    fprintf(stderr, "\t%s\t%s\n\t%s\t%s\n\t%s\t%s\n",
			"-seq <n>", "jump-point into song",
			"-stereo|-mono", "playback in 1/2 channels",
			"-sync\t", "display control");
	    fprintf(stderr, "\t%s\t%s\n\t%s\t%s\n\t%s\t%s\n",
			"-transpose|-downtranspose <n>",
					"output pitch control",
			"-verbose <n>", "output level control",
			"-volume <n>", "volume level control");
	}

	if ( ((error= ekprefs_destroy(&defPrefs)) != EK_ERR_NONE)
		|| ((error= playlist_clear(&playList)) != EK_ERR_NONE)
		|| ((error= eklist_destroy(&playList)) != EK_ERR_NONE) )
	{
	    fprintf(stderr, "Terminate failure: %s\n",
					EKERRFLAG_STRING_VALUE(error));
	    exit(1);
	}

  return 0;
}
