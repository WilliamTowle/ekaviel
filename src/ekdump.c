#ident "$ ekdump v1.01 27/05/1998 Wm.Towle $"

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
#include <stdlib.h>	/* atoi() */
#include <string.h>
#include <unistd.h>	/* unlink() */
#include "ekdump.h"

#include "argcv.h"
#include "ekmalloc.h"
#include "eksong.h"
#include "fwrite.h"
#include "loadbase.h"	/* compression(), uncompress() */

typedef enum ekParseState {
	parseState_filename, parseState_samples, parseState_samples_bad,
	} EkParseState;

EkErrFlag eksonglist_new(EkSongList **listPtr)
{
	if (listPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksonglist_new() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkSongList	*list= (EkSongList *)
				ALLOCATE_MEMORY(1, EkSongList,
					"ekdump::eksonglist_new()");

	    if ( (*listPtr= list) == NULL )
		return EK_ERR_MALLOC_FAILURE;
	    else
		return EK_ERR_NONE;
	}
}

EkErrFlag eksonglist_construct(EkSongList *list)
{
	if (list == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksonglist_construct() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    return eklist_construct(&EKSONGLIST_FIELD(list, samplist));
	}
}

EkErrFlag eksonglist_destroy(EkSongList *list)
{
	if (list == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksonglist_destroy() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else if (!eklist_empty(&EKSONGLIST_FIELD(list, samplist)))
	{
#ifdef DEVEL
	    fprintf(stderr, "eksonglist_destroy() called %s\n",
				"with non-empty notelist");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	    return EK_ERR_NONE;
}

EkErrFlag eksonglist_delete(EkSongList **listPtr)
{
	if (listPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksonglist_delete() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    FREE_MEMORY(*listPtr, "ekdump::eksonglist_delete()");
	    *listPtr= NULL;
	    return EK_ERR_NONE;
	}
}

#ifndef DEVEL
static
#endif
void instdetail(EkInstr *instr, int num)
{
  int	rlength= EKINSTR_FIELD(*instr, rpt_len);
  const char	*name= EKINSTR_FIELD(*instr, name);
  int	length= EKINSTR_FIELD(*instr, length);

	printf("--Instr. #%02d (\"%s\", %s)--\n",
			num, instName[num],
			(strlen(name) > 0)? name : "<unnamed>");
	if ( (name= EKINSTR_FIELD(*instr, filename)) != NULL )
	    printf("Instr. source: %s\n", name);

	printf("Instr. stats: Transp %+d,%d Vol %2d Len %6d%s",
		EKINSTR_FIELD(*instr, transpose),
		EKINSTR_FIELD(*instr, finetune),
		EKINSTR_FIELD(*instr, volume), length,
		(rlength > 0)? "::" : "\n");
	if ( rlength > 0 )
	{
	    printf("%li/%i\n",
		EKINSTR_FIELD(*instr, rpt_bgn), rlength);
	}
}

#ifndef DEVEL
static
#endif
void details(const char *archive, EkSong *song)
{
  EkInstr	*instr;
  int		i,
		playable= eksong_instrs_playable(song),
		unplayable= eksong_instrs_unplayable(song);

	printf("Song title  : %s\n", eksong_title(song));
	printf("Source      : %s (%s)\n",
		archive? archive : EKSONG_FIELD(*song, filename),
		archive? "compressed" : "not compressed"
		);
	printf("Song type   : %s\n", EKSONG_FIELD(*song, songType));
	printf("Song stats  : %d blocks, %d seqs\n",
				EKSONG_FIELD(*song, blocks),
				EKSONG_FIELD(*song, playseqs));
	printf("Inst. stats : %d instruments (%d unplayable) across %d of %d available slots\n",
				playable + unplayable, unplayable,
				EKSONG_FIELD(*song, lastinstr),
				EKSONG_FIELD(*song, instrsz));
	if ( (playable == 0) && (unplayable > 0) )
	    printf("WARNING     : All instrumentation of unsupported type(s)\n");

	instr= EKSONG_FIELD(*song, instr);
	for (i= 1; i <= EKSONG_FIELD(*song, lastinstr); i++)
	{
	  const	char	*name= EKINSTR_FIELD(*instr, name);
	  int		length= EKINSTR_FIELD(*instr, length);

	    if ( (strlen(name) > 0) || (length > 0) )
		instdetail(instr, i);
	    instr++;
	}
}

#ifndef DEVEL
static
#endif
Flag eval_range(const char *text, int *llim, int *ulim)
{
	if (strcmp(text, "all") == 0)
	    return FL_TRUE;
	else
	{
	  const char	*ch= text;

	    if (isdigit(*ch))
	    {
		if (llim) *llim= atoi(ch);
		while (isdigit(*ch)) ch++;
	    }

	    if (*ch == '.')
	    {
		ch++;
		if (*ch != '.') return FL_FALSE;
		ch++;
		if (*ch == '\0')
		    return FL_TRUE;
	    }

	    if (*ch == '\0')
	    {
		if (ulim && llim)
		    *ulim= *llim;
	    }
	    else if (isdigit(*ch))
	    {
		if (ulim) *ulim= atoi(ch);
		while (isdigit(*ch)) ch++;
	    }

	    return (*ch == '\0')? FL_TRUE : FL_FALSE;
	}
}

#ifndef DEVEL
static
#endif
void dump(EkSong *song, const char *range, const char *prefix)
{
  int	lower= 1, upper= EKSONG_FIELD(*song, lastinstr), i;

	eval_range(range, &lower, &upper);
	for (i= lower-1; i < upper; i++)
	{
	    if ( (i < 0) || (i >= EKSONG_FIELD(*song, lastinstr)) )
		fprintf(stderr, "%02d: out of range\n", i+1);
	    else
	    {
	      EkInstr	*instr= &EKSONG_FIELD(*song, instr)[i];

		if (EKINSTR_FIELD(*instr, length) == 0)
		{
		    if (strlen(EKINSTR_FIELD(*instr, name)) == 0)
			fprintf(stderr,
				"--Instr. #%02d (unused)--\n", i+1);
		    else
			fprintf(stderr,
			"--Instr. #%02d (comment field '%s')--\n",
			i+1, EKINSTR_FIELD(*instr, name));
		}
		else
		{
		  EkErrFlag	error;
		  char *filename= writename(
					EKINSTR_FIELD(*instr, filename),
					EKINSTR_FIELD(*instr, name),
					prefix);

		    instdetail(instr, i+1);
		    if ( (error= dump_sample(instr, prefix))
							== EK_ERR_NONE )
		    {
			printf("File '%s' saved OK\n", filename);
		    }
		    else
		    {
			fprintf(stderr,
				"Save failure '%s', writing '%s'\n",
				EKERRFLAG_STRING_VALUE(error),
				filename);
		    }
		    FREE_MEMORY(filename, "ekdump::dump(filename)");
		}
	    }
	}
}

#ifndef DEVEL
static
#endif
void ekdump(EkList *list, const char *prefix)
{
  EkSongList	*listent;

	while (eklist_next(list))
	{
	  const char	*file;
	  char		*tempfile;
	  int		compressID;
	  EkErrFlag	error;
	  EkSong	song;

	    listent= (EkSongList *)eklist_current(list);
	    file= EKSONGLIST_FIELD(listent, filename);

	    if ((compressID= compression(file)) == -1)
	    {
		tempfile= NULL;
	    }
	    else
	    {
		tempfile= uncompress(file, compressID);
	    }

	    if ((error= eksong_construct(&song)) == EK_ERR_NONE)
		error= eksong_load_bestguess(&song,
					tempfile? tempfile : file);
	    if (tempfile)
	    {
		unlink(tempfile);
		FREE_MEMORY(tempfile, "ekdump::tempfile");
	    }

	    if (error != EK_ERR_NONE)
		fprintf(stderr, "Song %s: Error -%s\n",
				file, EKERRFLAG_STRING_VALUE(error));
	    else if (eklist_empty(
				&EKSONGLIST_FIELD(listent, samplist)))
	    {
		details(tempfile? file : NULL, &song);
	    }
	    else
	    {
		printf("Song %s\n",
			strlen(EKSONG_FIELD(song, title)) > 0?
			EKSONG_FIELD(song, title) : "<unnamed>");
		while (eklist_next(
				&EKSONGLIST_FIELD(listent, samplist)))
		{
		    dump(&song, (const char *)eklist_current(
				&EKSONGLIST_FIELD(listent, samplist)),
			prefix);
		}
	    }

	    eksong_destroy(&song);
	}
}

#ifndef DEVEL
static
#endif
void dumplist_clear(EkList *list)
{
  EkSongList	*listent;

	while (eklist_next(list))
	{
	    listent= (EkSongList *)eklist_current(list);
	    while (eklist_next(&EKSONGLIST_FIELD(listent, samplist)))
	    {
		eklist_unlink(&EKSONGLIST_FIELD(listent, samplist));
	    }
	    eklist_unlink(list);
	    eksonglist_destroy(listent);
	}
}

int main(int argc, const char **argv)
{
  const char	*argText, *parText;
  Flag		done;
  EkParseState	pState= parseState_filename;
  EkList	dumplist;
  EkSongList	*listent;
  EkErrFlag	error;
  const char	*prefix= "Inst";

	if ( ((error= eklist_construct(&dumplist)) != EK_ERR_NONE)
		|| ((error=
			eksonglist_new(&listent)) != EK_ERR_NONE) )
	{
	    fprintf(stderr, "Initialise failure -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	    exit(1);
	}

  	initArg(argc, argv);

	done= FL_FALSE;
	while (nextOpt(&argText, &parText))
	{
	    if (strcmp(argText, "prefix") == 0)
	    {
		if (parText)
		    prefix= parText;
		else
		{
		    fprintf(stderr, "%s: Parameter for '%s' missing\n",
				argv[0], argText);
		    done= FL_TRUE;
		}
	    }
	    else
	    {
		fprintf(stderr, "%s: Bad/misplaced option '%s'\n",
			argv[0], argText);
		if (parText) rewindArg();
	    }
	}

	while (done == FL_FALSE)
	{
/* there should be no supplied options after the filename */
	    while (nextOpt(&argText, &parText))
	    {
		fprintf(stderr, "%s: Bad option '%s'\n",
						argv[0], argText);
		if (parText) rewindArg();
	    }

/* ...only arguments */
	    if (!nextArg(&argText))
		done= FL_TRUE;
	    else
	    {
		switch(pState)
		{
		case parseState_filename:
		    if ( ((error= eksonglist_construct(
					listent)) == EK_ERR_NONE)
			&& ((error= eklist_construct(
				&EKSONGLIST_FIELD(listent, samplist)))
					== EK_ERR_NONE) )
		    {
			EKSONGLIST_FIELD(listent, filename)= argText;
			eklist_add(&dumplist, listent);
		    }
		    pState= parseState_samples;
		    break;
		case parseState_samples:
		case parseState_samples_bad:
		    if (eval_range(argText, NULL, NULL))
			eklist_add(&EKSONGLIST_FIELD(listent, samplist),
				(void *)argText);
		    else
		    {
			fprintf(stderr, "%s: %s\n", argv[0],
				"Expected numeric range or \"all\"");
			pState= parseState_samples_bad;
		    }
		    break;
		}
	    }
	}

	switch(pState)
	{
	case parseState_samples:
	    ekdump(&dumplist, prefix);
	    break;
	default:
	    fprintf(stderr, "Usage:\n\t%s %s\n", argv[0],
			"<song> [ all | [<num>]..[<num>] ... ]");
	}

	dumplist_clear(&dumplist);

	if ( ((error= eklist_destroy(&dumplist)) != EK_ERR_NONE)
		|| ((error=
			eksonglist_delete(&listent)) != EK_ERR_NONE) )
	{
	    fprintf(stderr, "Terminate failure -%s\n",
				EKERRFLAG_STRING_VALUE(error));
	    exit(1);
	}
  return 0;
}
