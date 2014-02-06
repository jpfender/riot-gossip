export CFLAGS += -g
export LDFLAGS += -g

valgrind: CFLAGS += -g

# name of your project
export PROJECT = test_vtimer
export RIOTBASE = $(CURDIR)/../../../../RIOT
export QUIET ?= 1
export BOARD ?= msba2

USEMODULE += vtimer
USEMODULE += auto_init

ifneq (,$(findstring msba2,$(BOARD)))
    USEMODULE += msba2_base
    export INCLUDES += -I$(RIOTBASE)/drivers/cc110x_ng/include
endif

INCLUDES += -I./ -I$(RIOTBASE)/sys/include -I$(RIOTBASE)/drivers/include

include $(RIOTBASE)/Makefile.include
