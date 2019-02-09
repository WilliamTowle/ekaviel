#include <stdio.h>

int argnum,args,initd=0;
const char **arglist;

void initArg(int argc, const char **argv)
{
	initd=1;
	args= argc; arglist= argv;
	argnum= 0;
}

int rewindArg()
{
  int success = 1;
	if (argnum>0)
	    argnum--;
	else
	    success=0;
  return success;
}

int nextOpt(const char **optText, const char **optParam)
{
  int option = 0;
	*optText = (*optParam = NULL);
	if (argnum+1 < args)
	    if (arglist[argnum+1][0] == '-')
	    {
		argnum++; option = 1;
		*optText = arglist[argnum]+1;
		if (argnum+1 < args)
		    if (arglist[argnum+1][0] != '-')
		    {
			argnum++;
			*optParam = arglist[argnum];
		    }
	    }
  return option;
}

int nextArg(const char **argText)
{
	if (argnum+1 < args)
	{
	    argnum++;
	    *argText = arglist[argnum];
	    return 1;
	}
	else
	{
	    *argText = NULL;
	    return 0;
	}
}
