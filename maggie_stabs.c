#include <stabs.h>

#if __STDC_VERSION__ >= 201112L
 __asm("_maggieRegs=0xdff250;.globl _maggieRegs");  // double underscore for c11 asm
#else
 ABSDEF(maggieRegs, 0xdff250);
#endif