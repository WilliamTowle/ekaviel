/* Ekaviel v1.52, Wm.Towle 16/2/1996 */

#include <stdio.h>
#include "vchannel.h"

#include "effects.h"
#include "ekaudio.h"
#include "ekio.h"
#include "ekmalloc.h"

#ifdef DEVEL
#define DEVEL_R54A
#endif

#ifdef DEVEL_R54A
#define	FREQCH (227001280/outrate)
#endif

static EkVchannel	*vchan= NULL;
static int	vchuse= 0,
		vchans= 0,	/* useful to editor */
		buffSize;
static Flag	synchro;

void    (**fxtable) (FXPARAMS)= NULL;	/* array of fn-pointers */


const char *scale[12]= {
		"C-", "C#", "D-", "D#", "E-", "F-",
		"F#", "G-", "G#", "A-", "A#", "B-"
		};

/*****
 *	EkVchannel class
 *****/

EkErrFlag ekvchannel_new(EkVchannel **chanPtr, int reqd, int *made)
{
	if (chanPtr == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	  EkVchannel	*mem= (EkVchannel *)
				ALLOCATE_MEMORY(reqd, EkVchannel,
					"vchannel.c::ekvchannel_new()");

	    if ( (*chanPtr= mem) == NULL)
		return EK_ERR_MALLOC_FAILURE;
	    else
	    {
	      EkErrFlag error= EK_ERR_NONE;

		for (*made= 0; (*made < reqd) && (error == EK_ERR_NONE);
			(*made)++)
		{
		    error= ekvchannel_construct(mem);
		    if (error == EK_ERR_NONE) mem++;
		}

		return error;
	    }
	}
}

EkErrFlag ekvchannel_construct(EkVchannel *chan)
{
	if (chan == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	    EKVCHANNEL_FIELD(*chan, sourceRef)= NULL;
	    EKVCHANNEL_FIELD(*chan, fxcall)= NULL;
	    EKVCHANNEL_FIELD(*chan, fxdone)= NULL;
	    return EK_ERR_NONE;
	}
}

EkErrFlag ekvchannel_destroy(EkVchannel *chan)
{
	if (chan == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else	/* no memory allocation takes place in internals */
	    return EK_ERR_NONE;
}

EkErrFlag ekvchannel_delete(EkVchannel **chanPtr, int made)
{
	if (chanPtr == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	  EkErrFlag	error= EK_ERR_NONE;
	  EkVchannel	*chan= chanPtr[0];

	    for (; (made > 0) && (error == EK_ERR_NONE); made--)
	    {
		error= ekvchannel_destroy(chan);
		chan++;
	    }

	    return error;
	}
}

/*****
 *	Virtual channel manipulation
 *****/

int getnote_scale(const char *arg)
{
  char	notename[]= "---";	/* default */
  Flag	botch= FL_FALSE;
  int	answer= -1;

	if (strlen(arg) == 2)
	{
	    notename[0]= toupper(arg[0]);
	    if (!isxdigit(arg[1]))
		botch= FL_TRUE;
	    else
		notename[2]= arg[1];
	}
	else if (strlen(arg) == 3)
	{
	    notename[0]= toupper(arg[0]);
	    if (arg[1] == '#')
		notename[1]= '#';
	    else if (arg[1] != '-')
		botch= FL_TRUE;
	    if (!isxdigit(arg[2]))
		botch= FL_TRUE;
	    else
		notename[2]= arg[1];
	}
	else
	    botch= FL_TRUE;

	if (!botch)
	{
	  int	i;

	    for (i= 0; i < perTabSz; i++)
	    {
		if (strcmp(notename, ptchName[i]) == 0)
		    answer= i-1;
	    }
	}

	return answer;
}

PERIODFN_HEADER(period_l)
{
#ifdef DEVEL_R54A
  return ( ( FREQCH * perTable[0] ) << octaves )
	/ ( (perTable[pitch] + pbend + vbend) * c_ftune(tr,ft) );
#else
  return ( FREQCH * perTable[0] ) /
	( (perTable[pitch] + pbend + vbend) * c_ftune(tr,ft) );
#endif
}

PERIODFN_HEADER(period_u)
{
#ifdef DEVEL_R54A
  return ( ( FREQCH * perTable[0] ) << octaves)
	/ ( (new_periodit(pitch) + pbend + vbend) * c_ftune(tr,ft));
#else
  return ( FREQCH * perTable[0] ) /
	( (new_periodit(pitch) + pbend + vbend) * c_ftune(tr,ft));
#endif	/* DEVEL_R54A */
}

/*****
 *	Original vchannel stuff starts here
 *****/

Flag set_sync(Flag sync)	{ return synchro= sync; }
Flag toggle_sync()		{ return synchro= !synchro; }

EkErrFlag init_vchans(int chs, EkSetup *settings)
{
  int	chans;
  EkErrFlag error= EK_ERR_NONE;

	if ( (chs%4) == 0 ) chans= chs;
	else chans= chs + (4-chs%4);	/* multiple of 4 */

	if ( (error= init_notes()) != EK_ERR_NONE)
	    return error;

#ifdef DEVEL
fprintf(stderr, "vchannel.c: %s\n", EKSETUP_FIELD(*settings, stereo)? "stereo" : "mono");
#endif
	if ( (error= audio_init(
			EKSETUP_FIELD(*settings, outrate),
			EKSETUP_FIELD(*settings, stereo),
			EKSETUP_FIELD(*settings, bleed)
			)) != EK_ERR_NONE)
	{
	    return error;
	}

	if ( (error= ekvchannel_new(&vchan, chans, &vchans))
		!= EK_ERR_NONE )
	    return error;
	else
	{
	    vchuse= 0;	/* none used yet */
	    if (EKSETUP_FIELD(*settings, pitch) == NULL)
	    {
	      int	i;

		if ((EKSETUP_FIELD(*settings, pitch)=
			(int *)ALLOCATE_MEMORY(chans, int,
				"vchannel.c::init_vchans(pitch)"))
					== NULL)
		{
		    return EK_ERR_MALLOC_FAILURE;
		}
		else if ((EKSETUP_FIELD(*settings, ptch_bnd)=
			(int *)ALLOCATE_MEMORY(chans, int,
				"vchannel.c::init_vchans(ptch_bnd)"))
					== NULL)
		{
		    return EK_ERR_MALLOC_FAILURE;
		}
		else if ((EKSETUP_FIELD(*settings, trkvol)=
			(int *)ALLOCATE_MEMORY(chans, int,
				"vchannel.c::init_vchans(trkvol)"))
					== NULL)
		{
		    return EK_ERR_MALLOC_FAILURE;
		}

		for (i= 0; i<chans; i++)
		    EKSETUP_FIELD(*settings, trkvol)[i]= 64;
	    }

	    return init_fxtable(settings, &fxtable);
	}
}

int reset_vchans(EkSetup *settings)
{
  int	nmbr= EKSETUP_FIELD(*settings, tracks),
	i;

	if (nmbr > vchuse)
	{
	    for (i=vchuse; i<nmbr; i++)
		EKVCHANNEL_FIELD(vchan[i], sourceRef)= NULL;
	}

	/* player mechanism will invalidate vchannels with no source */
	for (i=vchuse; i<nmbr; i++)
	{	/* initialise previously unused channels... */
	    vchan[i].mvol= (EKSETUP_FIELD(*settings, trkvol)[i]
				* EKSETUP_FIELD(*settings, volume))>>6;
	    vchan[i].loop_count= 0;	/* stop unwanted looping */
	}
	vchuse= nmbr;

	getOutrate();	/* may have been externally changed */
	buffSize= outrate*33/50;
  return outrate;
}

void set_vchan(int chan, EkNote *note, EkInstr samp[], EkSetup *settings)
{
  EkVchannel	*vch= &vchan[chan];
  int		inum= EKNOTE_FIELD(*note, instNum),
		pitch= EKNOTE_FIELD(*note, pitch);

	EKVCHANNEL_FIELD(*vch, fxcall)= NULL;
	EKVCHANNEL_FIELD(*vch, fxdone)= NULL;
	EKVCHANNEL_FIELD(*vch, fxdat)= EKNOTE_FIELD(*note, fxdat);
	if (inum && pitch)
	{
	    inum--;
	    EKVCHANNEL_FIELD(*vch, volume)=
				EKINSTR_FIELD(samp[inum], volume);
	    if (pitch)
		EKVCHANNEL_FIELD(*vch, pitch)= pitch;

	    EKVCHANNEL_FIELD(*vch, transpose)=
		EKVCHANNEL_FIELD(*vch, basetranspose)=
				EKINSTR_FIELD(samp[inum], transpose)
				+ EKSETUP_FIELD(*settings, transpose)-1;
			    /* we use 0 as no note, so subtract 1 */

	    EKVCHANNEL_FIELD(*vch, finetune)=
				EKINSTR_FIELD(samp[inum], finetune);
				/* TODO: finetune ignored */

	    EKVCHANNEL_FIELD(*vch, sourceRef)= &samp[inum];
	    EKVCHANNEL_FIELD(*vch, srcPos)= 0;
	    EKVCHANNEL_FIELD(*vch, counter)= 0;
	    EKVCHANNEL_FIELD(*vch, srcRew)=
			EKVCHANNEL_FIELD(*vch, sourceRef)->rpt_len;
	    if (EKVCHANNEL_FIELD(*vch, srcRew))	/* repeating sample? */
	    {
		EKVCHANNEL_FIELD(*vch, srcEnd)=
			EKINSTR_FIELD(
				*EKVCHANNEL_FIELD(*vch, sourceRef),
				rpt_bgn)
			+ EKINSTR_FIELD(
				*EKVCHANNEL_FIELD(*vch, sourceRef),
				rpt_len);
	    }
	    else
	    {
		EKVCHANNEL_FIELD(*vch, srcEnd)=
			EKINSTR_FIELD(*EKVCHANNEL_FIELD(*vch, sourceRef),
				length);
	    }

	    EKVCHANNEL_FIELD(*vch, pause)= FL_FALSE;
	    EKVCHANNEL_FIELD(*vch, ptch_adj)=
			EKVCHANNEL_FIELD(*vch, ptch_bnd)= 0;
	    EKVCHANNEL_FIELD(*vch, vib_bend)= 0;

	    EKVCHANNEL_FIELD(*vch, fxcall)= NULL;
	    fxtable[EKNOTE_FIELD(*note, fx)](FXARGS);

	    EKSETUP_FIELD(*settings, pitch)[chan]=
			EKVCHANNEL_FIELD(*vch, pitch);
	    EKSETUP_FIELD(*settings, ptch_bnd)[chan]= 0;
#ifdef SCREAMTRACKER
	    EKVCHANNEL_FIELD(*vch, period)= 14317056 * 4 /* for ST */
		* c_ftune(EKVCHANNEL_FIELD(*vch, transpose), 0)
		/ ( 8363 * 16 * (periodit[EKVCHANNEL_FIELD(*vch, pitch)
				+ EKVCHANNEL_FIELD(*vch, transpose)]
				) );
#else	/* for now... */
	    EKVCHANNEL_FIELD(*vch, period)=
		EKSETUP_FIELD(*settings, periodfn)
			(EKVCHANNEL_FIELD(*vch, pitch), 0,0,
				EKVCHANNEL_FIELD(*vch, transpose),
				EKVCHANNEL_FIELD(*vch, finetune)
			);
#endif
	}
	else	/* continue sample */
	    fxtable[EKNOTE_FIELD(*note, fx)](FXARGS);
}

static
S_CHR lerp(EkVchannel *vch)
{
  int		thsIdx= EKVCHANNEL_FIELD(*vch, srcPos),
		nxtIdx= thsIdx + 1;
  EkInstr	*src= EKVCHANNEL_FIELD(*vch, sourceRef);
  S_CHR		temp1, temp2, lerp= 0;

	if ( nxtIdx >= 0 )
	{	/* if negative, we are delaying sample start */
	    if (nxtIdx > EKVCHANNEL_FIELD(*vch, srcEnd))
	    {
		if (EKVCHANNEL_FIELD(*vch, srcRew))
		    nxtIdx-=EKVCHANNEL_FIELD(*vch, srcRew);
		else
		    nxtIdx=EKVCHANNEL_FIELD(*vch, srcEnd);
	    }

	    temp1= ((S_CHR *)EKINSTR_FIELD(*src, waveform))[thsIdx];
	    temp2= ((S_CHR *)EKINSTR_FIELD(*src, waveform))[nxtIdx];

	    lerp= temp1 + ( ((temp2 - temp1)
			* EKVCHANNEL_FIELD(*vch, counter)) >> DIVIDE );
	}
  return lerp;
}

/*	based on the code in Modplay, which used sample transpose code
 *	by Ross Clements */
EkErrFlag flush_vchans(EkSetup *settings, int *commandInt)
{
  int		command= EKVL_C_NONE,
		i, scount, sent= 0,
		tcount= EKSETUP_FIELD(*settings, stempo)
			+ EKSETUP_FIELD(*settings, ttempo),
		send= buffSize/(EKSETUP_FIELD(*settings, ptempo));
  S_CHR		temp;
  long 		left, rght;
  EkVchannel	*vch;
  Flag		exit= FL_FALSE;
  EkErrFlag	error= audio_makeBuffers(send*tcount);

	if (error != EK_ERR_NONE) return error;

/* else ... */


	for (scount=0; (scount < tcount) && (!exit); scount++)
	{
	    for (i=0; i<vchuse ; i++)
		if ( EKVCHANNEL_FIELD(*(vch= vchan+i), fxcall) )
		{
		    EKVCHANNEL_FIELD(*vch, fxcall)(vch,scount,tcount);
		    EKVCHANNEL_FIELD(*vch, period)=
			EKSETUP_FIELD(*settings, periodfn)
				(EKVCHANNEL_FIELD(*vch, pitch),
				EKVCHANNEL_FIELD(*vch, ptch_bnd),
				EKVCHANNEL_FIELD(*vch, vib_bend),
				EKVCHANNEL_FIELD(*vch, transpose),
				EKVCHANNEL_FIELD(*vch, finetune));
		    EKSETUP_FIELD(*settings, ptch_bnd)[i]=
				EKVCHANNEL_FIELD(*vch, ptch_bnd);
		}

	    for (sent=0; sent<send; sent++)
	    {
	      Flag valid;

		left= 0; rght= 0;
		for (i=0; i<vchuse; i++)
		{
		    vch= &vchan[i];
		    valid= FL_FALSE;
		    if (EKVCHANNEL_FIELD(*vch, sourceRef))
			/* sample to play? */
		    {
			if ( (EKVCHANNEL_FIELD(*vch,
					    sourceRef)->length > 0)
			    && (EKVCHANNEL_FIELD(*vch,
					    srcPos) >= 0)
					    /* (ie. not paused) */
					    )
			    valid= FL_TRUE;
		    }

		    if (valid)
		    {
			temp= lerp(vch);
			EKVCHANNEL_FIELD(*vch, counter)+=
			    EKVCHANNEL_FIELD(*vch, period);
			EKVCHANNEL_FIELD(*vch, srcPos)+=
			    EKVCHANNEL_FIELD(*vch, counter)
						    >> DIVIDE;
			EKVCHANNEL_FIELD(*vch, counter) &= MODULUS;

			if (EKVCHANNEL_FIELD(*vch, srcPos)
			    > EKVCHANNEL_FIELD(*vch, srcEnd))
			{
			    if (EKVCHANNEL_FIELD(*vch, srcRew))
				EKVCHANNEL_FIELD(*vch, srcPos)-=
					EKVCHANNEL_FIELD(*vch, srcRew);
			    else /* sample has run out */
				EKVCHANNEL_FIELD(*vch,
					    sourceRef)= NULL;
			}
		    }
		    else temp= 0;

		    if ( (i%4)%3 == 0)
			left += temp * EKVCHANNEL_FIELD(*vch, mvol)
			    * EKVCHANNEL_FIELD(*vch, volume);
		    else rght += temp * EKVCHANNEL_FIELD(*vch, mvol)
			    * EKVCHANNEL_FIELD(*vch, volume);
		}

		audio_appendBuffers(vchuse, left,rght);
		if (EKSETUP_FIELD(*settings, interact) == 2)
		{
		  int icommand;
		    /* special case for interactivity 2 */
		    if ( (icommand= io_keycmd()) != EKVL_C_NONE )
				command= icommand;
		    exit|= ((command == EKVL_C_QUIT)
					|| (command == EKVL_C_NEXT));
		}
	    }
	}

	if (!exit)
	{
	    for (i=0; i<vchuse; i++)
	    {
		if (EKVCHANNEL_FIELD(vchan[i], fxdone))
			command= EKVCHANNEL_FIELD(vchan[i], fxdone)
					    (&vchan[i], settings);
	    }

	    if (EKSETUP_FIELD(*settings, interact) == 1)
	    {
		if (command == EKVL_C_NONE)
			command= io_keycmd();
		exit= (command == EKVL_C_QUIT)
			|| (command == EKVL_C_NEXT);
	    }

	    audio_flushBuffers(synchro);
	}

	if (commandInt) *commandInt= command;

  return EK_ERR_NONE;
}

Flag vchan_finished()
{
  Flag	done= FL_TRUE;
  int	i;

	for (i= 0; (i<vchuse) && (done == FL_TRUE); i++)
	{
	    if (EKVCHANNEL_FIELD(vchan[i], sourceRef) != NULL)
		done= FL_FALSE;
	}

  return done;
}

void close_vchans()
{
#ifdef DEVEL
;fprintf(stderr, "close_vchans()\n");
#endif
	if (vchan)
	{
	    FREE_MEMORY(vchan, "vchannel.c::close_vchans(vchan)");
	    vchan= NULL;
	}
	if (fxtable)
	{
	    FREE_MEMORY(fxtable, "vchannel.c::close_vchans(fxtable)");
	    fxtable= NULL;
	}

	zap_notes();
	if (audioInit) audio_close();
#ifdef DEVEL
;fprintf(stderr, "close()d\n");
#endif
}
