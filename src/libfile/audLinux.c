/* Ekaviel v1.54, Wm.Towle 25/06/1999 */

#include <stdio.h>

#include <fcntl.h>
#if 0
#include <sys/ioctl.h>	/* linux/ioctl.h */
#include <sys/soundcard.h>
#endif
#include <unistd.h>	/* write() to a file descriptor */
#include "ekaudio.h"
#include "ekmalloc.h"

#ifndef DEFAULT_OUTRATE
#define DEFAULT_OUTRATE 44100
#endif
#define MSAMP signed short

Flag	audioInit= FL_FALSE;
int	outrate= DEFAULT_OUTRATE;

#if 1	/* HAVE_FILEIO */
FILE		*output;
#else
static Flag	noWait;
static int	audio= -1;		/* /dev/dsp */
static int	dspSampSz= 16;		/* eight or sixteen bits */
#endif

static int	chans;
static int	bleed1= 255,
		bleed2= 0;
static MSAMP	*outSamp= NULL;
static int	buffPos;


EkErrFlag audio_init(int hertz, Flag stereo, U_CHR bleed)
{
#if 1	/* HAVE_FILEIO */
	if ( (output= fopen("output", "w")) == NULL )
	    return EK_ERR_NO_AUDIO;
#else
/* Linux requires we open /dev/dsp first... */
	if ( (audio= open("/dev/dsp", O_WRONLY, 0)) == -1 )
	    return EK_ERR_NO_AUDIO;


/* Characteristics/quality of output... */
	if ( ioctl(audio, SNDCTL_DSP_SAMPLESIZE, &dspSampSz) == -1 )
	    return EK_ERR_BAD_AUDCFG;
/* HMMM. `int format= AFMT_S16_LE` (16 bit little-endian)
	and `ioctl SNDCTL_DSP_SETFMT`
	?
	*/
	if ( ioctl(audio, SNDCTL_DSP_STEREO, &stereo) == -1 )
		/* FUTURE:
			&stereo - we might be refused mono.
			In which case... what?
			 */
	    return EK_ERR_BAD_AUDCFG;
	if (hertz == 0)
	    hertz= DEFAULT_OUTRATE;
	if ( ioctl(audio, SNDCTL_DSP_SPEED, &hertz) == -1 )
		/* FUTURE:
			Normal rates 11025, 22050, 32000, 41000 (Hz)
			48000 not widely supported (eg. SB!!!)
			*/
	    return EK_ERR_BAD_AUDCFG;

	noWait= FL_FALSE;
#endif

/* Buffering characteristics... */
/* FUTURE: Espie does 'SNDCTL_DSP_GETBLKSIZE' to make buffers */
	audioInit= FL_TRUE;
	outrate= hertz;
	chans= stereo? 2 : 1;
/* set bleed (channel "overlap").....
 * Level 0:	normal stereo, left and right as expected
 * Level 128:	mono (effectively)
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
#endif

void getOutrate()
{	/* Irix lets this change. Linux doesn't appear to. */	}

EkErrFlag audio_makeBuffers(int size)
{
	if ( (outSamp= (MSAMP *)
		REALLOCATE_MEMORY(outSamp, chans*size, MSAMP,
				"ekaudio.c::audio_makeBuffers()"))
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
/* Linux: 8-bit is unsigned, 16-bit signed? */
	if (chans != 2)
	    /* mono: left + right here */
	    outSamp[buffPos++]= (MSAMP) ((left+rght)/(16*vchans));
	else
	{   /* stereo: left first, right second */
	    outSamp[buffPos++]= (MSAMP)
			((left*bleed1 + rght*bleed2)/(2048*vchans));
	    outSamp[buffPos++]= (MSAMP)
			((rght*bleed1 + left*bleed2)/(2048*vchans));
	}
}

/* For when audio is closed immediately afterward */
void audio_discardBuffers()
{
#if 0
	noWait= FL_TRUE;
#endif
}

void audio_flushBuffers(Flag synchro)
{	/* "audio" is the result from open() /dev/dsp */
#if 1	/* HAVE_FILEIO */
	fwrite(outSamp, sizeof(MSAMP), buffPos, output);
#else
	write(audio, outSamp, buffPos);
#endif
#if 1
#ifdef DEVEL
	if (synchro)
	    fprintf(stderr, "FUTURE: FIXME. 'synchro' not supported\n");
#endif
#else
	if (synchro)
	    while (audioBusy(buffPos)) sginap(1);
#endif
}

void audio_close()
{
	if (audioInit)
	{
#if 1
#ifdef DEVEL
	    if (!noWait)
		fprintf(stderr, "FUTURE: FIXME: 'wait' not supported\n");
#endif
#else
	    if (!noWait)
		while (audioBusy(0)) sginap(1);
#endif
	}

	if (outSamp)
	{
	    FREE_MEMORY(outSamp, "ekaudio.c::audio_close()");
	    outSamp= NULL;
	}
#if 0
	noWait= FL_FALSE;	/* in case we open audio again */
#endif
}
