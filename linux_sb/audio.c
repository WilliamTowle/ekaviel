#include <i386at/sblast.h>
#include <malloc.h>
#include <stdio.h>
#include "audio.h"
#include "ekaviel.h"
#include "io.h"

#define DEF_OUTRATE 44100
#define MSAMP unsigned char	/* ignore choice of 16-bit; */
				/* most samples are 8-bit anyway */

boolean	audioInit= FALSE;
int	outrate= DEF_OUTRATE;	/* except SB wants twice this for stereo */

static int	bleed1= 255,
		bleed2= 0;

FILE *audio;

static boolean	noWait;
static MSAMP	*outSamp= NULL;
static int	buffPos;
static int	chans;



void audio_init(int hertz, boolean stereo, U_CHR bleed)
{
	audio= fopen("/dev/sb_dsp", "w");
	if (!audio)
	{
	    error1("Couldn't open audio device");
	    terminate();
	}
	else if (ioctl(fileno(audio), DSP_IOCTL_STEREO, stereo) == -1)
	{
	    error1("Couldn't set stereo/mono");
	    terminate();
	}
	else	/* only okay so far...*/
	{
	    if (stereo)
		chans= 2;
	    else
		chans= 1;
	    outrate= hertz*chans;

	    if (ioctl(fileno(audio), DSP_IOCTL_SPEED, &outrate) == -1)
	    {
		error1("Couldn't set requested sample frequency");
		terminate();
	    }
	}

	noWait= FALSE;
	audioInit= TRUE;

/* set bleed (channel "overlap").....
 * Level 0:	normal stero, expected channels used
 * Level 128:	mono
 * Level 255:	normal stereo, left and right channels switched
 ***/
	bleed1= 255-bleed;
	bleed2= bleed;
}

boolean audioBusy(int remain)  /* don't know how we tell */
{
#if 0
	if (audioInit)
	    return ( ALgetfilled(alport)>remain );
	else
#endif
	    return FALSE;
}

void getOutrate()	/* don't know how we tell... */
{
}

int audio_makeBuffers(int size)
{
/* Can't terminate() here - need to return fail and clean memory */
	if ( outSamp= (MSAMP *)
		realloc(outSamp, chans * size * sizeof(MSAMP)) )
	{
	    buffPos= 0;
	    return 1;
	}
	else
	{
	    error1("TODO: Realloc failure - memory still alloc'ed");
	    terminate();
	    return 0;
	}
}

void audio_appendBuffers(int vchans, long left, long rght)
{
/* Incoming long values are (vchans * [<signed char> * 64*64]) */
	if (chans == 2)
	{
	    outSamp[buffPos++]= (MSAMP)
			((left*bleed1 + rght*bleed2)/(4096*vchans));
	    outSamp[buffPos++]= (MSAMP)
			((rght*bleed1 + left*bleed2)/(4096*vchans));
	}
	else
	    outSamp[buffPos++]= (MSAMP) ((left+rght)/(32*vchans));
}

/* For when audio is closed immediately afterward */
void audio_discardBuffers()
{
	noWait= TRUE;
}

void audio_flushBuffers(boolean synchro)
{
	if (fwrite(outSamp, sizeof(MSAMP), buffPos, audio) != buffPos)
	    warn1("Incomplete transfer of sound data to soundcard!");
	if (synchro) while (audioBusy(buffPos)); /* CPU waster! */
}

void audio_close()
{
	if (audioInit)
	{
	    if (!noWait)
	    {
		while (audioBusy(0));	/* yuk- CPU waster */
	    }
	    /* BSD soundblaster waits, but SysV will abort (apparently) */

	    fclose(audio);
	    audioInit= FALSE;
	}
	if (outSamp)
	{
	    free (outSamp);
	    outSamp= NULL;
	}
	noWait= FALSE;	/* in case we open audio again */
}
