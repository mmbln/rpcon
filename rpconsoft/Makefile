
NAME	= rpcon

SRC	= rpcon.c USI_TWI_Slave.c

#############################################################################

OBJS	= $(subst .c,.o, $(SRC))
ELF	= $(NAME).elf
FHEX	= $(NAME).hex
EHEX	= $(NAME)-eeprom.hex

CFLAGS  += -mmcu=attiny26 -D__ATtiny26__ -Wall -Werror -Os -g -I.
CFLAGS	+= -DF_CPU=1000000 
LDFLAGS += -mmcu=attiny26 -g
# settings for ISP Stick
ADFLAGS += -p t26 -c usbtiny

CC 	= avr-gcc
LD 	= avr-gcc
OBJCOPY = avr-objcopy
SIZE	= avr-size
AD	= avrdude

all: $(ELF) $(FHEX) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) 
	$(SIZE) $(ELF)

$(FHEX) $(EHEX): $(ELF) 
	$(OBJCOPY) -j .text -j .data -O ihex $(ELF) $(FHEX)
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $(ELF) $(EHEX)

$(OBJS) $(ELF): Makefile

install: $(FHEX) $(EHEX) 
	$(AD) $(ADFLAGS) -V -e -q -q -v -v -U flash:w:$(FHEX):i -U eeprom:w:$(EHEX):i 

# Highbyte:
#   PB7 is reset !             (Fuse H4=1)
#   Enable serial programming  (Fuse H3=0)
#   EEPROM is not preserved    (Fuse H2=1)
#   Brownout disabled          (Fuse H1-H0=11)
# Lowbyte
#   Internal Clock             (Fuse L7=1)
#   CKOPT                      (Fuse L6=1)
#   Startup time 65ms          (Fuse L5-L4=10) 
#   CKSEL (8MHz)               (Fuse L3-L0=0100) 
install_fuses: $(FHEX) $(EHEX)
	$(AD) $(ADFLAGS) -U lfuse:w:0xc4:m -U hfuse:w:0x17:m

clean:	
	rm -f $(OBJS) $(ELF) $(EHEX) $(FHEX)  dump 

# End
