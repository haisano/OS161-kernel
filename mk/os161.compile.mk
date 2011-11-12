#
# OS/161 build environment: compile source files.
#
# Usage: use os161.prog.mk or os161.lib.mk
#
# Variables controlling this file:
#
# SRCS				.c and .S files to compile.
#
# Provides:
#
# OBJS				.o files from compilation.
#

# Objects list starts empty. It is added to below.
OBJS=

clean: cleancompile
cleancompile:
	rm -f $(MYBUILDDIR)/*.[oa]

distclean: distcleancompile
distcleancompile:
	rm -f .depend

#
# Depend: generate dependency information.
# Use gcc's -M argument for this.
#
# Note that we use -M rather than -MM, to get both system headers
# and program-local headers. This is because we *are* the system and
# we might well change those system headers.
#
# The awk scripts and the first sed invocation transform the results to
# have one file per line.
# 
# The second sed command replaces the value of $(INSTALLTOP) -- which
# is some pathname -- with the string $(INSTALLTOP). This makes the
# depend file independent of the value of $(INSTALLTOP).
#
# XXX: why the $p;$x? That seems like a no-op in this script... also,
# this logic is extremely opaque and could be done a lot better...
#
depend: dependcompile
dependcompile:
	$(CC) $(CFLAGS) $(MORECFLAGS) -M $(SRCS) |\
	  awk '{x=$$0~"^ ";for(i=1;i<=NF;i++){printf "%d %s\n",x,$$i;x=1; }}'|\
	  sed '/1 \\/d' | awk '{ printf "%s%s", $$1?" \\\n ":"\n", $$2 }' |\
	  sed 's|$(INSTALLTOP)|$$(INSTALLTOP)|;$$p;$$x' |\
          sed 's|^\([^ ]\)|$$(MYBUILDDIR)/\1|' > .deptmp
	mv -f .deptmp .depend

# We don't need to explicitly include .depend; our make does this on its own.
#.-include ".depend"

tags: tagscompile
tagscompile:
	ctags -wtd $(SRCS) *.h

#
# Compile rules.
# We can use the same rules for .c and .S because gcc knows how to handle
# .S files.
#
# Make it so typing "make foo.o" does the right thing.
#
.for _S_ in $(SRCS:M*.[cS])
OBJS+=$(MYBUILDDIR)/$(_S_:T:R).o
$(MYBUILDDIR)/$(_S_:T:R).o: $(_S_)
	$(CC) $(CFLAGS) $(MORECFLAGS) -c $(_S_) -o $(.TARGET)

$(_S_:T:R).o: $(MYBUILDDIR)/$(_S_:T:R).o
.PHONY: $(_S_:T:R).o
.endfor

# Make non-file rules PHONY.
.PHONY: clean cleancompile distclean distcleancompile 
.PHONY: depend dependcompile tags tagscompile

# End.
