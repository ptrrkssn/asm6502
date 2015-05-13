#
CC=gcc
CFLAGS=-Wall -funsigned-char

OBJS=asm.o symbols.o evald.o version.o

all: asm6502

asm.c: admodes.h evald.h opcodes.h symbols.h
evald.c: evald.h
symbols.c: symbols.h

asm6502: $(OBJS)
	$(CC) -o asm6502 $(OBJS)

version:
	git tag | sed -e 's/^v//' | nawk 'BEGIN {RC=1} {print "char version[] = \"" $$1 "\";" ; RC=0} END {exit RC}' >.version && mv .version version.c


clean:
	rm -f *~ core *.o asm6502 t/*.ent t/*.ext t/*.ith t/*.list t/*~

distclean: clean version

