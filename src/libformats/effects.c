#include <stdio.h>
#include "effects.h"

#include "ekmalloc.h"
#include "ekio.h"
#include "eknote.h"

int vibtable[32]= { /* protracker translation */
	0, 25, 49, 71, 90, 106, 117, 125, 127, 125, 117, 106, 90,
	71, 49, 25, 0, -25, -49, -71, -90, -106, -117, -125, -127,
	-125, -117, -106, -90, -71, -49, -25};

static EkErrFlag malloc_fxtable(void (***fxt)(FXPARAMS), int num)
{
  int	i;
	if (( *fxt= (void(**)(FXPARAMS))
		ALLOCATE_MEMORY(num, void(*)(FXPARAMS),
			"effects.c::malloc_fxtable()") ) == NULL)
	{
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    for (i=0; i<num; i++)	(*fxt)[i]= stfx_unexp;
	    return EK_ERR_NONE;
	}
}

EkErrFlag init_fxtable(EkSetup *settings, void (***fxt)(FXPARAMS))
{
  EkErrFlag	error= EK_ERR_NONE;

	/* can't do a switch, EkSongType is not an integer: */
	if (EKSETUP_FIELD(*settings, songType) == EK_SONG_PARTIAL)
	{
	    /* is okay for eksfplay */
	    if ((error= malloc_fxtable(fxt,1)) == EK_ERR_NONE)
		(*fxt)[0]= stfx_nothing;
	    return error;
	}
	else if ( (EKSETUP_FIELD(*settings, songType) == EK_SONG_MOD15)
		|| (EKSETUP_FIELD(*settings, songType) == EK_SONG_MOD31)
		)
	{
	    if ((error= malloc_fxtable(fxt,31)) == EK_ERR_NONE)
	    {
		(*fxt)[STFX_ARPEGGIO]	= stfx_arpeggio;
		(*fxt)[STFX_DOWN]	= stfx_ptchdown;
		(*fxt)[STFX_UP]		= stfx_ptchup;
		(*fxt)[STFX_PORTA]	= stfx_portamento;
		(*fxt)[STFX_VIBRATO]	= stfx_vibrato;
		(*fxt)[STFX_PRTASLIDE]	= stfx_ptaslide;
		(*fxt)[STFX_VIBSLIDE]	= stfx_vibslide;
		(*fxt)[STFX_OFFSET]	= stfx_offset;
		(*fxt)[STFX_VOLSLIDE]	= stfx_volslide;
		(*fxt)[STFX_FF]		= stfx_ff;
		(*fxt)[STFX_VOLUME]	= stfx_volume;
		(*fxt)[STFX_SKIP]	= stfx_skip;
		(*fxt)[STFX_EXTENDED]	= stfx_undef;	/* not valid */
		(*fxt)[STFX_SPEED]	= stfx_speed;

		(*fxt)[STFX_NONE]	= stfx_nothing;
		(*fxt)[STFX_SET_FILTER]	= stfx_setFilter;
		(*fxt)[STFX_SMOOTH_DOWN]= stfx_perDown;
		(*fxt)[STFX_SMOOTH_UP]	= stfx_perUp;
		(*fxt)[STFX_SET_GLISSANDO]= stfx_setGlissando;
		(*fxt)[STFX_CHG_FTUNE]	= stfx_finetune;
		(*fxt)[STFX_LOOP]	= stfx_loop;
		(*fxt)[STFX_RETRIG]	= stfx_retrig;
		(*fxt)[STFX_S_DOWNVOL]	= stfx_volDown;
		(*fxt)[STFX_S_UPVOL]	= stfx_volUp;
		(*fxt)[STFX_CUT]	= stfx_cut;
		(*fxt)[STFX_LATESTART]	= stfx_latestart;
		(*fxt)[STFX_EXTRA_TICKS]= stfx_extraTicks;
	    }

	    return error;
	}
	else if (EKSETUP_FIELD(*settings, songType) == EK_SONG_MMD)
	{
	    if ((error= malloc_fxtable(fxt,32)) == EK_ERR_NONE)
	    {
		(*fxt)[MEDFX_ARPEGGIO]	= stfx_arpeggio;
		(*fxt)[MEDFX_DOWN]	= medfx_ptchdown;
		(*fxt)[MEDFX_UP]	= medfx_ptchup;
		(*fxt)[MEDFX_PORTA]	= stfx_portamento;
		(*fxt)[MEDFX_VIBRATO]	= stfx_vibrato;
		(*fxt)[MEDFX_PRTASLIDE]	= stfx_ptaslide;
		(*fxt)[MEDFX_VIBSLIDE]	= stfx_vibslide;
		(*fxt)[MEDFX_SPEED2]	= medfx_speed2;
		(*fxt)[MEDFX_NO_10]	= stfx_undef;	/**/
		(*fxt)[MEDFX_FF]	= stfx_ff;
		(*fxt)[MEDFX_VOLUME]	= medfx_volume;
		(*fxt)[MEDFX_VOLSLIDE]	= stfx_volslide;
		(*fxt)[MEDFX_EXTENDED]	= stfx_undef;	/* not valid */
		(*fxt)[MEDFX_SPEED]	= medfx_speed;
		(*fxt)[MEDFX_NONE]	= stfx_nothing;
		(*fxt)[MEDFX_SMOOTH_DOWN]= stfx_volDown;
		(*fxt)[MEDFX_SMOOTH_UP]	= stfx_volUp;
		(*fxt)[MEDFX_CHG_FTUNE]	= stfx_finetune;
		(*fxt)[MEDFX_LOOP]	= stfx_undef;	/**/
		(*fxt)[MEDFX_OFFSET]	= stfx_offset;
		(*fxt)[MEDFX_S_UPVOL]	= stfx_undef;	/**/
		(*fxt)[MEDFX_S_DOWNVOL]	= stfx_undef;	/**/
		(*fxt)[MEDFX_NOTECUT]	= stfx_cut;
		(*fxt)[MEDFX_LATESTART]	= stfx_undef;
		(*fxt)[MEDFX_DELAY]	= stfx_undef;
	    }

	    return error;
	}
	else
	{
	    return EK_ERR_NO_FXTABLE;
	}
}

/*****
 * OctaMED effects (that are not also generic to ST)
 *****/

void medfx_ptchdown(FXPARAMS)
{
	if (EKSETUP_FIELD(*settings, flags1) & MMDF1_STSLIDE)
	    EKVCHANNEL_FIELD(*vch, fxcall)= stdo_ptchbend;
	else
	    EKVCHANNEL_FIELD(*vch, fxcall)= meddo_ptchbend;
	EKVCHANNEL_FIELD(*vch, ptch_adj)=-
				EKVCHANNEL_FIELD(*vch, fxdat);
}
void medfx_ptchup(FXPARAMS)
{
	if (EKSETUP_FIELD(*settings, flags1) & MMDF1_STSLIDE)
	    EKVCHANNEL_FIELD(*vch, fxcall)= stdo_ptchbend;
	else
	    EKVCHANNEL_FIELD(*vch, fxcall)= meddo_ptchbend;
	EKVCHANNEL_FIELD(*vch, ptch_adj)= EKVCHANNEL_FIELD(*vch, fxdat);
}
void meddo_ptchbend(DOPARAMS)
{
	EKVCHANNEL_FIELD(*vch, ptch_bnd)+=
				EKVCHANNEL_FIELD(*vch, ptch_adj); 
}


void medfx_speed(FXPARAMS)
{
  int fxdat= EKNOTE_FIELD(*note, fxdat);

	if (fxdat == 0)
	    EKVCHANNEL_FIELD(*vch, fxdone)= stdo_skip;
	else if (fxdat<=0xF0)
	    EKSETUP_FIELD(*settings, ptempo)= fxdat;
	else
	    switch(fxdat)
	    {
	    case 0xF1:
		EKVCHANNEL_FIELD(*vch, fxcall)= meddo_double;
		break;
	    case 0xF2:
		EKVCHANNEL_FIELD(*vch, pause)= FL_TRUE;
		EKVCHANNEL_FIELD(*vch, fxcall)= meddo_delayed;
		break;
	    case 0xF3:
		EKVCHANNEL_FIELD(*vch, fxcall)= meddo_triple;
		break;
	    }
}
void meddo_double (DOPARAMS)
{
#ifdef LITERAL	/* not actually how it's done */
	if ( (step%(int)(smax/2)) == 0 )
#else
	if (step%3 == 0)
#endif
	    EKVCHANNEL_FIELD(*vch, srcPos)= 0;
}
void meddo_delayed(DOPARAMS)
{
	if ( step == (int)(smax/2) )
	    EKVCHANNEL_FIELD(*vch, pause)= FL_FALSE;
}
void meddo_triple (DOPARAMS)
{
#ifdef LITERAL	/* not actually how it's done */
	if ( (step%(int)(smax/3)) == 0 )
#else
	if (step%2 == 0)
#endif
	    EKVCHANNEL_FIELD(*vch, srcPos)= 0;
}

void medfx_speed2(FXPARAMS)
{
	EKSETUP_FIELD(*settings, stempo)= EKNOTE_FIELD(*note, fxdat);
}

void medfx_volume(FXPARAMS)
{
	if (EKSETUP_FIELD(*settings, flags1) & MMDF1_VOLHEX)
	    EKVCHANNEL_FIELD(*vch, volume)= EKNOTE_FIELD(*note, fxdat);
	else
	    EKVCHANNEL_FIELD(*vch, volume)= Xbcd(EKNOTE_FIELD(*note, fxdat));
}


/******
 *	SoundTracker/ProTracker effects; generic to most formats,
 *	although there are some (minor) variations
 *****/

void stfx_nothing(FXPARAMS) {}

void stfx_arpeggio(FXPARAMS)
{
	if (EKVCHANNEL_FIELD(*vch, fxdat))
	    EKVCHANNEL_FIELD(*vch, fxcall)= stdo_arpeggio;
}
void stdo_arpeggio(DOPARAMS)
{
	switch(step%3)
	{
	case 0:
	    EKVCHANNEL_FIELD(*vch, transpose)=
				EKVCHANNEL_FIELD(*vch, basetranspose);
	    break;
	case 1:
	    EKVCHANNEL_FIELD(*vch, transpose)=
				EKVCHANNEL_FIELD(*vch, basetranspose)
				+ Hi(EKVCHANNEL_FIELD(*vch, fxdat));
	    break;
	case 2:
	    EKVCHANNEL_FIELD(*vch, transpose)=
				EKVCHANNEL_FIELD(*vch, basetranspose)
				+ Lo(EKVCHANNEL_FIELD(*vch, fxdat));
	    break;
	}
}

void stfx_cut(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_cut;
}
void stdo_cut(DOPARAMS)
{
	if (step == EKVCHANNEL_FIELD(*vch, fxdat))
	    EKVCHANNEL_FIELD(*vch, volume)= 0;
}

void stfx_extraTicks(FXPARAMS)
{
	EKSETUP_FIELD(*settings, ttempo)= EKVCHANNEL_FIELD(*vch, fxdat);
}

void stfx_ff(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxdone)= stdo_ff;
}
int stdo_ff(EkVchannel *vch, EkSetup *settings)
{
	EKSETUP_FIELD(*settings, blkPos)=
			EKSETUP_FIELD(*settings, blkLen);
	EKSETUP_FIELD(*settings, seqPos)= EKVCHANNEL_FIELD(*vch, fxdat);
  return EKVL_C_NEWPOS;
}

void stfx_finetune(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, finetune)= Nibble(EKNOTE_FIELD(*note, fxdat));	/* TODO: unsure */
}

void stfx_loop(FXPARAMS)
{
	if (EKNOTE_FIELD(*note, fxdat) == 0)	/* set loop point */
	{	/* won't do nested loops */
	    EKVCHANNEL_FIELD(*vch, loop_blkNum)= EKSETUP_FIELD(*settings, blkNum);
	    EKVCHANNEL_FIELD(*vch, loop_blkPos)= EKSETUP_FIELD(*settings, blkPos);
	}
	else EKVCHANNEL_FIELD(*vch, fxdone)= stdo_loop;	/* loop end, with count */
}
int stdo_loop(EkVchannel *vch, EkSetup *settings)
{
  int command= EKVL_C_NEWPOS;

	if (EKVCHANNEL_FIELD(*vch, loop_count) == 0)
	{				/* first hit of loop end */
	    EKVCHANNEL_FIELD(*vch, loop_count)=
				EKVCHANNEL_FIELD(*vch, fxdat);
	    EKSETUP_FIELD(*settings, blkNum)=
				EKVCHANNEL_FIELD(*vch, loop_blkNum);
	    EKSETUP_FIELD(*settings, blkPos)=
				EKVCHANNEL_FIELD(*vch, loop_blkPos)-1;
	}
	else	/* returning to end of loop */
	{
	    EKVCHANNEL_FIELD(*vch, loop_count)--;
	    if (EKVCHANNEL_FIELD(*vch, loop_count) == 0)
		command= EKVL_C_NONE;
	    else
	    {	/* loop not finished... */
		EKSETUP_FIELD(*settings, blkNum)=
				EKVCHANNEL_FIELD(*vch, loop_blkNum);
		EKSETUP_FIELD(*settings, blkPos)=
				EKVCHANNEL_FIELD(*vch, loop_blkPos)-1;
	    }
	}
  return command;
}

void stfx_latestart(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, srcPos)= -1;
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_latestart;
}
void stdo_latestart(DOPARAMS)
{
	if (step == EKVCHANNEL_FIELD(*vch, fxdat))
	    EKVCHANNEL_FIELD(*vch, srcPos)= 0;
}

void stfx_offset(FXPARAMS)
{
	if ( (EKVCHANNEL_FIELD(*vch, srcPos)=
		EKVCHANNEL_FIELD(*vch, fxdat)*256)
			> EKVCHANNEL_FIELD(*vch, srcEnd) )
	    EKVCHANNEL_FIELD(*vch, sourceRef)= NULL;
}

void stfx_perDown(FXPARAMS)
{
	if (!EKVCHANNEL_FIELD(*vch, glissando))
	{
	    if (EKVCHANNEL_FIELD(*vch, period) > perTable[0])
		EKVCHANNEL_FIELD(*vch, ptch_bnd)--;
	}
	else if (EKVCHANNEL_FIELD(*vch, pitch) > 0)
	    EKVCHANNEL_FIELD(*vch, pitch)--;
}

void stfx_perUp(FXPARAMS)
{
	if (!EKVCHANNEL_FIELD(*vch, glissando))
	{
	    if (EKVCHANNEL_FIELD(*vch, period)
			< perTable[perTabSz-1])
		EKVCHANNEL_FIELD(*vch, ptch_bnd)++;
	}
	else if (EKVCHANNEL_FIELD(*vch, pitch)
			> perTabSz-1)
		EKVCHANNEL_FIELD(*vch, pitch)++;
}

/* PORTAMENTO
 * is a pitch bend at given rate (or previous, if fxdat is zero)
 * from the previous note, towards the note specified.
 *  NB. Pitch adjust rate must always be +ve, not so for pitch slide
 */
void stfx_portamento(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_portamento;
	if (EKNOTE_FIELD(*note, pitch))
	    EKVCHANNEL_FIELD(*vch, ptch_tgt)= EKNOTE_FIELD(*note, pitch);

	EKVCHANNEL_FIELD(*vch, pitch)=
		EKSETUP_FIELD(*settings, pitch)[chan];
		/* restore old pitch */

	EKVCHANNEL_FIELD(*vch, ptch_bnd)=
		EKSETUP_FIELD(*settings, ptch_bnd)[chan];
		/* and pitch bend */

	if (EKVCHANNEL_FIELD(*vch, fxdat) > 0)
	    EKVCHANNEL_FIELD(*vch, ptch_adj)=
			EKVCHANNEL_FIELD(*vch, fxdat);
	else if (EKVCHANNEL_FIELD(*vch, ptch_adj) < 0)
	    EKVCHANNEL_FIELD(*vch, ptch_adj)=-
			EKVCHANNEL_FIELD(*vch, ptch_adj);
}
void stdo_portamento(DOPARAMS)
{
  int	prd_now= perTable[EKVCHANNEL_FIELD(*vch, pitch)
			+ EKVCHANNEL_FIELD(*vch, transpose)],
	prd_tgt= perTable[EKVCHANNEL_FIELD(*vch, ptch_tgt)
			+ EKVCHANNEL_FIELD(*vch, transpose)];

	if (prd_now < prd_tgt)
	{
	    if (prd_now + EKVCHANNEL_FIELD(*vch, ptch_bnd)
			+ EKVCHANNEL_FIELD(*vch, ptch_adj) > prd_tgt)
		EKVCHANNEL_FIELD(*vch, ptch_bnd)= prd_tgt - prd_now;
	    else
		EKVCHANNEL_FIELD(*vch, ptch_bnd)+=
			EKVCHANNEL_FIELD(*vch, ptch_adj);
	}
	if (prd_now > prd_tgt)
	{
	    if (prd_now+EKVCHANNEL_FIELD(*vch, ptch_bnd)
			- EKVCHANNEL_FIELD(*vch, ptch_adj) < prd_tgt)
		EKVCHANNEL_FIELD(*vch, ptch_bnd)= prd_tgt - prd_now;
	    else
		EKVCHANNEL_FIELD(*vch, ptch_bnd)-=
			EKVCHANNEL_FIELD(*vch, ptch_adj);
	}
}

/* continue portamento [to any pitch], slide volume at given rate */
void stfx_ptaslide(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_ptaslide;
	if (EKVCHANNEL_FIELD(*vch, pitch))
	    EKVCHANNEL_FIELD(*vch, ptch_tgt=
					EKVCHANNEL_FIELD(*vch, pitch);
	EKVCHANNEL_FIELD(*vch, pitch)=
				/* restore old pitch */
				EKSETUP_FIELD(*settings, pitch)[chan]);
	EKVCHANNEL_FIELD(*vch, vol_rate)=
				Nibble(EKNOTE_FIELD(*note, fxdat));
}
void stdo_ptaslide(DOPARAMS)
{
	stdo_portamento(vch, step, smax);
	stdo_volslide(vch, step, smax);
}

void stfx_ptchdown(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_ptchbend;
	EKVCHANNEL_FIELD(*vch, ptch_adj)=- EKVCHANNEL_FIELD(*vch, fxdat);
}
void stfx_ptchup(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_ptchbend;
	EKVCHANNEL_FIELD(*vch, ptch_adj)= EKVCHANNEL_FIELD(*vch, fxdat);
}
void stdo_ptchbend(DOPARAMS)
{
	if (step>0) EKVCHANNEL_FIELD(*vch, ptch_bnd)+=
				EKVCHANNEL_FIELD(*vch, ptch_adj); 
}

void stfx_retrig(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_retrig;
}
void stdo_retrig(DOPARAMS)
{
	if ( (step % EKVCHANNEL_FIELD(*vch, fxdat)) == 0 ) EKVCHANNEL_FIELD(*vch, srcPos)= 0;
}

void stfx_setFilter(FXPARAMS)
{
	switch(EKNOTE_FIELD(*note, fxdat))
	{
	case 0:
	    EKSETUP_FIELD(*settings, filter)= FL_FALSE;
	    warn1("Filter off command ignored");
	    break;
	case 1:
	    EKSETUP_FIELD(*settings, filter)= FL_TRUE;
	    warn1("Filter on command ignored");
	    break;
	}
}

void stfx_setGlissando(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, glissando)= (EKNOTE_FIELD(*note, fxdat) != 0);
}

void stfx_skip(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxdone)= stdo_skip;
}
int stdo_skip(EkVchannel *vch, EkSetup *settings)
{
	EKSETUP_FIELD(*settings, seqPos)++;
	EKSETUP_FIELD(*settings, blkPos)=
			Xbcd(EKVCHANNEL_FIELD(*vch, fxdat))-1;
  return EKVL_C_NEWPOS;
}

void stfx_speed(FXPARAMS)
{
  int fxdat= EKNOTE_FIELD(*note, fxdat);
	if (fxdat<0x20)
	    EKSETUP_FIELD(*settings, stempo)= fxdat;
	else
	    EKSETUP_FIELD(*settings, ptempo)= (fxdat*33)/125; /* percent of normal */
}

void stfx_vibrato(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_vibrato;
	if (EKNOTE_FIELD(*note, fxdat))
	{
	    EKVCHANNEL_FIELD(*vch, vib_rate)= Hi(EKNOTE_FIELD(*note, fxdat));
	    EKVCHANNEL_FIELD(*vch, vib_depth)= Lo(EKNOTE_FIELD(*note, fxdat));
	}
}
/* Literal protracker translation. Vibrato table could be better
 * scaled, though... limits on sample playback rate different!
 */
void stdo_vibrato(DOPARAMS)
{
	EKVCHANNEL_FIELD(*vch, vib_offs) += EKVCHANNEL_FIELD(*vch, vib_rate);
	EKVCHANNEL_FIELD(*vch, vib_offs) %= 64;
/* shouldn't use logical shift on signed values => /64 */
	EKVCHANNEL_FIELD(*vch, vib_bend)=
		(vibtable[EKVCHANNEL_FIELD(*vch, vib_offs) >> 1]
		* EKVCHANNEL_FIELD(*vch, vib_depth)) /64;
}

/* continue vibrato, slide volume at given rate */
void stfx_vibslide(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_vibslide;
	EKVCHANNEL_FIELD(*vch, vol_rate)=
				Nibble(EKNOTE_FIELD(*note, fxdat));
}
void stdo_vibslide(DOPARAMS)
{
	stdo_portamento(vch, step, smax);
	stdo_volslide(vch, step, smax);
}

void stfx_volslide(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, fxcall)= stdo_volslide;
	EKVCHANNEL_FIELD(*vch, vol_rate)=
				Nibble(EKNOTE_FIELD(*note, fxdat));
}
void stdo_volslide(DOPARAMS)
{
	if (step != 0)
	{
	    if (EKVCHANNEL_FIELD(*vch, vol_rate) < 0)
	    {
		if ( (EKVCHANNEL_FIELD(*vch, volume)+=
			EKVCHANNEL_FIELD(*vch, vol_rate)) < 0 )
		{
		    EKVCHANNEL_FIELD(*vch, volume)= 0;
		}
	    }
	    else
	    {
		if ( (EKVCHANNEL_FIELD(*vch, volume)+=
			EKVCHANNEL_FIELD(*vch, vol_rate)) > 64 )
		{
		    EKVCHANNEL_FIELD(*vch, volume)= 64;
		}
	    }
	}
}

void stfx_volDown(FXPARAMS)
{
	if ( (EKVCHANNEL_FIELD(*vch, volume)-=
			EKNOTE_FIELD(*note, fxdat)) < 0)
	{
	    EKVCHANNEL_FIELD(*vch, volume)= 0;
	}
}
void stfx_volUp(FXPARAMS)
{
	if ( (EKVCHANNEL_FIELD(*vch, volume)+=
			EKNOTE_FIELD(*note, fxdat)) > 64 )
	{
	    EKVCHANNEL_FIELD(*vch, volume)= 64;
	}
}

void stfx_volume(FXPARAMS)
{
	EKVCHANNEL_FIELD(*vch, volume)= EKNOTE_FIELD(*note, fxdat);	/* Xbcd(EKNOTE_FIELD(*note, fxdat)) ? */
}

void stfx_undef(FXPARAMS)
{
#ifdef NOTIFY
  char	err[80];
	sprintf(err,"Unsupported command on channel %d\n", chan);
	error1(err);
#endif
}
void stfx_unexp(FXPARAMS)
{
#ifdef NOTIFY
  char	err[80];
	sprintf(err, "Unexpected command %02X on channel %d\n",
		EKNOTE_FIELD(*note, fx, chan);
	error1(err);
#endif
}
