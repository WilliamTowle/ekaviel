#include "fwrite.h"

#include "ekmalloc.h"

static	U_CHR	savecount= 0;

char *writename(const char *filename, const char *instname,
		const char *prefix)
{
  const char	*name= filename? filename : instname,
		*colon= strrchr(name, ':');
  char		*use, *ws;

	if (strlen(name) == 0)
	{
	    savecount= (savecount + 1) % 100;
	    use= (char *)ALLOCATE_MEMORY(strlen(prefix)+3,char);
	    sprintf(use, "%s%02d", prefix, savecount);
	}
	else
	{
	    if (colon) name= colon+1;
	    use= (char *)ALLOCATE_MEMORY(strlen(prefix)
					+ strlen(name) + 1, char);
	    sprintf(use, "%s%s", prefix, name);
	}

	while ( (ws= strchr(use, ' ')) != NULL ) *ws= '_'; 

  return use;
}

EkErrFlag dump_sample(EkInstr *instr, const char *filename)
{
  FILE	*ofPtr;
  EkErrFlag	error= EK_ERR_NONE;

	if (filename == NULL)
	    return EK_ERR_MALLOC_FAILURE;

	if ( (ofPtr= fopen(filename, "wb")) == NULL )
	    error= EK_ERR_SAVEFAIL;
	else
	{
	  unsigned int wrote= fwrite(EKINSTR_FIELD(*instr, waveform),
				sizeof(S_CHR),
				EKINSTR_FIELD(*instr, length), ofPtr);
	    fclose(ofPtr);

	    if (wrote < EKINSTR_FIELD(*instr, length))
		error= EK_ERR_SAVEFAIL;
	}

	return error;
}
