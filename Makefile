TARGET	=test
FLAGS	=-V -mpic16 -p18f2550 --mplab-comp
CFLAGS	=-c $(FLAGS)
LFLAGS	=$(FLAGS) -llibc18f.lib
CC	=sdcc

all: $(TARGET).hex

$(TARGET).hex: test.o
	$(CC) $(LFLAGS) -o $@ test.o
	mv $@ $@.unix
	nkf -Lw $@.unix > $@

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.asm *.o *.hex *.lst *.cod *.unix

dis:
	gpdasm -s -p18f2550 $(TARGET).hex.unix > dis.asm
