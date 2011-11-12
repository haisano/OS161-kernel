#
# Makefile for hostcompat library
#
# defs.mk contains two special settings for us.
#
#   COMPAT_CFLAGS  contains our configuration cflags.
#   COMPAT_TARGETS are additional targets to run at install time.
#
# COMPAT_CFLAGS may include any of the following:
#
#   -DNEED_ERR     Compile err, errx, etc.
#
# COMPAT_TARGETS may include any of the following:
#
#   install-errh   Install an <err.h>
#

TOP=../../..
.include "$(TOP)/mk/os161.config.mk"

LIB=hostcompat
SRCS=err.c time.c hostcompat.c

CFLAGS+=$(COMPAT_CFLAGS)

MKDIRS=$(INSTALLTOP)/hostinclude

.include "$(TOP)/mk/os161.hostlib.mk"

includes: $(INSTALLTOP)/hostinclude .WAIT $(COMPAT_TARGETS)
	cp hostcompat.h $(INSTALLTOP)/hostinclude/
	ln -sf ../include/kern $(INSTALLTOP)/hostinclude/kern

install-errh:
	cp host-err.h $(INSTALLTOP)/hostinclude/err.h

.PHONY: includes install-errh
