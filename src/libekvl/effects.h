#ifndef __EFFECTS_H__
#define __EFFECTS_H__

#include "eknote.h"
#include "ekprefs.h"
#include "ektype.h"
#include "vchannel.h"

/* some effects names */
#define FXPARAMS int chan,EkNote *note,EkSetup *settings,EkVchannel *vch
#define FXARGS	chan,note,settings,vch
#define DOPARAMS EkVchannel *vch, int step, int smax

#define STFX_ARPEGGIO	0x00
#define STFX_DOWN	0x01
#define STFX_UP		0x02
#define STFX_PORTA	0x03
#define STFX_VIBRATO	0x04
#define STFX_PRTASLIDE	0x05
#define STFX_VIBSLIDE	0x06
#define STFX_OFFSET	9
#define STFX_VOLSLIDE	0x0A
#define STFX_FF		0x0B
#define STFX_VOLUME	0x0C
#define STFX_SKIP	0x0D
#define STFX_EXTENDED	14
#define STFX_SPEED	0x0F
#define STFX_NONE	0x10
#define EXT_BASE	16
#define STFX_SET_FILTER		(EXT_BASE + 0x00)
#define STFX_SMOOTH_DOWN	(EXT_BASE + 0x01)
#define STFX_SMOOTH_UP		(EXT_BASE + 0x02)
#define STFX_SET_GLISSANDO	(EXT_BASE + 0x03)
#define STFX_CHG_FTUNE		(EXT_BASE + 0x05)
#define STFX_LOOP		(EXT_BASE + 0x06)
#define STFX_RETRIG		(EXT_BASE + 0x09)
#define STFX_S_UPVOL		(EXT_BASE + 0x0A)
#define STFX_S_DOWNVOL		(EXT_BASE + 0x0B)
#define STFX_CUT		(EXT_BASE + 0x0C)
#define STFX_LATESTART		(EXT_BASE + 0x0D)
#define STFX_EXTRA_TICKS	(EXT_BASE + 0x0E)

#define MEDFX_ARPEGGIO	0x00
#define MEDFX_DOWN	0x01
#define MEDFX_UP	0x02
#define MEDFX_PORTA	0x03
#define MEDFX_VIBRATO	0x04
#define MEDFX_PRTASLIDE	0x05
#define MEDFX_VIBSLIDE	0x06
#define MEDFX_SPEED2	0x09
#define MEDFX_NO_10	0x0A	/* unknown */
#define MEDFX_FF	0x0B
#define MEDFX_VOLUME	0x0C
#define MEDFX_VOLSLIDE	0x0D
#define MEDFX_EXTENDED	14
#define MEDFX_SPEED	0x0F
#define MEDFX_NONE	0x10
#define EXT_BASE	16
#define MEDFX_SMOOTH_UP		(EXT_BASE + 1)
#define MEDFX_SMOOTH_DOWN	(EXT_BASE + 2)
#define MEDFX_CHG_FTUNE		(EXT_BASE + 5)
#define MEDFX_LOOP		(EXT_BASE + 6)
#define MEDFX_OFFSET		(EXT_BASE + 9)
#define MEDFX_S_UPVOL		(EXT_BASE + 10)
#define MEDFX_S_DOWNVOL		(EXT_BASE + 11)
#define MEDFX_NOTECUT		(EXT_BASE + 12)
#define MEDFX_LATESTART		(EXT_BASE + 13)
#define MEDFX_DELAY		(EXT_BASE + 14)



#define MDMF1_FILTERON   0x1     /* hardware low-pass filter */
#define MMDF1_JUMPINGON  0x2     /* jumping.. */
#define MMDF1_JUMP8TH    0x4     /* jump 8th.. */
#define MMDF1_INSTRSATT  0x8     /* instruments are attached (sng+samples)
                                   used only in saved MED-songs */
#define MMDF1_VOLHEX     0x10    /* volumes are represented as hex */
#define MMDF1_STSLIDE    0x20    /* no effects on 1st timing pulse (STS) */
#define MMDF1_8CHANNEL   0x40    /* OctaMED 8 channel song, examine this bit
                                   to find out which routine to use */
/* flags2 */
#define MMDF2_BMASK     0x1F
#define MMDF2_BPM       0x20

#define VBSF1_PLAY	1
#define VBSF1_STAT	2
#define VBSF1_WARN	4



/* the fuzz in note pitch */
#define FUZZ 2

/* for parameters that are split into two halves: */
#define Hi(x)	(x >> 4)
#define Lo(x)	(x & 0x0F)
#define Nibble(x) ( (Lo(x))?-Lo(x):Hi(x) )
#define Xbcd(x)	(x - (x>>4)*6)	/* hex -> (binary coded) decimal */

EkErrFlag init_fxtable(EkSetup *settings, void (***fxt)(FXPARAMS));

void medfx_ptchdown(FXPARAMS);
void medfx_ptchup(FXPARAMS);
void meddo_ptchbend(DOPARAMS);
void medfx_speed(FXPARAMS);
void meddo_double(DOPARAMS);
void meddo_delayed(DOPARAMS);
void meddo_triple(DOPARAMS);
void medfx_speed2(FXPARAMS);
void medfx_volume(FXPARAMS);

void stfx_nothing(FXPARAMS);
void stfx_arpeggio(FXPARAMS);
void stdo_arpeggio(DOPARAMS);
void stfx_cut(FXPARAMS);
void stdo_cut(DOPARAMS);
void stfx_extraTicks(FXPARAMS);
void stfx_ff(FXPARAMS);
int stdo_ff(EkVchannel *vch, EkSetup *settings);
void stfx_finetune(FXPARAMS);
void stfx_latestart(FXPARAMS);
void stdo_latestart(DOPARAMS);
void stfx_loop(FXPARAMS);
int stdo_loop(EkVchannel *vch, EkSetup *settings);
void stfx_offset(FXPARAMS);
void stfx_perDown(FXPARAMS);
void stfx_perUp(FXPARAMS);
void stfx_portamento(FXPARAMS);
void stdo_portamento(DOPARAMS);
void stfx_ptaslide(FXPARAMS);
void stdo_ptaslide(DOPARAMS);
void stfx_ptchdown(FXPARAMS);
void stfx_ptchup(FXPARAMS);
void stdo_ptchbend(DOPARAMS);
void stfx_retrig(FXPARAMS);
void stdo_retrig(DOPARAMS);
void stfx_setFilter(FXPARAMS);
void stfx_setGlissando(FXPARAMS);
void stfx_skip(FXPARAMS);
int stdo_skip(EkVchannel *vch, EkSetup *settings);
void stfx_speed(FXPARAMS);
void stfx_vibrato(FXPARAMS);
void stdo_vibrato(DOPARAMS);
void stfx_vibslide(FXPARAMS);
void stdo_vibslide(DOPARAMS);
void stfx_volslide(FXPARAMS);
void stdo_volslide(DOPARAMS);
void stfx_volDown(FXPARAMS);
void stfx_volUp(FXPARAMS);
void stfx_volume(FXPARAMS);
void stfx_undef(FXPARAMS);
void stfx_unexp(FXPARAMS);

#endif	/* __EFFECTS_H__ */
