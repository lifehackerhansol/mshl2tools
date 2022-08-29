ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	m3region
export TOPDIR		:=	$(CURDIR)
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

all: ../$(TARGET).nds
.PHONY: clean

clean:
	rm -f $(TARGET).arm9 main.o .map

### final build script ###
../$(TARGET).nds	:	$(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -b $(TARGET).bmp "Check M3 Region"
	mv -f	$(TARGET).nds ../

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
LDFLAGS	=	-s -specs=ds_arm9.specs $(ARCH) -mno-fpu -Wl,--gc-sections -Wl,-Map,$(notdir $*.map)
LDFLAGS	+=	-Wl,--section-start,.init=0x02000000

$(TARGET).arm9: main.c
	@echo Compiling...
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o main.o main.c
	@echo Linking...
	@$(CC) $(LDFLAGS) -o $(TARGET).arm9.elf main.o $(LIBPATHS) $(LIBS)
	@$(OBJCOPY) -O binary $(TARGET).arm9.elf $(TARGET).arm9
	@rm $(TARGET).arm9.elf
