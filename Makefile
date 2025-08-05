TARGET=maggie.library

AS=vasmm68k_mot
CXX=m68k-amigaos-g++
CC=m68k-amigaos-gcc

SOURCES=maggie.c \
		maggie_funcs.c \
		maggie_transform.c \
		maggie_lighting.c \
		maggie_draw.c \
		maggie_clip.c \
		maggie_linedraw.c \
		maggie_drawspans.c \
		raster/maggie_raster.s \
		maggie_texture.c \
		maggie_buffers.s \
		maggie_dxt1.c \
		maggie_stabs.c \
		maggie_debug.c

#		maggie_raster32.c \
#		maggie_raster16.c \

ASFLAGS=-m68080 -m68882 -quiet -Fhunk -I /opt/amiga/m68k-amigaos/ndk-include

CFLAGS=-std=c11 -noixemul -Ofast -fno-unsafe-math-optimizations -fomit-frame-pointer -m68080 -mregparm -mhard-float -I include -Wdouble-promotion

LFLAGS=-nostartfiles
LDLIBS=

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
