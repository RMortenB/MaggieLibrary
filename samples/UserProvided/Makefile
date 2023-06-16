TARGET=UserDraw

AS=vasmm68k_mot
CC=m68k-amigaos-gcc

SOURCES=userdraw.c

ASFLAGS=-m68080 -m68881 -quiet -Fhunk

CFLAGS=-std=c89 -Ofast -fomit-frame-pointer -m68080 -Wall -Wno-unused-function -DAMIGA=1 -I ../../include

LFLAGS=-L. -noixemul
LDLIBS=-lamiga -lm

OBJSC=$(SOURCES:.c=.o)
OBJS=$(OBJSC:.s=.o)

all: $(SOURCES) $(TARGET) Makefile

$(TARGET): $(OBJS) Makefile
	$(CC) $(LFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

strip:
	m68k-amigaos-strip $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
