# $Id: Makefile 5531 2010-02-26 14:45:45Z luigi $
#
# Top level makefile for building ipfw kernel and userspace.
# You can run it manually or also under the Planetlab build.
# Planetlab wants also the 'install' target.
#
# To build on system with non standard Kernel sources or userland files,
# you should run this with
#
#	make KERNELPATH=/path/to/linux-2.x.y.z USRDIR=/path/to/usr
#
# We assume that $(USRDIR) contains include/ and lib/ used to build userland.

DATE ?= $(shell date +%Y%m%d)
SNAPSHOT_NAME=$(DATE)-ipfw3.tgz
BINDIST=$(DATE)-dummynet-linux.tgz
WINDIST=$(DATE)-dummynet-windows.zip
_all: all

clean distclean:
	echo target is $(@)
	(cd ipfw && $(MAKE) $(@) )
	(cd dummynet2 && $(MAKE) $(@) )

all:
	echo target is $(@)
	(cd ipfw && $(MAKE) $(@) )
	(cd dummynet2 && $(MAKE) $(@) )
	# -- windows only
	- [ -f ipfw/ipfw.exe ] && cp ipfw/ipfw.exe binary/ipfw.exe
	- [ -f dummynet2/objchk_wxp_x86/i386/ipfw.sys ] && \
		cp dummynet2/objchk_wxp_x86/i386/ipfw.sys binary/ipfw.sys

snapshot:
	$(MAKE) distclean
	(cd ..; tar cvzhf /tmp/$(SNAPSHOT_NAME) --exclude .svn \
		--exclude README.openwrt --exclude tags --exclude NOTES \
		--exclude tcc-0.9.25-bsd \
		--exclude original_passthru \
		--exclude ipfw3.diff --exclude add_rules \
		ipfw3 )

bindist:
	$(MAKE) clean
	$(MAKE) all
	tar cvzf /tmp/$(BINDIST) ipfw/ipfw ipfw/ipfw.8 dummynet2/ipfw_mod.ko

windist:
	$(MAKE) clean
	-$(MAKE) all
	-rm /tmp/$(WINDIST)
	zip -r /tmp/$(WINDIST) binary -x \*.svn\*

install:
