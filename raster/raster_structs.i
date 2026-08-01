	include "exec/types.i"

	STRUCTURE TransVtx,0
		FLOAT TransVtx_PosX
		FLOAT TransVtx_PosY
		FLOAT TransVtx_PosZ
		FLOAT TransVtx_PosW
		FLOAT TransVtx_U
		FLOAT TransVtx_V
		FLOAT TransVtx_W
		FLOAT TransVtx_I
		LONG TransVtx_Size

	STRUCTURE EPos,0
		FLOAT EPos_xPosLeft
		FLOAT EPos_xPosRight
		FLOAT EPos_zLeft
		FLOAT EPos_zRight
		FLOAT EPos_iLeft
		FLOAT EPos_iRight
		FLOAT EPos_oowLeft
		FLOAT EPos_oowRight
		FLOAT EPos_uowLeft
		FLOAT EPos_uowRight
		FLOAT EPos_vowLeft
		FLOAT EPos_vowRight
		LONG EPos_Size

	STRUCTURE EAffPos,0
		FLOAT EAffPos_xPosLeft
		FLOAT EAffPos_xPosRight
		FLOAT EAffPos_zLeft
		FLOAT EAffPos_zRight
		FLOAT EAffPos_iLeft
		FLOAT EAffPos_iRight
		FLOAT EAffPos_uLeft
		FLOAT EAffPos_uRight
		FLOAT EAffPos_vLeft
		FLOAT EAffPos_vRight
		LONG EAffPos_Size

	STRUCTURE Scsr,0
		LONG Scsr_x0
		LONG Scsr_y0
		LONG Scsr_x1
		LONG Scsr_y1
		LONG Scsr_Size

	STRUCTURE Grad,0
		FLOAT Grad_oowDDA
		FLOAT Grad_uowDDA
		FLOAT Grad_vowDDA
		FLOAT Grad_zDDA
		FLOAT Grad_iDDA
		LONG Grad_Size

	STRUCTURE MaggieRegs,0
		APTR	Maggie_texture				;			/*  0 | 32bit texture source */
		APTR	Maggie_pixDest				;			/*  4 | 32bit Destination Screen Addr */
		APTR 	Maggie_depthDest			;			/*  8 | 32bit ZBuffer Addr */
		UWORD	Maggie_unused0				;			/* 12 */
		UWORD	Maggie_startLength			;			/* 14 | 16bit LEN and START */
		UWORD	Maggie_texSize				;			/* 16 | 16bit MIP texture size (10=1024/9=512/8=256/7=128/6=64) */
		UWORD	Maggie_mode				;			/* 18 | 16bit MODE (Bit0=Bilienar) (Bit1=Zbuffer) (Bit2=16bit output) */
		UWORD	Maggie_unused1				;			/* 20 */
		UWORD	Maggie_modulo				;			/* 22 | 16bit Destination Step */
		ULONG	Maggie_uDeltaDelta			;			/* 24 */
		ULONG	Maggie_vDeltaDelta			;			/* 28 */
		ULONG	Maggie_uCoord				;			/* 32 | 32bit U (8:24 normalised) */
		ULONG	Maggie_vCoord				;			/* 36 | 32bit V (8:24 normalised) */
		LONG	Maggie_uDelta				;			/* 40 | 32bit dU (8:24 normalised) */
		LONG	Maggie_vDelta				;			/* 44 | 32bit dV (8:24 normalised) */
		UWORD	Maggie_light				;			/* 48 | 16bit Light Ll (8:8) */
		WORD	Maggie_lightDelta			;			/* 50 | 16bit Light dLl (8:8) */
		ULONG	Maggie_lightRGBA			;			/* 52 | 32bit Light color (ARGB) */
		ULONG	Maggie_depthStart			;			/* 56 | 32bit Z (16:16) */
		LONG	Maggie_depthDelta			;			/* 58 | 32bit Delta (16:16) */
		LONG	Maggie_Size

GetScreenPtr MACRO 
	move.l	104(a6),\1
ENDM
GetDepthPtr MACRO 
	move.l	108(a6),\1
ENDM
GetXRes MACRO
	moveq	#0,\1
	move.w	100(a6),\1
ENDM
GetEdgesPtr MACRO
	lea		472(a6),\1
ENDM
GetScissorPtr MACRO
	lea		456(a6),\1
ENDM
GetGradientsPtr MACRO
	lea		52312(a6),\1
ENDM
