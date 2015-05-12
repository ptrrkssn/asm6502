#
CC=gcc
CFLAGS=-Wall -funsigned-char

OBJS=asm.o symbols.o evald.o

all: asm6502

asm.c: admodes.h evald.h opcodes.h symbols.h
evald.c: evald.h
symbols.c: symbols.h

asm6502: $(OBJS)
	$(CC) -o asm6502 $(OBJS)

clean:
	rm -f *~ core *.o asm6502

distclean: clean
	rm -f example.ent example.ext example.ith example.list
