#ifndef __LOADBASE_H__
#define __LOADBASE_H__

#include <stdio.h>
#include "ektype.h"

EkErrFlag fputat(FILE *ifPtr, long offs);
EkErrFlag fskip(FILE *ifPtr, long bytes);

S_CHR fgetSchr(FILE *ifPtr, EkErrFlag *errPtr);
U_CHR fgetUchr(FILE *ifPtr, EkErrFlag *errPtr);

U_LNG fgetUlng(FILE *ifPtr,EkErrFlag *errPtr);

U_SHT fgetUsht(FILE *ifPtr, EkErrFlag *errPtr);

EkErrFlag fgetStr(char **ptr, FILE *ifPtr, int len);

const char *find_file(const char *fname);

int compression(const char *argfile);
char *uncompress(const char *argfile, int compressID);

#if 0
#include <stdio.h>	/* FILE type definition */
#include "ekaviel.h"
#include "ekmalloc.h"

#define MAXPATHLEN 350


U_SHT fgetUsh(FILE *ifPtr,errFlag *errPtr);
U_SHT fgetRUsh(FILE *ifPtr,errFlag *errPtr);
U_LNG fgetRLong(FILE *ifPtr,errFlag *errPtr);
char *fgetStr(FILE *ifPtr,errFlag *errPtr, int len);


SONG *songAlloc();
void sfree(SONG *song);
SONG *null_song(SONG *song, char *reason);

errFlag load_xsamp(INSTR *samp, char *name);
errFlag load_samp(FILE *ifPtr, INSTR *samp);

int fileExtn(const char *name, const char *extn);
SONG *load_file(char *name, SETUP *settings);
#endif

#endif	/* __LOADBASE_H__ */
