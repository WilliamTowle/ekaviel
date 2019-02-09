#!/usr/local/bin/gmake

# *****	makefile, ekaviel 1.54b, Wm.Towle
# *	audio object compilation
# *****

DEFS	= $(AUDDEFS)
INCL	= -I$(ROOTPATH)/$(LIBSRC)

default: $(ROOTPATH)/ekaudio.o

$(ROOTPATH)/ekaudio.o: ekaudio.c ekaudio.h
	$(CC) $(DEFS) -c ekaudio.c -o $(ROOTPATH)/ekaudio.o $(INCL)
