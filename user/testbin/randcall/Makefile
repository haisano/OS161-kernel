# Makefile for badcall

TOP=../../..
.include "$(TOP)/mk/os161.config.mk"

PROG=randcall
SRCS=calls.c main.c
BINDIR=/testbin

.include "$(TOP)/mk/os161.prog.mk"

calls.c: gencalls.sh callspecs.txt
	./gencalls.sh callspecs.txt > calls.c

depend: predepend
predepend:
	$(MAKE) calls.c

clean: myclean
myclean:
	rm -f calls.c

.PHONY: predepend myclean
