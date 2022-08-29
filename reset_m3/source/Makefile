ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	M3DS
export TOPDIR		:=	$(CURDIR)
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

all: ../$(TARGET).nds
.PHONY: clean

clean:
	rm -f $(TARGET).arm9 main.o .map linkmaker

### final build script ###
../$(TARGET).nds	:	$(TARGET).arm9 linkmaker
	ndstool	-c $(TARGET).mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -e9 0x02000450 -b $(TARGET).bmp "reset_mse_06b dldi;Very fast reset/loader;for DSBooter"
	./linkmaker	$(TARGET).mse
	mselink	$(TARGET).mse /system/g6dsload.eng $(TARGET).nds
	dldipatch	../../xenofile/source/dldi/m3ds.dldi $(TARGET).nds
	mselink	$(TARGET).mse /system/g003rest.dat G003.nds
	dldipatch	../../xenofile/source/dldi/g003.dldi G003.nds
	rm		$(TARGET).mse
	mv -f		$(TARGET).nds ../
	dsbize		../$(TARGET).nds ../menu.xx 0x12
	mv -f		G003.nds ../
	dsbize		../G003.nds ../g003rest.dat 0x23

linkmaker:	linkmaker.c
	gcc -o linkmaker -O2 linkmaker.c

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
