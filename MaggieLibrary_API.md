

# maggie.library API Documentation

## Table of Contents

- [maggie.library API Documentation](#maggielibrary-api-documentation)
	- [Table of Contents](#table-of-contents)
	- [Linkage: Shared or Static](#linkage-shared-or-static)
		- [magCreateContext](#magcreatecontext)
		- [magDeleteContext](#magdeletecontext)
	- [Screen and Drawing Setup](#screen-and-drawing-setup)
		- [magSetScreenMemory](#magsetscreenmemory)
		- [magSetTexture](#magsettexture)
		- [magSetVertexBuffer](#magsetvertexbuffer)
		- [magSetIndexBuffer](#magsetindexbuffer)
		- [magSetDrawMode](#magsetdrawmode)
		- [magSetRGB](#magsetrgb)
	- [Depth Buffer](#depth-buffer)
		- [magGetDepthBuffer](#maggetdepthbuffer)
	- [Matrix Management](#matrix-management)
		- [magSetWorldMatrix](#magsetworldmatrix)
		- [magSetViewMatrix](#magsetviewmatrix)
		- [magSetPerspectiveMatrix](#magsetperspectivematrix)
	- [Drawing Primitives (User Pointer Mode)](#drawing-primitives-user-pointer-mode)
		- [magDrawTrianglesUP](#magdrawtrianglesup)
		- [magDrawIndexedTrianglesUP](#magdrawindexedtrianglesup)
		- [magDrawIndexedPolygonsUP](#magdrawindexedpolygonsup)
	- [Drawing Primitives (Buffered Mode)](#drawing-primitives-buffered-mode)
		- [magDrawTriangles](#magdrawtriangles)
		- [magDrawIndexedTriangles](#magdrawindexedtriangles)
		- [magDrawIndexedPolygons](#magdrawindexedpolygons)
	- [Span Drawing](#span-drawing)
		- [magDrawLinearSpan](#magdrawlinearspan)
		- [magDrawSpan](#magdrawspan)
	- [Buffer Management](#buffer-management)
		- [magAllocateVertexBuffer](#magallocatevertexbuffer)
		- [magUploadVertexBuffer](#maguploadvertexbuffer)
		- [magFreeVertexBuffer](#magfreevertexbuffer)
		- [magAllocateIndexBuffer](#magallocateindexbuffer)
		- [magUploadIndexBuffer](#maguploadindexbuffer)
		- [magFreeIndexBuffer](#magfreeindexbuffer)
	- [Texture management](#texture-management)
		- [magAllocateTexture](#magallocatetexture)
		- [magUploadTexture](#maguploadtexture)
		- [magFreeTexture](#magfreetexture)
	- [Scene and Drawing Control](#scene-and-drawing-control)
		- [magBeginScene](#magbeginscene)
		- [magEndScene](#magendscene)
	- [Immediate mode drawing](#immediate-mode-drawing)
		- [magBegin](#magbegin)
		- [magEnd](#magend)
		- [magVertex](#magvertex)
		- [magNormal](#magnormal)
		- [magTexCoord](#magtexcoord)
		- [magTexCoord3](#magtexcoord3)
		- [magColour](#magcolour)
	- [Clearing and Scissoring](#clearing-and-scissoring)
		- [magClear](#magclear)
		- [magClearColour](#magclearcolour)
		- [magClearDepth](#magcleardepth)
		- [magScissor](#magscissor)
	- [Lighting](#lighting)
		- [magSetLightType](#magsetlighttype)
		- [magSetLightPosition](#magsetlightposition)
		- [magSetLightDirection](#magsetlightdirection)
		- [magSetLightCone](#magsetlightcone)
		- [magSetLightAttenuation](#magsetlightattenuation)
		- [magSetLightColour](#magsetlightcolour)
	- [Sprites](#sprites)
		- [magDrawSprites](#magdrawsprites)
		- [magDrawSpritesUP](#magdrawspritesup)
	- [Vertex Attribute Uploads](#vertex-attribute-uploads)
		- [magUploadVertexPositions](#maguploadvertexpositions)
		- [magUploadVertexNormals](#maguploadvertexnormals)
		- [magUploadVertexTexCoords2](#maguploadvertextexcoords2)
		- [magUploadVertexTexCoords3](#maguploadvertextexcoords3)
		- [magUploadVertexColours](#maguploadvertexcolours)

MaggieLibrary is a 3D graphics rendering library for Amiga systems, providing functions for drawing, resource management, and scene control.<br>

- All functions respect the C calling convention, where d0/d1/a0/a1 can be assumed to be destroyed.
  - All other Dn/An registers are preserved.
- In addition, some functions will destroy the contents of all En registers.
- Other than the Maggie registers, no hardware registers are modified.
- The floating point rounding mode is assumed to be `To Zero` (RZ).

## Linkage: Shared or Static

maggie ships in two forms, built from the same sources:

- **`maggie.library`** — the resident/shared library. Open it at runtime with `OpenLibrary("maggie.library", 0)`, include `<proto/Maggie.h>`, and close it with `CloseLibrary`.
- **`libmaggie.a`** — a static archive linked straight into your executable (`make static` from the repo root). Include `<proto/Maggie_static.h>` instead of `<proto/Maggie.h>`, and link the archive before `-lamiga -lm`.

Every `magFoo(...)` entry point documented below is identical in both modes; the library base is passed in `a6` and hidden behind the global `MaggieBase`. Only the acquire/release of that base differs: `OpenLibrary`/`CloseLibrary` for the shared library, and `magCreateContext`/`magDeleteContext` (below) for the static archive. The static objects must be built with the same `-m68080 -mhard-float` ABI as the calling program. See `samples/StaticCube` for a worked example.

### magCreateContext

```c
struct Library *magCreateContext(void);
```
Create a maggie context for the statically linked library. This replaces `OpenLibrary("maggie.library", ...)`: it allocates and initialises the library base — FPU rounding mode, graphics.library, the scene semaphore, the depth buffer and the handle tables — exactly as the resident library's auto-init does.

- Inputs
  - None

- Outputs
  - Pointer to the library base, or `NULL` on failure.

- Notes
  - Only available in the static build (`libmaggie.a` + `<proto/Maggie_static.h>`).
  - Store the result in the global `MaggieBase`, just as you would the result of `OpenLibrary`; every other call finds the base there.
  - Pair each successful call with `magDeleteContext`.

### magDeleteContext

```c
void magDeleteContext(struct Library *base);
```
Tear down a context created by `magCreateContext`. Frees the depth buffer, any vertex/index buffers and textures still allocated, and closes graphics.library. This replaces `CloseLibrary`.

- Inputs
  - base
    - The base returned by `magCreateContext`. A `NULL` base is ignored.

- Outputs
  - None

- Notes
  - Only available in the static build.

## Screen and Drawing Setup
### magSetScreenMemory
```c
void magSetScreenMemory(APTR pixels, UWORD xres, UWORD yres);
```
Set the screen pixel buffer and resolution. The max resolution is 1920x1080.

- Inputs
  - pixels (a0)
    - Render target memory to draw to. All subsequent draws will go to this buffer
  - xres (d0)
    - X resolution of render target
  - yres (d1)
    - Yresolution of render target

- Outputs
  - None

### magSetTexture

```c
void magSetTexture(UWORD unit, UWORD txtr);
```

Bind a texture to a texture unit for rendering.
There is currently only one unit.

- Inputs
  - unit (d0)
    - Texture unit. Must be 0 currently
  - txtr (d1)
    - Texture handle returned from `magAllocateTexture`

- Outputs
  - None

### magSetVertexBuffer

```c
void magSetVertexBuffer(WORD vBuffer);
```
Set the active vertex buffer for rendering.

- Inputs
  - vBuffer (d0)
    - Vertex buffer handle returned from `magAllocateVertexBuffer`

- Outputs
  - None

### magSetIndexBuffer

```c
void magSetIndexBuffer(WORD iBuffer);
```
Set the active index buffer for rendering.

- Inputs
  - iBuffer (d0)
    - Index buffer handle returned from `magAllocateIndexBuffer`

- Outputs
  - None

### magSetDrawMode

```c
void magSetDrawMode(UWORD mode);
```
Set the current drawing mode.

- Inputs
  - mode (d0)
    - The current draw mode state

- Outputs
  - None

- Notes
  - The flags gets or'ed together, so
    - `MAG_DRAWMODE_DEPTHBUFFER | MAG_DRAWMODE_BILINEAR` enables the depth buffer together with bilinear texture filtering.
  - `MAG_DRAWMODE_NORMAL` This is just the mode to set when no other option is set.
  - `MAG_DRAWMODE_DEPTHBUFFER` Enables depth buffering. The depth buffer used is a 16 bit w-buffer, so adjust your projection matrix accordingly.
  - `MAG_DRAWMODE_BILINEAR` Enables bilinear textuer sampling
  - `MAG_DRAWMODE_32BIT` Must be specified when rendering to a 32 bit render target. The default is a 16 bit render target.
  - `MAG_DRAWMODE_LIGHTING` Enables the lights. See the [Lighting](#lighting) for how to use the lights.
  - `MAG_DRAWMODE_CULL_CCW` Inverts the cull mode to counter-clockwise. Culling is always done, and the default is clockwise.
  - `MAG_DRAWMODE_MIPMAP` Turns on auto mipmapping. The mipmapping is enabled even if no mipmaps are uploaded for the current texture, so beware!
  - `MAG_DRAWMODE_AFFINE_MAPPING` Turns off perspective correct textures. This runs a lot faster for smaller triangles.
  - `MAG_DRAWMODE_BLEND_REPLACE` Disables blending.
  - `MAG_DRAWMODE_BLEND_ADD` Enables additive blending. The draw will be added to the render target.
  - `MAG_DRAWMODE_BLEND_MUL` Enables multiplicative blending. The draw will be multiplied with the render target.
  - `MAG_DRAWMODE_TEXGEN_UV` Pass the UV coordinates through from the vertex buffer.
  - `MAG_DRAWMODE_TEXGEN_POS` Use the object position as texture coordinate.
  - `MAG_DRAWMODE_TEXGEN_NORMAL` Use the modelview space normal as the source for the texture coordinates.
  - `MAG_DRAWMODE_TEXGEN_REFLECT` Use the reflected modelview space normal as the source for the texture coordinates.

### magSetRGB

```c
void magSetRGB(ULONG rgb);
```
Set the current drawing colour. The texture and lighting will be multiplied with this colour.

- Inputs
  - rgb (d0)
    - The current RGB colour

- Outputs
  - None


## Depth Buffer

### magGetDepthBuffer

```c
UWORD *magGetDepthBuffer();
```
Get a pointer to the live depth buffer. Maggie does not snoop the CPU dcache, so there may be some interference from the dcache currently.

- Inputs
  - None

- Outputs
  - Depth buffer memory (d0)

- Notes
  - This is not a copy of the depth buffer. It's a pointer to internal memory allocated by maggie.library


## Matrix Management

The transformation pipeline goes<br>
`vertex buffer` -> `world matrix` -> `view matrix` `projection matrix` -> `viewport` -> `render target`<br>

For the most part, the semantics of the matrix names can be ignored. maggie.library will multiply them all together and only use a single matrix.<br>
Where this matters is for the texgen and the lighting. Lighting is done after applying the `world matrix`, and texgen after the `view matrix`.<br>
If those are not in use, the semantics can be ignored, as long as you put the full transform somewhere.<br>
Note that there is currently no way to affect the viewport transformation. It is simply ` * halfres + halfres`.

### magSetWorldMatrix

```c
void magSetWorldMatrix(float *matrix);
```
Set the world transformation matrix.

- Inputs
  - matrix (a0)
    - 4x4 matrix of floats for the world matrix

- Outputs
  - None

### magSetViewMatrix

```c
void magSetViewMatrix(float *matrix);
```
Set the view matrix. This is usually the inverse camera matrix.

- Inputs
  - matrix (a0)
    - 4x4 matrix of floats for the view matrix

- Outputs
  - None

### magSetPerspectiveMatrix

```c
void magSetPerspectiveMatrix(float *matrix);
```
Set the perspective projection matrix.<br>

- Inputs
  - matrix (a0)
    - 4x4 matrix of floats for the view matrix

- Outputs
  - None

- Notes
  - The maggie.library use a 16-bit w-buffer instead of the expected z-buffer. This will dictate pretty much how to set this up.
  - There is an example of this in the `maggie_vec.h` file, the `mat4_perspective` function.

## Drawing Primitives (User Pointer Mode)

### magDrawTrianglesUP

```c
void magDrawTrianglesUP(struct MaggieVertex *vtx, UWORD nVerts);
```
Draw triangles from user-provided vertex data.<br>

- Inputs
  - vtx (a0)
    - Vertex data.
  - nVerts (d0)
    - The number of vertices to draw.

- Outputs
  - None

- Notes
  - The user data will go through a transformation step on input, so it is slower than uploading a buffer for static data.
  - The vertex format is specified in the `maggie_vertex.h`.

### magDrawIndexedTrianglesUP
```c
void magDrawIndexedTrianglesUP(struct MaggieVertex *vtx, UWORD nVtx, UWORD *indx, UWORD nIndx);
```
Draw indexed triangles from user-provided data.<br>

- Inputs
  - vtx (a0)
    - Vertex data.
  - nVerts (d0)
    - The number of vertices to draw.
  - indx (a1)
    - Index data.
  - nIndx (d1)
    - Number of indices to draw.

- Outputs
  - None


- Notes
  - The user data will go through a transformation step on input, so it is slower than uploading a buffer for static data.

### magDrawIndexedPolygonsUP

```c
void magDrawIndexedPolygonsUP(struct MaggieVertex *vtx, UWORD nVtx, UWORD *indx, UWORD nIndx);
```
Draw indexed polygons from user-provided data.<br>

- Inputs
  - vtx (a0)
    - Vertex data.
  - nVerts (d0)
    - The number of vertices to draw.
  - indx (a1)
    - Index data.
  - nIndx (d1)
    - Number of indices to draw.

- Outputs
  - None


- Notes
  - The user data will go through a transformation step on input, so it is slower than uploading a buffer for static data.
  - The polygons in the index buffer are separated by a 0xffff token, so this vertex index is unavailable for use by the polygons.

## Drawing Primitives (Buffered Mode)

### magDrawTriangles

```c
void magDrawTriangles(UWORD startVtx, UWORD nVtx);
```
Draw triangles from the current vertex buffer.

- Inputs
  - startVtx (d0)
    - The start vertex to draw from.
  - nVtx (d1)
    - The number of vertices to draw.

- Outputs
  - None

### magDrawIndexedTriangles

```c
void magDrawIndexedTriangles(UWORD firstVtx, UWORD nVtx, UWORD startIndx, UWORD nIndx);
```
Draw indexed triangles from the current index and vertex buffers.

- Inputs
  - firstVtx (d0)
    - The first vertex i nthe buffer used.
  - nVtx (d1)
    - The number of vertices used.
  - startIndx (d2)
    - Start index to draw.
  - nIndx (d3)
    - Number of indices to draw

- Outputs
  - None


- Notes
  - Set nVtx so the range [firstVtx - (firstVtx +  nVtx)] includes all vertices used by the draw. nVtx is the amount of vertices the library needs to transform for the draw to be done.

### magDrawIndexedPolygons

```c
void magDrawIndexedPolygons(UWORD firstVtx, UWORD nVtx, UWORD startIndx, UWORD nIndx);
```
Draw indexed polygons from the current index and vertex buffers.

- Inputs
  - firstVtx (d0)
    - The first vertex i nthe buffer used.
  - nVtx (d1)
    - The number of vertices used.
  - startIndx (d2)
    - Start index to draw.
  - nIndx (d3)
    - Number of indices to draw

- Outputs
  - None

- Notes
  - Set nVtx so the range [firstVtx - (firstVtx +  nVtx)] includes all vertices used by the draw. nVtx is the amount of vertices the library needs to transform for the draw to be done.
  - The polygons in the index buffer are separated by a 0xffff token, so this vertex index is unavailable for use by the polygons.

## Span Drawing

### magDrawLinearSpan

NOTE: The span draw functions are disabled.
```c
void magDrawLinearSpan(struct SpanPosition *start, struct SpanPosition *end);
```

Draw a linear span between two positions.

- Inputs
  - start (a0)
    - start of span
  - end (a1)
    - end of span

- Outputs
  - None

- Notes
  - Does nothing.

### magDrawSpan

```c
void magDrawSpan(struct MaggieClippedVertex *start, struct MaggieClippedVertex *end);
```
Draw a span between two clipped vertices.

- Inputs
  - start (a0)
    - start of span
  - end (a1)
    - end of span

- Outputs
  - None

- Notes
  - Does nothing.


## Buffer Management

All buffers and textures are global and must be freed before program exit.

### magAllocateVertexBuffer

```c
UWORD magAllocateVertexBuffer(UWORD nVerts);
```
Allocate a vertex buffer.<br>
The buffer may be allocated bigger than you need, but not smaller.

- Inputs
  - nVerts (d0)
    - The max amount of vertices this buffer can hold.

- Outputs
  - Handle of the vertex buffer (d0)

### magUploadVertexBuffer

```c
void magUploadVertexBuffer(UWORD vBuffer, struct MaggieVertex *vtx, UWORD startVtx, UWORD nVerts);
```
Upload vertex data to a buffer.<br>

- Inputs
  - vBuffer (d0)
    - handle of the vertex buffer to upload to.
  - vtx (a0)
    - The vertex data to upload.
  - startVtx (d1)
    - The start vertex of the upload.
  - nVerts (d2)
    - The number of vertices to upload

- Outputs
  - None

- Notes
  - There is some data transformation when uploading vertex data, so it may be slower than expected.

### magFreeVertexBuffer

```c
void magFreeVertexBuffer(UWORD vBuffer);
```

Free a vertex buffer.

- Inputs
  - vBuffer (d0)
    - Handle of the vertex buffer to free.

- Outputs
  - None

### magAllocateIndexBuffer

```c
UWORD magAllocateIndexBuffer(UWORD nIndx);
```
Allocate an index buffer.

- Inputs
  - nIndx (d0)
    - The max amount of indices this buffer can hold.

- Outputs
  - Handle of the index buffer (d0)

### magUploadIndexBuffer

```c
void magUploadIndexBuffer(UWORD iBuffer, UWORD *indx, UWORD startIndx, UWORD nIndx);
```
Upload index data to a buffer.

- Inputs
  - vBuffer (d0)
    - Index buffer to upload to.
  - vtx (a0)
    - The index data to upload.
  - startVtx (d1)
    - The start index of the upload.
  - nVerts (d2)
    - The number of indices to upload

- Outputs
  - None

### magFreeIndexBuffer

```c
void magFreeIndexBuffer(UWORD iBuffer);
```
Free the index buffer.

- Inputs
  - iBuffer (d0)
    - Handle of the index buffer to free.

- Outputs
  - None

## Texture management

### magAllocateTexture

```c
UWORD magAllocateTexture(UWORD size);
```
Allocate a texture.

- Inputs
  - size (d0)
    - Log2 of the size of the texture.


- Notes
  - All textures are square, and power of two.
  - The size parameter is the log2 of the texture resolution (e.g. a 256x256 texture has a size of 8).

### magUploadTexture

```c
void magUploadTexture(UWORD txtr, UWORD mipmap, APTR data, UWORD format);
```

Upload texture data.

- Inputs
  - txtr (d0)
    - Handle of texture.
  - mipmap (d1)
    - The size of the mipmap to upload.
  - data (a0)
    - texture data.
  - format (d2)
    - Specify the formt of the data to upload.

- Outputs
  - None

- Notes
  - All textures have all mipmaps allocated.
  - The mipmap parameter is the log2 of the texture resolition.
  - The input format can be `MAG_TEXFMT_DXT1`, `MAG_TEXFMT_RGB`, or `MAG_TEXFMT_RGBA`.
  - The format used for texture data is always `MAG_TEXFMT_DXT1`.
    - When compressing the data, you can specify `MAG_TEXCOMP_HQ` to get a somewhat better quality against longer compression time.
  - `MAG_TEXFMT_RGBA` input is compressed to the 1 bit of alpha that DXT1 carries: a texel with an alpha below 128 becomes transparent, and Maggie leaves those pixels untouched when rasterizing. Alpha is never blended - it is either fully on or fully off.
    - The colour of a transparent texel is discarded, so it does not have to be a sensible colour, but the rest of the block still has to fit three colours instead of four. Keep a texture opaque if you do not need the cutout.

It's the same as for the size parameter for the `magAllocateTexture` function.

### magFreeTexture

```c
void magFreeTexture(UWORD txtr);
```

Free a texture.

- Inputs
  - txtr (d0)
    - Handle of texture.

- Outputs
  - None

## Scene and Drawing Control

### magBeginScene

```c
void magBeginScene();
```

Begin a new scene.

- Inputs
  - None

- Outputs
  - None

- Notes
  - This will take the library lock, so no other apps gets access until magEndScene is called.
  - The function also invalidates all matrices, lighting state, depth buffer and draw mode.

### magEndScene

```c
void magEndScene();
```

End the current scene.

- Inputs
  - None

- Outputs
  - None

- Notes
  - This will release the library lock.

## Immediate mode drawing

Immediate mode drawing is by far the slowest way to draw stuff with this library.<br>
It's good for getting things drawing quickly, but for anything more serious, the buffered draw functions are way faster.

### magBegin

```c
void magBegin();
```
Begin immediate mode drawing.<br>

- Inputs
  - None

- Outputs
  - None

- Notes
  - Only triangles are allowed.
  - Immediate drawing is quite slow as it just collects all the vertex attributes, and calls `magDrawTrianglesUP`.

### magEnd

```c
void magEnd();
```
Submit primitives for rendering.

- Inputs
  - None

- Outputs
  - None

### magVertex

```c
void magVertex(float x, float y, float z);
```
Specify a position in immediate mode.<br>

- Inputs
  - x (fp0)
  - y (fp1)
  - z (fp2)

- Outputs
  - None

- Notes
  - This will submit the current vertex, like with OpenGLs glBegin/glEnd.

### magNormal

```c
void magNormal(float x, float y, float z);
```
Specify a normal in immediate mode.

- Inputs
  - x (fp0)
  - y (fp1)
  - z (fp2)

- Outputs
  - None

### magTexCoord

```c
void magTexCoord(int texReg, float u, float v);
```
Specify 2D texture coordinate in immediate mode.

- Inputs
  - texReg (d0)
  - u (fp0)
  - v (fp1)

- Outputs
  - None

### magTexCoord3

```c
void magTexCoord3(int texReg, float u, float v, float w);
```
Specify 3D texture coordinate in immediate mode.

- Inputs
  - texReg (d0)
  - u (fp0)
  - v (fp1)
  - w (fp2)

- Outputs
  - None

### magColour

```c
void magColour(ULONG col);
```
Specify a colour in immediate mode.

- Inputs
  - col (d0)
    - RGBA colour

- Outputs
  - None

## Clearing and Scissoring

### magClear

```c
void magClear(UWORD buffers);
```
Clear specified buffers.

- Inputs
  - buffers (d0)
    - Flags specifying which buffers to clear

- Output
  - None

- Notes
  - `MAG_CLEAR_COLOUR` clear the colour buffer.
  - `MAG_CLEAR_DEPTH` clear the depth buffer.
  - `MAG_CLEAR_COLOUR | MAG_CLEAR_DEPTH` will clear both buffers.

### magClearColour

```c
void magClearColour(ULONG colour);
```
Set the clear colour.

- Inputs
  - col (d0)
    - RGBA colour

- Output
  - None

### magClearDepth

```c
void magClearDepth(UWORD depth);
```
Set the clear depth value.

- Inputs
  - depth (d0)
    - Depth to clear (usually $ffff)

- Output
  - None

### magScissor

```c
void magScissor(UWORD x, UWORD y, UWORD width, UWORD height);
```

Set the scissor rectangle for the draw.

- Inputs
  - x (d0)
  - y (d1)
  - width (d2)
  - height (d3)

- Output
  - None

- Notes
  - The values are clamped. x/y > 0x7fff clamps to 0
  - All values are in pixels

## Lighting

### magSetLightType

```c
void magSetLightType(UWORD light, UWORD type);
```

Set the type of a light source.

- Inputs
  - light (d0)
    - light number to set
  - type (d1)
    - `MAG_LIGHT_OFF` - Turn off this light.
    - `MAG_LIGHT_POINT` - This light is a point light source.
    - `MAG_LIGHT_DIRECTIONAL` - This light is a directional light source.
    - `MAG_LIGHT_SPOT` - This light is a spot light source.

- Outputs
  - None

- Notes
  - Lighting will replace the colour attribute in the vertices, and is only done when the `MAG_DRAWMODE_LIGHTING` draw mode is used.
  - There are a maximum of `MAG_MAX_LIGHTS` lights. It's currently set to 8.
  - All lights are set to `MAG_LIGHT_OFF` when `magBeginScene` is called.
  - The parameters are comparable (but not equal to) to the fixed function light sources in OpenGL.

### magSetLightPosition

```c
void magSetLightPosition(UWORD light, float x, float y, float z);
```

Set the position of a light.

- Inputs
  - light (d0)
    - Light number to set.
  - x (fp0)
  - y (fp1)
  - z (fp2)

- Outputs
  - None

- Notes
  - Unused for the `MAG_LIGHT_DIRECTIONAL` light type.

### magSetLightDirection

```c
void magSetLightDirection(UWORD light, float x, float y, float z);
```

Set the direction of a light.

- Inputs
  - light (d0)
    - Light number to set.
  - x (fp0)
  - y (fp1)
  - z (fp2)

- Outputs
  - None

- Notes
  - Unused for the `MAG_LIGHT_POINT` light type.

### magSetLightCone

```c
void magSetLightCone(UWORD light, float phi);
```
Set the cone angle for a spotlight.

- Inputs
  - light (d0)
    - Light number to set.
  - phi (fp0)
    - The angle of the cone in radians.

- Outputs
  - None


- Notes
  - Only used for the `MAG_LIGHT_SPOT` light type.

### magSetLightAttenuation

```c
void magSetLightAttenuation(UWORD light, float attenuation);
```
Set the attenuation factor for a light.

- Inputs
  - light (d0)
    - Light number to set.
  - attenuation (fp0)
    - How far the light reaches.

- Notes
  - The light will follow some inverse square law.

### magSetLightColour

```c
void magSetLightColour(UWORD light, ULONG colour);
```

Set the colour of a light.

- Inputs
  - light (d0)
    - Light number.
  - colour (d1)
    - RGBA value to use

- Outputs
  - None

- Notes
  - The colour is converted to grayscale, as the Maggie chip only has an intensity interpolator.

## Sprites

All sprites are drawn facing the view, so `MAG_DRAWMODE_AFFINE_MAPPING` is recommended.<br>
If not specified, the sprites will select the perspective pass, even though it's not needed.<br>
Subpixel corrections are always applied.

### magDrawSprites

```c
void magDrawSprites(UWORD startVtx, UWORD nSprites, float spriteSize);
```
Draw sprites from the current vertex buffer.

- Inputs
  - startVtx (d0)
    - First vertex to use.
  - nSprites (d1)
    - Number of sprites to draw.
  - spriteSize (fp0)
    - Size of the sprites.

- Outputs
  - None
  
-Notes
	This will only use the position and colour attribute from the vertex buffer.

### magDrawSpritesUP

```c
void magDrawSpritesUP(struct MaggieSpriteVertex *vtx, UWORD nSprites, float spriteSize);
```
Draw sprites from user-provided data.

- Inputs
  - vtx (a0)
    - Array of 'MaggieSpriteVertex' objects.
  - nSprites (d0)
    - Number of sprites to draw.
  - spriteSize (fp0)
    - Size of the sprites.

- Outputs
  - None
 
## Vertex Attribute Uploads

These funntions are for when you want to upload a single attribute for a vertex buffer.

### magUploadVertexPositions

```c
void magUploadVertexPositions(UWORD vBuffer, struct vec3 *vtx, UWORD startVtx, UWORD nVerts);
```
Upload vertex positions.

- Inputs
  - vBuffer (d0)
    - Handle to vertex buffer.
  - vtx (a0)
    - Positions to upload, in float xyz triplets.
  - startVtx (d1)
    - First vertex position to modify.
  - nVerts (d2)

- Outputs
  - None

### magUploadVertexNormals

```c
void magUploadVertexNormals(UWORD vBuffer, struct vec3 *normals, UWORD startVtx, UWORD nVerts);
```

Upload vertex normals.

- Inputs
  - vBuffer (d0)
    - Handle to vertex buffer.
  - vtx (a0)
    - Normals to upload, in float xyz triplets.
  - startVtx (d1)
    - First vertex normal to modify.
  - nVerts (d2)

- Outputs
  - None

### magUploadVertexTexCoords2

```c
void magUploadVertexTexCoords2(UWORD vBuffer, struct vec2 *texCoords, UWORD startVtx, UWORD nVerts);
```
Upload 2D texture coordinates.

- Inputs
  - vBuffer (d0)
    - Handle to vertex buffer.
  - vtx (a0)
    - Texture coordinates to upload, in float uv pairs.
  - startVtx (d1)
    - First vertex's texture coordinate to modify.
  - nVerts (d2)

- Outputs
  - None

### magUploadVertexTexCoords3

```c
void magUploadVertexTexCoords3(UWORD vBuffer, struct vec3 *texCoords, UWORD startVtx, UWORD nVerts);
```

Upload 3D texture coordinates.

- Inputs
  - vBuffer (d0)
    - Handle to vertex buffer.
  - vtx (a0)
    - Texture coordinates to upload, in float uvw triplets.
  - startVtx (d1)
    - First vertex's texture coordinate to modify.
  - nVerts (d2)

- Outputs
  - None

### magUploadVertexColours

```c
void magUploadVertexColours(UWORD vBuffer, ULONG *colours, UWORD startVtx, UWORD nVerts);
```
Upload vertex colours.

- Inputs
  - vBuffer (d0)
    - Handle to vertex buffer.
  - vtx (a0)
    - Colours to upload, in float uvw triplets.
  - startVtx (d1)
    - First vertex colour to modify.
  - nVerts (d2)

- Outputs
  - None

- Notes
  - The colours are all converted to grayscale on upload, because Maggie only interpolates intensities.
