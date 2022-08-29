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

export TARGET		:=	nds.inilink
export TOPDIR		:=	$(CURDIR)
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

all: ../$(TARGET).nds
.PHONY: clean

clean:
	rm -f $(TARGET).arm9 main.o LzmaDec.o .map
	rm -f warning_eos.b15lzma warning_eos.b15lzma.o warning_eos_b15lzma.h
	#rm -f warning_dstwo.b15lzma warning_dstwo.b15lzma.o warning_dstwo_b15lzma.h
	rm -f warning_m3sakura.b15lzma warning_m3sakura.b15lzma.o warning_m3sakura_b15lzma.h
	rm -f warning_r4.b15lzma warning_r4.b15lzma.o warning_r4_b15lzma.h
	rm -f warning_ismart.b15lzma warning_ismart.b15lzma.o warning_ismart_b15lzma.h
	rm -f warning_ez.b15lzma warning_ez.b15lzma.o warning_ez_b15lzma.h

### final build script ###
../$(TARGET).nds	:	$(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -b $(TARGET).bmp "MoonShell2 inilinker;xxxloader multibyte;R4TF/TTIO/SCDS/RPGS/M3DS"
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

B15LZMA := warning_eos.b15lzma.o warning_m3sakura.b15lzma.o warning_r4.b15lzma.o warning_ismart.b15lzma.o warning_ez.b15lzma.o

$(TARGET).arm9: main.c LzmaDec.c $(B15LZMA)
	@echo Compiling...
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o main.o main.c
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o LzmaDec.o LzmaDec.c
	@echo Linking...
	@$(CC) $(LDFLAGS) -o $(TARGET).arm9.elf main.o LzmaDec.o $(B15LZMA) $(LIBPATHS) $(LIBS)
	@$(OBJCOPY) -O binary $(TARGET).arm9.elf $(TARGET).arm9
	@rm $(TARGET).arm9.elf

warning_eos.b15lzma: warning_eos.png
	png2b15 < $(<) | lzma_alone e $(@) -lc1 -lp1 -d19 -si

warning_eos.b15lzma.o: warning_eos.b15lzma
	@$(bin2o)

warning_dstwo.b15lzma: warning_dstwo.png
	png2b15 < $(<) | lzma_alone e $(@) -lc1 -lp1 -d19 -si

warning_dstwo.b15lzma.o: warning_dstwo.b15lzma
	@$(bin2o)

warning_m3sakura.b15lzma: warning_m3sakura.png
	png2b15 < $(<) | lzma_alone e $(@) -lc1 -lp1 -d19 -si

warning_m3sakura.b15lzma.o: warning_m3sakura.b15lzma
	@$(bin2o)

warning_r4.b15lzma: warning_r4.png
	png2b15 < $(<) | lzma_alone e $(@) -lc1 -lp1 -d19 -si

warning_r4.b15lzma.o: warning_r4.b15lzma
	@$(bin2o)

warning_ismart.b15lzma: warning_ismart.png
	png2b15 < $(<) | lzma_alone e $(@) -lc1 -lp1 -d19 -si

warning_ismart.b15lzma.o: warning_ismart.b15lzma
	@$(bin2o)

warning_ez.b15lzma: warning_ez.png
	png2b15 < $(<) | lzma_alone e $(@) -lc1 -lp1 -d19 -si

warning_ez.b15lzma.o: warning_ez.b15lzma
	@$(bin2o)
