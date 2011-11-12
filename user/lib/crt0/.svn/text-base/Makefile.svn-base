#
# Makefile for crt0.o
#

TOP=../../..
.include "$(TOP)/mk/os161.config.mk"

SRCS=$(MACHINE)/crt0.S
MKDIRS=$(INSTALLTOP)/lib $(OSTREE)/lib $(MYBUILDDIR)

.include "$(TOP)/mk/os161.baserules.mk"
.include "$(TOP)/mk/os161.compile.mk"

all: $(MYBUILDDIR) .WAIT $(OBJS)

install-staging: $(INSTALLTOP)/lib
	cp $(OBJS) $(INSTALLTOP)/lib/

install: $(OSTREE)/lib
	cp $(OBJS) $(OSTREE)/lib/

