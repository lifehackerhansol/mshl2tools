#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	reset
#$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)


#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path in windows
#---------------------------------------------------------------------------------
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

.PHONY: $(TARGET).arm7 $(TARGET).arm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(TARGET).mse

#$(TARGET).ds.gba	: $(TARGET).nds

#---------------------------------------------------------------------------------
$(TARGET).mse	:	$(TARGET).arm7 $(TARGET).arm9 linkmaker
	ndstool	-c $(TARGET).mse -7 $(TARGET).arm7 -9 $(TARGET).arm9 -b $(TARGET).bmp "reset_mse_06b dldi;devkitARMr27;Very fast reset/loader"
	linkmaker	$(TARGET).mse
	ndstool	-c moonshl2alt.mse -7 $(TARGET).arm7 -9 $(TARGET).arm9 -b moonshl2alt.bmp "MoonShell2;Alternative Loader;devkitARMr21"
	linkmaker	moonshl2alt.mse
	mselink	moonshl2alt.mse /moonshl2/reload.dat moonshl2alt.nds
	rm		moonshl2alt.mse
	cp		$(TARGET).mse moonshl2alt.nds ../

linkmaker:	linkmaker.c
	gcc -o linkmaker -O2 linkmaker.c

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
	rm -f $(TARGET).ds.gba $(TARGET).mse $(TARGET).arm7 $(TARGET).arm9 linkmaker RPGS.nds R4DS.nds moonshl2alt.nds
