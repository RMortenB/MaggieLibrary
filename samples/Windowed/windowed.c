#include <stdio.h>
#include <proto/exec.h>
#include <proto/lowlevel.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>

#include <cybergraphx/cybergraphics.h>

#include <proto/Maggie.h>
#include <maggie_vec.h>
#include <maggie_vertex.h>
#include <maggie_flags.h>

/*****************************************************************************/

#define XRES 640
#define YRES 360
#define BPP 4
#define NFRAMES 3

#define MAGGIE_MODE 0x0b02		/* 640x360x16bpp */
typedef UWORD PixelFormat;

#define SCREENSIZE (XRES * YRES * BPP)

/*****************************************************************************/

struct Library *CyberGfxBase = NULL;
struct Library *MaggieBase = NULL;

/*****************************************************************************/

static volatile int vblPassed;

/*****************************************************************************/

UWORD LoadTexture(const char *filename)
{
	UBYTE *data = NULL;
	FILE *fp = fopen(filename, "rb");
	if(!fp)
		return 0xffff;

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 128, SEEK_SET);

	data = AllocMem(size - 128, MEMF_ANY);
	fread(data, 1, size - 128, fp);
	fclose(fp);

	UWORD txtr = magAllocateTexture(8);
	magUploadTexture(txtr, 8, data, 0);
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
	mat4 worldMatrix, viewMatrix, perspective;
	UWORD txtr;
	int vBuffer;
	int iBuffer;
	float xangle = 0.0f;
	float yangle = 0.0f;
	float distance = 4.0f;

	SetSR(0x800, 0x800);

	MaggieBase = OpenLibrary((UBYTE *)"maggie.library", 0);

	if(!MaggieBase)
	{
		printf("Can't open maggie.library\n");
		return 0;
	}

	CyberGfxBase = OpenLibrary((UBYTE *)"cybergraphics.library", 0);

	if(!CyberGfxBase)
	{
		printf("Can't open cybergraphics.library\n");
		CloseLibrary(MaggieBase);
		CloseLibrary(LowLevelBase);
		return 0;
	}

	mat4_identity(&worldMatrix);
	mat4_translate(&viewMatrix, 0.0f, 0.0f, distance);

	mat4_perspective(&perspective, 60.0f, 288.0f / (float)512, 0.1f, 50.0f);

	txtr = LoadTexture("TestTexture.dds");

	vBuffer = magAllocateVertexBuffer(6 * 4);
	iBuffer = magAllocateIndexBuffer(6 * 5 - 1);
	magUploadVertexBuffer(vBuffer, CubeVertices, 0, 6 * 4);
	magUploadIndexBuffer(iBuffer, CubeIndices, 0, 6 * 5 - 1);

	struct Window *win = OpenWindowTags(NULL,
										WA_InnerWidth, 512,
										WA_InnerHeight, 288,
										WA_MinWidth, 160,
										WA_MinHeight, 90,
										WA_MaxWidth, 1920,
										WA_MaxHeight, 1080,
										WA_Activate, TRUE,
										WA_SimpleRefresh, TRUE,
										WA_ReportMouse, TRUE,
										WA_DragBar, TRUE,
										WA_CloseGadget, TRUE,
										WA_DepthGadget, TRUE,
										WA_SizeGadget, TRUE,
										WA_RMBTrap, TRUE,
										WA_PubScreen, (ULONG)NULL,
										WA_Title, (ULONG)"Maggie library",
										WA_MouseQueue, 1,
										WA_IDCMP,	IDCMP_CLOSEWINDOW |
													IDCMP_MOUSEMOVE | IDCMP_MOUSEBUTTONS | IDCMP_DELTAMOVE |
													IDCMP_REFRESHWINDOW | IDCMP_ACTIVEWINDOW |
													IDCMP_NEWSIZE,
										TAG_DONE);
	int running = 1;

	APTR scrPixels = AllocMem(1920 * 1080 * 4, MEMF_ANY);

	int borderWidth = win->BorderLeft + win->BorderRight;
	int borderHeight = win->BorderTop + win->BorderBottom;
	int width = win->Width - borderWidth;
	int height = win->Height - borderHeight;

	while(running && scrPixels)
	{
		int msgClass = 0;
		Wait(1 << win->UserPort->mp_SigBit);
		struct IntuiMessage *msg;
		while(msg = (struct IntuiMessage *)GetMsg(win->UserPort))
		{
			msgClass = msg->Class;

			if(msgClass & IDCMP_MOUSEMOVE)
			{
				if(msg->Qualifier & IEQUALIFIER_LEFTBUTTON)
				{
					xangle += msg->MouseY * 0.01f;
					yangle -= msg->MouseX * 0.01f;
				}
				if(msg->Qualifier & IEQUALIFIER_RBUTTON)
				{
					distance += msg->MouseY * 0.05f;
				}
			}
			switch(msgClass & (IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE))
			{
				case IDCMP_CLOSEWINDOW :
					running = 0;
					break;
				case IDCMP_NEWSIZE:
					borderWidth = win->BorderLeft + win->BorderRight;
					borderHeight = win->BorderTop + win->BorderBottom;
					width = win->Width - borderWidth;
					height = win->Height - borderHeight;
					mat4_perspective(&perspective, 60.0f, height / (float)width, 0.1f, 50.0f);
					break;
			}
			ReplyMsg((struct Message *)msg);
		}
		WaitTOF();

		mat4 xRot, yRot;
		mat4_rotateX(&xRot, xangle);
		mat4_rotateY(&yRot, yangle);
		mat4_mul(&worldMatrix, &xRot, &yRot);

		mat4_translate(&viewMatrix, 0.0f, 0.0f, distance);

		magBeginScene();

		magSetDrawMode(MAG_DRAWMODE_32BIT | MAG_DRAWMODE_BILINEAR | MAG_DRAWMODE_LIGHTING);

		magSetScreenMemory(scrPixels, width, height);

		magClear(MAG_CLEAR_COLOUR);

		magSetWorldMatrix((float *)&worldMatrix);
		magSetViewMatrix((float *)&viewMatrix);
		magSetPerspectiveMatrix((float *)&perspective);

		magSetLightPosition(0, 0.0f, 0.0f, -10.0f);
		magSetLightDirection(0, 0.0f, 0.0f, 1.0f);
		magSetLightColour(0, 0x00ffffff);
		magSetLightAttenuation(0, 90.0f);
		magSetLightType(0, MAG_LIGHT_POINT);

		magSetLightColour(1, 0x00101010);
		magSetLightType(1, MAG_LIGHT_AMBIENT);

		magSetTexture(0, txtr);
		magSetVertexBuffer(vBuffer);
		magSetIndexBuffer(iBuffer);

		magDrawIndexedPolygons(0, 6 * 4, 0, 6 * 5 - 1);

		magEndScene();
		WritePixelArray(scrPixels, 0, 0, width * 4, win->RPort, win->BorderLeft, win->BorderTop, win->Width - borderWidth, win->Height - borderHeight, RECTFMT_ARGB);
	}
	CloseWindow(win);

	FreeMem(scrPixels, 1920 * 1080 * 4);

	magFreeTexture(txtr);
	magFreeVertexBuffer(vBuffer);
	magFreeIndexBuffer(iBuffer);

	CloseLibrary(CyberGfxBase);
	CloseLibrary(MaggieBase);

	return 0;
}
