TARGET=maggie.library

AS=vasmm68k_mot
CXX=m68k-amigaos-g++
CC=m68k-amigaos-gcc

SOURCES=maggie.c \
		maggie_funcs.c \
		maggie_clip.c \
		maggie_draw.c \
		maggie_transform.c \
		maggie_linedraw.c \
		maggie_raster.c \
		maggie_debug.c \

ASFLAGS=-m68080 -m68882 -quiet -Fhunk

CFLAGS=-std=c99 -Ofast -fomit-frame-pointer -m68080 -Wall -Wno-unused-function -Wno-unused-variable

LFLAGS=-nostartfiles 
LDLIBS=

OBJSC=$(SOURCES:.c=.o)
OBJS=$(OBJSC:.s=.o)

all: $(SOURCES) $(TARGET) Makefile

$(TARGET): $(OBJS) Makefile
	$(CC) $(LFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET)

purge: clean
	rm -f $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
