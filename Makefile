TARGET=maggie.library

AS=vasmm68k_mot
CXX=m68k-amigaos-g++
CC=m68k-amigaos-gcc
AR=m68k-amigaos-ar

STATIC_TARGET=libmaggie.a

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

# Static-library variant: the same objects as the resident build, minus the
# resident entry glue in maggie.c (romTag/functionTable/maggieInit/Open/Close/
# Expunge), plus the plain-C context lifecycle in maggie_static.c. Built with
# the identical CFLAGS so the -mregparm/-mhard-float ABI matches the callers.
STATIC_OBJS=$(filter-out maggie.o,$(OBJS)) maggie_static.o

all: $(SOURCES) $(TARGET) $(STATIC_TARGET) Makefile

$(TARGET): $(OBJS) Makefile
	$(CC) $(LFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET)

$(STATIC_TARGET): $(STATIC_OBJS) Makefile
	rm -f $(STATIC_TARGET)
	$(AR) rcs $(STATIC_TARGET) $(STATIC_OBJS)

static: $(STATIC_TARGET)

purge: clean
	rm -f $(TARGET)

strip:
	m68k-amigaos-strip $(TARGET)

clean:
	rm -f $(OBJS) maggie_static.o $(TARGET) $(STATIC_TARGET)
