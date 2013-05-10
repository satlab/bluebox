#
#             LUFA Library
#     Copyright (C) Dean Camera, 2012.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Use micro for BlueBox micro
BBBOARD     ?= standard

# Run "make help" for target help.
ARCH         = AVR8
BOARD        = USER
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = bluebox
SRC          = $(TARGET).c Descriptors.c bootloader.c spi.c adf7021.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = LUFA
CC_FLAGS    += -DUSE_LUFA_CONFIG_HEADER -IConfig/ -Wall -Wextra -Wno-unused-parameter
LD_FLAGS     =

ifeq ($(BBBOARD),standard)
MCU          = atmega32u4
CC_FLAGS    += -DBBSTANDARD
endif
ifeq ($(BBBOARD),micro)
MCU          = atmega32u2
CC_FLAGS    += -DBBMICRO
endif

CC_FLAGS    += -DFW_REVISION="\"$(shell git describe --abbrev=7 --dirty=+ --always)\""

# Default target
all:

program: all
	dfu-programmer $(MCU) erase 
	dfu-programmer $(MCU) flash $(TARGET).hex
	dfu-programmer $(MCU) start

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
