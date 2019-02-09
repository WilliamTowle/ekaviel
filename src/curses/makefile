#!/usr/local/bin/gmake

# *****	makefile, ekaviel 1.54b, Wm.Towle
# *	audio object compilation
# *****

DEFS	= $(GUIDEFS)
INCL	= -I$(ROOTPATH)/$(LIBPATH)

default: $(ROOTPATH)/ekgui.o

$(ROOTPATH)/ekgui.o: $(ROOTPATH)/makefile ekgui.c ekgui.h
	$(CC) $(DEFS) -c ekgui.c $(INCL)	\
		-o $(ROOTPATH)/ekgui.o
