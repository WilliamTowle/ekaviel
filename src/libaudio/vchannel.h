#ifndef __VCHANNEL_H__
#define __VCHANNEL_H__

#include "ekinstr.h"
#include "eknote.h"
#include "ekprefs.h"
#include "ektype.h"

#ifdef DEVEL
#define DEVEL_R54A
#endif

typedef struct ekVchannel
{
        EkInstr	*sourceRef;	/* sample to output */
        int     srcPos,		/* how far through */
                srcEnd,
                srcRew;
	U_CHR	fxdat;
	Flag	pause,		/* delay start until FL_FALSE */
		glissando;
        int	pitch;
	S_CHR	basetranspose,
		transpose,
		finetune;
	int	period,
		ptch_adj, ptch_bnd,	/* adjust rate and delta */
		ptch_tgt,
                counter,
		mvol,		/* master volume */
                volume, vol_rate,
		vib_rate, vib_depth,
		vib_offs, vib_bend,
		loop_blkNum, loop_blkPos, loop_count;
	void	(*fxcall) (struct ekVchannel *, int, int);
	int	(*fxdone) (struct ekVchannel *, EkSetup *settings);
	} EkVchannel;

EkErrFlag ekvchannel_new(EkVchannel **chan, int reqd, int *made);
EkErrFlag ekvchannel_construct(EkVchannel *chan);
#define EKVCHANNEL_FIELD(name, field)		\
	(name).field
EkErrFlag ekvchannel_destroy(EkVchannel *chan);
EkErrFlag ekvchannel_delete(EkVchannel **chan, int made);

#define DIVIDE 10
#define MODULUS 01777
/* limited and unlimited period calculation:
 * 	period_l() where CPU time limited [uses lookup table]
 * 	period_u() to unlimit CPU usage [does precise calculation]
 */

#ifndef DEVEL_R54A
#define	FREQCH ((7093790/outrate)*(32 << octaves))
#endif

int getnote_scale(const char *arg);
PERIODFN_HEADER(period_l);
PERIODFN_HEADER(period_u);
U_SHT c_ftune(S_CHR transp, S_CHR ftu);

Flag set_sync(Flag sync);
Flag toggle_sync();
EkErrFlag init_vchans(int chs, EkSetup *settings);
int reset_vchans(EkSetup *settings);
void set_vchan(int chan, EkNote *note, EkInstr samp[], EkSetup *settings);
EkErrFlag flush_vchans(EkSetup *settings, int *commandInt);
Flag vchan_finished();
void close_vchans();

#endif	/* __VCHANNEL_H__ */
