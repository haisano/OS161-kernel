# Makefile for sfsck

TOP=../../..
.include "$(TOP)/mk/os161.config.mk"

PROG=sfsck
SRCS=sfsck.c ../mksfs/disk.c ../mksfs/support.c
CFLAGS+=-I../mksfs
HOST_CFLAGS+=-I../mksfs
BINDIR=/sbin
HOSTBINDIR=/hostbin

.include "$(TOP)/mk/os161.prog.mk"
.include "$(TOP)/mk/os161.hostprog.mk"
