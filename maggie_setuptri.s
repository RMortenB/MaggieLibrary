	section	.text

	include	<exec/types.i>
	include	"raster/raster_structs.i"

;------------------------------------------------------------------------------
; ULONG MaggieSetupTri(struct MaggieTransVertex *v0 __asm("a0"),
;                      struct MaggieTransVertex *v1 __asm("a1"),
;                      struct MaggieTransVertex *v2 __asm("a2"),
;                      MaggieBase *lib             __asm("a6"));
;
; Per-triangle setup: backface cull, screen-y span, and the five per-polygon
; attribute gradients.
;
;   returns 0  -> skip this triangle (backfacing, or zero scanlines tall)
;   returns 1  -> draw it; lib->gradients, lib->primMinY and lib->primMaxY
;                 have been written
;
; The caller still emits the three edges and dispatches the span renderer.
;
; Why this is asm: the gradient block wants ~15 live float values at once. GCC
; only knows fp0-fp7, so it shuffles a third of them through the integer
; registers and saves d2-d7/a2-a6 + fp2-fp7 on entry - 116 bytes of stack
; traffic each way, on every triangle including the ~half that get culled.
; e0-e23 are invisible to the C ABI: using them costs no prologue, nothing ends
; up homeless, and there are enough registers to schedule around the latencies.
;
;------------------------------------------------------------------------------
; SCHEDULING
;
; FP instructions are not fused: one issues per cycle, results land ~6 cycles
; later (~10 for fdiv), issue is in-order, and the CPU stalls only when an
; operand is not ready. What matters is therefore how many independent
; instructions sit between each producer and its consumer, and the order below
; is chosen for that rather than for readability.
;
; Four things drive the layout:
;
;  1. The cull is one long chain - delta, product, difference, sign - about 5
;     deep, so ~30 cycles in which only ~13 instructions have anything to do.
;     Anything independent parked in those slots is free, INCLUDING on the
;     culled path, because those cycles were stalls either way. The y
;     truncations, the cullSign load, v0's five attributes and all ten attribute
;     deltas go there.
;
;  2. The five attribute gradients are five independent chains. Each gets its
;     own pair of temps and they are issued stage by stage - all five deltas,
;     then all five *dy2, and so on - which leaves four independent instructions
;     between every producer and consumer. Sharing one pair of temps across the
;     five chains, the obvious way to write it, serializes them into five
;     ~26-cycle chains and costs ~90 cycles more.
;
;  3. The screen-y span uses integer compares rather than fcmp/fbcc. Four FPU
;     compare-and-branch pairs cost ~30 cycles of pure stall, and the truncated
;     integers are wanted anyway. Integer compares also work on CCR, not FPCC,
;     so they slot into the FP stalls without disturbing the cull's pending
;     ftst - that is what lets the whole span block sit between ftst and fbgt.
;
;  4. Both long-latency results are started as early as their inputs allow and
;     consumed as late as possible: the reciprocal's -1.0 numerator is loaded up
;     front so the fdiv never waits on it, the fdiv is issued before the three
;     multiply stages that do not depend on it, and each compare is separated
;     from its branch by independent work.
;------------------------------------------------------------------------------
; REGISTERS
;   e0,e1,e2   y0,y1,y2, truncated in place
;   e3         x0, dead once both x deltas are formed
;   e4,e5      dx1,dx2, then recycled as the intensity chain's temps (operands
;              are read at issue and issue is in-order, so this is safe)
;   e6,e7      dy1,dy2, live until the last numerator
;   e8         p1 -> denom, live until the reciprocal
;   e9         p2
;   e10..e14   v0's w,u,v,z,i
;   e15        -1.0 -> -1/denom (see the sign note at the deltas)
;   e16..e23   the other four chains' temps
;   fp0        cullSign, then area
;   d0,d1,d2   iy0,iy1,iy2 -> min in d0, max in d1
;
; Clobbers d0/d1/d2 (d2 saved) and e0-e23. fp0 is used for the cull compare
; (caller-saved). fp1-fp7 and every callee-saved integer register other than the
; saved d2/a2 are left untouched.
;------------------------------------------------------------------------------

_MaggieSetupTri:
	movem.l	d2/a2,-(sp)

;--- positions -----------------------------------------------------------------

	fmove.s	TransVtx_PosY(a0),e0
	fmove.s	TransVtx_PosY(a1),e1
	fmove.s	TransVtx_PosY(a2),e2
	fmove.s	TransVtx_PosX(a0),e3
	fmove.s	TransVtx_PosX(a1),e4
	fmove.s	TransVtx_PosX(a2),e5

;--- the cull chain, with its stalls filled ------------------------------------
; One cross product serves as both the cull area and the gradient denominator -
; they are the same quantity.

	fsub	e0,e1,e6		; dy1 = y1 - y0
	fsub	e0,e2,e7		; dy2 = y2 - y0
	fintrz	e0,e0			; independent: fills the cull's stalls
	fsub	e3,e4			; dx1 = x1 - x0
	fsub	e3,e5			; dx2 = x2 - x0
	fintrz	e1,e1
	fintrz	e2,e2
	fmove.s	MB_cullSign(a6),fp0
	fmove.s	TransVtx_PosW(a0),e10
	fmove.s	TransVtx_U(a0),e11
	fmove.s	TransVtx_V(a0),e12
	fmove.s	TransVtx_PosZ(a0),e13
	fmove.l	TransVtx_I(a0),e14
	fmove.w	#-1,e15			; hoisted so the fdiv never waits on its numerator

	fmul	e7,e4,e8		; p1 = dx1 * dy2
	fmul	e6,e5,e9		; p2 = dx2 * dy1

;--- attribute deltas ----------------------------------------------------------
; Both deltas per attribute are formed NEGATED, as (f0 - f1) and (f0 - f2), so
; each fuses its load and subtract into one fsub.s <ea>,eSrc,eDst - two fewer
; instructions per attribute, and no register is needed for f1/f2 at all. The
; doubly-negated result is corrected by the negated reciprocal. Every step is an
; exact IEEE negation, so the stored gradient matches the straightforward form
; bit-for-bit (checked over 2M random and adversarial inputs) with one
; exception: when the gradient is exactly zero the sign of that zero can differ,
; which the DDA cannot observe - adding -0.0 and +0.0 to a finite value gives
; the same result.
;
; These sit here, ahead of the cull's ftst, purely as filler for the area
; multiply's latency. e4/e5 are recycled here from dx1/dx2, whose only readers
; were the two products issued above.

	fsub.s	TransVtx_PosW(a1),e10,e16
	fsub.s	TransVtx_PosW(a2),e10,e17
	fsub.s	TransVtx_U(a1),e11,e18
	fsub.s	TransVtx_U(a2),e11,e19
	fsub.s	TransVtx_V(a1),e12,e20

	fsub	e9,e8			; denom = p1 - p2, its inputs now well clear

	fsub.s	TransVtx_V(a2),e12,e21
	fsub.s	TransVtx_PosZ(a1),e13,e22
	fsub.s	TransVtx_PosZ(a2),e13,e23
	fsub.l	TransVtx_I(a1),e14,e4
	fsub.l	TransVtx_I(a2),e14,e5
	fmove.l	e0,d0			; iy0   three independent extractions, so none of
	fmove.l	e1,d1			; iy1   them waits on the one before
	fmove.l	e2,d2			; iy2

	fmul	e8,fp0			; area = denom * cullSign

;--- screen-y span, in integer -------------------------------------------------
; Truncation is monotonic, so min(trunc(y)) == trunc(min(y)) and this matches
; the C version exactly. Sitting between ftst and its fbgt is deliberate: these
; are CCR compares and cannot disturb the pending FPCC, so they cover the ftst
; latency for free.

	cmp.l	d1,d0
	ble.s	.sorted
	exg	d0,d1			; d0 = min(iy0,iy1), d1 = max(iy0,iy1)
.sorted:
	cmp.l	d2,d0
	ble.s	.gotMin
	move.l	d2,d0
.gotMin:
	lea	MB_gradients(a6),a0	; v0 is fully loaded, so a0 is free

	ftst	fp0

	cmp.l	d2,d1
	bge.s	.gotMax
	move.l	d2,d1
.gotMax:
	cmp.l	d1,d0
	beq.w	.skip			; zero scanlines tall

	move.l	d0,MB_primMinY(a6)
	move.l	d1,MB_primMaxY(a6)

	fbgt	.skip			; backfacing

;--- the reciprocal, and the stages that do not need it ------------------------
; area is the cull-signed fan area, so it is <= 0 here and the old two-sided
; epsilon test collapses to one compare. A zero scale zeroes every gradient,
; which is what the C stored explicitly for a degenerate triangle. The fdiv
; cannot be hoisted above this test: denom is exactly 0 for a fully degenerate
; triangle, which survives the cull, and dividing by it would trap on any host
; that has FPU exceptions enabled.
;
; Scale by the reciprocal only AFTER the subtraction. Folding it into dy1/dy2
; up front would save five multiplies but costs up to 0.4% relative error on
; sliver triangles, where the subtraction cancels heavily.

; The * dy2 stage MUST stay ahead of the compare. FP arithmetic sets the FPCC
; from its result, so scheduling these between the fcmp and the fbgt - which is
; what they used to do, to cover the compare's latency - made the branch test the
; sign of (i0-i1)*dy2 instead of the area. Ordinary triangles then took
; .degenerate, e15 became 0, all five gradients became 0, and the span renderer
; produced uDelta = vDelta = 0: one texel smeared across the span.
;
; Nothing may be placed between the fcmp and the fbgt unless it leaves the FPCC
; alone (integer instructions are fine - that is why the cull's ftst/fbgt pair
; above still works with the y-span block inside it).
	fmul	e7,e16			; * dy2
	fmul	e7,e18
	fmul	e7,e20
	fmul	e7,e22
	fmul	e7,e4

; The epsilon MUST be written as a hex bit pattern. vasm mis-assembles decimal
; float immediates without a diagnostic - "#-1e-6" emits -0.0, which would make
; this "fp0 > 0", always false after the cull, silently removing the guard and
; letting the fdiv divide by a zero denom.
	fcmp.s	#$B58637BD,fp0		; -1e-6
	fbgt	.degenerate
	fdiv	e8,e15			; -1.0 / denom
	bra.s	.scaled
.degenerate:
	fmove.w	#0,e15
.scaled:

;--- the remaining stages ------------------------------------------------------
; Intensity used fsub.l above - a signed long operand, matching the (float)(int)
; cast in C. It is clamped to 0..0xffff upstream, so the sign bit is never set.

	fmul	e6,e17			; * dy1
	fmul	e6,e19
	fmul	e6,e21
	fmul	e6,e23
	fmul	e6,e5

	fsub	e17,e16
	fsub	e19,e18
	fsub	e21,e20
	fsub	e23,e22
	fsub	e5,e4

	fmul	e15,e16			; * -1/denom
	fmul	e15,e18
	fmul	e15,e20
	fmul	e15,e22
	fmul	e15,e4

	fmove.s	e16,Grad_oowDDA(a0)
	fmove.s	e18,Grad_uowDDA(a0)
	fmove.s	e20,Grad_vowDDA(a0)
	fmove.s	e22,Grad_zDDA(a0)
	fmove.s	e4,Grad_iDDA(a0)

	moveq	#1,d0
	movem.l	(sp)+,d2/a2
	rts

.skip:
	moveq	#0,d0
	movem.l	(sp)+,d2/a2
	rts

	public	_MaggieSetupTri
