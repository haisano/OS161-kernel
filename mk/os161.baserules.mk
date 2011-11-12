#
# OS/161 build environment: some very basic rules.
#
# Individual program makefiles should use os161.prog.mk or
# os161.lib.mk instead of including this file directly.
#
# The variable MKDIRS is used to generate rules for creating
# (mostly installation) directories via os161.mkdirs.mk.

# Process this file only once even if included repeatedly
.if !defined(_BASERULES_MK_)
_BASERULES_MK_=# empty


#
# Establish that all these (basic) rules exist.
#
all depend install install-staging clean distclean tags: ;

# distclean implies clean
distclean: clean

.PHONY: all depend install install-staging clean distclean tags

#
# Some other derived rules.
#

# cleandir is the same as distclean (cleandir is the old BSD name)
cleandir: distclean

# "stage" is a good short name for install-staging
stage: install-staging

# dependall means depend then compile
dependall: depend .WAIT all

# build means depend, compile, and install-staging
build: dependall .WAIT install-staging

# rebuild cleans first
rebuild: clean .WAIT build

# fullrebuild does distclean
fullrebuild: distclean .WAIT build

# implement BUILDSYMLINKS
.if "$(BUILDSYMLINKS)" == "yes"
.if !exists(build)
all depend: buildlink
.endif
buildlink:
	ln -s $(MYBUILDDIR) build
clean: remove-buildlink
remove-buildlink:
	rm -f build
.PHONY: buildlink remove-buildlink
.endif

.PHONY: cleandir stage dependall build rebuild fullrebuild

.endif # _BASERULES_MK_

.include "$(TOP)/mk/os161.mkdirs.mk"

# End.
