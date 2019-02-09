#ifndef __EKINSTR_H__
#define __EKINSTR_H__

#include <ctype.h>
#include <stdio.h>	/* FILE definition */
#include <string.h>

#include "ektype.h"

typedef enum ekInsType {
	insType_unset= 0, insType_unplayable= -1,
	insType_sigChar=1	/* typical MOD */
	} EkInsType;

typedef struct ekInstr {
	EkInsType	insType;
	EkInstTuning	tuning;
	char		*name;
	const char	*filename;
	void		*waveform;	/* was (ASAMP *) */
	U_LNG		length,
			rpt_bgn,
			rpt_len;
	U_CHR		volume;
	S_CHR		transpose,
			finetune;
	U_CHR		midich, midipreset,
			hold, decay, suppress;
	} EkInstr;

EkErrFlag ekinstr_new(EkInstr **instPtr, int reqd);
EkErrFlag ekinstr_construct(EkInstr *inst);
#define EKINSTR_FIELD(name, value)	\
	(name).value
EkErrFlag ekinstr_load_bestguess(EkInstr *inst, const char *name);
EkErrFlag ekinstr_load_st(EkInstr *inst, FILE *ifPtr);
EkErrFlag ekinstr_destroy(EkInstr *inst);
EkErrFlag ekinstr_delete(EkInstr **instPtr);

#endif	/* __EKINSTR_H__ */
