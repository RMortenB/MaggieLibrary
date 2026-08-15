	sub.l	d0,d1; yLen
	ble.w	.allDone

	movem.l	d2-d6/a2-a6,-(a7)

	move.l	d1,d5

	GetEdgesPtr a2
	GetXRes d2
	mulu.w	d2,d0

	GetScreenPtr a3
	GetDepthPtr a4
	GetScissorPtr a5

	lea		(a3,d0.l*.bpp),a3			;pixels
	lea		(a4,d0.l*2),a4			;zbuffer

	fmove.w	#1,e4
	lea		$dff250,a0
.vLoop:
	fmove.s	(a2)+,e0				;xLeft  (0)
	fmove.s	(a2)+,e1				;xRight (4)

	fmove.l	e0,d3					;xStart = (int)xLeft;
	fmove.l	e1,d4					;xEnd = (int)xRight;

	; TODO : Take care of the scissor rect

;x0 - scissorLeft
  	cmp.l	(a5),d3					;if(scissorLeft > x0)
  	bge		.noClipX0

  	move.l	(a5),d3					;xStart = scissorLeft
.noClipX0:
	cmp.l	Scsr_x1(a5),d4 					;if(scissorRight < x1)
	ble		.noClipX1

	move.l	Scsr_x1(a5),d4					;xEnd = scissorRight
.noClipX1:
	sub.l	d3,d4					;len = x1 - x0
	ble		.noPixels

	fadd.l	d3,e4,e2				; xStart + 1

	lea		(a3,d3.l*.bpp),a1			;dstColPtr = pixels + xStart
	move.l	a1,Maggie_pixDest(a0)
	lea		(a4,d3.l*2),a1			;dstDepthPtr = zbuffer + xStart
	move.l	a1,Maggie_depthDest(a0)

	; Per-polygon fan-sum gradients for oow/u/v/z (both paths).
	GetGradientsPtr a1
	fmove.s	Grad_zDDA(a1),e11		; zDDA
	fmove.s	Grad_oowDDA(a1),e15		; wDDA
	fmove.s	Grad_uowDDA(a1),e18		; uDDA
	fmove.s	Grad_vowDDA(a1),e21		; vDDA

	; The row, walked forward with post-increment: this variant reads 0..24 with no gaps, so the load order is locked to the layout.
	fmove.s	(a2)+,e12				; iLeft   (8)
	fmove.s	(a2)+,e17				; uowLeft (12)
	fmove.s	(a2)+,e20				; vowLeft (16)
	fmove.s	(a2)+,e10				; zLeft   (20)
	fmove.s	(a2)+,e14				; oowLeft (24)

	fsub	e0,e2,e2				; preStep = 1 + xStart - xPosLeft

	ifne	.polyIntensity
	; polygon path: intensity is non-planar across >3 verts - interpolate edge to edge per scanline. The iRight load also closes the row.
	fsub	e0,e1,e3				; xDiff = xRight - xLeft
	fdiv	e3,e4,e3				; ooXLength = 1.0f / xDiff
	fmove.s	(a2)+,e13				; iowRight (28)
	fsub	e12,e13					; iDiff = iowRight - iowLeft
	fmul	e3,e13					; iDDA = iDiff * ooXLength
	else
	; triangle path: intensity is planar, take the fan-sum gradient. iRight goes unread, so close the row by hand.
	fmove.s	Grad_iDDA(a1),e13		; iDDA
	addq.l	#4,a2					; skip iRight (28)
	endc

	fmul	e2,e11,e7				; preStepZ = preStep * zDDA
	fmul	e2,e13,e8				; preStepI = preStep * iDDA
	fmul	e2,e15,e16				; preStepW = preStep * wDDA
	fmul	e2,e18,e19				; preStepU = preStep * uDDA
	fmul	e2,e21,e22				; preStepV = preStep * vDDA
	fadd	e7,e10					; depthStart = zLeft + preStepZ
	fadd	e8,e12					; light = iLeft + preStepI
	fadd	e16,e14					; wPos = oowLeft + preStepW
	fadd	e19,e17					; uPos = uLeft + preStepU
	fadd	e22,e20					; vPos = vLeft + preStepV

	fdiv	e14,e17,e5
	fdiv	e14,e20,e6

	moveq	#16,d6

	fsub.s	#$4f000000,e10

	fmoverz.l	e12,d1
	fmoverz.l	e13,d3

	fmove.l e10,d0
	add.l	#-2147483648,d0

	move.w	d1,Maggie_light(a0)
	move.w	d3,Maggie_lightDelta(a0)

	move.l	d0,Maggie_depthStart(a0)
	fmove.l	e11,Maggie_depthDelta(a0)

	fmove.l	e5,Maggie_uCoord(a0)
	fmove.l	e6,Maggie_vCoord(a0)

	cmp.l	#16,d4
	blt.s	.singlePixels

	fmul.l	d6,e15,e7 ; wDDAFullRun
	fmul.l	d6,e18,e8 ; uDDAFullRun
	fmul.l	d6,e21,e9 ; vDDAFullRun
.loopSpan16:
	fadd	e7,e14
	fadd	e8,e17
	fadd	e9,e20

	fdiv	e14,e17,e10
	fdiv	e14,e20,e11

	sub.l	#16,d4

	fsub	e5,e10,e12
	fsub	e6,e11,e13

	fmul.s	#$3d800000,e12	; 1 / 16
	fmul.s	#$3d800000,e13	; 1 / 16

	fmove	e10,e5
	fmove	e11,e6

	fmove.l	e12,Maggie_uDelta(a0)
	fmove.l	e13,Maggie_vDelta(a0)
	move.w	d6,Maggie_startLength(a0)

	or.l	#$8000,d6

	cmp.l	#16,d4
	bge.s		.loopSpan16
.singlePixels:
	tst.l	d4
	beq.s	.rowDone

	fdiv.l	d4,e4,e7

	fmul.l	d4,e15
	fmul.l	d4,e18
	fmul.l	d4,e21

	fadd	e15,e14
	fadd	e18,e17
	fadd	e21,e20

	fdiv	e14,e17,e10
	fdiv	e14,e20,e11

	fsub	e5,e10
	fsub	e6,e11

	fmul	e7,e10
	fmul	e7,e11

	and.l	#$8000,d6
	or.l	d6,d4

	fmove.l	e10,Maggie_uDelta(a0)
	fmove.l	e11,Maggie_vDelta(a0)
	move.w	d4,Maggie_startLength(a0)

	; Drawn-row tail: the walk consumed all 32 bytes, so a2 already sits on the next row. Duplicated rather than shared with .noPixels, which owes a2 24 bytes.
.rowDone:
	lea		(a3,d2.l*.bpp),a3			;pixels += modulo
	lea		(a4,d2.l*2),a4			;pixels += modulo

	subq.l	#1,d5 ; ylen
	bne.w	.vLoop

	movem.l	(a7)+,d2-d6/a2-a6
	rts

	; Empty-span tail: only xPosLeft/xPosRight were consumed, so 24 bytes remain.
.noPixels:
	lea		24(a2),a2
	lea		(a3,d2.l*.bpp),a3			;pixels += modulo
	lea		(a4,d2.l*2),a4			;pixels += modulo

	subq.l	#1,d5 ; ylen
	bne.w	.vLoop

	movem.l	(a7)+,d2-d6/a2-a6
.allDone:
	rts
