TARGET=StaticCube

AS=vasmm68k_mot
CC=m68k-amigaos-gcc

SOURCES=staticcube.c

# The static maggie library. Build it first from the repo root: `make static`.
MAGGIELIB=../../libmaggie.a

ASFLAGS=-m68080 -m68881 -quiet -Fhunk

# Same float/CPU ABI as the library. -mhard-float is required (float args are
# passed in fp0..fp2); -fomit-frame-pointer frees a6 for the hidden base.
CFLAGS=-std=c11 -Ofast -fomit-frame-pointer -m68080 -mhard-float -Wall -Wno-unused-function -DAMIGA=1 -I ../../include

LFLAGS=-L. -noixemul
# Archive before -lamiga/-lm so its members' libm/libc references resolve.
LDLIBS=$(MAGGIELIB) -lamiga -lm

OBJSC=$(SOURCES:.c=.o)
OBJS=$(OBJSC:.s=.o)

all: $(SOURCES) $(TARGET) Makefile

$(TARGET): $(OBJS) $(MAGGIELIB) Makefile
	$(CC) $(LFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

strip:
	m68k-amigaos-strip $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
