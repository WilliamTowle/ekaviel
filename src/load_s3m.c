#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "loadbase.h"
#include "load_s3m.h"

static
errFlag load_s3m_orders(FILE *ifPtr,SONG *song, int seq_size)
{
  errFlag
	error= FL_TRUE;
  int	i,p;

	song->blocks= 0;
	if (song->playseq= (PSEQ *) calloc(seq_size, sizeof(PSEQ)) )
	{
	    error= FL_FALSE;
	    for (i=0; (i<seq_size) && (error == FL_FALSE); i++)
	    {
		p= fgetUch(ifPtr, &error);
fprintf(stderr,"Got s3m_order %d\n", p);
		if (p == 255) seq_size--;	/* makes list even length */
		else if (p == 254)
		    song->playseq[i]= PLAYSEQ_DIVIDER;
		else
		{
		    if (p >= song->blocks) song->blocks= p+1;
		    song->playseq[i]= p;
		}
	    }
	    if (song->blocks <= 0) error= FL_TRUE;
	}
	song->seqLength= seq_size;
  return error;
}

static
errFlag load_s3m_scrsheader(FILE *ifPtr, INSTR *instr, U_LNG *pptr)
{
  errFlag	error= FL_FALSE;
  U_LNG		rpt_end;
  char		id[4];

	instr->filename= fgetStr(ifPtr,&error, 12);
fprintf(stderr, "Instrument name: %s\n", instr->filename);
	*pptr= (fgetUch(ifPtr,&error)
		| (fgetRUsh(ifPtr,&error) << 8))*16;
fprintf(stderr,"Sample offset in file: oct+%o\n", *pptr);

	instr->length= fgetRLong(ifPtr,&error);
	instr->rpt_bgn= fgetRLong(ifPtr,&error);
	rpt_end= fgetRLong(ifPtr,&error);
	error |= (rpt_end < instr->rpt_len);
	error |= ( (instr->waveform= (ASAMP *) calloc(instr->length,
			sizeof(ASAMP))) == NULL );
	if (instr->rpt_len = rpt_end - instr->rpt_bgn)
	    instr->rpt_point= &(instr->waveform[instr->rpt_bgn]);

	instr->volume= fgetUch(ifPtr,&error);
	fskip(ifPtr,1, &error);
	switch(fgetUch(ifPtr,&error))
	{
	case 0:	/* not compressed */
	    break;
	case 1:	/* DP30ADPCM packing */
	    error= FL_TRUE;
	    break;
	default:
	    error= FL_TRUE;
	}

fprintf(stderr,"Flags=%d\n", fgetUch(ifPtr,&error));
fprintf(stderr,"C2Speed = %d\n", fgetRLong(ifPtr,&error));
	fskip(ifPtr,4, &error);	/* unused*/
	fskip(ifPtr,4, &error);	/* run-time player information */

	instr->name= fgetStr(ifPtr,&error, 28);
	instr->transpose= instr->finetune= 0;
	instr->hold= instr->decay= 0;
	instr->midich= instr->suppress= instr->midipreset= 0;
	fread(id, sizeof(char),4, ifPtr);
	error |= (strncmp(id,"SCRS", 4) != 0);

	/* TODO: (?) if (!error) instr->type= insType_sigChar */
  return error;
}

static
errFlag load_s3m_scriheader(FILE *ifPtr, INSTR *instr, U_LNG *pptr)
{ return FL_FALSE; }

static
errFlag load_s3m_pattData(FILE *ifPtr, BLOCK *block)
{
  return FL_FALSE;
}

static
errFlag load_s3m_paraPtrs(FILE *ifPtr, SONG *song)
{
  int	instrs= song->instrs,
	pattns= song->blocks;
  U_LNG	*instPtrs= (U_LNG *) calloc(instrs, sizeof(U_LNG)),
	*pattPtrs= (U_LNG *) calloc(pattns, sizeof(U_LNG));
  errFlag
	error= (instPtrs == NULL) || (pattPtrs == NULL);
  int	i;

	song->instrs= instrs;
	ekMalloc_instrs(song, &error);
	song->block= (BLOCK *) calloc(pattns, sizeof(BLOCK));
	error |= (song->block == NULL);
fprintf(stderr,"Instrs=%d, Blocks=%d (mem? err=%d)\n", instrs,pattns,error);

	for (i=0; (i<instrs) && (!error); i++)
	    instPtrs[i]= fgetRUsh(ifPtr,&error)*16;
	for (i=0; (i<pattns) && (!error); i++)
	    pattPtrs[i]= fgetRUsh(ifPtr,&error)*16;

	for (i=0; (i<instrs) && (!error); i++)
	{
	  U_CHR	type;
	    fputat(ifPtr, instPtrs[i],&error);
fprintf(stderr,"Inst #%d is at oct+%o\n", i,instPtrs[i]);
	    switch(type= fgetUch(ifPtr,&error))
	    {
	    case 1:	/* sample */
		error |= load_s3m_scrsheader(ifPtr, &(song->instr[i]),
						&instPtrs[i]);
		break;
	    case 2:	/* adlib */
	    case 3:
	    case 4:
	    case 5:
	    case 6:
	    case 7:
		/* TODO: Variable 'type' probably useful here */
		error |= load_s3m_scriheader(ifPtr, &(song->instr[i]),
						&instPtrs[i]);
		break;
	    default:
		error= FL_TRUE;
	    }
	}
  return error;
}


SONG *load_s3m(FILE *ifPtr)
{
  errFlag
	error= FL_FALSE;
  SONG	*song= ekMalloc_song(&error);
  int	ordNum;
  Flag	sign;

	if (error)
	    return null_song(song, "Memory allocation error");
	fputat(ifPtr, 0,&error);
	if (error) return null_song(song, "File seek error");

	song->title= fgetStr(ifPtr,&error, 28);
	if (!(song->title))
	    return null_song(song, "String allocation/read error");

	if (fgetUch(ifPtr,&error) != 0x1A) error= FL_TRUE;
	/* "Type"; 16= ST3 module */
fprintf(stderr,"Type=%x\n", fgetUch(ifPtr,&error));

	fskip(ifPtr,2, &error);
	ordNum= fgetRUsh(ifPtr,&error);
	song->instrs= fgetRUsh(ifPtr,&error);
	song->blocks= fgetRUsh(ifPtr,&error);
fprintf(stderr,"Flags=%x\n", fgetRUsh(ifPtr,&error));
	fskip(ifPtr,2, &error);		/* tracker version number*/

	switch(fgetRUsh(ifPtr,&error))
	{	/* 1 for signed samples (old), 2 for unsigned samples */
	case 1:
	    sign= FL_TRUE;
	    break;
	case 2:
	    sign= FL_FALSE;
	    break;
	default:
	    return null_song(song, "Sample type error");
	}
	fskip(ifPtr,4, &error);		/* @0x2C: ID = 'S' 'C' 'R' 'M' */
	fprintf(stderr,"globalVol=%d\n", fgetUch(ifPtr,&error));
	fprintf(stderr,"initial speed=%d\n", fgetUch(ifPtr,&error));
	fprintf(stderr,"initial tempo=%d\n", fgetUch(ifPtr,&error));
	fprintf(stderr,"master vol (bit 8->stereo) = %d\n",
						fgetUch(ifPtr,&error));
	fskip(ifPtr,10, &error);
	fprintf(stderr,"special= %d\n", fgetRUsh(ifPtr,&error));
	fskip(ifPtr,32, &error); fprintf(stderr,"CHANNEL SETTINGS!\n");

	error |= load_s3m_orders(ifPtr, song,ordNum);
#if 1
	error |= load_s3m_paraPtrs(ifPtr, song);
#else
/* at offset 70h+orders */
	error |= load_s3m_instrs(ifPtr,song, song->instrs);
/* at offset 70h+orders+instrs*2 */
	error |= load_s3m_patterns(ifPtr,song, song->blocks);
/* actual sample data, one assumes */
	error |= load_s3m_samples(ifPtr,song,song->instrs);
#endif

	if (error) return null_song(song, "Just about to return when error");
  return song;
}
