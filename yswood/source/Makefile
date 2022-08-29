ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET		:=	yswood
export TOPDIR		:=	$(CURDIR)
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

all: ../$(TARGET).nds
.PHONY: clean

clean:
	rm -f $(TARGET).arm9 main.o .map linkmaker

### final build script ###
../$(TARGET).nds	:	$(TARGET).arm9 linkmaker
	ndstool	-c $(TARGET).mse -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -r9 0x02000000 -b $(TARGET).bmp "YSMenu with WoodEngine"
	linkmaker	$(TARGET).mse
	mselink	$(TARGET).mse /ysmenu.dat $(TARGET).nds
	rm		$(TARGET).mse
	mv -f		$(TARGET).nds ../

linkmaker:	linkmaker.c
	gcc -o linkmaker -O2 linkmaker.c

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

$(TARGET).arm9: main.c
	@echo Compiling...
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o main.o main.c
	@echo Linking...
	@$(CC) $(LDFLAGS) -o $(TARGET).arm9.elf main.o $(LIBPATHS) $(LIBS)
	@$(OBJCOPY) -O binary $(TARGET).arm9.elf $(TARGET).arm9
	@rm $(TARGET).arm9.elf
