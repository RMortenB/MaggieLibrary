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

#define MAGGIE_MAX_XRES 1920
#define MAGGIE_MAX_YRES 1080

/*****************************************************************************/

#define PIXEL_RUN 32

#define PROFILE 0

/*****************************************************************************/

#define REG(a,v) v __asm(#a)

/*****************************************************************************/

#define MAX_VERTEX_BUFFERS 1024
#define MAX_INDEX_BUFFERS 1024
#define MAX_TEXTURES 1024

/*****************************************************************************/

#define IMM_MODE_MAGGIE_VERTS 1023

/*****************************************************************************/

typedef struct
{
	float xPos;
	float zow;
	float oow;
	float uow;
	float vow;
	float iow;
} magEdgePos;

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

struct MaggieBase;
typedef struct MaggieBase MaggieBase;

/*****************************************************************************/

struct MaggieBase
{
	struct Library lib;
	struct ExecBase *sysBase;
	int segList;
	int initialised;

	/*******************/

	struct GfxBase *gfxBase;

	struct SignalSemaphore lock;

	/*******************/

	UWORD xres;
	UWORD yres;
	APTR screen;
	UWORD *depth;

	/*******************/

	ULONG clearColour;
	UWORD clearDepth;

	/*******************/

	UWORD texSize;
	APTR texture;

	/*******************/

	UWORD drawMode;

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

	int vBuffer;
	int iBuffer;
	int txtrIndex;

	/*******************/

	UWORD *depthBuffer;

	/*******************/

	int nIModeVtx;
	UWORD immModeVtx;
	struct MaggieVertex ImmVtx;

	/*******************/

	ULONG *vertexBuffers[MAX_VERTEX_BUFFERS];
	ULONG *indexBuffers[MAX_INDEX_BUFFERS];
	ULONG *textures[MAX_TEXTURES];

	/*******************/

	magLight lights[MAG_MAX_LIGHTS];

	/*******************/

	magEdgePos magLeftEdge[MAGGIE_MAX_YRES];
	magEdgePos magRightEdge[MAGGIE_MAX_YRES];

#if PROFILE
	struct
	{
		ULONG lines;
		ULONG spans;
		ULONG trans;
		ULONG frame;
		ULONG clear;

		ULONG count;
		ULONG linesmin;
		ULONG spansmin;
		ULONG transmin;
		ULONG clearmin;
		ULONG framemin;
		ULONG linesmax;
		ULONG spansmax;
		ULONG transmax;
		ULONG clearmax;
		ULONG framemax;
	} profile;
#endif
};

/*****************************************************************************/

typedef struct
{
	APTR	texture;			/* 32bit texture source */
	APTR	pixDest;			/* 32bit Destination Screen Addr */
	APTR 	depthDest;			/* 32bit ZBuffer Addr */
	UWORD	unused0;
	UWORD	startLength;		/* 16bit LEN and START */
	UWORD	texSize;			/* 16bit MIP texture size (9=512/8=256/7=128/6=64) */
	UWORD	mode;				/* 16bit MODE (Bit0=Bilienar) (Bit1=Zbuffer) (Bit2=16bit output) */
	UWORD	unused1;
	UWORD	modulo;				/* 16bit Destination Step */
	ULONG	unused2;
	ULONG	unused3;
	ULONG	uCoord;				/* 32bit U (8:24 normalised) */
	ULONG	vCoord;				/* 32bit V (8:24 normalised) */
	ULONG	uDelta;				/* 32bit dU (8:24 normalised) */
	ULONG	vDelta;				/* 32bit dV (8:24 normalised) */
	UWORD	light;				/* 16bit Light Ll (8:8) */
	UWORD	lightDelta;			/* 16bit Light dLl (8:8) */
	ULONG	lightRGBA;			/* 32bit Light color (ARGB) */
	ULONG	depthStart;			/* 32bit Z (16:16) */
	ULONG	depthDelta;			/* 32bit Delta (16:16) */
} __attribute__((packed)) MaggieRegs;

/*****************************************************************************/

static volatile MaggieRegs * const maggieRegs = (MaggieRegs *)0xdff250;

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

/*****************************************************************************/
// All Buffers/textures are GLOBAL, and must be freed at exit.
/*****************************************************************************/

// Allocate vertex buffers.

UWORD magAllocateVertexBuffer(REG(d0, UWORD nVerts), REG(a6, MaggieBase *lib));
void magUploadVertexBuffer(REG(d0, UWORD vBuffer), REG(a0, struct MaggieVertex *vtx), REG(d1, UWORD startVtx), REG(d2, UWORD nVerts), REG(a6, MaggieBase *lib));
void magFreeVertexBuffer(REG(d0, UWORD vBuffer), REG(a6, MaggieBase *lib));

/*****************************************************************************/

// Allocate index buffers

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

ULONG GetTextureMipMapSize(UWORD texSize);
ULONG GetTextureSize(UWORD texSize);
ULONG GetTexturePixelWidth(UWORD texSize);
ULONG GetTexturePixelHeight(UWORD texSize);
ULONG GetTextureMipMapOffset(UWORD topLevel, UWORD mipmap);
APTR GetTextureData(ULONG *mem);

/*****************************************************************************/

void PrepareVertexBuffer(struct MaggieTransVertex *transDst, struct MaggieVertex *vtx, UWORD nVerts);
void TransformVertexBuffer(struct MaggieTransVertex * restrict dstVtx, struct MaggieVertex * restrict vtx, UWORD nVerts, MaggieBase *lib);
void TexGenBuffer(struct MaggieTransVertex *dstVtx, struct MaggieVertex *vtx, UWORD nVerts, MaggieBase *lib);

/*****************************************************************************/

ULONG RGBToGrayScale(ULONG rgb);

/*****************************************************************************/

UWORD GetVBNumVerts(ULONG *mem);
struct MaggieVertex *GetVBVertices(ULONG *mem);
UBYTE *GetVBClipCodes(ULONG *mem);
struct MaggieTransVertex *GetVBTransVertices(ULONG *mem);

/*****************************************************************************/

UWORD GetIBNumIndices(ULONG *mem);
UWORD *GetIBIndices(ULONG *mem);

/*****************************************************************************/

void DrawEdge(struct MaggieTransVertex *vtx0, struct MaggieTransVertex *vtx1, MaggieBase *lib);

/*****************************************************************************/

void DrawSpans(int miny, int maxy, MaggieBase *lib);

/*****************************************************************************/

void FlushImmediateMode(MaggieBase *lib);

/*****************************************************************************/

int ClipPolygon(struct MaggieTransVertex *verts, int nVerts);

/*****************************************************************************/

void LightBuffer(struct MaggieTransVertex *dest, struct MaggieVertex *src, int nVerts, MaggieBase *lib);

/*****************************************************************************/
// Asm functions

void magFastClear(void *buffer __asm("a0"), ULONG nBytes __asm("d0"), ULONG data __asm("d1"));

/*****************************************************************************/


#if PROFILE
ULONG GetClocks();
#endif

#endif // MAGGIE_INTERNAL_H_INCLUDED
