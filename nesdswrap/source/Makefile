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

export TARGET		:=	nes.nesdswrap
export TOPDIR		:=	$(CURDIR)
export PATH		:=	$(DEVKITARM)/bin:$(PATH)

all: ../$(TARGET).nds
.PHONY: clean

clean:
	rm -f $(TARGET).arm9 main.o LzmaDec.o .map
	rm -f nesterds_nds.lzma nesterds_nds.lzma.o nesterds_nds_lzma.h
	rm -f nesDS_nds.lzma nesDS_nds.lzma.o nesDS_nds_lzma.h
	rm -f nesDS_r62_nds.lzma nesDS_r62_nds.lzma.o nesDS_r62_nds_lzma.h
	rm -f nesDS_r69_nds.lzma nesDS_r69_nds.lzma.o nesDS_r69_nds_lzma.h
	rm -f nesDS_r87_nds.lzma nesDS_r87_nds.lzma.o nesDS_r87_nds_lzma.h
	rm -f nesds_arm7.lzma nesds_arm7.lzma.o nesds_arm7_lzma.h
	rm -f nesds.lzma nesds.lzma.o nesds_lzma.h
	rm -f nesds_ex031.lzma nesds_ex031.lzma.o nesds_ex031_lzma.h
	rm -f nesds_ex040.lzma nesds_ex040.lzma.o nesds_ex040_lzma.h
	rm -f nesds_ex042.lzma nesds_ex042.lzma.o nesds_ex042_lzma.h
	rm -f nesds_ex043.lzma nesds_ex043.lzma.o nesds_ex043_lzma.h
	rm -f nesds_ex044.lzma nesds_ex044.lzma.o nesds_ex044_lzma.h

### final build script ###
../$(TARGET).nds	:	$(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 ../../arm7/arm7.bin -9 $(TARGET).arm9 -b $(TARGET).bmp "nesDS wrapper;ORG/0.31/0.40/0.42/0.43/0.44;with nesterds"
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
LDFLAGS	=	-s -specs=ds_arm9.specs $(ARCH) -Wl,--gc-sections -Wl,-Map,$(notdir $*.map)
LDFLAGS	+=	-Wl,--section-start,.init=0x02000000

BINLZMA := nesterds_nds.lzma.o nesds_arm7.lzma.o nesds.lzma.o nesds_ex031.lzma.o nesds_ex040.lzma.o nesds_ex042.lzma.o nesds_ex043.lzma.o nesds_ex044.lzma.o nesDS_nds.lzma.o nesDS_r62_nds.lzma.o nesDS_r69_nds.lzma.o nesDS_r87_nds.lzma.o

$(TARGET).arm9: main.c LzmaDec.c $(BINLZMA)
	@echo Compiling...
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o main.o main.c
	@$(CC) $(CFLAGS) $(INCLUDE) -c -o LzmaDec.o LzmaDec.c
	@echo Linking...
	@$(CC) $(LDFLAGS) -o $(TARGET).arm9.elf main.o LzmaDec.o $(BINLZMA) $(LIBPATHS) $(LIBS)
	@$(OBJCOPY) -O binary $(TARGET).arm9.elf $(TARGET).arm9
	@rm $(TARGET).arm9.elf

#compression buffer must be less than 1MB (-d17)
nesterds_nds.lzma.o: nesterds_nds.lzma
	@$(bin2o)

nesterds_nds.lzma: nesterds.nds
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesDS_r62_nds.lzma.o: nesDS_r62_nds.lzma
	@$(bin2o)

nesDS_r62_nds.lzma: nesDS_r62.nds
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesDS_r69_nds.lzma.o: nesDS_r69_nds.lzma
	@$(bin2o)

nesDS_r87_nds.lzma: nesDS_r87.nds
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesDS_r87_nds.lzma.o: nesDS_r87_nds.lzma
	@$(bin2o)

nesDS_r69_nds.lzma: nesDS_r69.nds
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesDS_nds.lzma.o: nesDS_nds.lzma
	@$(bin2o)

nesDS_nds.lzma: nesDS.nds
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesds_arm7.lzma.o: nesds_arm7.lzma
	@$(bin2o)

nesds_arm7.lzma: nesds_arm7.bin
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesds.lzma.o: nesds.lzma
	@$(bin2o)

nesds.lzma: nesds.bin
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesds_ex031.lzma.o: nesds_ex031.lzma
	@$(bin2o)

nesds_ex031.lzma: nesds_ex031.bin
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesds_ex040.lzma.o: nesds_ex040.lzma
	@$(bin2o)

nesds_ex040.lzma: nesds_ex040.bin
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesds_ex042.lzma.o: nesds_ex042.lzma
	@$(bin2o)

nesds_ex042.lzma: nesds_ex042.bin
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesds_ex043.lzma.o: nesds_ex043.lzma
	@$(bin2o)

nesds_ex043.lzma: nesds_ex043.bin
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

nesds_ex044.lzma.o: nesds_ex044.lzma
	@$(bin2o)

nesds_ex044.lzma: nesds_ex044.bin
	lzma_alone e $(<) $(@) -lc0 -lp2 -d17

