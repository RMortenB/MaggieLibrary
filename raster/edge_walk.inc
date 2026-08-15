; One scanline-edge DDA. Walks an edge down the screen and stores its attributes
; into one column of the edge table, one row per scanline.
;
;	void DrawEdge…(void *edge		a0,
;				const MaggieTransVertex *v0	a1,
;				const MaggieTransVertex *v1	a2,
;				float ooYLen			fp0,
;				float preStep			fp1,
;				int lineLen			d0);
;
; The includer sets which columns to emit:
;
;	.rightSide	0 = left column, 1 = right column
;	.doI		emit intensity (always set for a left column; for a right column only when the source can need iRight)
;	.doZ		emit z	 (MAG_DRAWMODE_DEPTHBUFFER only, left column)
;	.doOow		emit 1/w (perspective only, left column)
;
; A variant emits only what its span renderer reads back; u/v are left-column only. Stores in .loop are in ascending offset order.
;
; Register map, accumulator / per-scanline delta:
;	x	e12 / e0		z	e16 / e4
;	i	e13 / e1		oow	e17 / e5
;	u	e14 / e2		preStep products: e6-e11
;	v	e15 / e3
; e0-e23 are a separate bank from fp0-fp7, so fp0/fp1 survive untouched.

	ifne	.rightSide
.offX	=	EPos_xPosRight
	else
.offX	=	EPos_xPosLeft
	endc

;--- v0: the starting value of each column -------------------------------------
; Intensity is an int in the vertex and a float in the edge table: fmove.l in, fmove.s out.

	fmove.s	TransVtx_PosX(a1),e12
	ifne	.doI
	fmove.l	TransVtx_I(a1),e13
	endc
	ifeq	.rightSide
	fmove.s	TransVtx_U(a1),e14
	fmove.s	TransVtx_V(a1),e15
	endc
	ifne	.doZ
	fmove.s	TransVtx_PosZ(a1),e16
	endc
	ifne	.doOow
	fmove.s	TransVtx_PosW(a1),e17
	endc

;--- v1, then delta = (v1 - v0) * ooYLen ---------------------------------------

	fmove.s	TransVtx_PosX(a2),e0
	ifne	.doI
	fmove.l	TransVtx_I(a2),e1
	endc
	ifeq	.rightSide
	fmove.s	TransVtx_U(a2),e2
	fmove.s	TransVtx_V(a2),e3
	endc
	ifne	.doZ
	fmove.s	TransVtx_PosZ(a2),e4
	endc
	ifne	.doOow
	fmove.s	TransVtx_PosW(a2),e5
	endc

	fsub	e12,e0
	ifne	.doI
	fsub	e13,e1
	endc
	ifeq	.rightSide
	fsub	e14,e2
	fsub	e15,e3
	endc
	ifne	.doZ
	fsub	e16,e4
	endc
	ifne	.doOow
	fsub	e17,e5
	endc

	fmul	fp0,e0
	ifne	.doI
	fmul	fp0,e1
	endc
	ifeq	.rightSide
	fmul	fp0,e2
	fmul	fp0,e3
	endc
	ifne	.doZ
	fmul	fp0,e4
	endc
	ifne	.doOow
	fmul	fp0,e5
	endc

;--- pre-step the accumulators onto the first scanline centre ------------------

	fmul	fp1,e0,e6
	ifne	.doI
	fmul	fp1,e1,e7
	endc
	ifeq	.rightSide
	fmul	fp1,e2,e8
	fmul	fp1,e3,e9
	endc
	ifne	.doZ
	fmul	fp1,e4,e10
	endc
	ifne	.doOow
	fmul	fp1,e5,e11
	endc

	fadd	e6,e12
	ifne	.doI
	fadd	e7,e13
	endc
	ifeq	.rightSide
	fadd	e8,e14
	fadd	e9,e15
	endc
	ifne	.doZ
	fadd	e10,e16
	endc
	ifne	.doOow
	fadd	e11,e17
	endc

;--- one row per scanline, stores in ascending offset order ---------------------

.loop:
	fmove.s	e12,.offX(a0)			; xPosLeft (0) / xPosRight (4)
	fadd	e0,e12
	ifeq	.rightSide
	fmove.s	e13,EPos_iLeft(a0)		; 8
	fadd	e1,e13
	fmove.s	e14,EPos_uLeft(a0)		; 12
	fadd	e2,e14
	fmove.s	e15,EPos_vLeft(a0)		; 16
	fadd	e3,e15
	ifne	.doZ
	fmove.s	e16,EPos_zLeft(a0)		; 20
	fadd	e4,e16
	endc
	ifne	.doOow
	fmove.s	e17,EPos_oowLeft(a0)	; 24
	fadd	e5,e17
	endc
	else
	ifne	.doI
	fmove.s	e13,EPos_iRight(a0)		; 28
	fadd	e1,e13
	endc
	endc

	lea		EPos_Size(a0),a0

	subq.l	#1,d0
	bne.s	.loop

	rts
