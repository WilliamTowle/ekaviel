#include "load_med.h"

#include "ekinstr.h"
#include "ekmalloc.h"
#include "loadbase.h"
#include "load_st.h"

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd0_song(EkSong *song, FILE *ifPtr, long offs)
{
  int		instrs= EKSONG_FIELD(*song, instrsz),
		inst;
  EkErrFlag	error= eksong_construct_instrs(song, instrs);

	if (error == EK_ERR_NONE) error= fputat(ifPtr, offs);

	for (inst= 0; (inst < instrs) && (error == EK_ERR_NONE); inst++)
	{
	    EKINSTR_FIELD(EKSONG_FIELD(*song, instr[inst]), rpt_bgn)=
					2*fgetUsht(ifPtr, &error);
	    EKINSTR_FIELD(EKSONG_FIELD(*song, instr[inst]), rpt_len)=
					2*fgetUsht(ifPtr, &error);
	    EKINSTR_FIELD(EKSONG_FIELD(*song, instr[inst]), midich)=
					fgetUchr(ifPtr, &error);
	    EKINSTR_FIELD(EKSONG_FIELD(*song, instr[inst]), midipreset)=
					fgetUchr(ifPtr, &error);
	    EKINSTR_FIELD(EKSONG_FIELD(*song, instr[inst]), volume)=
					fgetUchr(ifPtr, &error);
	    EKINSTR_FIELD(EKSONG_FIELD(*song, instr[inst]), transpose)=
					fgetSchr(ifPtr, &error);
	}
	if (error != EK_ERR_NONE) return error;

	EKSONG_FIELD(*song, blocks)= fgetUsht(ifPtr, &error);
	EKSONG_FIELD(*song, playseqs)= fgetUsht(ifPtr, &error);

	/* Theoretical maxblocks is 256, but we have a value in
	 * song->blocks (whereas in MOD format we don't), and
	 * we pass that in instead: */
	if (error == EK_ERR_NONE)
	    error= load_st_seqlist(song, ifPtr,
				256, EKSONG_FIELD(*song, blocks));
	if (error != EK_ERR_NONE) return error;

	EKSONG_FIELD(*song, tempo)= fgetUsht(ifPtr, &error);
	EKSONG_FIELD(*song, transpose)= fgetSchr(ifPtr, &error);
	EKSONG_FIELD(*song, flags1)= fgetUchr(ifPtr, &error);
	EKSONG_FIELD(*song, flags2)= fgetUchr(ifPtr, &error);
	EKSONG_FIELD(*song, fineTempo)= fgetUchr(ifPtr, &error);

	if (error == EK_ERR_NONE)
	    error= eksong_construct_trkvols(song, 16);
	for (inst= 0; (inst < 16) && (error == EK_ERR_NONE); inst++)
	    EKSONG_FIELD(*song, trkvol[inst])= fgetUchr(ifPtr, &error);

	EKSONG_FIELD(*song, mstrvol)= fgetUchr(ifPtr, &error);
	EKSONG_FIELD(*song, lastinstr)= fgetUchr(ifPtr, &error);

  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd0_note(EkNote *note, FILE *ifPtr)
{
	if (note == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "load_mmd0_note() called %s\n",
				"with NULL note");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkErrFlag	error= EK_ERR_NONE;
	  U_CHR		uc[3], ch;

	    for (ch= 0; (ch < 3) && (error == EK_ERR_NONE); ch++)
		uc[ch]= fgetUchr(ifPtr, &error);
	    if (error != EK_ERR_NONE) return error;

	    EKNOTE_FIELD(*note, pitch)	= uc[0] & 0x3F;
	    for (; EKNOTE_FIELD(*note, pitch) > perTabSz;
			EKNOTE_FIELD(*note, pitch)-= 12);
	    EKNOTE_FIELD(*note, instNum)=
			((uc[0] & 0x40)>>1) | ((uc[0] & 0x80) >> 3)
					| ((uc[1] & 0xF0) >> 4);
	    EKNOTE_FIELD(*note, fx)	= uc[1] & 0xF;
	    EKNOTE_FIELD(*note, fxdat)	= uc[2];
#ifndef SLOW_DISPLAY
	    eknote_set_text(note);
#endif
	  return EK_ERR_NONE;
	}
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd0_block(EkBlock *block, FILE *ifPtr)
{
  EkErrFlag	error= EK_ERR_NONE;
  int		tracks= EKBLOCK_FIELD(*block, tracks)=
				fgetUchr(ifPtr, &error),
		lines= (EKBLOCK_FIELD(*block, lines)=
				fgetUchr(ifPtr, &error)+1);

	if (error == EK_ERR_NONE)
	    error= ekblock_construct_notes(block, lines, tracks);

	if (error == EK_ERR_NONE)
	{
	  EkNote	**note= EKBLOCK_FIELD(*block, note);

	    for (; (lines > 0) && (error == EK_ERR_NONE); lines--)
	    {
	      int	tr;

		for (tr= 0; (tr < tracks)
				&& (error == EK_ERR_NONE); tr++)
		{
		    error= load_mmd0_note(&(*note)[tr], ifPtr);
		}

		if (error == EK_ERR_NONE) note++;
	   }
	}

  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd1_note(EkNote *note, FILE *ifPtr)
{
	if (note == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "load_mmd1_note() called %s\n",
				"with NULL note");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkErrFlag	error= EK_ERR_NONE;
	  U_CHR		uc[4], ch;

	    for (ch= 0; (ch < 4) && (error == EK_ERR_NONE); ch++)
		uc[ch]= fgetUchr(ifPtr, &error);
	    if (error != EK_ERR_NONE) return error;

	    EKNOTE_FIELD(*note, pitch)	= uc[0];
	    for (; EKNOTE_FIELD(*note, pitch) > perTabSz;
			EKNOTE_FIELD(*note, pitch)-= 12);
	    EKNOTE_FIELD(*note, instNum)= uc[1] & 0x3F;
	    EKNOTE_FIELD(*note, fx)	= uc[2];
	    EKNOTE_FIELD(*note, fxdat)	= uc[3];
#ifndef SLOW_DISPLAY
	    eknote_set_text(note);
#endif

	  return error;
	}
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd1_block(EkBlock *block, FILE *ifPtr, U_LNG *xtradata)
{
  EkErrFlag	error= EK_ERR_NONE;
  int		tracks= EKBLOCK_FIELD(*block, tracks)=
				fgetUsht(ifPtr, &error),
		lines= (EKBLOCK_FIELD(*block, lines)=
				fgetUsht(ifPtr, &error)+1);

	if (error == EK_ERR_NONE)
	    error= ekblock_construct_notes(block, lines, tracks);

	if (error == EK_ERR_NONE)
	{
	  EkNote	**note= EKBLOCK_FIELD(*block, note);

	    *xtradata= fgetUlng(ifPtr, &error);

	    for (; (lines > 0) && (error == EK_ERR_NONE); lines--)
	    {
	      int	tr;

		for (tr= 0; (tr < tracks)
				&& (error == EK_ERR_NONE); tr++)
		{
		    error= load_mmd1_note(&(*note)[tr], ifPtr);
		}

		if (error == EK_ERR_NONE) note++;
	    }
	}
  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd_blocks(EkSong *song, FILE *ifPtr,
			long offs, int blks, const char *id)
{
  U_LNG		*blokData= (U_LNG *)ALLOCATE_CLRMEM(blks, U_LNG),
		*xblkData= (U_LNG *)ALLOCATE_CLRMEM(blks, U_LNG);
  int		*blokNlen= (int *)ALLOCATE_CLRMEM(blks, int),
		i;
  EkErrFlag	error= ( (blokData == NULL) || (xblkData == NULL)
			|| (blokNlen == NULL) )? EK_ERR_MALLOC_FAILURE
						: EK_ERR_NONE;

	if ( (error == EK_ERR_NONE)
		&& (error= eksong_construct_blocks(song, blks))
						== EK_ERR_NONE )
		error= fputat(ifPtr, offs);
	if (error != EK_ERR_NONE) return error;

	for (i= 0; (i < blks) && (error == EK_ERR_NONE); i++)
	    blokData[i]= fgetUlng(ifPtr, &error);

	for (i= 0; (i < blks) && (error == EK_ERR_NONE); i++)
	{
	    if ( (error= fputat(ifPtr, blokData[i])) == EK_ERR_NONE )
	    {
	        switch (id[3])
		{
		case '0':	/* MMD0 */
		    error= load_mmd0_block(
				&EKSONG_FIELD(*song, block[i]), ifPtr);
		    xblkData[i]= 0;
		    break;
		default:	/* MMD1 and up */
		    error= load_mmd1_block(
				&EKSONG_FIELD(*song, block[i]), ifPtr,
				&xblkData[i]);
		}

		if ( EKBLOCK_FIELD(EKSONG_FIELD(*song, block[i]),
			tracks) > EKSONG_FIELD(*song, maxtracks) )
		{
		    EKSONG_FIELD(*song, maxtracks)=
			EKBLOCK_FIELD( EKSONG_FIELD(
					*song, block[i]), tracks);
		}
	    }
	}

	for (i= 0; (i < blks) && (error == EK_ERR_NONE); i++)
	    if (xblkData[i])
	    {
		if ((error= fputat(ifPtr, xblkData[i])) == EK_ERR_NONE)
		{
		    error= fskip(ifPtr, 4);
		    xblkData[i]= fgetUlng(ifPtr, &error);
		    blokNlen[i]= (int)fgetUlng(ifPtr, &error);
		    /* six more (value-zero) longs in MMD1 */
		}
	    }

	for (i= 0; (i < blks) && (error == EK_ERR_NONE); i++)
	    if (xblkData[i])
	    {
		if ((error= fputat(ifPtr, xblkData[i])) == EK_ERR_NONE)
		    error= fgetStr( &EKBLOCK_FIELD(
				EKSONG_FIELD(*song, block[i]), name),
						ifPtr, blokNlen[i] );
	    }

  return error;
}

/* load hold and decay values, and miscellaneous other sample data. */
#ifndef DEVEL
static
#endif
EkErrFlag load_mmd_hodes(EkInstr *instr, FILE *ifPtr,
			long offs, long recs, long sz)
{
  EkErrFlag	error= fputat(ifPtr, offs);
  long		i;

	for (i= 0; (i < recs) && (error == EK_ERR_NONE); i++)
	{
	  int	j;

	    EKINSTR_FIELD(instr[i], hold)= fgetUchr(ifPtr, &error);
	    EKINSTR_FIELD(instr[i], decay)= fgetUchr(ifPtr, &error);
	    EKINSTR_FIELD(instr[i], suppress)= fgetUchr(ifPtr, &error);
	    EKINSTR_FIELD(instr[i], finetune)= fgetSchr(ifPtr, &error);

		/* in case record size has changed: */
	    for (j= 4; (j < sz) && (error == EK_ERR_NONE); j++)
		error= fskip(ifPtr, 1);
	}

  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd_instnames(EkInstr *samp, FILE *ifPtr,
				long offs, long recs, int sz)
{
  EkErrFlag	error= EK_ERR_NONE;
  long		i;

	for (i= 0; (i < recs) && (error == EK_ERR_NONE); i++)
	{
	    if ( (error= fputat(ifPtr, offs+i*sz)) == EK_ERR_NONE )
		error= fgetStr(&EKINSTR_FIELD(samp[i],name), ifPtr, sz);
	}
	
  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd_xdat(EkSong *song, FILE *ifPtr, long offs)
{
  U_LNG		hodeRecs, snamRecs,
		hodeOffs, snamOffs, stitOffs,
		miscOffs;	/* editor preferences. Palette etc? */
  int		hodeSize, snamSize, stitSize;

  EkErrFlag	error= fputat(ifPtr, offs);

	if (error == EK_ERR_NONE) error= fskip(ifPtr, 4);
	hodeOffs= fgetUlng(ifPtr, &error);
	hodeRecs= fgetUsht(ifPtr, &error);
	hodeSize= fgetUsht(ifPtr, &error);
	if (error == EK_ERR_NONE)
	    if ( (error= fskip(ifPtr, 4)) == EK_ERR_NONE )
		error= fskip(ifPtr, 4);

	snamOffs= fgetUlng(ifPtr, &error);
	snamRecs= fgetUsht(ifPtr, &error);
	snamSize= fgetUsht(ifPtr, &error);
	if (error == EK_ERR_NONE) error= fskip(ifPtr, 4);

	miscOffs= fgetUlng(ifPtr, &error);
	if ( (error == EK_ERR_NONE)
	    && ((error= fskip(ifPtr, 4)) == EK_ERR_NONE) )
		error= fskip(ifPtr, 4);

	stitOffs= fgetUlng(ifPtr, &error);
	stitSize= (int)fgetUlng(ifPtr, &error);	/* ??: 2pad+Usht */
	/* 8 more (zero-value) longs */
	if (error != EK_ERR_NONE) return error;

	if ( (error= load_mmd_hodes(EKSONG_FIELD(*song, instr),
				ifPtr, hodeOffs, hodeRecs, hodeSize))
			!= EK_ERR_NONE )
	    return error;

	if ( snamOffs
		&& ((error= load_mmd_instnames(
			EKSONG_FIELD(*song, instr), ifPtr,
			snamOffs, snamRecs, snamSize))
				!= EK_ERR_NONE) )
	    return error;

	if ( stitOffs
		&& ((error= fputat(ifPtr, stitOffs)) != EK_ERR_NONE) )
	    error= fgetStr(&EKSONG_FIELD(*song, title),
						ifPtr, stitSize);

  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd_instr(EkInstr *inst, FILE *ifPtr)
{
  EkErrFlag	error= EK_ERR_NONE;
  void		*waveform;

	EKINSTR_FIELD(*inst, length)= fgetUlng(ifPtr, &error);
	if (error != EK_ERR_NONE) return error;

	waveform= ALLOCATE_CLRMEM(EKINSTR_FIELD(*inst, length), S_CHR);
	if ( (EKINSTR_FIELD(*inst, waveform)= waveform) == NULL )
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	  U_LNG	i;

	    if (fgetUsht(ifPtr, &error) != 0)
		EKINSTR_FIELD(*inst, insType)= insType_unplayable;
	    else	/* SoundTracker-type (sb) sample */
		EKINSTR_FIELD(*inst, insType)= insType_sigChar;

	    for (i= 0; (i < EKINSTR_FIELD(*inst, length))
			&& (error == EK_ERR_NONE); i++)
	    {
		((S_CHR *)EKINSTR_FIELD(*inst, waveform))[i]=
					fgetSchr(ifPtr, &error);
	    }
	}

  return error;
}

#ifndef DEVEL
static
#endif
EkErrFlag load_mmd_instrs(EkSong *song, FILE *ifPtr, long offs, int lastinst)
{
  U_LNG		*sampData=
			(U_LNG *)ALLOCATE_CLRMEM(lastinst, sizeof(U_LNG));
  EkErrFlag	error= sampData? fputat(ifPtr, offs)
				: EK_ERR_MALLOC_FAILURE;

	if (error == EK_ERR_NONE)
	{
	  int	i;

	    for (i= 0; (i < lastinst) && (error == EK_ERR_NONE); i++)
		sampData[i]= fgetUlng(ifPtr, &error);
	    for (i= 0; (i < lastinst) && (error == EK_ERR_NONE); i++)
		if (sampData[i])	/* valid sample? */
		{
		  EkInstr	*inst= &EKSONG_FIELD(*song, instr[i]);

		    if ( (error= fputat(ifPtr, sampData[i]))
				== EK_ERR_NONE )
		    {
			error= load_mmd_instr(inst, ifPtr);
		    }
		    else
		    {
			EKINSTR_FIELD(*inst, length)= 0;
			EKINSTR_FIELD(*inst, waveform)= NULL;
		    }
		}
	}

  return error;
}

EkErrFlag load_med(EkSong *song, FILE *ifPtr, const char *id)
{
  EkErrFlag	error;
  U_LNG		fileSize, songOffs, blokOffs, sampOffs, xdatOffs;

	if (song == NULL) return EK_ERR_MALLOC_FAILURE;
	if ( (error= fputat(ifPtr, 0)) != EK_ERR_NONE ) return error;

	EKSONG_FIELD(*song, instrsz)= 63;

	if ( (error= fskip(ifPtr, 4)) != EK_ERR_NONE ) return error;
	fileSize = fgetUlng(ifPtr, &error);
	songOffs = fgetUlng(ifPtr, &error);
	if (error == EK_ERR_NONE) error= fskip(ifPtr, 4);
	blokOffs = fgetUlng(ifPtr, &error);
	if (error == EK_ERR_NONE) error= fskip(ifPtr, 4);
	sampOffs = fgetUlng(ifPtr, &error);
	if (error == EK_ERR_NONE) error= fskip(ifPtr, 4);
	xdatOffs = fgetUlng(ifPtr, &error);
	if (error == EK_ERR_NONE) error= fskip(ifPtr, 4);
	/* remainder of 52 bytes irrelevant */
	if (error != EK_ERR_NONE) return error;

	if ( (error= load_mmd0_song(song, ifPtr, songOffs))
		!= EK_ERR_NONE )
	    return error;

	EKSONG_FIELD(*song, maxtracks)= 0;
	if (blokOffs)
	    if ( (error= load_mmd_blocks(song, ifPtr, blokOffs,
				EKSONG_FIELD(*song, blocks), id))
			!= EK_ERR_NONE )
		return error;

	if (xdatOffs)
	    if ( (error= load_mmd_xdat(song, ifPtr, xdatOffs))
			!= EK_ERR_NONE )
		return error;

	if (sampOffs)
	{
	    if ( (error= load_mmd_instrs(song, ifPtr, sampOffs,
			EKSONG_FIELD(*song, lastinstr))) != EK_ERR_NONE )
		return error;
	}
	else
	{
	  int	missing= 0,
		i;

	    for (i= 0; (i <= EKSONG_FIELD(*song, lastinstr))
			&& (error == EK_ERR_NONE); i++)
	    {
	      EkInstr	*inst= &EKSONG_FIELD(*song, instr[i]);

		if ( EKINSTR_FIELD(*inst, name)
			&& strlen(EKINSTR_FIELD(*inst, name)) )
		{
		  const char	*filename;
		    if ( (filename=
			find_file(EKINSTR_FIELD(*inst, name))) != NULL )
		    {
			if (ekinstr_load_bestguess(inst, filename)
					!= EK_ERR_NONE)
			    missing++;
		    }
		}
	    }

	    if (missing) error= EK_ERR_MISSING_INSTRS;
	}

  return error;
}
