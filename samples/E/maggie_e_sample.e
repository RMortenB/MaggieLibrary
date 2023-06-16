OPT OSVERSION=39
OPT 020
OPT 881
OPT FPEXP

MODULE	'dos/dos','exec','exec/memory',
		'maggie_e','maggie_helper'

/*****************************************************************************/

PROC printMatrix(matrix:PTR TO mat4)
	DEF fstr[20]:STRING
	DEF i, j
	FOR i := 0 TO 3
		FOR j := 0 TO 3
			WriteF('\s ', RealF(fstr, matrix.m[i * 4 + j], 2))
		ENDFOR
		WriteF('\n')
	ENDFOR
ENDPROC

/*****************************************************************************/

OBJECT maggieVertex
	pos: vec3
	normal: vec3
	tex: vec3
	colour:LONG
ENDOBJECT

/*****************************************************************************/

PROC loadTexture()
	DEF texture
	DEF data: PTR TO CHAR
	DEF i
	DEF seed = 63251
	DEF fp

	data := AllocVec(8192, MEMF_ANY)

	FOR i := 0 TO 8192
		data[i] := seed/256
		seed := seed * 17321 + 79187
	ENDFOR

	IF fp := Open('TestTexture.dds', OLDFILE)
		Seek(fp, 128 + 32768, OFFSET_BEGINNING)
		Read(fp, data, 8192)
		Close(fp)
	ELSE
		WriteF('Can\at open file\n')
	ENDIF

	texture := magAllocateTexture(7)
	magUploadTexture(texture, 7, data, 0)
	FreeVec(data)
ENDPROC texture

/*****************************************************************************/

/* This is the list of vertices for the cube. It's in the maggieVertex structure declared above.
   In IEEE 754 : $bf800000 = -1.0, and $3f800000 = 1.0. */

cubeVertices:
	LONG $bf800000, $bf800000, $bf800000, $00000000, $00000000, $bf800000, $3f800000, $3f800000, $3f800000, $00ffffff
	LONG $bf800000, $3f800000, $bf800000, $00000000, $00000000, $bf800000, $3f800000, $00000000, $3f800000, $00ffffff
	LONG $3f800000, $3f800000, $bf800000, $00000000, $00000000, $bf800000, $00000000, $00000000, $3f800000, $00ffffff
	LONG $3f800000, $bf800000, $bf800000, $00000000, $00000000, $bf800000, $00000000, $3f800000, $3f800000, $00ffffff
	LONG $bf800000, $bf800000, $3f800000, $00000000, $00000000, $3f800000, $00000000, $3f800000, $3f800000, $00ffffff
	LONG $3f800000, $bf800000, $3f800000, $00000000, $00000000, $3f800000, $3f800000, $3f800000, $3f800000, $00ffffff
	LONG $3f800000, $3f800000, $3f800000, $00000000, $00000000, $3f800000, $3f800000, $00000000, $3f800000, $00ffffff
	LONG $bf800000, $3f800000, $3f800000, $00000000, $00000000, $3f800000, $00000000, $00000000, $3f800000, $00ffffff
	LONG $bf800000, $bf800000, $bf800000, $00000000, $bf800000, $00000000, $00000000, $3f800000, $3f800000, $00ffffff
	LONG $3f800000, $bf800000, $bf800000, $00000000, $bf800000, $00000000, $3f800000, $3f800000, $3f800000, $00ffffff
	LONG $3f800000, $bf800000, $3f800000, $00000000, $bf800000, $00000000, $3f800000, $00000000, $3f800000, $00ffffff
	LONG $bf800000, $bf800000, $3f800000, $00000000, $bf800000, $00000000, $00000000, $00000000, $3f800000, $00ffffff
	LONG $bf800000, $3f800000, $bf800000, $00000000, $3f800000, $00000000, $3f800000, $3f800000, $3f800000, $00ffffff
	LONG $bf800000, $3f800000, $3f800000, $00000000, $3f800000, $00000000, $3f800000, $00000000, $3f800000, $00ffffff
	LONG $3f800000, $3f800000, $3f800000, $00000000, $3f800000, $00000000, $00000000, $00000000, $3f800000, $00ffffff
	LONG $3f800000, $3f800000, $bf800000, $00000000, $3f800000, $00000000, $00000000, $3f800000, $3f800000, $00ffffff
	LONG $bf800000, $bf800000, $bf800000, $bf800000, $00000000, $00000000, $3f800000, $00000000, $3f800000, $00ffffff
	LONG $bf800000, $bf800000, $3f800000, $bf800000, $00000000, $00000000, $00000000, $00000000, $3f800000, $00ffffff
	LONG $bf800000, $3f800000, $3f800000, $bf800000, $00000000, $00000000, $00000000, $3f800000, $3f800000, $00ffffff
	LONG $bf800000, $3f800000, $bf800000, $bf800000, $00000000, $00000000, $3f800000, $3f800000, $3f800000, $00ffffff
	LONG $3f800000, $bf800000, $bf800000, $3f800000, $00000000, $00000000, $3f800000, $3f800000, $3f800000, $00ffffff
	LONG $3f800000, $3f800000, $bf800000, $3f800000, $00000000, $00000000, $3f800000, $00000000, $3f800000, $00ffffff
	LONG $3f800000, $3f800000, $3f800000, $3f800000, $00000000, $00000000, $00000000, $00000000, $3f800000, $00ffffff
	LONG $3f800000, $bf800000, $3f800000, $3f800000, $00000000, $00000000, $00000000, $3f800000, $3f800000, $00ffffff

cubeIndices:
	INT  0,  1,  2,  3, $ffff
	INT  4,  5,  6,  7, $ffff
	INT  8,  9, 10, 11, $ffff
	INT 12, 13, 14, 15, $ffff
	INT 16, 17, 18, 19, $ffff
	INT 20, 21, 22, 23


PROC createCubeVertexBuffer()
	DEF vBuffer
	vBuffer := magAllocateVertexBuffer(24)
	magUploadVertexBuffer(vBuffer, {cubeVertices}, 0, 24)
ENDPROC vBuffer

PROC createCubeIndexBuffer()
	DEF iBuffer

	iBuffer := magAllocateIndexBuffer(29)
	magUploadIndexBuffer(iBuffer, {cubeIndices}, 0, 29)
ENDPROC iBuffer

/*****************************************************************************/

PROC main()
	DEF modePtr: PTR TO INT
	DEF modeRdPtr: PTR TO INT
	DEF screenPtr: PTR TO LONG
	DEF screenRdPtr: PTR TO LONG
	DEF oldScreen
	DEF oldMode
	DEF worldMatrix: mat4
	DEF viewMatrix: mat4
	DEF perspectiveMatrix: mat4
	DEF xRot: mat4
	DEF yRot: mat4
	DEF xangle
	DEF yangle
	DEF vBuffer
	DEF iBuffer
	DEF texture
	DEF frames[3]: ARRAY OF LONG
	DEF frameNum
	DEF frameBufferMemPtr
	DEF screenSize

	screenSize := 460800

	frameBufferMemPtr := AllocMem(Mul(3, screenSize), MEMF_ANY)
	frames[0] := frameBufferMemPtr
	frames[1] := frames[0] + screenSize
	frames[2] := frames[1] + screenSize
	frameNum := 0;

	modeRdPtr := $dfe1f4
	screenRdPtr := $dfe1ec
	modePtr := $dff1f4
	screenPtr := $dff1ec

	mat4_translate(viewMatrix, 0.0, 0.0, 3.5);
	mat4_perspective(perspectiveMatrix, 60.0, 0.5625, 0.5, 100.0)

	magInit()

	vBuffer := createCubeVertexBuffer()
	iBuffer := createCubeIndexBuffer()
	texture := loadTexture()

	Forbid()

	oldMode := modeRdPtr[]
	oldScreen := screenRdPtr[]

	modePtr[] := $0b02
	screenPtr[] := frames[0]

	WHILE((Mouse() AND 1) = 0)
		screenPtr[] := frames[frameNum]

		frameNum := frameNum + 1

		IF frameNum = 3
			frameNum := 0
		ENDIF

		magBeginScene()

		magSetScreenMemory(frames[frameNum], 640, 360)

		magSetDrawMode(MAG_DRAWMODE_BILINEAR OR MAG_DRAWMODE_LIGHTING)
		magClear(MAG_CLEAR_COLOUR)

		mat4_rotateX(xRot, xangle);
		mat4_rotateY(yRot, yangle);
		mat4_mul(worldMatrix, xRot, yRot);

		magSetWorldMatrix(worldMatrix)
		magSetViewMatrix(viewMatrix)
		magSetPerspectiveMatrix(perspectiveMatrix)

		magSetLightPosition(0, 0.0, 0.0, -20.0)
		magSetLightDirection(0, 0.0, 0.0, -1.0)
		magSetLightColour(0, $ffffff)
		magSetLightAttenuation(0, 250.0)
		magSetLightType(0, MAG_LIGHT_POINT)

		magSetLightColour(1, $0f0f0f)
		magSetLightType(1, MAG_LIGHT_AMBIENT)

		magSetVertexBuffer(vBuffer)
		magSetIndexBuffer(iBuffer)
		magSetTexture(0, texture)

		magDrawIndexedPolygons(0, 24, 0, 29);

		magEndScene()

		xangle := !xangle + 0.01
		yangle := !yangle + 0.0123
	ENDWHILE

	modePtr[] := oldMode AND $ffff
	screenPtr[] := oldScreen

	Permit()

	magFreeVertexBuffer(vBuffer)
	magFreeIndexBuffer(iBuffer)
	magFreeTexture(texture)

	magClose()

	FreeMem(frameBufferMemPtr, Mul(3, screenSize));
ENDPROC
