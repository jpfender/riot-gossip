export CFLAGS += -g
export LDFLAGS += -g

valgrind: CFLAGS += -g

# name of your project
export PROJECT = riot-gossip
export RIOTBASE = $(CURDIR)/../../RIOT
export BOARD = native
export QUIET ?= 1

USEMODULE += auto_init
USEMODULE += config
USEMODULE += nativenet
USEMODULE += rtc
USEMODULE += transceiver
USEMODULE += vtimer
USEMODULE += random

#USEMODULE += ltc4150 # depends on hwtimer
#USEMODULE += uart0
#USEMODULE += posix

INCLUDES += -I./

include $(RIOTBASE)/Makefile.include
