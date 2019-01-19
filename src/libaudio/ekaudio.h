#ifndef __EKAUDIO_H__
#define __EKAUDIO_H__

#include "ektype.h"

#ifndef DEFAULT_OUTRATE
#define	DEFAULT_OUTRATE 44100
#endif

extern	int	outrate;
extern	Flag	audioInit;

EkErrFlag audio_init(int hertz, Flag stereo, U_CHR bleed);
Flag audioBusy(int remain);
void getOutrate();
EkErrFlag audio_makeBuffers(int size);
void audio_appendBuffers(int vchans, long left, long rght);
void audio_discardBuffers();
void audio_flushBuffers(Flag synchro);
void audio_close();

#endif	/* __EKAUDIO_H__ */
