SIMBA_ROOT = /Users/kolen/items/simba
export PATH := $(PATH):/Users/kolen/.platformio/packages/toolchain-xtensa/bin/

NAME = pm_sensor
BOARD ?= nodemcu

INC += ../../libsds011/src
SRC += ../../libsds011/src/sds011.c sds011_thread.c

ifeq ($(BOARD),LINUX)
CC = /usr/local/bin/gcc-8
endif

include $(SIMBA_ROOT)/make/app.mk
