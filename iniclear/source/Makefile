ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	iniclear
export TOPDIR		:=	$(CURDIR)
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

all: ../$(TARGET).nds
.PHONY: clean

clean:
	rm -f $(TARGET).arm9 $(TARGET).r4isdhc.arm9 main.o .map r4isdhc

r4isdhc:	r4isdhc.c
	gcc -o r4isdhc -O2 r4isdhc.c

### final build script ###
../$(TARGET).nds	:	$(TARGET).arm9 r4isdhc
	r4isdhc	$(TARGET).arm9 $(TARGET).r4isdhc.arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "iniclear launcher;R4TF/TTIO/SCDS/RPGS/M3DS"
	cp		$(TARGET).nds ez5sys.bin
	dldipatch	../../xenofile/source/dldi/ez5h.dldi ez5sys.bin
	cp		$(TARGET).nds ez5isys.bin
	dldipatch	../../xenofile/source/dldi/ez5i.dldi ez5isys.bin
	cp		$(TARGET).nds ismat.dat
	dldipatch	../../xenofile/source/dldi/imat.dldi ismat.dat
	cp		$(TARGET).nds ttmenu.dat
	dldipatch	../../xenofile/source/dldi/ttio.dldi ttmenu.dat
	cp		$(TARGET).nds akmenu4.nds
	dldipatch	../../xenofile/source/dldi/rpgs.dldi akmenu4.nds
	cp		$(TARGET).nds akmenu4_akrpg.nds
	dldipatch	../../xenofile/source/dldi/rpgn.dldi akmenu4_akrpg.nds
	cp		$(TARGET).nds akmenu4_acek.nds
	dldipatch	../../xenofile/source/dldi/xxxx.dldi akmenu4_acek.nds
	cp		$(TARGET).nds m3ds.nds
	dldipatch	../../xenofile/source/dldi/m3ds.dldi m3ds.nds
	dsbize		m3ds.nds ../menu.xx 0x12
	cp		$(TARGET).nds g003.nds
	dldipatch	../../xenofile/source/dldi/g003.dldi g003.nds
	dsbize		g003.nds ../g003rest.dat 0x23
	cp		$(TARGET).nds r4tf.nds
	dldipatch	../../xenofile/source/dldi/r4tf.dldi r4tf.nds
	r4crypt	-e < r4tf.nds > ../_ds_menu.dat
	rm		m3ds.nds g003.nds r4tf.nds
	ndstool	-c r4.dat -7 ../../arm7/arm7.bin -9 $(TARGET).r4isdhc.arm9 -e9 0x02000000 -b $(TARGET).bmp "iniclear launcher;R4TF/TTIO/SCDS/RPGS/M3DS"
	dldipatch	../../xenofile/source/dldi/demo.dldi r4.dat
	ndstool	-c scfw.sc -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "iniclear launcher;R4TF/TTIO/SCDS/RPGS/M3DS" -g ENG0 00 MSFORSC
	cp		scfw.sc dsgame.nds
	#cp		scfw.sc dsgame_ismart.nds
	dldipatch	../../xenofile/source/dldi/dsi2.dldi dsgame.nds
	dldipatch	../../xenofile/source/dldi/scds.dldi scfw.sc
	mv -f	$(TARGET).nds ez5sys.bin ez5isys.bin ismat.dat ttmenu.dat akmenu4.nds akmenu4_akrpg.nds akmenu4_acek.nds r4.dat scfw.sc dsgame.nds ../

### compile main.c ###
LIBDIRS	:=	$(LIBNDS)
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
LIBS	:= ../../libprism/libprism.a -lnds9
ARCH	:=	-mthumb -mthumb-interwork
CFLAGS	:=	-Wall -O2 \
			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer \
			-ffast-math -fdata-sections \
			$(ARCH) -DARM9
LDFLAGS	=	-s -specs=ds_arm9.specs $(ARCH) -Wl,--gc-sections -Wl,-Map,$(notdir $*.map)
LDFLAGS	+=	-Wl,--section-start,.init=0x02000450

$(TARGET).arm9: main.c
	@echo Compiling...
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o main.o main.c
	@echo Linking...
	@$(CC) $(LDFLAGS) -o $(TARGET).arm9.elf main.o $(LIBPATHS) $(LIBS)
	@$(OBJCOPY) -O binary $(TARGET).arm9.elf $(TARGET).arm9
	@rm $(TARGET).arm9.elf
