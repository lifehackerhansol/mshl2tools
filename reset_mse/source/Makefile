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

.PHONY: $(TARGET).arm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: $(TARGET).mse

#$(TARGET).ds.gba	: $(TARGET).nds

#---------------------------------------------------------------------------------
$(TARGET).mse	:	$(TARGET).arm9 linkmaker r4isdhc
	r4isdhc	$(TARGET).arm9 $(TARGET).r4isdhc.arm9
	ndstool	-c $(TARGET).mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "reset_mse_06b dldi;devkitARMr31;Very fast reset/loader"
	linkmaker	$(TARGET).mse
	cp		$(TARGET).mse R4DS.mse
	cp		$(TARGET).mse TTIO.mse
	cp		$(TARGET).mse RPGS.mse
	dldipatch	r4ds.dldi R4DS.mse
	mselink	R4DS.mse /_ds_menu.dat R4DS.nds
	dldipatch	ttio.dldi TTIO.mse
	mselink	TTIO.mse /ttmenu.dat TTIO.nds
	dldipatch	rpgs.dldi RPGS.mse
	mselink	RPGS.mse /akmenu4.nds RPGS.nds
	ndstool	-c DEMO.mse -7 ../../arm7/arm7.bin -9 $(TARGET).r4isdhc.arm9 -e9 0x02000000 -b $(TARGET).bmp "reset_mse_06b dldi;devkitARMr31;Very fast reset/loader"
	linkmaker	DEMO.mse
	dldipatch	demo.dldi DEMO.mse
	mselink	DEMO.mse /r4.dat DEMO.nds
	ndstool	-c SCDS.mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "reset_mse_06b dldi;devkitARMr31;Very fast reset/loader" -g ENG0 00 MSFORSC
	linkmaker	SCDS.mse
	dldipatch	dsone.dldi SCDS.mse
	mselink	SCDS.mse /msforsc.nds SCDS.nds
	ndstool	-c moonshl2alt.mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b moonshl2alt.bmp "MoonShell2;Alternative Loader;devkitARMr31"
	linkmaker	moonshl2alt.mse
	mselink	moonshl2alt.mse /moonshl2/reload.dat moonshl2alt.nds
	rm		RPGS.mse R4DS.mse SCDS.mse DEMO.mse TTIO.mse moonshl2alt.mse $(TARGET).r4isdhc.arm9
	cp		$(TARGET).mse RPGS.nds R4DS.nds SCDS.nds DEMO.nds TTIO.nds moonshl2alt.nds ../
	cp		moonshl2alt.nds ../../
	dsbize		../moonshl2alt.nds ../g6dsload.eng 0x12
	dldipatch	m3ds.dldi ../g6dsload.eng

linkmaker:	linkmaker.c
	gcc -o linkmaker -O2 linkmaker.c

r4isdhc:	r4isdhc.c
	gcc -o r4isdhc -O2 r4isdhc.c

#---------------------------------------------------------------------------------
$(TARGET).arm9	: arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	rm -f $(TARGET).ds.gba $(TARGET).mse $(TARGET).arm9 linkmaker RPGS.nds R4DS.nds SCDS.nds DEMO.nds TTIO.nds moonshl2alt.nds $(TARGET).r4isdhc.arm9 r4isdhc
