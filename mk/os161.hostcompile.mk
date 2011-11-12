#
# OS/161 build environment: compile source files for the host system.
#
# Usage: use os161.hostprog.mk or os161.hostlib.mk
#
# Variables controlling this file:
#
# SRCS				.c and .S files to compile.
#
# Provides:
#
# HOST_OBJS			.ho files from compilation.
#

# Objects list starts empty. It is added to below.
HOST_OBJS=

# .ho is a host object.
.SUFFIXES: .ho

clean: cleanhostcompile
cleanhostcompile:
	rm -f $(MYBUILDDIR)/*.ho $(MYBUILDDIR)/*.ha

distclean: distcleanhostcompile
distcleanhostcompile:
	rm -f .hostdepend

#
# Depend: generate dependency information.
# Use gcc's -MM argument for this.
#
# Note that we use -MM rather than -M, so we don't get system headers.
# They would be host system headers and we don't want to get involved
# with those.
#
# The awk scripts and the first sed invocation transform the results to
# have one file per line.
# 
# The second sed command changes the .o filenames in gcc's output
# to .ho names.
#
# The third sed command replaces the value of $(INSTALLTOP) -- which
# is some pathname -- with the string $(INSTALLTOP). This makes the
# depend file independent of the value of $(INSTALLTOP).
#
# XXX: why the $p;$x? That seems like a no-op in this script... also,
# this logic is extremely opaque and could be done a lot better...
#
depend: dependhostcompile
dependhostcompile:
	$(HOST_CC) $(HOST_CFLAGS) -DHOST -MM $(SRCS) |\
	  awk '{x=$$0~"^ ";for(i=1;i<=NF;i++){printf "%d %s\n",x,$$i;x=1; }}'|\
	  sed '/1 \\/d' | awk '{ printf "%s%s", $$1?" \\\n ":"\n", $$2 }' |\
	  sed 's/\.o/\.ho/' |\
	  sed 's|$(INSTALLTOP)|$$(INSTALLTOP)|;$$p;$$x' |\
          sed 's|^\([^ ]\)|$$(MYBUILDDIR)/\1|' > .deptmp
	mv -f .deptmp .depend

# We do need to explicitly include .hostdepend, unlike .depend.
.-include ".hostdepend"

# No tags for host programs.
tags: tagshostcompile
tagshostcompile: ;

#
# Compile rules.
# We can use the same rules for .c and .S because gcc knows how to handle
# .S files.
#
.for _S_ in $(SRCS:M*.[cS])
HOST_OBJS+=$(MYBUILDDIR)/$(_S_:T:R).ho
$(MYBUILDDIR)/$(_S_:T:R).ho: $(_S_)
	$(HOST_CC) $(HOST_CFLAGS) -DHOST -c $(_S_) -o $(.TARGET)
.endfor

# Make non-file rules PHONY.
.PHONY: clean cleanhostcompile distclean distcleanhostcompile 
.PHONY: depend dependhostcompile tags tagshostcompile

# End.
