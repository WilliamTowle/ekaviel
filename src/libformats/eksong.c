#include "eksong.h"

#include "effects.h"
#include "ekmalloc.h"
#include "load_med.h"
#include "load_st.h"

/*****
 *	EkBlock class
 *****/

EkErrFlag ekblock_new(EkBlock **blockPtr, int reqd)
{
	if (blockPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekblock_new() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkBlock *block= (EkBlock *)ALLOCATE_MEMORY(reqd, EkBlock,
					"eksong.c::ekblock_new()");

	    if ( (*blockPtr= block) == NULL)
		return EK_ERR_MALLOC_FAILURE;
	    else
		return EK_ERR_NONE;
	}
}

EkErrFlag ekblock_construct(EkBlock *block)
{
	if (block == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekblock_construct() called %s\n",
				"with NULL block");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    EKBLOCK_FIELD(*block, name)= NULL;
	    EKBLOCK_FIELD(*block, tracks)=
			EKBLOCK_FIELD(*block, lines)= 0;
	    EKBLOCK_FIELD(*block, note)= NULL;

	    return EK_ERR_NONE;
	}
}

EkErrFlag ekblock_construct_notes(EkBlock *block,
			int lines, int tracks)
{
	if (EKBLOCK_FIELD(*block, note) != NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekblock_construct_notes() called %s\n",
				"with pre-allocated notes");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkNote	**line= (EkNote **)
				ALLOCATE_MEMORY(lines, EkNote *,
					"eksong.c::ekblock_construct_notes()");

	    if ( (EKBLOCK_FIELD(*block, note)= line) == NULL )
		return EK_ERR_MALLOC_FAILURE;
	    else
	    {
	      EkErrFlag	error= EK_ERR_NONE;

		EKBLOCK_FIELD(*block, lines)= 0;
		for (; (lines > 0)&&(error == EK_ERR_NONE); lines--)
		{
		    *line= NULL;
		    error= eknote_new(line, tracks);
		    if (error == EK_ERR_NONE)
		    {
			EKBLOCK_FIELD(*block, lines)++;
			line++;
		    }
		}

		return error;
	    }
	}
}

EkErrFlag ekblock_destroy_notes(EkBlock *block)
{
	if (EKBLOCK_FIELD(*block, note) == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekblock_destroy() called %s\n",
				"with NULL note[][]");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkNote	**line= EKBLOCK_FIELD(*block, note);
	  EkErrFlag 	error= EK_ERR_NONE;

	    for(; (EKBLOCK_FIELD(*block, lines) > 0)
					&& (error == EK_ERR_NONE);
			EKBLOCK_FIELD(*block, lines)--)
	    {
		error= eknote_delete(line);
		if (error == EK_ERR_NONE)
		{
		    FREE_MEMORY(*line,
				"eksong.c::ekblock_destroy_notes()");
		    line++;
		}
	    }

	    if (error == EK_ERR_NONE)
		EKBLOCK_FIELD(*block, note)= NULL;

	    return error;
	}
}

EkErrFlag ekblock_destroy(EkBlock *block)
{
	if (block == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekblock_destroy() called %s\n",
				"with NULL block");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    if (EKBLOCK_FIELD(*block, name) != NULL)
		FREE_MEMORY(EKBLOCK_FIELD(*block, name),
			"eksong.c::ekblock_destroy()");
	    return ekblock_destroy_notes(block);
	}
}

EkErrFlag ekblock_delete(EkBlock **blockPtr)
{
	if (blockPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekblock_delete() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    FREE_MEMORY(*blockPtr, "eksong.c::ekblock_delete()");
	    *blockPtr= NULL;
	    return EK_ERR_NONE;
	}
}

/*****
 *	EkSong class
 *****/

EkErrFlag eksong_construct(EkSong *song)
{
	if (song == NULL)
	    return EK_ERR_MALLOC_FAILURE;

	EKSONG_FIELD(*song, songType)= EK_SONG_PARTIAL;
	EKSONG_FIELD(*song, filename)= NULL;
	EKSONG_FIELD(*song, title)= NULL;
	EKSONG_FIELD(*song, trkvol)= NULL;
	EKSONG_FIELD(*song, instrsz)= 
		EKSONG_FIELD(*song, lastinstr)= 0;
	EKSONG_FIELD(*song, instr)= NULL;

	EKSONG_FIELD(*song, blocks)= 0;
	EKSONG_FIELD(*song, block)= NULL;
	EKSONG_FIELD(*song, playseqs)= 0;
	EKSONG_FIELD(*song, playseq)= NULL;

	EKSONG_FIELD(*song, flags1)=
		EKSONG_FIELD(*song, flags2)= 0;
	EKSONG_FIELD(*song, transpose)= 0;

  return EK_ERR_NONE;
}

EkErrFlag eksong_load_bestguess(EkSong *song, const char *filename)
{
  EkErrFlag	error= EK_ERR_NONE;
  FILE		*inFile;
  char		id[5];

	if (song == NULL)
	    return EK_ERR_MALLOC_FAILURE;

	inFile= fopen(filename, "r");

#if 0	/* loading s3m */
#ifdef DEVEL
	if (inFile /* && (error == EK_ERR_NONE) */
		&& (EKSONG_FIELD(*song, songType) == EK_SONG_PARTIAL))
	{
	    fseek(inFile, 0x2c,SEEK_SET);
	    fread(id, sizeof(char),4,inFile);
	    if (strncmp(id, "SCRM",4) == 0)
	    {
		init_notes();	/* getnote() requires */
		if ((error= load_s3m(song, inFile)) == EK_ERR_NONE)
		    EKSONG_FIELD(*song, songType)= EK_SONG_SCREAM;
	    }
	}
#endif
#endif

	if (inFile && (error == EK_ERR_NONE)
		&& (EKSONG_FIELD(*song, songType) == EK_SONG_PARTIAL))
	{
	    fseek(inFile, 0,SEEK_SET);
	    fread(id, sizeof(char),4,inFile);
	    if (strncmp(id, "MMD",3) == 0)
	    {
		init_notes();	/* getnote() requires */
		if ((error= load_med(song, inFile,id)) == EK_ERR_NONE)
		{
		    EKSONG_FIELD(*song, songType)= EK_SONG_MMD;
		    EKSONG_FIELD(*song, filename)= filename;
		}
	    }
	}

	if (inFile && (error == EK_ERR_NONE)
		&& (EKSONG_FIELD(*song, songType) == EK_SONG_PARTIAL))
	{
	    fseek(inFile, 1080,SEEK_SET);
	    fread(id, sizeof(char),4,inFile);
	    if ( (strncmp(id,"M.K.",4) == 0)
		|| (strncmp(id, "M!K!",4) == 0)	 /* PT: >64 blocks */
		|| (strncmp(id, "FLT4",4) == 0) )
	    {
		init_notes();	/* getnote() requires */
		if ( (error= load_st(song, inFile, 32,4))
				== EK_ERR_NONE)
		{
		    EKSONG_FIELD(*song, songType)= EK_SONG_MOD31;
		}
	    }
	    else if (strncmp(id, "6CHN",4) == 0)
	    {
		init_notes();	/* getnote() requires */
		if ( (error= load_st(song, inFile, 32,6))
				== EK_ERR_NONE)
		{
		    EKSONG_FIELD(*song, songType)= EK_SONG_MOD31;
		}
	    }
	    else if (strncmp(id, "8CHN",4) == 0)
	    {
		init_notes();	/* getnote() requires */
		if ( (error= load_st(song, inFile, 32,8))
				== EK_ERR_NONE)
		{
		    EKSONG_FIELD(*song, songType)= EK_SONG_MOD31;
		}
	    }

	    if ( (error == EK_ERR_NONE)
		&& (EKSONG_FIELD(*song, songType) != EK_SONG_PARTIAL) )
	    {
		EKSONG_FIELD(*song, filename)= filename;
		EKSONG_FIELD(*song, flags1)= MMDF1_STSLIDE;
		EKSONG_FIELD(*song, flags2)= 0;
	    }
	}

	if (inFile && (error == EK_ERR_NONE)
		&& (EKSONG_FIELD(*song, songType) == EK_SONG_PARTIAL))
	{	/* No ID tag in ST16, so we try it last. */
	    init_notes();	/* getnote() requires */
	    if ( (error= load_st(song, inFile, 16, 4))
			== EK_ERR_NONE )
	    {
		EKSONG_FIELD(*song, songType)= EK_SONG_MOD15;
		EKSONG_FIELD(*song, filename)= filename;
		EKSONG_FIELD(*song, flags1)= MMDF1_STSLIDE;
		EKSONG_FIELD(*song, flags2)= 0;
	    }
	}

	if (inFile) fclose(inFile);

	if ((error == EK_ERR_NONE)
		&& (EKSONG_FIELD(*song, songType) == EK_SONG_PARTIAL))
	    return EK_ERR_NOT_SUPPORTED;
  return error;	/* EK_ERR_NONE if song OK */
}

EkErrFlag eksong_construct_instrs(EkSong *song, int instrs)
{
	if (EKSONG_FIELD(*song, instr) != NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksong_construct_instrs() called %s\n",
				"with non-NULL instr");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkErrFlag	error= ekinstr_new(
				&EKSONG_FIELD(*song, instr), instrs);
	  EkInstr	*instr= EKSONG_FIELD(*song, instr);

	    EKSONG_FIELD(*song, instrsz)= 0;
	    EKSONG_FIELD(*song, lastinstr)= 0;
	    for (; (instrs > 0) && (error == EK_ERR_NONE); instrs--)
	    {
		if ( (error= ekinstr_construct(instr)) == EK_ERR_NONE )
		{
		    instr++;
		    EKSONG_FIELD(*song, instrsz)++;
		}
	    }

	    return error;
	}
}

int eksong_instrs_playable(EkSong *song)
{
	if (EKSONG_FIELD(*song, instr) == NULL)
	    return 0;
	else
	{
	  int	count= 0, in;

	    for (in= 0; in < EKSONG_FIELD(*song, lastinstr); in++)
	    {
	      EkInstr	*inst= &EKSONG_FIELD(*song, instr[in]);

		if ( (EKINSTR_FIELD(*inst, length) > 0)
			&& (EKINSTR_FIELD(*inst, insType)
					!= insType_unplayable)
			&& (EKINSTR_FIELD(*inst, insType)
					!= insType_unset) )
		    count++;
	    }

	    return count;
	}
}

int eksong_instrs_unplayable(EkSong *song)
{
	if (EKSONG_FIELD(*song, instr) == NULL)
	    return 0;
	else
	{
	  int	count= 0, in;

	    for (in= 0; in < EKSONG_FIELD(*song, lastinstr); in++)
	    {
	      EkInstr	*inst= &EKSONG_FIELD(*song, instr[in]);

		if (EKINSTR_FIELD(*inst, insType) == insType_unplayable)
		    count++;
	    }

	    return count;
	}
}

EkErrFlag eksong_destroy_instrs(EkSong *song)
{
	if (EKSONG_FIELD(*song, instr) == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksong_destroy_instrs() called %s\n",
				"with NULL instr");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    return ekinstr_destroy(EKSONG_FIELD(*song, instr));
	}
}

EkErrFlag eksong_construct_blocks(EkSong *song, int blocks)
{
	if (EKSONG_FIELD(*song, block) != NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksong_construct_blocks() called %s\n",
				"with pre-allocated blocks");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkErrFlag	error= ekblock_new(
				&EKSONG_FIELD(*song, block), blocks);
	  EkBlock	*block= EKSONG_FIELD(*song, block);

	    EKSONG_FIELD(*song, blocks)= 0;
	    for (; (blocks > 0) && (error == EK_ERR_NONE); blocks--)
	    {
		error= ekblock_construct(block);
		if (error == EK_ERR_NONE)
		{
		    block++;
		    EKSONG_FIELD(*song, blocks)++;
		}
	    }

	    return error;
	}
}

EkErrFlag eksong_destroy_blocks(EkSong *song)
{
	if (EKSONG_FIELD(*song, block) == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksong_destroy_blocks() called %s\n",
				"with NULL blocks");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  int		blocks= EKSONG_FIELD(*song, blocks);
	  EkBlock	*block= EKSONG_FIELD(*song, block);
	  EkErrFlag	error= EK_ERR_NONE;

	    for (; (blocks > 0) && (error == EK_ERR_NONE); blocks--)
	    {
		error= ekblock_destroy(block);
		if (error == EK_ERR_NONE) block++;
	    }

	    return error;
	}
}

EkErrFlag eksong_construct_trkvols(EkSong *song, int tracks)
{
	if (EKSONG_FIELD(*song, trkvol) != NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksong_construct_trkvols() called %s\n",
				"with allocated trkvol[]");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  int	*trkvol= (int *)ALLOCATE_MEMORY(tracks, int,
					"eksong.c::eksong_construct_trkvols()");

	    if ( (EKSONG_FIELD(*song, trkvol)= trkvol) == NULL )
		return EK_ERR_MALLOC_FAILURE;

	    for (; tracks > 0; tracks--)
	    {
		*trkvol= 64;
		trkvol++;
	    }

	    return EK_ERR_NONE;
	}
}

EkErrFlag eksong_destroy_trkvols(EkSong *song)
{
	if (EKSONG_FIELD(*song, trkvol) == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksong_destroy_blocks() called %s\n",
				"with NULL trkvol[]");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    FREE_MEMORY(EKSONG_FIELD(*song, trkvol),
			"eksong.c::eksong_destroy_trkvols()");
	    EKSONG_FIELD(*song, trkvol)= NULL;
	    return EK_ERR_NONE;
	}
}

const char *eksong_title(EkSong *song)
{	/* additional "const" because of the possibility it's
		"unnamed" */
  char	*title= EKSONG_FIELD(*song, title);

  return title? title : "<unnamed>";
}

EkErrFlag eksong_destroy(EkSong *song)
{
	if (song == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eksong_destroy() called %s\n",
				"with NULL song pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkErrFlag	error= EK_ERR_NONE;

	    if (EKSONG_FIELD(*song, title) != NULL)
		FREE_MEMORY(EKSONG_FIELD(*song, title),
				"eksong.c::eksong_destroy()");

	    if ( (error= eksong_destroy_trkvols(song)) != EK_ERR_NONE )
		return error;

	    if ( (error= eksong_destroy_instrs(song)) == EK_ERR_NONE )
		error= ekinstr_delete(&EKSONG_FIELD(*song, instr));
	    if (error != EK_ERR_NONE) return error;

	    if ( (error= eksong_destroy_blocks(song)) == EK_ERR_NONE )
		error= ekblock_delete(&EKSONG_FIELD(*song, block));

	    FREE_MEMORY(EKSONG_FIELD(*song, playseq),
			"eksong.c::eksong_destroy(playseq)");
	    return error;
	}
}
