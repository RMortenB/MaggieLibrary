	section .text

;	void magFastClear(void *buffer __asm("a0"), ULONG nBytes__asm("d0"), ULONG data __asm("d1"));

_magFastClear:
	vperm	#$76547654,d1,d1,e0
.loop:
	store	e0,(a0)+
	subq.l	#8,d0
	bne.s	.loop
	rts

	public _magFastClear
