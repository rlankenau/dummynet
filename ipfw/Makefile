#
# $Id: Makefile 5617 2010-03-04 13:23:58Z svn_magno $
#
# GNUMakefile to build the userland part of ipfw on Linux and Windows
#
# enable extra debugging information
# Do not set with = or := so we can inherit from the caller
OSARCH := $(shell uname)
$(warning Building userland ipfw for $(VER) $(OSARCH))

# common flags
EXTRA_CFLAGS += -O1
EXTRA_CFLAGS += -Wall
EXTRA_CFLAGS += -include ../glue.h
EXTRA_CFLAGS += -I ./include_e -I ./include

TARGET := ipfw
ifneq ($(VER),openwrt)
ifeq ($(OSARCH),Linux)
    EXTRA_CFLAGS += -D__BSD_VISIBLE
    EXTRA_CFLAGS += -Werror
else # must be Cygwin ?
ifeq ($(TCC),)
    EXTRA_CFLAGS += -I/cygdrive/c/WinDDK/7600.16385.0/inc/ddk
    EXTRA_CFLAGS += -I .
    EXTRA_CFLAGS += -pipe -Wall
else
    # TCC points to the root of tcc tree
    CC=$(TCC)/tcc
    EXTRA_CFLAGS += -DTCC -I..
    EXTRA_CFLAGS += -I$(TCC)/include/winapi -I$(TCC)/include
    EXTRA_CFLAGS += -nostdinc

    EDIRS  += arpa net netinet sys
    EFILES += err.h grp.h netdb.h pwd.h sysexits.h
    EFILES += arpa/inet.h
    EFILES += net/if.h
    EFILES += netinet/in.h netinet/in_systm.h netinet/ip.h
    EFILES += netinet/ip_icmp.h
    EFILES += sys/cdefs.h sys/wait.h
    EFILES += sys/ioctl.h sys/socket.h
endif
    # EXTRA_CFLAGS += -D_WIN32 # see who defines it
    EXTRA_CFLAGS += -Dsetsockopt=wnd_setsockopt
    EXTRA_CFLAGS += -Dgetsockopt=wnd_getsockopt
    EXTRA_CFLAGS += -DEMULATE_SYSCTL
    EDIRS  += net netinet
    EFILES += net/ethernet.h net/route.h
    EFILES += netinet/ether.h netinet/icmp6.h
    EFILES += sys/sysctl.h
    TARGET := ipfw.exe
endif
endif # !openwrt

CFLAGS += $(EXTRA_CFLAGS)
# Location of OS headers and libraries. After our stuff.
USRDIR?= /usr
ifeq ($(TCC),)
    CFLAGS += -I$(USRDIR)/include
    LDFLAGS += -L$(USRDIR)/lib
else
    LDFLAGS += -L$(TCC)/lib -ltcc1 -L. -lws2_32
endif

OBJS = ipfw2.o dummynet.o main.o ipv6.o qsort_r.o
OBJS += expand_number.o humanize_number.o glue.o

# we don't use ALTQ
CFLAGS += -DNO_ALTQ
#OBJS += altq.o

all: $(TARGET)
	echo "Done build for $(OSARCH) VER $(VER)"

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJS) : ipfw2.h ../glue.h include_e

# support to create empty dirs and files in include_e/
# EDIRS is the list of directories, EFILES is the list of files.
EDIRS   += sys netinet
EFILES  += sys/sockio.h libutil.h

M ?= $(shell pwd)

include_e:
	echo "running in $M"
	-@rm -rf $(M)/include_e opt_*
	-@mkdir -p $(M)/include_e
	-@(cd $(M)/include_e; mkdir -p $(EDIRS); touch $(EFILES) )
	-@(cd $(M)/include_e/netinet; \
		for i in ip_fw.h ip_dummynet.h tcp.h; do \
		cp ../../../dummynet2/include/netinet/$$i .; done; )

clean distclean:
	-rm -f $(OBJS) $(TARGET)
	-rm -rf include/netinet/ include_e
