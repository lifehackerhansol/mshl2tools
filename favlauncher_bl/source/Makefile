#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	favlauncher_bl
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
all: $(TARGET).nds

#$(TARGET).ds.gba	: $(TARGET).nds

#---------------------------------------------------------------------------------
$(TARGET).nds	:	$(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "FavLauncher;with external bootlib;devkitARMr31"
	cp	$(TARGET).nds akmenu4.nds
	dldipatch	rpgs.dldi akmenu4.nds
	ndstool	-c msforsc.nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "FavLauncher;with external bootlib;devkitARMr31" -g ENG0 00 MSFORSC
	dldipatch	dsone.dldi msforsc.nds
	cp	$(TARGET).nds msforsc.nds akmenu4.nds ../

#---------------------------------------------------------------------------------
$(TARGET).arm9	: arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	rm -f $(TARGET).ds.gba msforsc.nds akmenu4.nds $(TARGET).nds $(TARGET).arm9
