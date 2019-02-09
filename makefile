#!/usr/local/bin/gmake

# *****
# *	User-defined configuration
# *****

# CC is prone to giving silly warnings. Use gcc(g++) over cc
# [UoL SCS]: Use "gcc-32" to compile on O2 boxes.
# NB. Public users probably don't need -Wall, and definitely no -D...
CCE	=	`./ekconfig --compiler` -g -Wall -DDEVEL -DOCTAVES=4
CC	=	$(CCE)

# *****
# *	Internal configuration
# *****

AUDSRC	=	`./ekconfig --audsrc`
EXESRC	=	src
GUISRC	=	src/curses
LIBSRC	=	src/libekvl

AUDOBJ	=	ekaudio.o
GUIOBJ	=	ekgui.o
LIBOBJ	=	argcv.o effects.o eklist.o \
		ekprefs.o eknote.o eksong.o ektype.o		\
		fwrite.o loadbase.o load_med.o load_st.o	\
		player.o

AUDDEFS	= -DDEFAULT_OUTRATE=22050
#GUIDEFS = -DHAS_EKGUI
GUIDEFS = -DHAS_EKGUI
#IODEFS	= -DSLOW_DISPLAY
IODEFS	=
#OSDEFS	= -DIRIX4 -DAMIGA -DHAS_GZIP
OSDEFS	= -DHAS_GZIP
#DEFS	= -DSLOWCPU
DEFS	= $(IODEFS) $(OSDEFS)

INCL	= -I$(LIBSRC)
AUDINCL	= -I$(AUDSRC)
GUIINCL	= -I$(GUISRC)

LIBNAME	= ekvl
LIBS	= -L. -l$(LIBNAME) -lm
AUDLIBS	= `./ekconfig --audlibs`
GUILIBS	= -lcurses

# *****
# *	Recipes start here
# *****

default:	ekdump
#default:	all

clean	:
	-/bin/rm *.o core 1>/dev/null 2>&1
vclean	: clean
	-/bin/rm lib$(LIBNAME).a ekaviel ekdump eksfplay 1>/dev/null 2>&1

all: 		ekaviel ekdump eksfplay

# *****
# *	Final executables
# *****

ekaviel: ekaviel.o audobj guiobj libobj
	$(CCE) $(AUDDEFS) ekaviel.o $(AUDOBJ) $(GUIOBJ)	\
			$(LIBS) $(AUDLIBS) $(GUILIBS) -o ekaviel

ekdump: ekdump.o libobj
	$(CCE) ekdump.o $(LIBS) -o ekdump

eksfplay: eksfplay.o audobj libobj
	$(CCE) $(AUDDEFS) eksfplay.o $(AUDOBJ)	\
			$(LIBS) $(AUDLIBS) -o eksfplay

# *****
# *	"Mid-end" objects
# *****


audobj:
	$(MAKE) -C $(AUDSRC) ROOTPATH=`pwd`			\
		CC="$(CC)" AUDDEFS="$(AUDDEFS)" LIBSRC=$(LIBSRC)

libobj:
	$(MAKE) -C $(LIBSRC) ROOTPATH=`pwd` LIBNAME=$(LIBNAME)	\
		CC="$(CC)" AUDPATH=$(AUDSRC)

guiobj:
	$(MAKE) -C $(GUISRC) ROOTPATH=`pwd`			\
		CC="$(CC)" LIBPATH=$(LIBSRC)

# *****
# *	"Front-end" objects
# ****h

ekaviel.o: makefile $(EXESRC)/ekaviel.c $(EXESRC)/ekaviel.h	\
		$(LIBSRC)/argcv.h	\
		$(LIBSRC)/eklist.h	\
		$(LIBSRC)/ekmalloc.h	$(LIBSRC)/ekprefs.h	\
		$(LIBSRC)/eksong.h	\
		$(LIBSRC)/ektype.h	\
		$(GUISRC)/ekgui.h	\
		$(LIBSRC)/ekio.h	\
		$(LIBSRC)/loadbase.h	\
		$(LIBSRC)/player.h	\
		$(LIBSRC)/vchannel.h
	$(CC) $(DEFS) $(GUIDEFS) -c $(EXESRC)/ekaviel.c \
			$(INCL) $(GUIINCL)

ekdump.o: makefile $(EXESRC)/ekdump.c $(EXESRC)/ekdump.h	\
		$(LIBSRC)/argcv.h	$(LIBSRC)/eklist.h	\
		$(LIBSRC)/ekmalloc.h	$(LIBSRC)/eksong.h	\
		$(LIBSRC)/ektype.h	$(LIBSRC)/fwrite.h
	$(CC) $(DEFS) -c $(EXESRC)/ekdump.c $(INCL)

eksfplay.o: makefile $(EXESRC)/eksfplay.c $(EXESRC)/eksfplay.h	\
		$(LIBSRC)/argcv.h	\
		$(LIBSRC)/eklist.h	$(LIBSRC)/ekmalloc.h	\
		$(LIBSRC)/eknote.h				\
		$(LIBSRC)/ekprefs.h	$(LIBSRC)/ektype.h	\
		$(LIBSRC)/vchannel.h
	$(CC) $(DEFS) -c $(EXESRC)/eksfplay.c $(INCL)

#X# *****
#X# *	Library objects/creation
#X# *****
#X
#Xlib$(LIBNAME).a: $(LIBOBJ)
#X	$(MAKE) -C $(LIBSRC) ROOTPATH=`pwd` CC="$(CC)"		\
#X		AUDDEFS="$(AUDDEFS)" AUDSRC="$(AUDSRC)"		\
#X		GUIDEFS="$(GUIDEFS)" GUISRC="$(GUISRC)"
#X	ar ruvs lib$(LIBNAME).a $(LIBOBJ)
