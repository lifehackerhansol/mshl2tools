#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	iniclear
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
$(TARGET).nds	:	$(TARGET).arm9 r4isdhc
	r4isdhc	$(TARGET).arm9 $(TARGET).r4isdhc.arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "iniclear launcher;R4TF/TTIO/SCDS/RPGS/M3DS"
	cp $(TARGET).nds ttmenu.dat
	dldipatch ttio.dldi ttmenu.dat
	cp $(TARGET).nds akmenu4.nds
	dldipatch rpgs.dldi akmenu4.nds
	ndstool	-c r4.dat -7 ../../arm7/arm7.bin -9 $(TARGET).r4isdhc.arm9 -e9 0x02000000 -b $(TARGET).bmp "iniclear launcher;R4TF/TTIO/SCDS/RPGS/M3DS"
	dldipatch demo.dldi r4.dat
	ndstool	-c msforsc.nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "iniclear launcher;R4TF/TTIO/SCDS/RPGS/M3DS" -g ENG0 00 MSFORSC
	dldipatch	dsone.dldi msforsc.nds
	cp	$(TARGET).nds ttmenu.dat akmenu4.nds r4.dat msforsc.nds ../

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
	rm -f $(TARGET).ds.gba $(TARGET).nds $(TARGET).arm9 $(TARGET).arm9 ttmenu.dat akmenu4.nds r4.dat msforsc.nds $(TARGET).r4isdhc.arm9 r4isdhc

