mkdir clib
mkdir defines
mkdir inline
mkdir lvo
mkdir pragma
mkdir pragmas
mkdir proto

rm clib/*
rm defines/*
rm inline/*
rm lvo/*
rm pragma/*
rm proto/*
rm pragmas/*

cp ../maggie.h clib/Maggie_protos.h

# asm
fd2pragma -s 20 -i maggie.fd -c clib/Maggie_protos.h -t lvo

# SAS-C
fd2pragma -s 6 -i maggie.fd -c clib/Maggie_protos.h -t pragma
fd2pragma -s 35 -i maggie.fd -c clib/Maggie_protos.h -t proto

# Storm V4
fd2pragma -s 43 -i maggie.fd -c clib/Maggie_protos.h -t inline
fd2pragma -s 39 -i maggie.fd -c clib/Maggie_protos.h -t proto

# GCC
fd2pragma -s 40 -i maggie.fd -c clib/Maggie_protos.h -t inline
fd2pragma -s 35 -i maggie.fd -c clib/Maggie_protos.h -t proto
cp inline/Maggie.h defines/Maggie.h

# Static-link glue (base hidden, direct calls into libmaggie.a) - derived from
# the GCC inline header above so it stays in ABI lockstep with maggie.fd. Uses
# the single canonical generator in ../include.
awk -f ../include/genstatic.awk inline/Maggie.h > proto/Maggie_static.h

# VBCC
fd2pragma -s 70 -i maggie.fd -c clib/Maggie_protos.h -t inline
fd2pragma -s 38 -i maggie.fd -c clib/Maggie_protos.h -t proto

# All
fd2pragma -s 80 -i maggie.fd -c clib/Maggie_protos.h -t pragmas

