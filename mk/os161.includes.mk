#
# OS/161 build environment: include file installation.
#
# Set INCLUDES to a list of pairs:
#
#    source-dir destination-dir
#
# where the destination dir should begin with include/.
# All header files in each dir are copied.
#
# Optionally set INCLUDELINKS to a list of pairs
#
#    target linkname
#
# This will create symlinks target <- linkname. linkname
# should begin with include/. target usually won't.
#
#
# If you forcibly make install, header files are copied to $(OSTREE).
#

all depend install-staging clean distclean tags: ;

.for _SRC_ _DEST_ in $(INCLUDES)
MKDIRS+=$(INSTALLTOP)/$(_DEST_)
includes: $(INSTALLTOP)/$(_DEST_)

MKDIRS+=$(OSTREE)/$(_DEST_)
install: $(OSTREE)/$(_DEST_)
.endfor

includes:
.for _SRC_ _DEST_ in $(INCLUDES)
	$(TOP)/mk/installheaders.sh $(_SRC_) $(INSTALLTOP)/$(_DEST_)
.endfor
.for _TGT_ _LINK_ in $(INCLUDELINKS)
	ln -sf $(_TGT_) $(INSTALLTOP)/$(_LINK_)
.endfor

install:
.for _SRC_ _DEST_ in $(INCLUDES)
	$(TOP)/mk/installheaders.sh $(_SRC_) $(OSTREE)/$(_DEST_)
.endfor
.for _TGT_ _LINK_ in $(INCLUDELINKS)
	ln -sf $(_TGT_) $(INSTALLTOP)/$(_LINK_)
.endfor

.PHONY: all depend install-staging clean distclean tags
.PHONY: includes install

.include "$(TOP)/mk/os161.baserules.mk"


foo:
	echo $(OSTREE)

