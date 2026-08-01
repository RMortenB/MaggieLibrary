#ifndef MAGGIE_INTERNAL_H_INCLUDED
#define MAGGIE_INTERNAL_H_INCLUDED

/*****************************************************************************/
#include <stdint.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <exec/semaphores.h>
#include <graphics/gfxbase.h>

#include "maggie_vertex.h"
#include "maggie_vec.h"
#include "maggie_flags.h"

/*****************************************************************************/

#define MAGGIE_MAX_XRES	1920
#define MAGGIE_MAX_YRES	1080

#define SWIZZLE_BOTTOM_BIT 0

/*****************************************************************************/

#define PIXEL_RUN 16
#define PIXEL_RUNSHIFT 4

#define PROFILE 0

/*****************************************************************************/

#define REG(a,v) v __asm(#a)

/*****************************************************************************/

#define MAX_VERTEX_BUFFERS 10240
#define MAX_INDEX_BUFFERS 10240
#define MAX_TEXTURES 10240

/*****************************************************************************/

#define IMM_MODE_MAGGIE_VERTS 1023

/*****************************************************************************/

typedef struct
{
	float xPosLeft;
	float xPosRight;
	float zowLeft;
	float zowRight;
	float iowLeft;
	float iowRight;
	float oowLeft;
	float oowRight;
	float uowLeft;
	float uowRight;
	float vowLeft;
	float vowRight;
} magEdgePos;

/*****************************************************************************/

typedef struct
{
	float xPosLeft;
	float xPosRight;
	float zowLeft;
	float zowRight;
	float iowLeft;
	float iowRight;
	float uLeft;
	float uRight;
	float vLeft;
	float vRight;
} magEdgePosAffine;

/*****************************************************************************/

typedef struct
{
	float oowDDA;
	float uowDDA;
	float vowDDA;
	float zDDA;
	float iDDA;
} magGradients;

/*****************************************************************************/

typedef struct
{
	int type;
	vec3 pos;
	vec3 dir;
	ULONG colour;
	float attenuation;
	float phi;
} magLight;

/*****************************************************************************/

typedef struct
{
	int x0;
	int y0;
	int x1;
	int y1;
} ScissorRect;

/*****************************************************************************/

struct MaggieBase;
typedef struct MaggieBase MaggieBase;

/*****************************************************************************/

typedef struct
{
	WORD x, y, z; // 8:8 fixed point
} MaggieNormal;

/*****************************************************************************/

typedef struct
{
	UWORD nVerts;
	ULONG memSize;
	vec3 *positions;
	MaggieNormal *normals;
	struct MaggieTexCoord *uvs;
	ULONG *colours;
	UBYTE *clipCodes;
	struct MaggieTransVertex *transVerts;
} VertexBufferMemory;

/*****************************************************************************/

typedef struct
{
	ULONG texSize;
	ULONG memSize;
	UWORD mipMaps;
	UWORD format;
	UBYTE *allocPtr;
	UBYTE alignmentPadding[32 - (sizeof(ULONG) * 3 + sizeof(UWORD) * 2)];
	UBYTE data[];
} magTexture;

/*****************************************************************************/

struct MaggieBase
{
	struct Library lib;
	struct ExecBase *sysBase;
	int segList;
	int initialised;
	int hasMaggie;

	/*******************/

	struct GfxBase *gfxBase;

	/*******************/

	struct SignalSemaphore lock;

	/*******************/

	UWORD xres;
	UWORD yres;
	APTR screen;
	UWORD *depthBuffer;

	/*******************/

	ULONG clearColour;
	UWORD clearDepth;

	/*******************/

	UWORD texSize;
	APTR texture;

	/*******************/

	UWORD drawMode;
	UWORD textureFlags;

	/*******************/

	ULONG colour;

	/*******************/

	mat4 worldMatrix;
	mat4 viewMatrix;
	mat4 perspectiveMatrix;

	mat4 modelViewProj;
	mat4 modelView;
	int dirtyMatrix;

	/*******************/

	ScissorRect scissor;
	union
	{
		magEdgePos magEdge[MAGGIE_MAX_YRES];
		magEdgePosAffine magEdgeAffine[MAGGIE_MAX_YRES];
	};

	// Per-polygon fan-sum gradients: written by ComputeGradients, read by the
	// perspective+depth rasterizer via GetGradientsPtr. Placed right after the
	// magEdge union so the hardcoded asm offsets (xres/screen/depth/scissor/edges
	// = 100/104/108/456/472) stay put. Its own offset is guarded by a
	// _Static_assert in maggie_draw.c against GetGradientsPtr in raster_structs.i.
	magGradients gradients;

	/*******************/

	int vBuffer;
	int iBuffer;
	int txtrIndex;

	/*******************/

	int nIModeVtx;
	UWORD immModeVtx;
	struct MaggieVertex ImmVtx;

	UWORD upVertexBuffer;
	UWORD upIndexBuffer;

	/*******************/

	VertexBufferMemory *vertexBuffers[MAX_VERTEX_BUFFERS];
	ULONG *indexBuffers[MAX_INDEX_BUFFERS];
	magTexture *textures[MAX_TEXTURES];

	/*******************/

	magLight lights[MAG_MAX_LIGHTS];

	/*******************/

#if PROFILE
	struct
	{
		ULONG nLinePixels;
		ULONG lines;
		ULONG spans;
		ULONG trans;
		ULONG frame;
		ULONG clear;
		ULONG light;
		ULONG draw;
		ULONG nPixels;
		ULONG texgen;

		ULONG count;
	} profile;
#endif
	int frameCounter;
	APTR dummyTextureData;

	// Two-path rasterizer selection (C-side only, not read by asm):
	// sourceIsPoly = the primitive being drawn came from magDrawIndexedPolygons.
	// polyIntensity = use the per-scanline intensity rasterizer path, set by the
	// draw dispatchers to (sourceIsPoly && vertexCount > 3): a >3-vertex polygon
	// has non-planar Gouraud intensity that the fan-sum gradient can't represent,
	// so it needs edge-to-edge interpolation. Triangles (and clipped-to-triangle
	// polys, and clipped triangles that became polys) are planar -> triangle path.
	int sourceIsPoly;
	int polyIntensity;
};

/*****************************************************************************/

typedef struct
{
	APTR	texture;			/*  0 | 32bit texture source */
	APTR	pixDest;			/*  4 | 32bit Destination Screen Addr */
	APTR 	depthDest;			/*  8 | 32bit ZBuffer Addr */
	UWORD	unused0;			/* 12 */
	UWORD	startLength;		/* 14 | 16bit LEN and START */
	UWORD	texSize;			/* 16 | 16bit MIP texture size (10=1024/9=512/8=256/7=128/6=64) */
	UWORD	mode;				/* 18 | 16bit MODE (Bit0=Bilienar) (Bit1=Zbuffer) (Bit2=16bit output) */
	UWORD	unused1;			/* 20 */
	UWORD	modulo;				/* 22 | 16bit Destination Step */
	ULONG	unused2;			/* 24 */
	ULONG	unused3;			/* 28 */
	ULONG	uCoord;				/* 32 | 32bit U (8:24 normalised) */
	ULONG	vCoord;				/* 36 | 32bit V (8:24 normalised) */
	LONG	uDelta;				/* 40 | 32bit dU (8:24 normalised) */
	LONG	vDelta;				/* 44 | 32bit dV (8:24 normalised) */
	UWORD	light;				/* 48 | 16bit Light Ll (8:8) */
	WORD	lightDelta;			/* 50 | 16bit Light dLl (8:8) */
	ULONG	lightRGBA;			/* 52 | 32bit Light color (ARGB) */
	ULONG	depthStart;			/* 56 | 32bit Z (16:16) */
	LONG	depthDelta;			/* 58 | 32bit Delta (16:16) */
} __attribute__((packed)) MaggieRegs;

/*****************************************************************************/

extern volatile MaggieRegs maggieRegs;

/*****************************************************************************/
/* This is the "public section". Internal prototypes that'll go into headers */
/*****************************************************************************/

// These are reset on EndDraw.
void magSetScreenMemory(REG(a0, APTR *pixels), REG(d0, UWORD xres), REG(d1, UWORD yres), REG(a6, MaggieBase *lib));
void magSetTexture(REG(d0, UWORD unit), REG(d1, UWORD txtr), REG(a6, MaggieBase *lib));
void magSetDrawMode(REG(d0, UWORD mode), REG(a6, MaggieBase *lib));
void magSetRGB(REG(d0, ULONG rgb), REG(a6, MaggieBase *lib));

/*****************************************************************************/

UWORD *magGetDepthBuffer(REG(a6, MaggieBase *lib)); // This is the live depth buffer!

/*****************************************************************************/
/*****************************************************************************/

// These are reset on EndDraw.
void magSetWorldMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib));
void magSetViewMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib));
void magSetPerspectiveMatrix(REG(a0, float *matrix), REG(a6, MaggieBase *lib));
void magScissor(REG(d0, UWORD x0), REG(d1, UWORD y0), REG(d2, UWORD x1), REG(d3, UWORD x2), REG(a6, MaggieBase *lib));

/*****************************************************************************/

// Calls OwnBlitter/DisownBlitter
void magDrawTrianglesUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a6, MaggieBase *lib));
void magDrawIndexedTrianglesUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a1, UWORD *indx), REG(d1, UWORD nIndx), REG(a6, MaggieBase *lib));
void magDrawIndexedPolygonsUP(REG(a0, struct MaggieVertex *vtx), REG(d0, UWORD nVerts), REG(a1, UWORD *indx), REG(d1, UWORD nIndx), REG(a6, MaggieBase *lib));
// TODO : Clipped / screenspace path

/*****************************************************************************/

void magSetVertexBuffer(REG(d0, WORD vBuffer), REG(a6, MaggieBase *lib));
void magSetIndexBuffer(REG(d0, WORD iBuffer), REG(a6, MaggieBase *lib));

/*****************************************************************************/

// Calls OwnBlitter/DisownBlitter
void magDrawTriangles(REG(d0, UWORD startVtx), REG(d1, UWORD nVerts), REG(a6, MaggieBase *lib));
void magDrawIndexedTriangles(REG(d0, UWORD firstVtx), REG(d1, UWORD nVerts), REG(d2, UWORD startIndx), REG(d3, UWORD nIndx), REG(a6, MaggieBase *lib));
void magDrawIndexedPolygons(REG(d0, UWORD firstVtx), REG(d1, UWORD nVerts), REG(d2, UWORD startIndx), REG(d3, UWORD nIndx), REG(a6, MaggieBase *lib));
// TODO : fast partial draw path.

/*****************************************************************************/

void magDrawLinearSpan(REG(a0, struct SpanPosition *start), REG(a1, struct SpanPosition *end), REG(a6, MaggieBase *lib));
void magDrawSpan(REG(a0, struct MaggieClippedVertex *start), REG(a1, struct MaggieClippedVertex *end), REG(a6, MaggieBase *lib));

/*****************************************************************************/

void magDrawSprites(REG(d0, UWORD startVtx), REG(d1, UWORD nSprites), REG(fp0, float spriteSize), REG(a6, MaggieBase *lib));
void magDrawSpritesUP(REG(a0, struct MaggieSpriteVertex *vtx), REG(d0, UWORD nSprites), REG(fp0, float spriteSize), REG(a6, MaggieBase *lib));

/*****************************************************************************/
// All Buffers/textures are GLOBAL, and must be freed at exit.
/*****************************************************************************/
// vertex buffers.

UWORD magAllocateVertexBuffer(REG(d0, UWORD nVerts), REG(a6, MaggieBase *lib));
void magUploadVertexBuffer(REG(d0, UWORD vBuffer), REG(a0, struct MaggieVertex *vtx), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib));
void magFreeVertexBuffer(REG(d0, UWORD vBuffer), REG(a6, MaggieBase *lib));

/*****************************************************************************/
// Upload individual vertex attributes. The start and counts are in vertices.
void magUploadVertexPositions(REG(d0, UWORD vBuffer), REG(a0, vec3 *vtx), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib));
void magUploadVertexNormals(REG(d0, UWORD vBuffer), REG(a0, vec3 *normals), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib));
void magUploadVertexNormalsPacked(REG(d0, UWORD vBuffer), REG(a0, signed char *vtx), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib));
void magUploadVertexTexCoords2(REG(d0, UWORD vBuffer), REG(a0, vec2 *texCoords), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib));
void magUploadVertexTexCoords3(REG(d0, UWORD vBuffer), REG(a0, vec3 *texCoords), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib));
void magUploadVertexColours(REG(d0, UWORD vBuffer), REG(a0, ULONG *vtx), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib));

/*****************************************************************************/
// index buffers

UWORD magAllocateIndexBuffer(REG(d0, UWORD nIndx), REG(a6, MaggieBase *lib));
void magUploadIndexBuffer(REG(d0, UWORD iBuffer), REG(a0, UWORD *indx), REG(d1, UWORD startIndx), REG(d2, UWORD nIndx), REG(a6, MaggieBase *lib));
void magFreeIndexBuffer(REG(d0, UWORD iBuffer), REG(a6, MaggieBase *lib));

/*****************************************************************************/

// Allocate texture
UWORD magAllocateTexture(REG(d0, UWORD size), REG(a6, MaggieBase *lib));
void magUploadTexture(REG(d0, UWORD txtr), REG(d1, UWORD mipmap), REG(a0, APTR data), REG(d2, UWORD format), REG(a6, MaggieBase *lib));
void magFreeTexture(REG(d0, UWORD txtr), REG(a6, MaggieBase *lib));

/*****************************************************************************/

// Library semaphore Lock. May reset _all_ state.
void magBeginScene(REG(a6, MaggieBase *lib));
void magEndScene(REG(a6, MaggieBase *lib));

/*****************************************************************************/

// Immediate mode, or "slow mode"..
// Calls OwnBlitter/DisownBlitter.
void magBegin(REG(a6, MaggieBase *lib));
void magEnd(REG(a6, MaggieBase *lib));

void magVertex(REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(a6, MaggieBase *lib));
void magNormal(REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(a6, MaggieBase *lib));
void magTexCoord(REG(d0, UWORD texReg), REG(fp0, float u), REG(fp1, float v), REG(a6, MaggieBase *lib));
void magTexCoord3(REG(d0, UWORD texReg), REG(fp0, float u), REG(fp1, float v), REG(fp2, float w), REG(a6, MaggieBase *lib));
void magColour(REG(d0, ULONG col), REG(a6, MaggieBase *lib));

/*****************************************************************************/

void magClear(REG(d0, UWORD buffers), REG(a6, MaggieBase *lib));
void magClearColour(REG(d0, ULONG colour), REG(a6, MaggieBase *lib));
void magClearDepth(REG(d0, UWORD depth), REG(a6, MaggieBase *lib));

/*****************************************************************************/

void magSetLightType(REG(d0, UWORD light), REG(d1, UWORD type), REG(a6, MaggieBase *lib));
void magSetLightPosition(REG(d0, UWORD light), REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(a6, MaggieBase *lib));
void magSetLightDirection(REG(d0, UWORD light), REG(fp0, float x), REG(fp1, float y), REG(fp2, float z), REG(a6, MaggieBase *lib));
void magSetLightCone(REG(d0, UWORD light), REG(fp0, float phi), REG(a6, MaggieBase *lib));
void magSetLightAttenuation(REG(d0, UWORD light), REG(fp0, float attenuation), REG(a6, MaggieBase *lib));
void magSetLightColour(REG(d0, UWORD light), REG(d1, ULONG colour), REG(a6, MaggieBase *lib));

/*****************************************************************************/

// Private functions

ULONG GetTextureMipMapSize(UWORD format, UWORD texSize);
ULONG GetTextureSize(UWORD format, UWORD texSize);
ULONG GetTexturePixelWidth(UWORD texSize);
ULONG GetTexturePixelHeight(UWORD texSize);
ULONG GetTextureMipMapOffset(UWORD format, UWORD topLevel, UWORD mipmap);

/*****************************************************************************/

void CompressRGB(UBYTE *dst, UBYTE *src, int width, int height, int pixelSize, int quality, MaggieBase *lib);
void DeCompressDXT1(UBYTE *dst, UBYTE *src, int width, int height, MaggieBase *lib);
void SwizzleDXT1Texture(APTR data, int xres, int yres);

/*****************************************************************************/

void PrepareVertexBuffer(struct MaggieTransVertex *transDst, struct MaggieVertex *vtx, UWORD nVerts);
void TransformVertexPositions(struct MaggieTransVertex * restrict dstVtx, vec3 * restrict vtx, UWORD nVerts, MaggieBase *lib);
void TexGenBuffer(VertexBufferMemory *src, int startIndex, int nVerts, MaggieBase *lib);


/*****************************************************************************/
// For UP stuff..
void TransformVertexPositionsFromVertices(struct MaggieTransVertex * restrict dstVtx, struct MaggieVertex * restrict vtx, UWORD nVerts, MaggieBase *lib);

/*****************************************************************************/

void TransformToSpriteBuffer(struct MaggieSpriteVertex * restrict dstVtx, vec3 * restrict vtx, UWORD nVerts, MaggieBase *lib);
void TransformSpriteBuffer(struct MaggieSpriteVertex * restrict dstVtx, struct MaggieSpriteVertex * restrict vtx, UWORD nVerts, MaggieBase *lib);

/*****************************************************************************/

ULONG RGBToGrayScale(ULONG rgb);

/*****************************************************************************/

UWORD GetIBNumIndices(ULONG *mem);
UWORD *GetIBIndices(ULONG *mem);

/*****************************************************************************/

void DrawEdge(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, int miny, int maxy, MaggieBase *lib);

/*****************************************************************************/

void DrawSpans(int miny, int maxy, MaggieBase *lib);

/*****************************************************************************/

void FlushImmediateMode(MaggieBase *lib);

/*****************************************************************************/

int ClipPolygon(struct MaggieTransVertex *verts, int nVerts);

/*****************************************************************************/

void LightBuffer(VertexBufferMemory *src, int startIndex, int nVerts, MaggieBase *lib);

/*****************************************************************************/
// Asm functions

void magFastClear(void *buffer __asm("a0"), ULONG nBytes __asm("d0"), ULONG data __asm("d1"));

/*****************************************************************************/

static APTR GetTextureData(magTexture *txtr)
{
	return txtr->data;
}

/*****************************************************************************/

static int GetTexSizeIndex(magTexture *txtr)
{
	return txtr->texSize;
}

/*****************************************************************************/

#if PROFILE
ULONG GetClocks();
#endif

/*****************************************************************************/

UWORD GetUserVertexBuffer(MaggieBase *lib);
UWORD GetUserIndexBuffer(MaggieBase *lib);

/*****************************************************************************/

#endif // MAGGIE_INTERNAL_H_INCLUDED
