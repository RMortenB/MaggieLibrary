	sub.l d0,d1
	ble.w .noVlinesToDo

	movem.l    d2-d5/a2-a4,-(a7)

	fmove.w	#1,e4
	lea		$dff250,a0

	move.l  d1,d4

	GetEdgesPtr a2

	; Per-polygon fan-sum gradients (both paths): u/v hoisted once.
	GetGradientsPtr a1
	fmove.s	Grad_uowDDA(a1),e18		; uDDA
	fmove.s	Grad_vowDDA(a1),e21		; vDDA
	fmove.l	e18,Maggie_uDelta(a0)
	fmove.l	e21,Maggie_vDelta(a0)
	ifne	.polyIntensity
	; polygon path: intensity is non-planar - approximate with one DDA taken from the middle scanline, still hoisted.
	move.l	d4,d2
	lsr.l	#1,d2
	mulu.w	#EPos_Size,d2
	fmove.s	EPos_xPosRight(a2,d2),e0
	fsub.s	EPos_xPosLeft(a2,d2),e0	; xDiff at middle scanline
	fdiv	e0,e4,e0					; ooXLength = 1 / xDiff
	fmove.s	EPos_iRight(a2,d2),e13
	fsub.s	EPos_iLeft(a2,d2),e13	; iDiff at middle scanline
	fmul	e0,e13						; iDDA = iDiff * ooXLength
	else
	; triangle path: intensity is planar, take it from the fan-sum gradient.
	fmove.s	Grad_iDDA(a1),e13			; iDDA
	endc
	fmoverz.l	e13,d3
	move.w	d3,Maggie_lightDelta(a0)	; hoisted (both paths)

	GetXRes d2
	mulu.w	d2,d0
	GetScreenPtr a3
	GetScissorPtr a4

	lea		(a3,d0.l*.bpp),a3		; pixels
.vLoop:
	fmove.s	(a2)+,e0				; xLeft  (0)
	fmove.s	(a2)+,e1				; xRight (4)

	fmove.l	e0,d0					; xStart = (int)xLeft;
	fmove.l	e1,d1					; xEnd = (int)xRight;

	cmp.l	(a4),d0					;if(scissorLeft > x0)
	bge		.noClipX0

	move.l	(a4),d0					;xStart = scissorLeft
.noClipX0:
	cmp.l	Scsr_x1(a4),d1 			;if(scissorRight < x1)
	ble		.noClipX1

	move.l	Scsr_x1(a4),d1			;xEnd = scissorRight
.noClipX1:

	sub.l	d0,d1					; len = x1 - x0
	ble		.noPixels

	fadd.l	d0,e4,e2

	lea		(a3,d0.l*.bpp),a1		; dstColPtr = pixels + xStart
	move.l	a1,Maggie_pixDest(a0)

	fsub	e0,e2,e2				; preStep = 1 + xStart - edges[i].xPosLeft
	; The row, walked forward with post-increment: this variant stops at vLeft (16).
	fmove.s	(a2)+,e12				; iLeft  (8)
	fmove.s	(a2)+,e17				; uLeft  (12)
	fmove.s	(a2)+,e20				; vLeft  (16)
	fmul	e2,e13,e8				; preStepI = preStep * iDDA
	fmul	e2,e18,e19				; preStepU = preStep * uDDA
	fmul	e2,e21,e22				; preStepV = preStep * vDDA
	fadd	e8,e12					; light = iLeft + preStepI
	fadd	e19,e17					; uPos = uLeft + preStepU
	fadd	e22,e20					; vPos = vLeft + preStepV

	fmove.l	e12,d0

	fmove.l	e17,Maggie_uCoord(a0)
	fmove.l	e20,Maggie_vCoord(a0)
	move.w	d0,Maggie_light(a0)
	move.w	d1,Maggie_startLength(a0)

	; Drawn-row tail: 12 bytes remain. Duplicated rather than shared with .noPixels, which owes a2 a different amount.
	lea		12(a2),a2				; zLeft (20) -> next row
	lea		(a3,d2.l*.bpp),a3		; pixels += modulo

	subq.l	#1,d4 ; ylen
	bne		.vLoop

	movem.l	(a7)+,d2-d5/a2-a4
	rts

	; Empty-span tail: only xPosLeft/xPosRight were consumed.
.noPixels:
	lea		24(a2),a2
	lea		(a3,d2.l*.bpp),a3		; pixels += modulo

	subq.l	#1,d4 ; ylen
	bne		.vLoop

	movem.l	(a7)+,d2-d5/a2-a4

.noVlinesToDo:
	rts
