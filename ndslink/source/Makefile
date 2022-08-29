define bin2o
	bin2s $< | $(AS) -o $(@)
	echo "extern const u8" `(echo $(@F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(@F) | tr . _)`.h
	echo "extern const u8" `(echo $(@F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(@F) | tr . _)`.h
	echo "extern const u32" `(echo $(@F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(@F) | tr . _)`.h
endef

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	ndslink
export TOPDIR		:=	$(CURDIR)
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

all: ../$(TARGET).nds
.PHONY: clean

clean:
	rm -f $(TARGET).arm9 main.o .map nds.mshl2wrap.nds.o nds_mshl2wrap_nds.h

### final build script ###
../$(TARGET).nds	:	$(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -b $(TARGET).bmp "NDSLink on DS"
	mv -f	$(TARGET).nds ../

### compile main.c ###
LIBDIRS	:=	$(LIBNDS)
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
LIBS	:= ../../libprism/libprism.a -lfat -lnds9
ARCH	:=	-mthumb -mthumb-interwork
CFLAGS	:=	-Wall -O2 \
			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer \
			-ffast-math -fdata-sections \
			$(ARCH) -DARM9
LDFLAGS	=	-s -specs=ds_arm9.specs $(ARCH) -mno-fpu -Wl,--gc-sections -Wl,-Map,$(notdir $*.map)
LDFLAGS	+=	-Wl,--section-start,.init=0x02000000

$(TARGET).arm9: main.c nds.mshl2wrap.nds.o
	@echo Compiling...
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o main.o main.c
	@echo Linking...
	@$(CC) $(LDFLAGS) -o $(TARGET).arm9.elf main.o nds.mshl2wrap.nds.o $(LIBPATHS) $(LIBS)
	@$(OBJCOPY) -O binary $(TARGET).arm9.elf $(TARGET).arm9
	@rm $(TARGET).arm9.elf

nds.mshl2wrap.nds.o: ../../mshl2wrap/nds.mshl2wrap.nds
	@$(bin2o)
