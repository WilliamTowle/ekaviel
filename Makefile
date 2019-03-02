#!/usr/bin/make
# last mod WmT, 2009-03-02	[ (c) and GPLv2 2007-2019 ]

TOPLEV:=$(shell pwd)
SRCDIR:=${TOPLEV}/src
BINDIR:=${TOPLEV}/build-temp

HAVE_CONFIG_MH:=$(shell if [ -r config.mh ] ; then echo y ; else echo n ; fi)
ifeq (${HAVE_CONFIG_MH},y)
include config.mh
endif

# ,-----
# |	Assistance
# +-----

.PHONY: help
default: help

USAGE_RULES:=
EXE_TARGETS:=

include ${SRCDIR}/ekdump.mh
include ${SRCDIR}/eksfplay.mh
include ${SRCDIR}/ekaviel.mh

help:
ifneq (${HAVE_CONFIG_MH},y)
	@echo "WARNING - Configuration file '${CONFIG}' is missing" 1>&2
endif
ifeq (${USAGE_RULES},)
	@echo "This 'Makefile' lacks descriptive help. Rules are:"
	@grep '^[a-z][a-z]*:' Makefile | sed 's/^/	/ ; s/:.*//' | sort -u
else
	@echo "This 'Makefile' has rules for:"
	@for R in ${USAGE_RULES} ; do printf "\t$$R\n" ; done
endif

USAGE_RULES+= "all - build all of ${EXE_TARGETS}"
all:	${EXE_TARGETS}

# ,-----
# |	Misc
# +-----

ifneq (${BINDIR},${TOPLEV})
${BINDIR}:
	mkdir -p ${BINDIR}
endif


# ,-----
# |	Clean
# +-----

USAGE_RULES+= "clean - remove temporary files"
USAGE_RULES+= "realclean - clean as per fresh tarball extract"

.PHONY: clean
clean:
	-rm -f ${CLEAN_TARGETS}

realclean: clean
	-rm -f ${REALCLEAN_TARGETS}
#	-rm -f scripts/detect.cache
