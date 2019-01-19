#include <math.h>
#include <string.h>	/* strdup() */
#include <stdio.h>	/* sprintf() */
#include "eknote.h"

#include "ekmalloc.h"

#ifdef DEVEL
#define DEVEL_R54A
#endif

/*****
 *
 *****/

const char *instName[] = {
	" -", " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9",
	" A", " B", " C", " D", " E", " F", " G", " H", " I", " J",
	" K", " L", " M", " N", " O", " P", " Q", " R", " S", " T",
	" U", " V",
	"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
	"1A", "1B", "1C", "1D", "1E", "1F", "1G", "1H", "1I", "1J",
	"1K", "1L", "1M", "1N", "1O", "1P", "1Q", "1R", "1S", "1T",
	"1U", "1V"
	};

#ifdef DEVEL
#ifdef OCTAVES
const U_SHT	octaves= OCTAVES;
#else
const U_SHT	octaves= 4;
#endif
#else
const U_SHT	octaves= 4;	/* 5 implies include C-0 */
#endif
U_SHT		*ftuTable= NULL,
		*perTable= NULL;
char		**ptchName= NULL;
int		perTabSz= 0;

/*****
 *	EkNote class
 *****/

EkErrFlag eknote_new(EkNote **notePtr, int reqd)
{
	if (notePtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eknote_new() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  EkNote *note= (EkNote *)ALLOCATE_MEMORY(reqd, EkNote,
					"eknote.c::eknote_new()");

	    if ( (*notePtr= note) == NULL)
		return EK_ERR_MALLOC_FAILURE;
	    else
		return EK_ERR_NONE;
	}
}

EkErrFlag eknote_construct(EkNote *note)
{
	if (note == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	    EKNOTE_FIELD(*note, fx)= 0;
	    EKNOTE_FIELD(*note, fxdat)= 0;
#ifndef SLOW_DISPLAY
	    strcpy(EKNOTE_FIELD(*note, text), "");
#endif
	    return EK_ERR_NONE;
	}
}

#ifndef SLOW_DISPLAY
EkErrFlag eknote_set_text(EkNote *note)
{
	if (note == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else
	{
	    sprintf(EKNOTE_FIELD(*note, text), EKNOTE_TEXT_FMT);
	    return EK_ERR_NONE;
	}
}
#endif

EkErrFlag eknote_destroy(EkNote *note)
{
#ifdef DEVEL
;fprintf(stderr, "eknote_destroy()\n");
#endif
	if (note == NULL)
	    return EK_ERR_MALLOC_FAILURE;
	else	/* no memory allocation takes place in fields */
	    return EK_ERR_NONE;
#ifdef DEVEL
;fprintf(stderr, "destroy()ed\n");
#endif
}

EkErrFlag eknote_delete(EkNote **notePtr)
{
	if (notePtr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "eknote_delete() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	    FREE_MEMORY(*notePtr, "eknote.c::eknote_delete()");
	    *notePtr= NULL;
	    return EK_ERR_NONE;
	}
}

/*****
 *
 *****/

static
double	power(double x, double y) { return exp(y * log(x)); }

EkErrFlag init_notes()
{
	if (perTabSz > 0)
	    return EK_ERR_NONE;
	else
	{
	  int	i;
	  EkErrFlag error= EK_ERR_NONE;

	    perTabSz= octaves*12;
	    perTable= (U_SHT *)ALLOCATE_CLRMEM(perTabSz, U_SHT,
				"eknote.c::init_notes(perTable)");
	    ftuTable= (U_SHT *)ALLOCATE_CLRMEM(96,U_SHT,
				"eknote.c::init_notes(ftuTable)");
#if 0	/* ??? */
	    ptchName= (char **)ALLOCATE_CLRMEM((octaves+1)*12,char *,
				"eknote.c::init_notes(ptchName)");
#else
	    ptchName= (char **)ALLOCATE_CLRMEM(perTabSz+1,char *,
				"eknote.c::init_notes(ptchName)");
#endif

	    if (perTable == NULL)
		return EK_ERR_MALLOC_FAILURE;
	    else
	    {
		for (i= 0; i < perTabSz; i++)
		{
		    perTable[i]= (U_SHT) ( (107 << octaves)
						/ (2*power(2,i/12.0)) );
		}
	    }

	    if (ftuTable == NULL)
	    {
		FREE_MEMORY(perTable, "eknote.c::init_notes(perTable)");
		perTabSz= 0;
		return EK_ERR_MALLOC_FAILURE;
	    }
	    else
	    {
		for (i=0; i<96; i++)
		{
		  double d;	/* Larry Wall, eat your heart out! */

		    d= i/(double) 96;
#ifdef DEVEL_R54A
		    ftuTable[i]= (U_SHT)( perTable[0]/power(2,d) );
#else
		    ftuTable[i]= (U_SHT)( (perTable[0])/power(2,d) );
#endif	/* DEVEL_R54A */
		}
	    }

	    if (ptchName == NULL)
	    {
		FREE_MEMORY(perTable, "eknote.c::init_notes(perTable)");
		perTabSz= 0;
		FREE_MEMORY(ftuTable, "eknote.c::init_notes(ftuTable)");
		return EK_ERR_MALLOC_FAILURE;
	    }
	    else
	    {
	      char	name[4],
			*n[12]= {"C-", "C#", "D-", "D#", "E-", "F-",
				"F#", "G-", "G#", "A-", "A#", "B-"};

		if ( (ptchName[0]= strdup("---")) == NULL)
		    error= EK_ERR_MALLOC_FAILURE;

		for (i= 0; (i < perTabSz)
				&& (error == EK_ERR_NONE); i++)
		{
		    sprintf(name, "%s%X", n[i%12], (i/12)+1);
		    if ( (ptchName[i+1]= strdup(name)) == NULL)
			error= EK_ERR_MALLOC_FAILURE;
		}
	    }

	  return error;
	}
}

int getnote(int pitch)
{
  int i;
	if (pitch == 0) return 0;
	for (i= 0; i < perTabSz; i++)
	    if (perTable[i] <= pitch) return i+1;
  return i;
}

int new_periodit(int pitch)
{
  U_SHT per= ftuTable[ (pitch % 12)*8 ];
  U_CHR div;
	for (div= (pitch/12); div > 0 ; div--) per /= 2;
  return per;
}

U_SHT c_ftune(S_CHR transp, S_CHR ftu)
{
  S_CHR	mod= (transp % 12)*8 + ftu,
	div= (transp/12) + ((mod < 0)? -1:0) ;
  U_SHT	per= ftuTable[mod + ((mod < 0)? 96:0)];

	for (; div < 0; div++) per *= 2;
	for (; div > 0; div--) per /= 2;

  return per;
}

void zap_notes()
{
	if (perTable != NULL)
	{
	    FREE_MEMORY(perTable, "eknote.c::zap_notes(perTable)");
	    perTabSz= 0; perTable= NULL;
	}

	if (ftuTable != NULL)
	{
	    FREE_MEMORY(ftuTable, "eknote.c::zap_notes(ftuTable)");
	    ftuTable= NULL;
	}

	if (ptchName != NULL)
	{
	    FREE_MEMORY(ptchName, "eknote.c::zap_notes(ptchName)");
	    ptchName= NULL;
	}
}
