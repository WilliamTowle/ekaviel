/* Ekaviel v1.54, Wm.Towle 25/06/1999 */

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include "ekaudio.h"
#include "ekmalloc.h"

#ifndef DEFAULT_OUTRATE
#define DEFAULT_OUTRATE 44100
#endif
#define MSAMP signed short

Flag	audioInit= FL_FALSE;
int	outrate= DEFAULT_OUTRATE;

static Flag	noWait;
static int	chans;
static int	bleed1= 255,
		bleed2= 0;
static MSAMP	*outSamp= NULL;
static int	buffPos;

static int	audio= -1;		/* /dev/dsp */
static int	dspSampSz= 16;		/* eight or sixteen bits */

EkErrFlag audio_init(int hertz, Flag stereo, U_CHR bleed)
{
/* Linux requires we open /dev/dsp first... */
	if ( (audio= open("/dev/dsp", O_WRONLY, 0)) == -1 )
	    return EK_ERR_NO_AUDIO;

/* Characteristics/quality of output... */
	if ( ioctl(audio, SNDCTL_DSP_SAMPLESIZE, &dspSampSz) == -1 )
	    return EK_ERR_NO_AUDIO;
	if ( ioctl(audio, SNDCTL_DSP_STEREO, &stereo) == -1 )
	    return EK_ERR_NO_AUDIO;
	if (hertz == 0)
	    hertz= DEFAULT_OUTRATE;
	if ( ioctl(audio, SNDCTL_DSP_SPEED, &hertz) == -1 )
	    return EK_ERR_NO_AUDIO;

/* Buffering characteristics... */
	audioInit= FL_TRUE;
	outrate= hertz;
	noWait= FL_FALSE;
	chans= stereo? 2 : 1;
/* set bleed (channel "overlap").....
 * Level 0:	normal stero, expected channels used
 * Level 128:	mono
 * Level 255:	normal stereo, left and right channels switched
 ***/
	bleed1= 255-bleed;
	bleed2= bleed;

  return EK_ERR_NONE;
}

#if 0
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
#endif

EkErrFlag audio_makeBuffers(int size)
{
	if ( (outSamp= (MSAMP *)
		REALLOCATE_MEMORY(outSamp, chans*size, MSAMP))
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

#if 0	/* sginap() and/or audioBusy() */
void audio_flushBuffers(Flag synchro)
{
	write(audio, outSamp, buffPos);
	if (synchro)
	    while (audioBusy(buffPos)) sginap(1);
}
#endif

#if 0	/* audioBusy() again... */
void audio_close()
{
	if (audioInit)
	{
	    if (!noWait)
		while (audioBusy(0)) sginap(1);

	    
	}

	if (outSamp)
	{
	    FREE_MEMORY(outSamp);
	    outSamp= NULL;
	}
	noWait= FL_FALSE;	/* in case we open audio again */
}
#endif
