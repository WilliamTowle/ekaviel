#ifndef __EKTYPE_H__
#define __EKTYPE_H__

#define MAX_SAMPLE_LENGTH 500000
#define PSEQ	signed short

typedef unsigned char	U_CHR;
typedef signed char	S_CHR;
typedef unsigned short	U_SHT;
typedef signed short	S_SHT;
typedef unsigned long	U_LNG;

typedef unsigned char Flag;
#ifndef FL_TRUE
#define FL_TRUE		1
#define FL_FALSE	0
#endif



typedef const char *EkErrFlag;
extern EkErrFlag	EK_ERR_NONE;
extern EkErrFlag	EK_ERR_NOT_SUPPORTED;
extern EkErrFlag	EK_ERR_FILENOTFOUND;
extern EkErrFlag	EK_ERR_FILEINCOMPLETE;
extern EkErrFlag	EK_ERR_FILECORRUPT;
extern EkErrFlag	EK_ERR_MALLOC_FAILURE;
extern EkErrFlag	EK_ERR_INSTCORRUPT;
extern EkErrFlag	EK_ERR_NO_AUDIO;
extern EkErrFlag	EK_ERR_NO_FXTABLE;
extern EkErrFlag	EK_ERR_SAVEFAIL;
extern EkErrFlag	EK_ERR_MISSING_INSTRS;
extern EkErrFlag	EK_ERR_DEVEL;

#define EKERRFLAG_STRING_VALUE(error)		\
	((error != EK_ERR_NONE)? error : "No error")

typedef const char	*EkSongType;
extern EkSongType	EK_SONG_PARTIAL;
#ifdef DEVEL
extern EkSongType	EK_SONG_SCREAM;
#endif
extern EkSongType	EK_SONG_MMD;
extern EkSongType	EK_SONG_MOD31;
extern EkSongType	EK_SONG_MOD15;

typedef const char	*EkInstTuning;
extern EkInstTuning	EK_INSTUNE_UNKNOWN;
extern EkInstTuning	EK_INSTUNE_SB_PERFECT;
extern EkInstTuning	EK_INSTUNE_SB_COARSE;
extern EkInstTuning	EK_INSTUNE_SB_FINE;

#define PERIODFN_PARMS			int, int,int, S_CHR,S_CHR
#define	PERIODFN_DECLARE_MEMBER(name)	int (*name)(PERIODFN_PARMS);
#define PERIODFN_ARGS			int pitch,		\
					int pbend, int vbend,	\
					S_CHR tr, S_CHR ft
#define PERIODFN_HEADER(func)		int func(PERIODFN_ARGS)

#endif	/* __EKTYPE_H__ */
