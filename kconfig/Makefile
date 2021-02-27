# ===========================================================================
# Kernel configuration targets
# These targets are used from top-level makefile

.PHONY = menuconfig clean

HOSTCC = cc

CFLAGS += $(HOST_CFLAGS) $(HOST_EXTRACFLAGS) -pipe

# lxdialog stuff
check-lxdialog  := ./lxdialog/check-lxdialog.sh

# Use recursively expanded variables so we do not call gcc unless
# we really need to do so. (Do not call gcc as part of make mrproper)
HOST_EXTRACFLAGS += $(shell /usr/bin/bash $(check-lxdialog) -ccflags) -DLOCALE

# ===========================================================================
# Shared Makefile for the various kconfig executables:
# conf:	  Used for defconfig, oldconfig and related targets
# nconf:  Used for the nconfig target.
#         Utilizes ncurses
# mconf:  Used for the menuconfig target
#         Utilizes the lxdialog package
# qconf:  Used for the xconfig target
#         Based on Qt which needs to be installed to compile it
# gconf:  Used for the gconfig target
#         Based on GTK+ which needs to be installed to compile it
# object files used by all kconfig flavours

lxdialog := lxdialog/checklist.o lxdialog/util.o lxdialog/inputbox.o
lxdialog += lxdialog/textbox.o lxdialog/yesno.o lxdialog/menubox.o

mconf-objs = mconf.o zconf.tab.o $(lxdialog)

# Check that we have the required ncurses stuff installed for lxdialog (menuconfig)
.PHONY += dochecklxdialog
dochecklxdialog:
	/usr/bin/bash $(check-lxdialog) -check $(HOSTCC) $(HOST_EXTRACFLAGS) $(HOSTLOADLIBES_mconf)

always := dochecklxdialog

# Add environment specific flags
HOST_EXTRACFLAGS += $(shell /usr/bin/bash check.sh $(HOSTCC) $(HOSTCFLAGS))

# generated files seem to need this to find local include files
HOSTCFLAGS_zconf.lex.o	:= -I.
HOSTCFLAGS_zconf.tab.o	:= -I.

LEX_PREFIX_zconf	:= zconf
YACC_PREFIX_zconf	:= zconf

HOSTLOADLIBES_mconf   = $(shell /usr/bin/bash $(check-lxdialog) -ldflags $(HOSTCC))

zconf.tab.o: zconf.lex.c zconf.hash.c

menuconfig: $(mconf-objs)
	$(CC) $(HOSTLOADLIBES_mconf) -o mconf $(mconf-objs)

clean:
	rm -rf $(mconf-objs)
	rm -f mconf
