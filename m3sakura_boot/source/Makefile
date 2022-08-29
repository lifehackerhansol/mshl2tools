#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	m3sakura
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
$(TARGET).mse	:	$(TARGET).arm9 linkmaker
	ndstool	-c $(TARGET).mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -r9 0x02000450 -b $(TARGET).bmp "m3sakura DLDI boot;for multicart"
	linkmaker	$(TARGET).mse
	mselink	$(TARGET).mse /m3sakura.dat m3sakura.nds
	cp		m3sakura.nds ../

linkmaker:	linkmaker.c
	gcc -o linkmaker -O2 linkmaker.c

#---------------------------------------------------------------------------------
$(TARGET).arm9	: arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	rm -f $(TARGET).ds.gba $(TARGET).mse $(TARGET).arm9 linkmaker m3sakura.nds
