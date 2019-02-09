#include <stdio.h>	/* sprintf() */
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ekinstr.h"

#include "ekaudio.h"	/* extern int outrate */
#include "ekmalloc.h"
#include "loadbase.h"

EkErrFlag ekinstr_new(EkInstr **instPtr, int reqd)
{
	if (instPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekinstr_new() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkInstr	*instr= (EkInstr *)
				ALLOCATE_MEMORY(reqd, EkInstr);

	    if ( (*instPtr= instr) == NULL)
		return EK_ERR_MALLOC_FAILURE;
	    else
		return EK_ERR_NONE;
	}
}

EkErrFlag ekinstr_construct(EkInstr *inst)
{
	if (inst == NULL)
	    return EK_ERR_MALLOC_FAILURE;

	EKINSTR_FIELD(*inst, insType)= insType_unset;
	EKINSTR_FIELD(*inst, tuning)= EK_INSTUNE_UNKNOWN;
	EKINSTR_FIELD(*inst, name)= NULL;
	EKINSTR_FIELD(*inst, filename)= NULL;
	EKINSTR_FIELD(*inst, length)= 0;
	EKINSTR_FIELD(*inst, rpt_len)= 0;
	EKINSTR_FIELD(*inst, waveform)= NULL;

	EKINSTR_FIELD(*inst, volume)= 64;
	EKINSTR_FIELD(*inst, transpose)= 0;
	EKINSTR_FIELD(*inst, finetune)= 0;

	EKINSTR_FIELD(*inst, midich)= 0;
	EKINSTR_FIELD(*inst, midipreset)= 0;
	EKINSTR_FIELD(*inst, hold)= 0;
	EKINSTR_FIELD(*inst, decay)= 0;
	EKINSTR_FIELD(*inst, suppress)= 0;

  return EK_ERR_NONE;
}

EkErrFlag ekinstr_load_bestguess(EkInstr *inst, const char *filename)
{
	if (inst == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	  EkErrFlag error= EK_ERR_NONE;
	  FILE	*inFile= fopen(filename, "r");
	  struct stat	statbuf;

	    if (!inFile)
		return EK_ERR_FILENOTFOUND;

	    if (stat(filename, &statbuf) != 0)
		return EK_ERR_FILECORRUPT;

	    /* assume ST (raw signed char) */
	    EKINSTR_FIELD(*inst, length)= statbuf.st_size;
	    EKINSTR_FIELD(*inst, filename)= filename;
	    error= ekinstr_load_st(inst, inFile);

	    /* only if error == EK_ERR_NONE? */
	    fclose(inFile);

	  return error;
	}
}

EkErrFlag ekinstr_load_st(EkInstr *inst, FILE *ifPtr)
{
  EkErrFlag	error= EK_ERR_NONE;

	if ( (EKINSTR_FIELD(*inst, waveform)=
		ALLOCATE_MEMORY(EKINSTR_FIELD(*inst, length), S_CHR))
			== NULL )
	{
	    error= EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  unsigned int i;

	    for (i=0; (i<EKINSTR_FIELD(*inst, length))
			&& (error == EK_ERR_NONE); i++)
	    {
		((S_CHR *)EKINSTR_FIELD(*inst, waveform))[i]=
					fgetSchr(ifPtr, &error);
	    }
	}

	if (error == EK_ERR_NONE)
	    EKINSTR_FIELD(*inst, insType)= insType_sigChar;

  return error;
}

EkErrFlag ekinstr_destroy(EkInstr *inst)
{
	if (inst == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekinstr_destroy() called %s\n",
		    		"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    FREE_MEMORY(inst);
	    return EK_ERR_NONE;
	}
}

EkErrFlag ekinstr_delete(EkInstr **instPtr)
{
	if (instPtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "ekinstr_delete() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    FREE_MEMORY(*instPtr);
	    *instPtr= NULL;
	    return EK_ERR_NONE;
	}
}
