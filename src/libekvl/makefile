#!/usr/local/bin/gmake

# *****	makefile, ekaviel 1.54b, Wm.Towle
# *	audio object compilation
# *****

LIBSRC	= .
INCL	= -I$(LIBSRC)

AUDSRC	= $(ROOTPATH)/$(AUDPATH)
AUDINCL	= -I$(AUDSRC)
#GUIINCL	= -I$(ROOTPATH)/$(GUISRC)

OBJS	= $(ROOTPATH)/argcv.o	$(ROOTPATH)/effects.o	\
	$(ROOTPATH)/ekinstr.o	$(ROOTPATH)/ekio.o	\
	$(ROOTPATH)/eklist.o	$(ROOTPATH)/eknote.o	\
	$(ROOTPATH)/ekprefs.o	$(ROOTPATH)/eksong.o	\
	$(ROOTPATH)/ektype.o	$(ROOTPATH)/fwrite.o	\
	$(ROOTPATH)/loadbase.o	\
	$(ROOTPATH)/load_med.o	$(ROOTPATH)/load_st.o	\
	$(ROOTPATH)/player.o	$(ROOTPATH)/vchannel.o

# *****

default: $(ROOTPATH)/lib$(LIBNAME).a

$(ROOTPATH)/lib$(LIBNAME).a: $(OBJS)
	ar ruvs $(ROOTPATH)/lib$(LIBNAME).a $(OBJS)

# *****

$(ROOTPATH)/argcv.o: $(ROOTPATH)/makefile argcv.c argcv.h
	$(CC) $(DEFS) -c argcv.c -o $(ROOTPATH)/argcv.o
$(ROOTPATH)/effects.o: $(ROOTPATH)/makefile effects.c effects.h	\
		$(LIBSRC)/ekmalloc.h	$(LIBSRC)/eknote.h	\
		$(LIBSRC)/ekprefs.h	$(LIBSRC)/ektype.h	\
		$(LIBSRC)/vchannel.h
	$(CC) $(DEFS) -c $(LIBSRC)/effects.c -o $(ROOTPATH)/effects.o
$(ROOTPATH)/ekinstr.o: $(ROOTPATH)/makefile ekinstr.c ekinstr.h
	$(CC) $(DEFS) $(AUDDEFS)			\
		-c ekinstr.c -o $(ROOTPATH)/ekinstr.o	\
		$(INCL) $(AUDINCL)
$(ROOTPATH)/ekio.o: $(ROOTPATH)/makefile ekio.c ekio.h		\
		$(AUDSRC)/ekaudio.h	$(LIBSRC)/ekinstr.h	\
		$(LIBSRC)/eknote.h	$(LIBSRC)/ektype.h	\
		$(LIBSRC)/vchannel.h
	$(CC) $(DEFS) $(AUDDEFS) $(GUIDEFS)		\
		-c ekio.c -o $(ROOTPATH)/ekio.o		\
		$(INCL) $(AUDINCL) $(GUIINCL)
$(ROOTPATH)/eklist.o: $(ROOTPATH)/makefile eklist.c eklist.h	\
		$(LIBSRC)/ekmalloc.h	$(LIBSRC)/ektype.h
	$(CC) $(DEFS) -c eklist.c -o $(ROOTPATH)/eklist.o
$(ROOTPATH)/eknote.o: $(ROOTPATH)/makefile eknote.c eknote.h	\
		$(LIBSRC)/ekmalloc.h	$(LIBSRC)/ektype.h
	$(CC) $(DEFS) -c eknote.c -o $(ROOTPATH)/eknote.o
$(ROOTPATH)/ekprefs.o: $(ROOTPATH)/makefile ekprefs.c ekprefs.h	\
		$(AUDSRC)/ekaudio.h	$(LIBSRC)/ekmalloc.h	\
		$(LIBSRC)/eksong.h	$(LIBSRC)/ektype.h	\
		$(LIBSRC)/vchannel.h
	$(CC) $(DEFS) $(AUDDEFS)			\
		-c ekprefs.c -o $(ROOTPATH)/ekprefs.o	\
		$(INCL) $(AUDINCL)
$(ROOTPATH)/eksong.o: $(ROOTPATH)/makefile eksong.c eksong.h 	\
		$(LIBSRC)/effects.h	$(LIBSRC)/ekinstr.h	\
		$(LIBSRC)/ekmalloc.h	$(LIBSRC)/eknote.h	\
		$(LIBSRC)/ektype.h	$(LIBSRC)/load_st.h	\
		$(LIBSRC)/load_med.h
	$(CC) $(DEFS) -c $(LIBSRC)/eksong.c -o $(ROOTPATH)/eksong.o
$(ROOTPATH)/ektype.o: $(ROOTPATH)/makefile ektype.c ektype.h
	$(CC) $(DEFS) -c ektype.c -o $(ROOTPATH)/ektype.o
$(ROOTPATH)/fwrite.o: $(ROOTPATH)/makefile fwrite.c fwrite.h	\
		$(LIBSRC)/ekinstr.h	$(LIBSRC)/ekmalloc.h	\
		$(LIBSRC)/ektype.h
	$(CC) $(DEFS) -c $(LIBSRC)/fwrite.c -o $(ROOTPATH)/fwrite.o
$(ROOTPATH)/loadbase.o: $(ROOTPATH)/makefile loadbase.c loadbase.h \
		$(LIBSRC)/ekmalloc.h	$(LIBSRC)/ektype.h
	$(CC) $(DEFS) -c loadbase.c -o $(ROOTPATH)/loadbase.o
$(ROOTPATH)/load_med.o: $(ROOTPATH)/makefile load_med.c load_med.h \
		$(LIBSRC)/ekinstr.h	$(LIBSRC)/eksong.h	\
		$(LIBSRC)/ektype.h	$(LIBSRC)/loadbase.h	\
		$(LIBSRC)/load_st.h
	$(CC) $(DEFS) -c $(LIBSRC)/load_med.c -o $(ROOTPATH)/load_med.o
$(ROOTPATH)/load_st.o: $(ROOTPATH)/makefile load_st.c load_st.h	\
		$(LIBSRC)/effects.h	$(LIBSRC)/ekinstr.h	\
		$(LIBSRC)/ekmalloc.h	\
		$(LIBSRC)/eknote.h	$(LIBSRC)/eksong.h	\
		$(LIBSRC)/ektype.h	$(LIBSRC)/loadbase.h
	$(CC) $(DEFS) -c $(LIBSRC)/load_st.c -o $(ROOTPATH)/load_st.o
$(ROOTPATH)/player.o: $(ROOTPATH)/makefile player.c player.h	\
		$(LIBSRC)/ekio.h	\
		$(LIBSRC)/eknote.h	\
		$(LIBSRC)/ekprefs.h	$(LIBSRC)/eksong.h	\
		$(LIBSRC)/ektype.h	\
		$(LIBSRC)/vchannel.h
	$(CC) $(DEFS) -c $(LIBSRC)/player.c -o $(ROOTPATH)/player.o
$(ROOTPATH)/vchannel.o: $(ROOTPATH)/makefile vchannel.c vchannel.h \
		$(LIBSRC)/ekio.h				\
		$(LIBSRC)/effects.h	$(AUDSRC)/ekaudio.h	\
		$(LIBSRC)/ekinstr.h	$(LIBSRC)/ekmalloc.h	\
		$(LIBSRC)/eknote.h	$(LIBSRC)/ekprefs.h	\
		$(LIBSRC)/ektype.h
	$(CC) $(DEFS) $(AUDDEFS)			\
		-c vchannel.c -o $(ROOTPATH)/vchannel.o	\
		$(INCL) $(AUDINCL)
