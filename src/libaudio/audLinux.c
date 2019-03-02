/* Ekaviel v1.54, Wm.Towle 25/06/1999 */

#ifdef DEVEL
#include <stdio.h>
#endif

#include <fcntl.h>
#include <alsa/asoundlib.h>
#include "ekaudio.h"
#include "ekmalloc.h"

#ifndef DEFAULT_OUTRATE
#define DEFAULT_OUTRATE 44100
#endif
#define MSAMP signed short

Flag	audioInit= FL_FALSE;
int	outrate= DEFAULT_OUTRATE;

#if 0	/* Obsolete: OS_Irix */
static Flag	noWait;
#endif
static int	chans;
static int	bleed1= 255,
		bleed2= 0;
static MSAMP	*outSamp= NULL;
static int	buffPos;

#if 0	/* Obsolete: Linux, OSS style */
static int	audio= -1;		/* /dev/dsp */
static int	dspSampSz= 16;		/* eight or sixteen bits */
#else	/* ALSA */
snd_pcm_t	*pcm_handle= NULL;
snd_pcm_hw_params_t	*hw_params;
#endif


EkErrFlag audio_init(int hertz, Flag stereo, U_CHR bleed)
{
#if 0	/* Obsolete: Linux, OSS style */
#ifdef DEVEL
fprintf(stderr, "opening device...\n");
#endif
	if ( (audio= open("/dev/dsp", O_WRONLY, 0)) == -1 )
	    return EK_ERR_NO_AUDIO;

#ifdef DEVEL
fprintf(stderr, "sound quality\n");
#endif
/* Characteristics/quality of output... */
	if ( ioctl(audio, SNDCTL_DSP_SAMPLESIZE, &dspSampSz) == -1 )
	    return EK_ERR_BAD_AUDCFG;
/* HMMM. `int format= AFMT_S16_LE` (16 bit little-endian)
	and `ioctl SNDCTL_DSP_SETFMT`
	?
	*/
#ifdef DEVEL
fprintf(stderr, "%s\n", stereo? "stereo(!!)" : "mono");
#endif
	if ( ioctl(audio, SNDCTL_DSP_STEREO, &stereo) == -1 )
#ifdef DEVEL
	fprintf(stderr, "configured: %s\n", stereo? "stereo" : "mono");
#else
		/* FUTURE:
			&stereo - we might be refused mono.
			In which case... what?
			 */
	    return EK_ERR_BAD_AUDCFG;
#endif
#ifdef DEVEL
fprintf(stderr, "hertz\n");
#endif
	if (hertz == 0)
	    hertz= DEFAULT_OUTRATE;
	if ( ioctl(audio, SNDCTL_DSP_SPEED, &hertz) == -1 )
		/* FUTURE:
			Normal rates 11025, 22050, 32000, 41000 (Hz)
			48000 not widely supported (eg. SB!!!)
			*/
	    return EK_ERR_BAD_AUDCFG;
#else
#ifdef DEVEL
fprintf(stderr, "opening PCM_DEVICE 'default'...\n");
#endif
	snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	snd_pcm_hw_params_alloca(&hw_params);
	snd_pcm_hw_params_any(pcm_handle, hw_params);

#ifdef DEVEL
fprintf(stderr, "sound quality\n");
#endif

	/* Set parameters : interleaved channels, 16 bits little endian, 44100Hz, 2 channels */
	snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &outrate, 0);
	snd_pcm_hw_params_set_channels(pcm_handle, hw_params, chans= stereo? 2 : 1);

	/* Assign them to the playback handle and free the parameters structure */
	snd_pcm_hw_params(pcm_handle, hw_params);
	//snd_pcm_hw_params_free(hw_params);

	/* Prepare & Play */
	snd_pcm_prepare(pcm_handle);
#endif

	audioInit= FL_TRUE;
	outrate= hertz;
#if 0	/* Obsolete: OS_Irix */
	noWait= FL_FALSE;
#endif
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
{	/* Irix lets this change. Linux (OSS) doesn't appear to. */	}

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
#if 0	/* Obsolete: OS_Irix */
	noWait= FL_TRUE;
#endif
}

void audio_flushBuffers(Flag synchro)
{
#if 0	/* Obsolete: Linux, OSS style */
	write(audio, outSamp, buffPos);
#else
	/* buf should be '[signed] short' type */
	//if ((err = snd_pcm_writei(playback_handle, buf, 128)) != 128) {
	snd_pcm_writei(pcm_handle, outSamp, buffPos);
#endif
#if 1
#ifdef DEVEL
	if (synchro)
	    fprintf(stderr, "FUTURE: FIXME. 'synchro' not supported\n");
#endif
#else	/* OS_Irix */
	if (synchro)
	    while (audioBusy(buffPos)) sginap(1);
#endif
}

void audio_close()
{
	if (audioInit)
	{
#if 1
	    snd_pcm_drain(pcm_handle);
	    snd_pcm_close(pcm_handle);
	    /* ?and snd_pcm_hw_params_free(hw_params); */
#else	/* Obsolete: OS_Irix */
	    if (!noWait)
		while (audioBusy(0)) sginap(1);
#endif
	}

	if (outSamp)
	{
	    FREE_MEMORY(outSamp, "ekaudio.c::audio_close()");
	    outSamp= NULL;
	}
#if 0	/* Obsolete: OS_Irix */
	noWait= FL_FALSE;	/* in case we open audio again */
#endif
}
