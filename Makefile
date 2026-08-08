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
		maggie_setuptri.s \
		maggie_setuppoly.s \
		maggie_dxt1.c \
		maggie_stabs.c \
		maggie_debug.c

#		maggie_raster32.c \
#		maggie_raster16.c \

ASFLAGS=-m68080 -m68882 -quiet -Fhunk -I /opt/amiga/m68k-amigaos/ndk-include

# -fexcess-precision=fast: -Ofast implies it, but -std=c11 silently forces it
# back to `standard`, which makes the backend round every float intermediate to
# single via a Dn round-trip. That inflates live ranges past the 8 FPU registers
# and the setup code ends up spilling extended (12-byte) values to the stack -
# 1320 such spills library-wide, ~40 of them per triangle in DrawTriangle alone.
# Restoring `fast` keeps intermediates in the register file (which is also more
# accurate, not less) and removes essentially all of them.
#
# -MMD -MP: emit header dependencies. This matters more than usual here - the asm
# reaches into MaggieBase at offsets hardcoded as MB_* in raster/raster_structs.i,
# and _Static_asserts in maggie_draw.c are what keep the two honest. Without
# header deps an edit to maggie_internal.h left stale objects behind, so those
# asserts were never re-evaluated and a moved field corrupted silently.
CFLAGS=-std=c11 -fexcess-precision=fast -noixemul -Ofast -fno-unsafe-math-optimizations -fomit-frame-pointer -m68080 -mregparm -mhard-float -I include -Wdouble-promotion -MMD -MP

# Optional profiling build, TextOut'd per frame at magEndScene. Needs nothing from
# the game — leave the game uninstrumented. (This is the lib's own profiler, NOT
# -finstrument-functions; the latter would need the game's __cyg_* hooks to link.)
#
#   make static PROFILE=1   Frame / Clear / Spans / Setup (+Trans/TexGen/Light)
#                           and per-primitive averages. Spans is the raster,
#                           Setup is the CPU cost of feeding it.
#   make static PROFILE=2   also the per-edge timer. It calls GetClocks twice per
#                           edge and nearly doubles DrawEdge, so it inflates both
#                           Lines and Setup - read it on its own, not alongside
#                           the raster-vs-setup split.
#
# Must be a number: it is passed through as the value of the PROFILE macro.
ifdef PROFILE
CFLAGS += -DPROFILE=$(PROFILE)
endif

LFLAGS=-nostartfiles
LDLIBS=

OBJSC=$(SOURCES:.c=.o)
OBJS=$(OBJSC:.s=.o)
DEPS=$(filter %.d,$(SOURCES:.c=.d))

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
	rm -f $(OBJS) $(DEPS) maggie_static.o maggie_static.d $(TARGET) $(STATIC_TARGET)

# The .s files have no automatic dependency scanning, so name the shared include
# explicitly - it carries the hardcoded MaggieBase offsets.
raster/maggie_raster.o maggie_buffers.o maggie_setuptri.o maggie_setuppoly.o: raster/raster_structs.i

-include $(DEPS) maggie_static.d
