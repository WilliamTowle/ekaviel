#include "load_st.h"

#include "effects.h"	/* various effect constants */
#include "ekinstr.h"
#include "ekmalloc.h"
#include "eknote.h"
#include "loadbase.h"

/*****
 *	SoundTracker/ProTracker load routines
 *****/

#ifndef DEVEL
static
#endif
EkErrFlag load_st_sampdata(EkInstr *inst, FILE *ifPtr)
{
	if (inst == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	  EkErrFlag	error;

	    error= fgetStr(&EKINSTR_FIELD(*inst, name), ifPtr, 22);
	    if (error != EK_ERR_NONE) return error;

	    EKINSTR_FIELD(*inst, length)= fgetUsht(ifPtr, &error);
	    EKINSTR_FIELD(*inst, transpose)= 0;

	    EKINSTR_FIELD(*inst, finetune)= fgetUchr(ifPtr, &error);
	    if (error != EK_ERR_NONE) return error;
	    if (EKINSTR_FIELD(*inst, finetune) > 15)
		EKINSTR_FIELD(*inst, finetune)= 0;
	    if (EKINSTR_FIELD(*inst, finetune) == 0)
		EKINSTR_FIELD(*inst, tuning)= EK_INSTUNE_SB_PERFECT;
	    else
		EKINSTR_FIELD(*inst, tuning)= EK_INSTUNE_SB_FINE;

	    EKINSTR_FIELD(*inst, volume)= fgetUchr(ifPtr, &error);
	    EKINSTR_FIELD(*inst, rpt_bgn)= fgetUsht(ifPtr, &error);
	    EKINSTR_FIELD(*inst, rpt_len)= fgetUsht(ifPtr, &error);
	    if (error != EK_ERR_NONE) return error;
/*****
 *	Some old(er) module formats confuse the sample data type,
 *	storing the values in bytes when they mean words (double-bytes).
 *	kawai-k1's sample repeats aren't stored the same way, either.
 *****/
	    if ( EKINSTR_FIELD(*inst, rpt_bgn)
		+ EKINSTR_FIELD(*inst, rpt_len) -1
		> EKINSTR_FIELD(*inst, length) )
	    {
		EKINSTR_FIELD(*inst, rpt_bgn)/= 2;
	    }
	    if ( EKINSTR_FIELD(*inst, rpt_bgn)
		+ EKINSTR_FIELD(*inst, rpt_len)
		> EKINSTR_FIELD(*inst, length) )
	    {
		EKINSTR_FIELD(*inst, rpt_len)=
			EKINSTR_FIELD(*inst, length)
			- EKINSTR_FIELD(*inst, rpt_bgn);
	    }

/*****
 *	And because they're word values and ekaviel wants bytes...
 *****/
	    EKINSTR_FIELD(*inst, length)*= 2;
	    EKINSTR_FIELD(*inst, rpt_bgn)*= 2;
	    EKINSTR_FIELD(*inst, rpt_len)*= 2;

/*****
 *	Some trackers' empty instruments have length 2
 *****/

	    if (EKINSTR_FIELD(*inst, length) <= 2)
	    {
		EKINSTR_FIELD(*inst, waveform)= NULL;
		EKINSTR_FIELD(*inst, length)= 0;
	    }
	    else if ( (EKINSTR_FIELD(*inst, waveform)=
			(void *)ALLOCATE_MEMORY(
				EKINSTR_FIELD(*inst, length), S_CHR))
					== NULL )
	    {
		return EK_ERR_MALLOC_FAILURE;
	    }

	    if (EKINSTR_FIELD(*inst, rpt_len) <= 2)
		EKINSTR_FIELD(*inst, rpt_len)= 0;

	    if (EKINSTR_FIELD(*inst, length) > MAX_SAMPLE_LENGTH)
		return EK_ERR_FILECORRUPT;

	    return EK_ERR_NONE;
	}
}

#ifndef DEVEL
static
#endif
EkErrFlag load_st_samplist(EkSong *song, FILE *ifPtr)
{
  int		instrs= EKSONG_FIELD(*song, instrsz);
  EkErrFlag	error= eksong_construct_instrs(song, instrs);

	if (error == EK_ERR_NONE)
	{
	  int		inst;

	    for (inst= 0; (inst < instrs) && (error == EK_ERR_NONE);
			inst++)
	    {
	      EkInstr	*instr= &EKSONG_FIELD(*song, instr[inst]);
		error= load_st_sampdata(instr, ifPtr);
		if (error == EK_ERR_NONE)
		{
		    if (EKINSTR_FIELD(*instr, length))
			EKSONG_FIELD(*song, lastinstr)= inst+1;
		    instr++;
		}
	    }
	}

  return error;
}

EkErrFlag load_st_seqlist(	EkSong *song, FILE *ifPtr,
				int seq_size, int maxblks)
{
  EkErrFlag	error= EK_ERR_NONE;
  int		i;

	EKSONG_FIELD(*song, blocks)= 0;
	if ( (EKSONG_FIELD(*song, playseq)=
		(PSEQ *)ALLOCATE_CLRMEM(seq_size, PSEQ)) == NULL )
	{
	    return EK_ERR_MALLOC_FAILURE;
	}

	for (i=0; (i < seq_size) && (error == EK_ERR_NONE); i++)
	{
	  PSEQ	p= fgetUchr(ifPtr,&error);

	    if (p >= maxblks)
		p= 0;
	    if (p >= EKSONG_FIELD(*song, blocks))
		EKSONG_FIELD(*song, blocks)= p+1;

	    EKSONG_FIELD(*song, playseq)[i]= p;
	}

	if ( (error == EK_ERR_NONE)
			&& (EKSONG_FIELD(*song, blocks) <= 0) )
	    return EK_ERR_FILECORRUPT;

  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_st_note(EkNote *note, FILE *ifPtr)
{
	if (note == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "load_st_note() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkErrFlag	error= EK_ERR_NONE;
	  U_CHR		uc[4];

	    uc[0]= fgetUchr(ifPtr,&error);
	    uc[1]= fgetUchr(ifPtr,&error);
	    uc[2]= fgetUchr(ifPtr,&error);
	    uc[3]= fgetUchr(ifPtr,&error);

	    if (error != EK_ERR_NONE) return error;

	    EKNOTE_FIELD(*note, instNum)= (uc[0] & 0x10) | (uc[2] >> 4);
	    EKNOTE_FIELD(*note, pitch)= getnote(
					((uc[0] & 0xF)<<8) | uc[1] );

	    if ((EKNOTE_FIELD(*note, fx)= uc[2] & 0xF) != STFX_EXTENDED)
		EKNOTE_FIELD(*note, fxdat)= uc[3];
	    else
	    {
		EKNOTE_FIELD(*note, fx)=
			    EXT_BASE + Hi(EKNOTE_FIELD(*note, fxdat));
		EKNOTE_FIELD(*note, fxdat)=
			    Lo(EKNOTE_FIELD(*note, fxdat));
	    }

#ifndef SLOW_DISPLAY
	    eknote_set_text(note);
#endif

	  return error;
	}
}

#ifndef DEVEL
static
#endif
EkErrFlag load_st_block(	EkBlock *block, FILE *ifPtr,
				int tracks, int lines)
{
  EkErrFlag	error= EK_ERR_NONE;

	EKBLOCK_FIELD(*block, name)= NULL;
	EKBLOCK_FIELD(*block, tracks)= tracks;
	EKBLOCK_FIELD(*block, lines)= lines;

	if ( (error= ekblock_construct_notes(block, lines, tracks))
		== EK_ERR_NONE )
	{
	  EkNote **note= EKBLOCK_FIELD(*block, note);

	    for (; (lines > 0) && (error == EK_ERR_NONE); lines--)
	    {
	      int	t;

		for (t=0; (t < tracks) && (error == EK_ERR_NONE); t++)
		{
#if 1
		    error= load_st_note(&(*note)[t], ifPtr);
#else
		    error= load_st_note(&(block->note[i][j]), ifPtr);
#endif
		}

		if (error == EK_ERR_NONE) note++;
	    }
	}
  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_st_blocks(EkSong *song, FILE *ifPtr, int chans)
{
  int		blocks= EKSONG_FIELD(*song, blocks);
  EkErrFlag	error= eksong_construct_blocks(song, blocks);

	if (error == EK_ERR_NONE)
	{
	  EkBlock	*block= EKSONG_FIELD(*song, block);
	    for (; (blocks > 0) && (error == EK_ERR_NONE); blocks--)
	    {
		error= load_st_block(block, ifPtr, chans, 64);
		if (error == EK_ERR_NONE) block++;
	    }
	}

  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_st_samps(EkSong *song, FILE *ifPtr)
{

	if (EKSONG_FIELD(*song, instrsz) == 0)
	    return EK_ERR_FILECORRUPT;
	else
	{
	  EkErrFlag	error= EK_ERR_NONE;
	  int		i;

	    for (i= 0; (i < EKSONG_FIELD(*song, instrsz))
			&& (error == EK_ERR_NONE); i++)
	    {
		if (EKINSTR_FIELD(EKSONG_FIELD(*song, instr)[i],
				length))
		{
		    error= ekinstr_load_st(
			&(EKSONG_FIELD(*song, instr)[i]), ifPtr);
		}
	    }

	    return error;
	}
}

EkErrFlag load_st(EkSong *song, FILE *ifPtr, int samps, int chans)
{
  EkErrFlag	error= EK_ERR_NONE;

	if (song == NULL) return EK_ERR_MALLOC_FAILURE;

	if ((error= fputat(ifPtr, 0)) != EK_ERR_NONE) return error;

	if ( (error= eksong_construct_trkvols(song, chans))
		!= EK_ERR_NONE )
	    return error;
	EKSONG_FIELD(*song, mstrvol)= 64;

	if ( (error= fgetStr(&EKSONG_FIELD(*song, title), ifPtr, 20))
		!= EK_ERR_NONE)
	    return error;

	EKSONG_FIELD(*song, tempo)= 33;
	EKSONG_FIELD(*song, fineTempo)= 6;
	EKSONG_FIELD(*song, instrsz)= samps-1;
	EKSONG_FIELD(*song, maxtracks)= chans;

	if ( (error= load_st_samplist(song, ifPtr)) != EK_ERR_NONE )
	    return error;

	EKSONG_FIELD(*song, playseqs)= fgetUchr(ifPtr, &error);
	if (error != EK_ERR_NONE) return error;
	if (EKSONG_FIELD(*song, playseqs) == 0)
	    return EK_ERR_FILECORRUPT;

	fgetUchr(ifPtr, &error);	/* this byte's meaning varies,
					but is not important to us */

	if ( (error= load_st_seqlist(song, ifPtr, 128,64))
		!= EK_ERR_NONE )
	    return error;

/*****
 *	Here's where the 4-character ID field appears in newer MODs
 *****/
	if (samps != 16)
	{
	    error= fskip(ifPtr, 4);
	    if (error != EK_ERR_NONE) return EK_ERR_FILECORRUPT;
	}

	if ((error= load_st_blocks(song, ifPtr, chans)) != EK_ERR_NONE)
	    return error;

	return load_st_samps(song, ifPtr);
}
