ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	reset
export TOPDIR		:=	$(CURDIR)
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

all: ../$(TARGET).mse
.PHONY: clean

clean:
	rm -f $(TARGET).arm9 main.o .map linkmaker r4isdhc

### final build script ###
../$(TARGET).mse	:	$(TARGET).arm9 linkmaker r4isdhc
	./r4isdhc	$(TARGET).arm9 $(TARGET).r4isdhc.arm9
	ndstool	-c $(TARGET).mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "reset_mse_06b dldi;Very fast reset/loader"
	./linkmaker	$(TARGET).mse
	mselink	$(TARGET).mse /ez5sys.bin EZ5H.nds
	dldipatch	../../xenofile/source/dldi/ez5h.dldi EZ5H.nds
	mselink	$(TARGET).mse /ez5isys.bin EZ5i.nds
	dldipatch	../../xenofile/source/dldi/ez5i.dldi EZ5i.nds
	mselink	$(TARGET).mse /system/ismat.dat IMAT.nds
	dldipatch	../../xenofile/source/dldi/imat.dldi IMAT.nds
	cp		IMAT.nds Mati.nds
	mselink	$(TARGET).mse /ttmenu.dat TTIO.nds
	dldipatch	../../xenofile/source/dldi/ttio.dldi TTIO.nds
	mselink	$(TARGET).mse /_ds_menu.dat R4DS.nds
	dldipatch	../../xenofile/source/dldi/r4ds.dldi R4DS.nds
	mselink	$(TARGET).mse /akmenu4.nds RPGS.nds
	dldipatch	../../xenofile/source/dldi/rpgs.dldi RPGS.nds
	mselink	$(TARGET).mse /akmenu4.nds RPGN.nds
	dldipatch	../../xenofile/source/dldi/rpgn.dldi RPGN.nds
	mselink	$(TARGET).mse /akmenu4.nds XXXX.nds
	dldipatch	../../xenofile/source/dldi/xxxx.dldi XXXX.nds
	ndstool	-c DEMO.mse -7 ../../arm7/arm7.bin -9 $(TARGET).r4isdhc.arm9 -e9 0x02000000 -b $(TARGET).bmp "reset_mse_06b dldi;Very fast reset/loader"
	./linkmaker	DEMO.mse
	mselink	DEMO.mse /r4.dat DEMO.nds
	dldipatch	../../xenofile/source/dldi/demo.dldi DEMO.nds
	ndstool	-c SCDS.mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "reset_mse_06b dldi;Very fast reset/loader" -g ENG0 00 MSFORSC
	./linkmaker	SCDS.mse
	mselink	SCDS.mse /msforsc.nds SCDS.nds
	dldipatch	../../xenofile/source/dldi/scds.dldi SCDS.nds
	mselink	SCDS.mse /_dstwo/dstwo.nds DSI2.nds
	dldipatch	../../xenofile/source/dldi/dsi2.dldi DSI2.nds
	mselink	SCDS.mse /_ismart/ismart.nds DSI2_ismart.nds
	dldipatch	../../xenofile/source/dldi/dsi2.dldi DSI2_ismart.nds
	ndstool	-c moonshl2alt.mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b moonshl2alt.bmp "MoonShell2;Alternative Loader"
	./linkmaker	moonshl2alt.mse
	mselink	moonshl2alt.mse /moonshl2/reload.dat moonshl2alt.nds
	rm		DEMO.mse SCDS.mse moonshl2alt.mse $(TARGET).r4isdhc.arm9
	mv -f		$(TARGET).mse EZ5H.nds EZ5i.nds IMAT.nds Mati.nds TTIO.nds R4DS.nds RPGS.nds RPGN.nds XXXX.nds DEMO.nds SCDS.nds DSI2.nds DSI2_ismart.nds moonshl2alt.nds ../
	cp		../moonshl2alt.nds ../../
	dsbize		../moonshl2alt.nds ../menu.xx 0x12
	dldipatch	../../xenofile/source/dldi/m3ds.dldi ../menu.xx
	dsbize		../moonshl2alt.nds ../g003rest.dat 0x23
	dldipatch	../../xenofile/source/dldi/g003.dldi ../g003rest.dat

linkmaker:	linkmaker.c
	gcc -o linkmaker -O2 linkmaker.c

r4isdhc:	r4isdhc.c
	gcc -o r4isdhc -O2 r4isdhc.c

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
