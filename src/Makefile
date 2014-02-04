export CFLAGS += -g
export LDFLAGS += -g

valgrind: CFLAGS += -g

# name of your project
export PROJECT = riot-gossip
export RIOTBASE = $(CURDIR)/../../RIOT
export QUIET ?= 1
export BOARD = msba2

USEMODULE += random
USEMODULE += vtimer
USEMODULE += rtc
USEMODULE += auto_init

ifneq (,$(findstring msba2,$(BOARD)))
    USEMODULE += cc110x_ng
    export INCLUDES += -I$(RIOTBASE)/drivers/cc110x_ng/include
    USEMODULE += msba2_base
endif

ifneq (,$(findstring avsextrem,$(BOARD)))
    USEMODULE += cc110x_ng
    export INCLUDES += -I$(RIOTBASE)/drivers/cc110x_ng/include
endif


ifneq (,$(findstring native,$(BOARD)))
    USEMODULE += nativenet
    USEMODULE += transceiver
endif


#USEMODULE += ltc4150 # coulomb counter, depends on hwtimer
#USEMODULE += uart0
#USEMODULE += posix
#USEMODULE += config  # stores config on board/flash

INCLUDES += -I./ -I$(RIOTBASE)/sys/include -I$(RIOTBASE)/drivers/include

include $(RIOTBASE)/Makefile.include
