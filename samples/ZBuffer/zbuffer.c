#include <stdio.h>

#include <proto/exec.h>
#include <proto/lowlevel.h>

#include <proto/Maggie.h>
#include <proto/cybergraphics.h>
#include <maggie_vec.h>
#include <maggie_vertex.h>
#include <maggie_flags.h>
#include <cybergraphx/cybergraphics.h>

/*****************************************************************************/

#define XRES 640
#define YRES 360
#define BPP 4
#define NFRAMES 3

#define SCREENSIZE (XRES * YRES * BPP)

/*****************************************************************************/

#define MAGGIE_MODE 0x0b05

/*****************************************************************************/

struct Library *LowLevelBase;
struct Library *MaggieBase;

/*****************************************************************************/

static volatile int vblPassed;

/*****************************************************************************/

static int VBLInterrupt()
{
	vblPassed = 1;
	return 0;
}

/*****************************************************************************/

void WaitVBLPassed()
{
	while(!vblPassed)
		;
	vblPassed = 0;
}

/*****************************************************************************/

UWORD LoadTexture(const char *filename)
{
	UBYTE *data = NULL;
	UWORD txtr;
	int size;
	FILE *fp = fopen(filename, "rb");
	if(!fp)
	{
		printf("No texture %s\n", filename);
		return 0xffff;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 128, SEEK_SET);

	data = AllocMem(size - 128, MEMF_ANY);
	fread(data, 1, size - 128, fp);

	fclose(fp);

	txtr = magAllocateTexture(6);
	magUploadTexture(txtr, 6, data + 256 * 128 + 128 * 64, 0);
	FreeMem(data, size - 128);

	return txtr;
}

/*****************************************************************************/

static struct MaggieVertex CubeVertices[6 * 4] = 
{
	{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, { { 1.0f, 1.0f } }, 0x00ffffff },
	{ {-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, { { 1.0f, 0.0f } }, 0x00ffffff },
	{ { 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, { { 0.0f, 0.0f } }, 0x00ffffff },
	{ { 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, { { 0.0f, 1.0f } }, 0x00ffffff },

	{ {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, { { 0.0f, 1.0f } }, 0x00ffffff },
	{ { 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, { { 1.0f, 1.0f } }, 0x00ffffff },
	{ { 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, { { 1.0f, 0.0f } }, 0x00ffffff },
	{ {-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, { { 0.0f, 0.0f } }, 0x00ffffff },

	{ {-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, { { 0.0f, 1.0f } }, 0x00ffffff },
	{ { 1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, { { 1.0f, 1.0f } }, 0x00ffffff },
	{ { 1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, { { 1.0f, 0.0f } }, 0x00ffffff },
	{ {-1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, { { 0.0f, 0.0f } }, 0x00ffffff },

	{ {-1.0f,  1.0f, -1.0f}, {0.0f,  1.0f, 0.0f}, { { 1.0f, 1.0f } }, 0x00ffffff },
	{ {-1.0f,  1.0f,  1.0f}, {0.0f,  1.0f, 0.0f}, { { 1.0f, 0.0f } }, 0x00ffffff },
	{ { 1.0f,  1.0f,  1.0f}, {0.0f,  1.0f, 0.0f}, { { 0.0f, 0.0f } }, 0x00ffffff },
	{ { 1.0f,  1.0f, -1.0f}, {0.0f,  1.0f, 0.0f}, { { 0.0f, 1.0f } }, 0x00ffffff },

	{ {-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, { { 1.0f, 0.0f } }, 0x00ffffff },
	{ {-1.0f, -1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, { { 0.0f, 0.0f } }, 0x00ffffff },
	{ {-1.0f,  1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, { { 0.0f, 1.0f } }, 0x00ffffff },
	{ {-1.0f,  1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, { { 1.0f, 1.0f } }, 0x00ffffff },

	{ { 1.0f, -1.0f, -1.0f}, { 1.0f, 0.0f, 0.0f}, { { 1.0f, 1.0f } }, 0x00ffffff },
	{ { 1.0f,  1.0f, -1.0f}, { 1.0f, 0.0f, 0.0f}, { { 1.0f, 0.0f } }, 0x00ffffff },
	{ { 1.0f,  1.0f,  1.0f}, { 1.0f, 0.0f, 0.0f}, { { 0.0f, 0.0f } }, 0x00ffffff },
	{ { 1.0f, -1.0f,  1.0f}, { 1.0f, 0.0f, 0.0f}, { { 0.0f, 1.0f } }, 0x00ffffff },
};

/*****************************************************************************/

UWORD CubeIndices[5 * 6 - 1] =
{
	 0,  1,  2,  3, 0xffff,
	 4,  5,  6,  7, 0xffff,
	 8,  9, 10, 11, 0xffff,
	12, 13, 14, 15, 0xffff,
	16, 17, 18, 19, 0xffff,
	20, 21, 22, 23
};

/*****************************************************************************/

int main(int argc, char *argv[])
{
	float xangle = 0.345f;
	float yangle = 0.123f;
	volatile UWORD *SAGA_ScreenModeRead = (UWORD *)0xdfe1f4;
	volatile ULONG *SAGA_ChunkyDataRead = (ULONG *)0xdfe1ec;
	volatile UWORD *SAGA_ScreenMode = (UWORD *)0xdff1f4;
	volatile ULONG *SAGA_ChunkyData = (ULONG *)0xdff1ec;
	UWORD oldMode;
	ULONG oldScreen;
	UBYTE *screenMem;
	UBYTE *screenPixels[NFRAMES];
	UWORD txtr;
	float targetRatio;
	mat4 worldMatrix, viewMatrix, perspective;
	int currentBufferNumber = 0;
	APTR vblHandle;
	int iBuffer, vBuffer;

	MaggieBase = OpenLibrary((UBYTE *)"maggie.library", 0);

	if(!MaggieBase)
	{
		printf("Can't open maggie.library\n");
		return 0;
	}
	LowLevelBase = OpenLibrary((UBYTE *)"lowlevel.library", 0);
	if(!LowLevelBase)
	{
		CloseLibrary(MaggieBase);
		printf("Can't open lowlevel.library\n");
		return 0;
	}

	txtr = LoadTexture("TestTexture.dds");
	if(txtr == 0xffff)
	{
		printf("no Load %d\n", txtr);
		return 0;
	}

	screenMem = AllocMem(SCREENSIZE * NFRAMES, MEMF_ANY | MEMF_CLEAR);

	screenPixels[0] = screenMem;
	for(int i = 1; i < NFRAMES; ++i)
	{
		screenPixels[i] = screenPixels[i - 1] + SCREENSIZE;
	}

	SystemControl(SCON_TakeOverSys, TRUE, TAG_DONE);

	oldMode = *SAGA_ScreenModeRead;
	oldScreen = *SAGA_ChunkyDataRead;

	*SAGA_ScreenMode = MAGGIE_MODE;

	targetRatio = 9.0f / 16.0f;

	mat4_perspective(&perspective, 60.0f, targetRatio, 0.5f, 100.0f);
	mat4_identity(&worldMatrix);
	mat4_translate(&viewMatrix, 0.0f, 0.0f, 20.0f);

	vblHandle = AddVBlankInt((APTR)VBLInterrupt, NULL);
	vBuffer = magAllocateVertexBuffer(6 * 4);
	magUploadVertexBuffer(vBuffer, CubeVertices, 0, 6 * 4);

	iBuffer = magAllocateIndexBuffer(6 * 5 - 1);
	magUploadIndexBuffer(iBuffer, CubeIndices, 0, 6 * 5 - 1);

	while(!(ReadJoyPort(0) & JPF_BUTTON_RED)) /* While left mouse button not pressed */
	{
		ULONG *pixels;
		mat4 xRot, yRot, rotMatrix;

		*SAGA_ChunkyData = (ULONG)screenPixels[currentBufferNumber];

		currentBufferNumber = (currentBufferNumber + 1) % NFRAMES;
		pixels = (ULONG *)screenPixels[currentBufferNumber];

		WaitVBLPassed();

		mat4_rotateX(&xRot, xangle);
		mat4_rotateY(&yRot, yangle);
		mat4_mul(&rotMatrix, &xRot, &yRot);

		magBeginScene();
#if BPP == 2
		magSetDrawMode(MAG_DRAWMODE_BILINEAR | MAG_DRAWMODE_LIGHTING | MAG_DRAWMODE_DEPTHBUFFER);
#else
		magSetDrawMode(MAG_DRAWMODE_32BIT | MAG_DRAWMODE_BILINEAR | MAG_DRAWMODE_LIGHTING | MAG_DRAWMODE_DEPTHBUFFER);
#endif
		magSetScreenMemory((APTR)pixels, XRES, YRES);

		magClear(MAG_CLEAR_COLOUR | MAG_CLEAR_DEPTH);

		magSetLightPosition(0, 0.0f, 0.0f, -20.0f);
		magSetLightDirection(0, 0.0f, 0.0f, -1.0f);
		magSetLightColour(0, 0x00ffffff);
		magSetLightAttenuation(0, 250.0f);
		magSetLightCone(0, 15.0f * 3.1415927f / 180.0f);
		magSetLightType(0, MAG_LIGHT_SPOT);

		magSetLightColour(1, 0x00ffffff);
		magSetLightType(1, MAG_LIGHT_AMBIENT);

		magSetTexture(0, txtr);

		magSetVertexBuffer(vBuffer);
		magSetIndexBuffer(iBuffer);

		magSetViewMatrix((float *)&viewMatrix);
		magSetPerspectiveMatrix((float *)&perspective);

		for(int i = 0; i < 7; ++i)
		{
			for(int j = 0; j < 7; ++j)
			{
				mat4 transMatrix;

				if((i^j) & 1)
					continue;
				mat4_translate(&transMatrix, (float)(i - 3.0f) * 2.5f, 0.0f, (float)(j - 3.0f) * 2.5f);
				mat4_mul(&worldMatrix, &rotMatrix, &transMatrix);
				magSetWorldMatrix((float *)&worldMatrix);

				magDrawIndexedPolygons(0, 6 * 4, 0, 6 * 5 - 1);
			}
		}
		magEndScene();

		xangle += 0.01f;
		yangle += 0.0123f;
	}
	magFreeTexture(txtr);
	magFreeVertexBuffer(vBuffer);
	magFreeIndexBuffer(iBuffer);

	RemVBlankInt(vblHandle);

	*SAGA_ScreenMode = oldMode;
	*SAGA_ChunkyData = oldScreen;

	SystemControl(SCON_TakeOverSys, FALSE, TAG_DONE); /* Restore system */

	FreeMem(screenMem, SCREENSIZE * NFRAMES);

	CloseLibrary(LowLevelBase);
	CloseLibrary(MaggieBase);

	return 0;
}
