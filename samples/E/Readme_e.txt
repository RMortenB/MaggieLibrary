
 The "maggie_e.m" file is generated from the "maggie_e_asm.s" file with vasm, and o2m:

vasmm68k_mot maggie_e_asm.s -m68000 -m68881 -nosym -I../include -Fhunk -pic -o maggie_e.o
o2m maggie_e

Assuming all things went well, this should produce the "maggie_e.m" file.

The maggie_helper.e has some vector helper functions.
"maggie_helper.m" file is produced with:

evo maggie_helper.e

The modules needs to be installed in the usual place, reachable by the "emodules:" assign.

The maggie.library API is the same as for the C version, so those docs still apply.

-Morten-

