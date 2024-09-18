
# maggie.library

 The maggie.library is a helper library to draw texture mapped geometry with the Maggie chipset.

### Drawing with the maggie.library is sort of simple

All calls to maggie for drawing a given frame should be wrapped in

```
void magBeginScene()
void magEndScene()
```

This will take a library wide semaphore to stop other tasks for accessing it.
If you're taking over the system, you may skip this step. The semaphore may be taken for a few of the other functions normally only used during setup, e.g. allocate/free.

### There are 3 different paths to drawing geometry (fastest to slowest)

#### 1. Draw with buffers
```
void magDrawTriangles(UWORD startIndex, UWORD nIndices)
void magDrawIndexedTriangles(UWORD startVertex, UWORD nVertices, UWORD startIndex, UWORD nIndices)
void magDrawIndexedPolygons(UWORD startVertex, UWORD nVertices, UWORD startIndex, UWORD nIndices)
```

To hand over Vertex and Index data, you need to allocate buffers
```
UWORD vertexBuffer = magAllocateVertexBuffer(UWORD numberOfVertices)
UWORD indexBuffer = magAllocateIndexBuffer(UWORD numberOfIndices)
```

Data to the buffers is uploaded with
```
void magUploadVertexData(UWORD vBuffer, struct MaggieVertex *vtx, UWORD startVtx, UWORD nVerts)
void magUploadIndexData(UWORD iBuffer, UWORD *indx, UWORD startIndx, UWORD nIndx)
```

Buffers should be freed with
```
void magFreeVertexBuffer(UWORD vertexBuffer)
void magFreeIndexBuffer(UWORD indexBuffer)
```
They are global.

You set the current vertex and index buffers with 
```
void magSetVertexBuffer(UWORD vertexBuffer)
void magSetIndexBuffer(UWORD indexBuffer)
```

As there is some foot work involved in prepping the data, this is the fastest path.

#### 2. Draw with user pointers
```
magDrawTrianglesUP
magDrawIndexedTrianglesUP
magDrawIndexedPolygonsUP
```

Works same as above, except with user provided data. The data pointed to isn't changed by the library.

The vertes format of 1. and 2. are
```
typedef struct vec3
{
	float x, y, z;
} vec3;

struct MaggieTexCoord
{
	float u, v, w;
};

struct MaggieVertex
{
	vec3 pos;
	vec3 normal;
	struct MaggieTexCoord tex[MAGGIE_MAX_TEXCOORDS];
	ULONG colour;
};
```

MAGGIE_MAX_TEXCOORDS is defined to be 1 at this point. It may change in the future if someone asks to use more than one texture in the draw..

#### 3. Draw with immediate mode
```
void magBegin()
void magTexCoord(float u, float v)
void magTexCoord3(float u, float v, float w)
void magColour(ULONG col)
void magVertex(float x, float y, float z)
void magEnd()
```

Textures:
The textures need to be allocated
```
UWORD texture = magAllocateTexture(UWORD size)
```

and freed with
```
void magFreeTexture(UWORD texture)
```
Texture data is uploaded with 
```
void magUploadTexture(UWORD texture, UWORD mipmap, textureData, UWORD format)
```

You can set the current texture with 
```
magSetTexture(UWORD unit, UWORD texture)
```

unit is < MAGGIE_MAX_TEXCOORDS.

The sizes supported are
* 9 - 512x512
* 8 - 256x256
* 7 - 128x128
* 6 -  64x 64

#### About polygons

The polygons should be non-concave and flat in 3D space, and flat in UV space. If they're not, they may look a little odd.
I will draw them with this assumption.
They can have up to 256 sides. There is no check for more sides.

When creating the index buffer for more than one polygon, you can insert $ffff into the index list. This will start a new polygon.
The last vertex will be closed to the first, duplicates are not needed (shouldn't matter either, it'll just be slower).

When drawing e.g flat walls, etc. it is faster to use quads than triangle pairs. You can insert co-linear vertices to make sure no cracks
appear when drawing e.g. a doorway into a flat wall.


#### About texture coordinates
Texture coordinates are normalised to 0.0f .. 1.0f range for one tile across the texture.
The limits of these values are unknown (I know there is a limit, but I don't know what it is).


#### About draw buffers

The colour buffer has to be provided, and the depth buffer is internal
You set the framebuffer pointer with 
magSetScreenMemory(APTR frame, UWORD xres, UWORD yres)

Max allowed resolution is 1280x720

The format is set with
```
void magSetDrawMode(UWORD flags)
```
Valid modes are
```
#define MAG_DRAWMODE_NORMAL		0x0000
#define MAG_DRAWMODE_DEPTHBUFFER	0x0001
#define MAG_DRAWMODE_BILINEAR		0x0002
#define MAG_DRAWMODE_32BIT		0x0004
#define MAG_DRAWMODE_LIGHTING		0x0008
```

There is a reasonable Clear function
```
void magClear(UWORD flags);
```

where the flags are 
```
#define MAG_CLEAR_COLOUR	0x0001
#define MAG_CLEAR_DEPTH		0x0002
```

Calling magClear twice, once for depth and once for colour, is not encouraged, but not a lot slower.

#### Transformations

The transformation pipe is just multiplying the vertices with 3 matrices
```
void magSetWorldMatrix(float *world)
void magSetViewMatrix(float *view)
void magSetPerspectiveMatrix(float *perspective)
```

The world and view matrices are only used for lighting, so if lighing is not needed the transform can be rolled into any of the three matrices.

Each takes a 4x4 matrix.

#### Lighting
All lighting is done after the world transform is applied to the vertices. The lights are given in this space ("world space").


Example code is available..
