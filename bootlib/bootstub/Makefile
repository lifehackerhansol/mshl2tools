export	PATH	:=	$(DEVKITARM)/bin:$(PATH)

TARGET	:=	bootstub
	
$(TARGET).bin: $(TARGET).elf
	arm-none-eabi-objcopy -O binary $< $@

$(TARGET).elf: $(TARGET).s Makefile	
	arm-none-eabi-gcc -Wl,-Ttext=0 -x assembler-with-cpp -nostartfiles -nostdlib $(TARGET).s -o $@


clean:
	rm -f $(TARGET).elf
