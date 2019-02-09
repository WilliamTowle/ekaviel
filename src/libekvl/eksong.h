#ifndef __EKSONG_H__
#define __EKSONG_H__

#include "ekinstr.h"
#include "eknote.h"
#include "ektype.h"


typedef struct ekBlock {
	char	*name;
	int	tracks,lines;
	EkNote	**note;
	} EkBlock;

EkErrFlag ekblock_new(EkBlock **blockPtr, int reqd);
EkErrFlag ekblock_construct(EkBlock *block);
#define EKBLOCK_FIELD(name, value)		\
	(name).value
EkErrFlag ekblock_construct_notes(EkBlock *block,
					int lines, int tracks);
EkErrFlag ekblock_destroy_notes(EkBlock *block);
EkErrFlag ekblock_destroy(EkBlock *block);
EkErrFlag ekblock_delete(EkBlock **blockPtr);

typedef struct ekSong {
	EkSongType	songType;
	const char	*filename;
	char		*title;	/* 20 chars. in SoundTracker */

	int		mstrvol,
			*trkvol,
			maxtracks;
	U_SHT		tempo;		/* primary tempo */
	U_CHR		fineTempo;	/* 2ndary tempo
						(delay betw. notes) */

	U_CHR		instrsz,
			lastinstr;
	EkInstr		*instr;

	int		blocks,
			playseqs;
	PSEQ		*playseq;
	EkBlock		*block;

	/* OctaMED */
	int		flags1, flags2;
	S_CHR		transpose;
	} EkSong;

EkErrFlag eksong_construct(EkSong *song);
#define EKSONG_FIELD(name, value)	\
	(name).value
EkErrFlag eksong_load_bestguess(EkSong *song, const char *filename);
EkErrFlag eksong_construct_instrs(EkSong *song, int instrs);
int eksong_instrs_playable(EkSong *song);
int eksong_instrs_unplayable(EkSong *song);
EkErrFlag eksong_destroy_instrs(EkSong *song);
EkErrFlag eksong_construct_blocks(EkSong *song, int blocks);
EkErrFlag eksong_destroy_blocks(EkSong *song);
EkErrFlag eksong_construct_trkvols(EkSong *song, int tracks);
EkErrFlag eksong_destroy_trkvols(EkSong *song);
char *eksong_title(EkSong *song);
EkErrFlag eksong_destroy(EkSong *song);

#endif	/* __EKSONG_H__ */
