#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	xenofile
export TOPDIR		:=	$(CURDIR)

#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path in windows
#---------------------------------------------------------------------------------
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

.PHONY: $(TARGET).arm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: ../$(TARGET).nds
r4isdhc:	r4isdhc.c
	gcc -o r4isdhc -O2 r4isdhc.c

a9_02000000:	a9_02000000.c
	gcc -o a9_02000000 -O2 a9_02000000.c

#---------------------------------------------------------------------------------
../$(TARGET).nds	:	$(TARGET).arm9 r4isdhc a9_02000000
	r4isdhc	$(TARGET).arm9 $(TARGET).r4isdhc.arm9
	a9_02000000	$(TARGET).arm9 $(TARGET).02000000.arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "XenoFile;simple and sophisticated filer"
	cp		$(TARGET).nds ez5sys.bin
	dldipatch	dldi/ez5h.dldi ez5sys.bin
	cp		$(TARGET).nds ez5isys.bin
	dldipatch	dldi/ez5i.dldi ez5isys.bin
	cp		$(TARGET).nds ismat.dat
	dldipatch	dldi/imat.dldi ismat.dat
	cp		$(TARGET).nds ttmenu.dat
	dldipatch	dldi/ttio.dldi ttmenu.dat
	cp		$(TARGET).nds akmenu4.nds
	dldipatch	dldi/rpgs.dldi akmenu4.nds
	cp		$(TARGET).nds akmenu4_akrpg.nds
	dldipatch	dldi/rpgn.dldi akmenu4_akrpg.nds
	cp		$(TARGET).nds akmenu4_acek.nds
	dldipatch	dldi/xxxx.dldi akmenu4_acek.nds
	cp		$(TARGET).nds m3ds.nds
	dldipatch	dldi/m3ds.dldi m3ds.nds
	dsbize		m3ds.nds ../g6dsload.eng 0x12
	cp		$(TARGET).nds g003.nds
	dldipatch	dldi/g003.dldi g003.nds
	dsbize		g003.nds ../g003rest.dat 0x23
	cp		$(TARGET).nds r4tf.nds
	dldipatch	dldi/r4tf.dldi r4tf.nds
	r4crypt	-e < r4tf.nds > ../_ds_menu.dat
	ndstool	-c $(TARGET)_02000000.nds -7 ../../arm7/arm7.bin -9 $(TARGET).02000000.arm9 -e9 0x02000000 -b $(TARGET).bmp "XenoFile;simple and sophisticated filer"
	ndstool	-c r4.dat -7 ../../arm7/arm7.bin -9 $(TARGET).r4isdhc.arm9 -e9 0x02000000 -b $(TARGET).bmp "XenoFile;simple and sophisticated filer"
	dldipatch	dldi/demo.dldi r4.dat
	ndstool	-c msforsc.nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "XenoFile;simple and sophisticated filer" -g ENG0 00 MSFORSC
	dldipatch	dldi/scds.dldi msforsc.nds
	rm -f		m3ds.nds g003.nds r4tf.nds
	mv -f	$(TARGET).nds $(TARGET)_02000000.nds ez5sys.bin ez5isys.bin ismat.dat ttmenu.dat akmenu4.nds akmenu4_akrpg.nds akmenu4_acek.nds r4.dat msforsc.nds ../

#---------------------------------------------------------------------------------
$(TARGET).arm9	: arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:
	png2b15 < keyboard.png | zlibrawstdio -e > arm9/data/keyboard.b15z
	png2b15 < keyboard_shift.png | zlibrawstdio -e > arm9/data/keyboard_shift.b15z
	$(MAKE) -C arm9

#---------------------------------------------------------------------------------
clean:
	$(MAKE) -C arm9 clean
	rm -f arm9/data/keyboard{,_shift}.b15z
	rm -f $(TARGET).arm9 $(TARGET).r4isdhc.arm9 $(TARGET).02000000.arm9 r4isdhc a9_02000000
