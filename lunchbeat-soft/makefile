PRG            = lunchbeat
OBJ            = lunchbeat.o 
COMPILE_TARGET = atmega328p
UPLOAD_TARGET  = atmega328p

INCLUDE_PATH   = -I.././lunchbeat

upload: $(PRG).hex 
	avrdude -c usbasp -p $(UPLOAD_TARGET) -U flash:w:$(PRG).hex

a: $(PRG).elf lst hex

$(PRG).elf: $(OBJ)
	avr-gcc -g -Wall -mmcu=$(COMPILE_TARGET) -o $@ $^ 

%.o: %.c
	avr-gcc -g -Wall -O2 -mmcu=$(COMPILE_TARGET) -fno-exceptions -ffunction-sections -fdata-sections $(INCLUDE_PATH) -c $< 

lst:  $(PRG).lst
%.lst: %.elf
	avr-objdump -h -S $< > $@

hex:  $(PRG).hex
%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

clean:
	rm -rf *.o $(PRG).elf *.lst *.hex

