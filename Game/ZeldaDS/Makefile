#---------------------------------------------------------------------------------
# ROOT MAKEFILE
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

GAME_TITLE     := ZeldaDS
GAME_SUBTITLE1 := Antonio Maiorano (daroou)
GAME_SUBTITLE2 := www.firebell.org/guardiansoft
#TODO: Make an icon...
GAME_ICON      := $(DEVKITPRO)/libnds/icon.bmp

# Include Nitro FS folder (structure + content will be packed into rom)
NITRODATA	:=	nitrofiles
_ADDFILES	:=	-d $(CURDIR)/$(NITRODATA)

# As long as this var is set, compiler errors will be formatted for visual studio
export VCBUILDHELPER_COMMAND := whatever

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# CONFIG-specific values
#---------------------------------------------------------------------------------
TARGET_NAME := $(shell basename $(CURDIR))

# If user passes in anything but CONFIG=DEBUG, we build RELEASE
# NOTE: Even though we set CONFIG here, if CONFIG=<value> is passed in to make, it overrides this assignment
CONFIG := RELEASE

ifeq ($(CONFIG), DEBUG)
	TARGET_NAME := $(TARGET_NAME)_d
endif


#---------------------------------------------------------------------------------
# export vars for other makefiles
#---------------------------------------------------------------------------------
export TARGET		:=	$(TARGET_NAME)
export TOPDIR		:=	$(CURDIR)



.PHONY: $(TARGET).arm7 $(TARGET).arm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(TARGET).nds

#---------------------------------------------------------------------------------
$(TARGET).nds	:	$(TARGET).arm7 $(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 $(TARGET).arm7 -9 $(TARGET).arm9 -b $(GAME_ICON) "$(GAME_TITLE);$(GAME_SUBTITLE1);$(GAME_SUBTITLE2)" $(_ADDFILES)

#---------------------------------------------------------------------------------
$(TARGET).arm7	: arm7/$(TARGET).elf
$(TARGET).arm9	: arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	$(MAKE) -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	$(MAKE) -C arm7 clean
	rm -f $(TARGET).nds $(TARGET).arm7 $(TARGET).arm9

#---------------------------------------------------------------------------------
printsize:
	@echo.
	@echo Elf size breakdown:
	$(DEVKITARM)/bin/arm-eabi-size.exe arm9/$(TARGET).arm9.elf arm7/$(TARGET).arm7.elf
