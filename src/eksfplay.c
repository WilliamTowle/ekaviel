#ident "$ eksfplay v1.0 27/05/1998 Wm.Towle $"

/*****
 *	 Easther, Ke'lle', Vernie, Louise:
 *	 	" No matter what they say,
 *	 	No words I know can keep me away from your love;
 *	 	They say I'm crazy, when it comes to you. "
 *	 You will forever remain a great influence to me.
 *	 May you live long and prosper. Stay gorgeous,
 *	 						Wills.
 *****/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>	/* atoi() */
#include <string.h>
#include "eksfplay.h"

#include "argcv.h"
#include "ekmalloc.h"
#include "eknote.h"
#include "vchannel.h"

typedef enum ekParseState {
	parseState_initial, parseState_filename, parseState_noteCount,
	parseState_notes, parseState_done
	} EkParseState;

EkErrFlag ekplaylist_new(EkPlayList **listPtr, int reqd)
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
				ALLOCATE_MEMORY(reqd, EkPlayList,
					"eksfplay::ekplaylist_new()");

	    if ( (*listPtr= list) == NULL )
		return EK_ERR_MALLOC_FAILURE;
	    else
		return EK_ERR_NONE;
	}
}

EkErrFlag ekplaylist_construct(EkPlayList *plist, EkPrefs *prefs)
{
	if (plist == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	  EkErrFlag	error;

	    if ( (error= eklist_construct(
			&EKPLAYLIST_FIELD(*plist, notelist)))
					== EK_ERR_NONE )
	    {
		error= eksetup_construct(
				&EKPLAYLIST_FIELD(*plist,settings),
				prefs);
	    }
	    return error;
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
	else if (!eklist_empty(&EKPLAYLIST_FIELD(*list, notelist)))
	{
#ifdef DEVEL
	    fprintf(stderr, "ekplaylist_destroy() called %s\n",
				"with non-empty notelist");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkErrFlag	error;

	    if ( (error= eksetup_destroy(
		&EKPLAYLIST_FIELD(*list, settings))) == EK_ERR_NONE )
	    {
		error= eklist_destroy(
				&EKPLAYLIST_FIELD(*list, notelist));
	    }
	    return error;
	}
}

EkErrFlag ekplaylist_delete(EkPlayList **listPtr, int made)
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
	    FREE_MEMORY(*listPtr, "eksfplay::ekplaylist_delete()");
	    *listPtr= NULL;
	    return EK_ERR_NONE;
	}
}

static
void eksfplay(EkList *list)
{
  EkErrFlag	error= EK_ERR_NONE;
  EkInstr	instr;
  EkNote	note;

	if ( (error= ekinstr_construct(&instr)) != EK_ERR_NONE )
	    fprintf(stderr, "Initialise error -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	else if ( (error= eknote_construct(&note)) != EK_ERR_NONE)
	    fprintf(stderr, "Initialise error -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	else
	{
	  Flag	nonfatal= FL_TRUE;

	    while (nonfatal && eklist_next(list))
	    {
	      const char	*file;
	      EkPlayList	*listent= (EkPlayList *)
						eklist_current(list);
	      EkSetup		*settings= NULL;

		file= EKPLAYLIST_FIELD(*listent, filename);
		error= ekinstr_load_bestguess(&instr, file);
		if (error != EK_ERR_NONE)
		{
		    nonfatal= FL_FALSE;
		    fprintf(stderr, "Sample '%s': %s\n", file,
					EKERRFLAG_STRING_VALUE(error));
		}

		if (nonfatal)
		{
		    settings= &EKPLAYLIST_FIELD(*listent, settings);

		    if (EKSETUP_FIELD(*settings, verbose))
			printf("%s\n", file);

		/* audio_init() done here too... */
		    error= init_vchans(1, settings);
		    if ( error != EK_ERR_NONE )
		    {
			nonfatal= FL_FALSE;
			fprintf(stderr, "Sample '%s': %s\n", file,
					EKERRFLAG_STRING_VALUE(error));
		    }
		}

		if (nonfatal)
		{
		    while (eklist_next(&EKPLAYLIST_FIELD(*listent, notelist)))
		    {
		      char	*notestr= (char *)
				eklist_current(&EKPLAYLIST_FIELD(
						*listent, notelist));
		      int	pitch= getnote_scale(notestr);

			EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
				tracks)= 1;
			EKSETUP_FIELD(
				EKPLAYLIST_FIELD(*listent, settings),
				outrate)= reset_vchans(
				&EKPLAYLIST_FIELD(*listent, settings));

			if (pitch <= 0)
			    fprintf(stderr,
				"'%s': Out of range or not a note\n",
				notestr);
			else
			{
			    EKNOTE_FIELD(note, pitch)= pitch;
			    EKNOTE_FIELD(note, instNum)= 1; /* instr; */
			    if (EKSETUP_FIELD(*settings, verbose))
				printf("\t%s\n", notestr);
			    set_vchan(0, &note, &instr,
				&EKPLAYLIST_FIELD(*listent, settings));
			    do
			    {
				error= flush_vchans(
						&EKPLAYLIST_FIELD(
						*listent, settings),
						NULL);
			    }
			    while ( (vchan_finished() == FL_FALSE)
					&& (error == EK_ERR_NONE) );
			}
		    }

		}

		close_vchans();	/* audio/fxtable/vchan */
	    }
	}

	eknote_destroy(&note);
	ekinstr_destroy(&instr);
}

static void playlist_clear(EkList *list)
{
	while (eklist_next(list))
	{
	  EkPlayList	*listent= (EkPlayList *)eklist_current(list);
	    while (eklist_next(&EKPLAYLIST_FIELD(*listent, notelist)))
	    {
		eklist_unlink(&EKPLAYLIST_FIELD(*listent, notelist));
	    }
	    eklist_unlink(list);
	    ekplaylist_destroy(listent);
	}
}

int main(int argc, const char **argv)
{
  EkParseState	pState= parseState_initial;
  const char	*argText, *parText;
  Flag		done= FL_FALSE;
  int		noteCount= 0;
  EkList	playlist;
  EkPlayList	*listent= NULL;
  EkPrefs	defPrefs;
  EkErrFlag	error;

	if ( ((error= eklist_construct(&playlist)) != EK_ERR_NONE)
		|| ((error= ekprefs_construct(&defPrefs)) != EK_ERR_NONE)
		|| ((error= ekplaylist_new(&listent, 1)) != EK_ERR_NONE) )
	{
	    fprintf(stderr, "Initialise error -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	    exit(1);
	}

  	initArg(argc, argv);
	while (done == FL_FALSE)
	{
	  Flag	missing= FL_FALSE;
/* check out the supplied options... */
	    while (nextOpt(&argText, &parText))
	    {
		if (strcmp(argText, "limit") == 0)
		{
		    EKPREFS_FIELD(defPrefs, periodfn)= period_l;
		    if (parText) rewindArg();
		}
		else if (strcmp(argText, "unlimit") == 0)
		{
		    EKPREFS_FIELD(defPrefs, periodfn)= period_u;
		    if (parText) rewindArg();
		}
		else if (strcmp(argText, "outrate") == 0)
		{
		    if (parText == NULL)
			missing= FL_TRUE;
		    else
			EKPREFS_FIELD(defPrefs, outrate)= atoi(parText);
		}
		else if (strcmp(argText, "verbose") == 0)
		{
		    EKPREFS_FIELD(defPrefs, verbose)= 1;
		    if (parText) rewindArg();
		}
		else if (strcmp(argText, "volume") == 0)
		{
		    if (parText == NULL)
			missing= FL_TRUE;
		    else
			EKPREFS_FIELD(defPrefs, volume)= atoi(parText);
		}
		else
		{
		    fprintf(stderr, "%s: Bad option '%s'\n",
						argv[0], argText);
		    if (parText) rewindArg();
		}

		if (missing)
		{
		    fprintf(stderr,
				"%s: Option '%s' missing argument\n",
				argv[0], argText);
		}
	    }

/* ...and the arguments */
	    if (!nextArg(&argText))
		done= FL_TRUE;
	    else
	    {
		switch(pState)
		{
		case parseState_initial:
		case parseState_done:
		    if ( (error=
			ekplaylist_construct(listent, &defPrefs))
					!= EK_ERR_NONE )
		    {
			fprintf(stderr, "Aborting: %s\n",
					EKERRFLAG_STRING_VALUE(error));
			exit(1);
		    }
		    EKSETUP_FIELD( EKPLAYLIST_FIELD(*listent, settings),
					songType)= EK_SONG_PARTIAL;
		    /* and... */
		case parseState_filename:
		    EKPLAYLIST_FIELD(*listent, filename)= argText;
		    eklist_add(&playlist, listent);
		    pState= parseState_noteCount;
		    break;
		case parseState_noteCount:
		    if (isdigit(argText[0]))
		    {
			noteCount= atoi(argText);
			pState= parseState_notes;
			break;
		    }
		    /* no note count */
		    noteCount= 1;
		case parseState_notes:
		    eklist_add(&EKPLAYLIST_FIELD(*listent, notelist),
				(void *)argText);
		    noteCount--;
		    if (noteCount == 0)
			pState= parseState_done;
		}
	    }
	}

	switch(pState)
	{
	case parseState_done:
	    eksfplay(&playlist);
	    break;
	case parseState_initial:
	    fprintf(stderr, "Usage:\n\t%s %s\nOptions:\n\t%s\n",
			argv[0],
			"[options] <filename [notecount] note(s)>[...]",
			"-limit|-unlimit -outrate <hz> -verbose -volume <0-63>");
	    break;
	default:
	    fprintf(stderr, "%s\n", "Incomplete args");
	}

	playlist_clear(&playlist);
	if ( ((error= ekprefs_destroy(&defPrefs)) != EK_ERR_NONE)
		|| ((error= ekplaylist_delete(&listent, 1)) != EK_ERR_NONE)
		|| ((error= eklist_destroy(&playlist)) != EK_ERR_NONE) )
	{
	    fprintf(stderr, "Termination error -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	    exit(1);
	}

  return 0;
}
