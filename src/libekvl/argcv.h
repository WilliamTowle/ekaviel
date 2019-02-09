#ifndef __ARGCV_H__
#define __ARGCV_H__

void initArg(int argc, const char **argv);
int rewindArg();
int nextOpt(const char **optText, const char **optParam);
int nextArg(const char **argText);

#endif	/* __ARGCV_H__ */
