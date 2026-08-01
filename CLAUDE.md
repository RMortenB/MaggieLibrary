# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

`maggie.library` is an AmigaOS shared library (resident library) that drives the Maggie 3D chipset on Apollo/Vampire V4 (68080). It exposes a small fixed-function pipeline — transform, light, clip, rasterize — to host code via the Amiga shared-library calling convention.

## Build and test commands

The toolchain is the Bebbo `m68k-amigaos-gcc` cross-compiler plus `vasmm68k_mot`. There is no lint target and no automated test framework; tests are manual Amiga executables.

- Build the library: `make` in the repo root → produces both `maggie.library` (resident/shared) and `libmaggie.a` (static)
- Build only the static library: `make static` → `libmaggie.a` (same objects as the shared build minus `maggie.o`'s resident glue, plus `maggie_static.c`'s `magCreateContext`/`magDeleteContext`). Host code includes `<proto/Maggie_static.h>` instead of `<proto/Maggie.h>` and links `libmaggie.a`; see `samples/StaticCube`.
- Clean / strip: `make clean`, `make strip`
- Build a sample: `make -C samples/ZBuffer` (same pattern for other sample dirs)
- Build a test: `make -C Tests/Stars` (or `Landscape`, `Scroller`)
- Package a release: `./makePackage.sh` — regenerates fd glue, builds everything, and assembles `package/`
- Regenerate Amiga FD/proto/pragma headers from `include/maggie.fd`: `cd include && ./genfd.sh` (or `fd/genfd.sh` for the packaging tree). Requires `fd2pragma`. This also emits the static-link header `proto/Maggie_static.h` via `include/genstatic.awk` (derived from the generated GCC `inline/Maggie.h`) — do not hand-edit it.
- `samples/*/build.sh` and `Tests/*/build.sh` rebuild the root library, copy it into that sample, build the sample, then run `amigaCompile runscript.txt` against a hardcoded `AMIGAHOST` — inspect the script before running unchanged.

## High-level architecture

The top-level tree is the live build. `MaggieLibrary_v3/` is an older snapshot kept for reference — do not modify it as part of library changes.

### Library entry and state

- `maggie.c` is the resident-library entry point. It defines `romTag`, `functionTable` (slot order matches `include/maggie.fd`), and `maggieInit`/`Open`/`Close`/`Expunge`. `maggieInit` sets FPU rounding mode (via `setRoundingModeRZ` in asm), opens `graphics.library`, initializes the scene semaphore, allocates the persistent depth buffer sized for `MAGGIE_MAX_XRES` × `MAGGIE_MAX_YRES`, and clears the global handle tables.
- `MaggieBase` (in `maggie_internal.h`) is the single global state struct: screen info, matrices, scissor, current buffers, handle tables (`vertexBuffers`, `indexBuffers`, `textures`), lights, per-scanline edge scratch, and function pointers `DrawEdge` / `DrawSpans` selected by draw mode.
- **Important**: `functionTable` slot order is the ABI — its order must stay in lockstep with `include/maggie.fd`. Adding/reordering an exported function means updating both, plus regenerating the `fd/` headers.

### Amiga calling convention

All public entry points bind parameters to 68k registers using `REG(a,v) v __asm(#a)` and take the library pointer in `a6` (e.g. `magFoo(REG(d0, UWORD x), REG(a6, MaggieBase *lib))`). The public `maggie.h` declares the plain C signatures used by callers going through the `fd`/proto glue; `maggie_internal.h` declares the register-bound versions used inside the library. The two must match semantically.

### Rendering pipeline (staged)

Data flows roughly: upload → transform → (texgen) → (light) → clip → rasterize.

- `maggie_funcs.c` — state setters, buffer/index allocation, scene lifetime, immediate-mode setup.
- `maggie_transform.c` — recomputes `modelViewProj` / `modelView` from the three matrices (`dirtyMatrix` gate), transforms vertex positions, generates texgen UVs for the `MAG_DRAWMODE_TEXGEN_*` modes.
- `maggie_lighting.c` — grayscale-intensity lighting into the trans-vertex buffer; vertex colours are intensity, not full RGB.
- `maggie_draw.c` — main draw dispatcher. The `*UP` entry points copy user data into internal scratch buffers and fall through to the normal buffered path. Computes clip codes and culls, then either forwards unclipped primitives to the span renderer or hands clipped ones to `maggie_clip.c`.
- `maggie_clip.c` — frustum clipping for partially visible polygons.
- `maggie_drawspans.c` — selects between affine/perspective and 16/32-bit span renderers based on `drawMode`, populating `lib->DrawEdge` and `lib->DrawSpans`.
- `maggie_linedraw.c` — line rasterization.
- `raster/maggie_raster.s` plus `raster/raster_*.inc` — the inner scanline loops (affine, perspective, with/without depth). The `.inc` files are included multiple times with different `.bpp`/variant settings to emit specialised versions.
- `maggie_buffers.s` — fast-clear, matrix-load helpers, rounding-mode setup.
- `maggie_texture.c` / `maggie_dxt1.c` — texture upload; converts RGB/RGBA input to the internal DXT1/RGBA layout Maggie expects, including swizzle and mipmap offset math.

### Public headers and Amiga glue

- `maggie.h` — public C prototypes (host-side).
- `include/maggie_vertex.h`, `include/maggie_flags.h`, `include/maggie_vec.h` — public types and flag macros.
- `include/maggie.fd` — the canonical Amiga function-definition list. Source of truth for ABI slot order.
- `include/{clib,proto,pragma,pragmas,inline,lvo,defines}/` — generated by `genfd.sh` via `fd2pragma`, one variant per Amiga compiler (SAS-C, Storm, GCC, VBCC, etc.). Do not hand-edit; regenerate from `maggie.fd`.

## Conventions and invariants

- **Global resource handles**: vertex buffers, index buffers, and textures are library-global handles (`MAX_VERTEX_BUFFERS` / `MAX_INDEX_BUFFERS` / `MAX_TEXTURES` = 10240 each). Callers must pair every `magAllocate*` with a `magFree*`; unfreed handles are cleaned up at library expunge.
- **Scene semaphore**: `magBeginScene` acquires `lib->lock` and resets per-frame state (lights to `MAG_LIGHT_OFF`, default colour); `magEndScene` releases it. Allocate/free paths also take the semaphore. Host code taking over the system can skip Begin/End.
- **Polygon separator**: `0xffff` in an index buffer starts a new polygon in the `magDrawIndexedPolygons*` paths.
- **Limits**: `MAGGIE_MAX_TEXCOORDS == 1`, `MAG_MAX_LIGHTS == 8`, `MAG_MAX_POLYSIZE == 256`. The renderer does not validate polygon-side count.
- **Framebuffer ownership**: caller supplies the colour buffer via `magSetScreenMemory`; the library owns one persistent depth buffer sized at init.
- **Draw mode switching**: `magSetDrawMode` rebinds `DrawEdge` / `DrawSpans` function pointers between affine and perspective variants — changes to the span renderers must preserve the signatures used in `maggie_internal.h`.
- **Vertex colours are intensity**: the lighting and upload paths convert to grayscale/16-bit intensity before rasterization. Any change to lighting must preserve this assumption or update the rasterizer inputs too.
- **Compiler flags**: the library is built `-std=c11 -Ofast -fno-unsafe-math-optimizations -m68080 -mregparm -mhard-float`. The `-mregparm` ABI matters — do not add object files compiled without it.
