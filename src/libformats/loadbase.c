#include <stdio.h>	/* tempnam() */
#include <stdlib.h>	/* getenv(), system() */
#include <string.h>
#include <limits.h>	/* PATH_MAX */
#include "loadbase.h"

#include "ekmalloc.h"

#ifndef TEMP_DIR
#define TEMP_DIR	"/tmp"
#endif

static char tempfile[FILENAME_MAX];

/*****
 *	File position operations
 *****/

EkErrFlag fputat(FILE *ifPtr, long offs)
{
  long	nowat= ftell(ifPtr);

	if (nowat == offs)
	    return EK_ERR_NONE;
	else if (nowat < offs)
	    return fskip(ifPtr, offs - nowat);
	else	/* (nowat > offs) */
	{
	    fseek(ifPtr, 0,SEEK_SET);
	    return fskip(ifPtr, offs);
	}
}

EkErrFlag fskip(FILE *ifPtr, long bytes)
{
  EkErrFlag	error= EK_ERR_NONE;

	for (;(bytes > 0) && (error == EK_ERR_NONE); bytes--)
	    fgetUchr(ifPtr, &error);

  return error;
}

/*****
 *	File read operations
 *****/

S_CHR fgetSchr(FILE *ifPtr, EkErrFlag *errPtr)
{
  S_CHR	sc= 0;

	if (*errPtr == EK_ERR_NONE)
	{
	    sc= getc(ifPtr);

	    if (feof(ifPtr))
		*errPtr= EK_ERR_FILEINCOMPLETE;
	    else if (ferror(ifPtr))
		*errPtr= EK_ERR_FILECORRUPT;
	}
  return sc;
}

U_CHR fgetUchr(FILE *ifPtr, EkErrFlag *errPtr)
{
  U_CHR	uc= 0;

	if (*errPtr == EK_ERR_NONE)
	{
	    uc= getc(ifPtr);

	    if (feof(ifPtr))
		*errPtr= EK_ERR_FILEINCOMPLETE;
	    else if (ferror(ifPtr))
		*errPtr= EK_ERR_FILECORRUPT;
	}
  return uc;
}

U_LNG fgetUlng(FILE *ifPtr, EkErrFlag *errPtr)
{
  U_LNG	ul= fgetUchr(ifPtr,errPtr);
	ul= (ul << 8) | fgetUchr(ifPtr,errPtr);
	ul= (ul << 8) | fgetUchr(ifPtr,errPtr);
  return (ul << 8) | fgetUchr(ifPtr,errPtr);
}

U_SHT fgetUsht(FILE *ifPtr, EkErrFlag *errPtr)
{	/* standard read-short is probably endian dependent */
  U_SHT		i= fgetSchr(ifPtr,errPtr) << 8;
  return	i | fgetUchr(ifPtr,errPtr);
}

EkErrFlag fgetStr(char **ptr, FILE *ifPtr, int len)
{
	if (ptr == NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "fgetStr() called %s\n",
				"with NULL pointer");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else if (*ptr != NULL)
	{
#ifdef DEVEL
	    fprintf(stderr, "fgetStr() called %s\n",
				"with allocated memory");
#endif
	    return EK_ERR_MALLOC_FAILURE;
	}
	else
	{
	  char		*sptr= (char *)ALLOCATE_MEMORY(len+1,char,
					"loadbase.c::fgetStr()");

	    if ( (*ptr= sptr) == NULL )
		return EK_ERR_MALLOC_FAILURE;
	    else
	    {
	      EkErrFlag	error= EK_ERR_NONE;

		for (; (len > 0) && (error == EK_ERR_NONE); len--)
		{
		    *sptr= fgetUchr(ifPtr, &error);
		    if (error == EK_ERR_NONE) sptr++;
		}
		*sptr= '\0';	/* add string terminator. */

		return error;
	    }
	}
}

/*****
 *	File find operations
 *****/

static
Flag file_exists(const char *fname)
{
  FILE *temp;

	if ( (temp= fopen(fname, "r")) == NULL )
	    return FL_FALSE;
	else
	{
	    fclose(temp);
	    return FL_TRUE;
	}
}

const char *find_file(const char *fname)
{	/* tempname won't be used if not required */
  char *colon;
  char *path= getenv("MODPATH");
  static char buffer[PATH_MAX];
  int len;

      /* first, check the current directory */
	if (file_exists(fname))
	    return fname;
	while(path)
	{
	    colon= strchr(path, ':');
	    if (colon)
		len= colon - path;
	    else
		len= strlen(path);
	    if (len < PATH_MAX)
	    {
		strncpy(buffer, path, len);
		buffer[len]= '/';
		if (len + strlen(fname) < PATH_MAX - 5)
		{
		    strcpy(buffer + len + 1, fname);
		    if (file_exists(buffer))
			return buffer;
		}
	    }
	    if (colon)
		path= colon + 1;
	    else
		return NULL;
	}
  return NULL;
}

/*****
 *	Compression handling
 *****/

typedef struct {
	char	*extn;	/* extension used */
	char	*patt;	/* pattern for extract command */
	} ARC;

static ARC
archive[]= {
	{ ".czip",	"unczip %s > %s"	},
	{ ".tgz",	"tar -xz %s %s"		}, /* GNU tar */
#ifdef HAS_GZIP	/* if FS is case insensitive, .z and .Z will match  */
	{ ".gz",	"gzip -dc %s > %s"	},
	{ ".z",		"gzip -dc %s > %s"	},
#else
	{ ".Z",		"zcat %s > %s"		},
#endif
	{ ".s",		"shorten -x %s > %s"	},
	{ ".shn",	"shorten -x %s > %s"	},
	{ ".zoo",	"zoo xpq %s > %s"	},
#ifdef AMIGA
	{ ".lzh",	"lha -q p %s > %s"	},
	{ ".lha",	"lha -q p %s > %s"	},
#else
	{ ".lzh",	"lha pq %s > %s"	},
	{ ".lha",	"lha pq %s > %s"	},
#endif
	{ ".zip",	"unzip -pq %s > %s"	},
	{ ".arc",	"arc pn %s > %s"	}, /* PC: PKunzip? */
	{ ".pp",	"ppunpack %s %s"	}, /* Extract syntax??? */
	{ NULL,		NULL			}
	};

static
int file_extn(const char *name, const char *extn)
{	/* NOT it, so it doesn't return zero when they match */
  return !strcasecmp(name+strlen(name)-strlen(extn), extn);
}

int compression(const char *argfile)
{
  int		idx= -1,
		arcIdx;

	for (arcIdx= 0; (idx == -1) && (archive[arcIdx].extn); arcIdx++)
	    if (file_extn(argfile, archive[arcIdx].extn))
		idx= arcIdx;

  return idx;
}

char *uncompress(const char *argfile, int compressID)
{
  char	cmdstr[PATH_MAX];	/* somewhat arbitrary choice :-( */
	sprintf(tempfile, "%s/ekvlXXXXXX", TEMP_DIR);

	sprintf(cmdstr, archive[compressID].patt, argfile, tempfile);
	system(cmdstr);
  return strdup(tempfile);
}


#if 0	/* load_s3m */
U_SHT fgetRUsh(FILE *ifPtr,errFlag *errPtr)
{
  U_SHT	i= fgetUch(ifPtr,errPtr);
  return (fgetUch(ifPtr,errPtr) << 8) | i;
}

U_LNG fgetRLong(FILE *ifPtr,errFlag *errPtr)
{
  U_LNG ul= 0;
  int	i;
	for (i=0; (i<4) && (!*errPtr); i++)
	    ul= ul + ( fgetUch(ifPtr,errPtr) * (1 << (8*i)) );
  return ul;
}



#endif	/* if 0 */
