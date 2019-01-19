#ifndef __LOAD_ST_H__
#define __LOAD_ST_H__

#include <stdio.h>
#include "eksong.h"
#include "ektype.h"

EkErrFlag load_st_seqlist(EkSong *song, FILE *ifPtr, int seq_size, int maxblks);
EkErrFlag load_st(EkSong *song, FILE *ifPtr, int samps, int chans);

#endif
