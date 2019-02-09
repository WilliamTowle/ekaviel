#include "ekaviel.h"

void audio_init(int hertz, boolean stereo, U_CHR bleed);
boolean audioBusy(int remain);
void getOutrate();
int audio_makeBuffers(int size);
void audio_appendBuffers(int vchans, long left, long rght);
void audio_discardBuffers();
void audio_flushBuffers(boolean synchro);
void audio_close();
