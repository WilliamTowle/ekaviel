/* Ekaviel v1.52, Wm.Towle 16/2/1996 */

#include </usr/include/audio.h>
#ifdef IRIX4
extern	int sginap(long ticks);
#else
#include <unistd.h>	/* proper place for sginap */
#endif

#include "ekaudio.h"
#include "ekmalloc.h"

#define DEF_OUTRATE 44100
#define MSAMP signed short

Flag	audioInit= FL_FALSE;
int	outrate= DEF_OUTRATE;

static int	bleed1= 255,
		bleed2= 0;

static ALconfig	audioCfg;

static Flag	noWait;
static MSAMP	*outSamp= NULL;
static int	buffPos;
static int	chans;
static ALport	alport;
static long	alFreq[2]= {AL_OUTPUT_RATE, DEF_OUTRATE};
#if 0	/* Allows us to test for external set volume to zero */
static long	alGain[]= {AL_LEFT_SPEAKER_GAIN,1,
				AL_RIGHT_SPEAKER_GAIN,1};
#endif

EkErrFlag audio_init(int hertz, Flag stereo, U_CHR bleed)
{	/* FUTURE.
		Linux differentiates between being unable to
		open the audio and failed audio configuration
		(EK_ERR_BAD_AUDCFG)
		*/
/* set frequency of outbound samples */
	if (hertz != 0)
	{
	    alFreq[1]= hertz;
	    ALsetparams(AL_DEFAULT_DEVICE, alFreq, 2);
	}
	/* What outrate did we actually get? */
	ALgetparams(AL_DEFAULT_DEVICE, alFreq, 2);
	outrate= (int) alFreq[1];

	audioCfg= ALnewconfig();
	noWait= FL_FALSE;

/* set stereo or mono output */
	if (stereo)
	{
	    ALsetchannels(audioCfg, AL_STEREO); chans=2;
	}
	else
	{
	    ALsetchannels(audioCfg, AL_MONO); chans=1;
	}
	ALsetwidth(audioCfg, AL_SAMPLE_16);
	alport= ALopenport("ekaviel", "w", audioCfg);
	if (alport == NULL)
	    return EK_ERR_NO_AUDIO;
	else
	{
	    audioInit= FL_TRUE;

/* set bleed (channel "overlap").....
 * Level 0:	normal stero, expected channels used
 * Level 128:	mono
 * Level 255:	normal stereo, left and right channels switched
 ***/
	    bleed1= 255-bleed;
	    bleed2= bleed;
	    return EK_ERR_NONE;
	}
}

Flag audioBusy(int remain)
{
	if (audioInit)
	    return ( ALgetfilled(alport)>remain );
	else
	    return FL_FALSE;
}

void getOutrate()
{
	ALgetparams(AL_DEFAULT_DEVICE, alFreq, 2);
	outrate= (int) alFreq[1];
}

EkErrFlag audio_makeBuffers(int size)
{
	if ( (outSamp= (MSAMP *)
		REALLOCATE_MEMORY(outSamp, chans*size, MSAMP,
					"ekaudio::audio_makeBuffers()"))
		== NULL )
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	    buffPos= 0;
	    return EK_ERR_NONE;
	}
}

void audio_appendBuffers(int vchans, long left, long rght)
{
/* Incoming long values are (vchans * [<signed char> * 64*64]) */
	if (chans != 2)
	    outSamp[buffPos++]= (MSAMP) ((left+rght)/(16*vchans));
	else
	{
	    outSamp[buffPos++]= (MSAMP)
			((left*bleed1 + rght*bleed2)/(2048*vchans));
	    outSamp[buffPos++]= (MSAMP)
			((rght*bleed1 + left*bleed2)/(2048*vchans));
	}
}

/* For when audio is closed immediately afterward */
void audio_discardBuffers()
{
	noWait= FL_TRUE;
}

void audio_flushBuffers(Flag synchro)
{
	ALwritesamps(alport, outSamp, buffPos);
	if (synchro)
	    while (audioBusy(buffPos)) sginap(1);
}

void audio_close()
{
	if (audioInit)
	{
	    if (!noWait)
		while (audioBusy(0)) sginap(1);

	    ALcloseport(alport);
	    ALfreeconfig(audioCfg);
	    audioInit= FL_FALSE;
	}

	if (outSamp)
	{
	    FREE_MEMORY(outSamp, "ekaudio::audio_close()");
	    outSamp= NULL;
	}
	noWait= FL_FALSE;	/* in case we open audio again */
}
