#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	nds.m3loader
#$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)


#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path in windows
#---------------------------------------------------------------------------------
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

.PHONY: $(TARGET).arm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(TARGET).mse

#$(TARGET).ds.gba	: $(TARGET).nds

#---------------------------------------------------------------------------------
$(TARGET).mse	:	$(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -r9 0x02000450 -b $(TARGET).bmp "m3loader extlink"
	cp		$(TARGET).nds ../

#---------------------------------------------------------------------------------
$(TARGET).arm9	: arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	rm -f $(TARGET).ds.gba $(TARGET).mse $(TARGET).arm9 linkmaker nds.m3loader.nds
