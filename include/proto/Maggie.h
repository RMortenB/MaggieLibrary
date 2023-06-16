#ifndef _PROTO_MAGGIE_H
#define _PROTO_MAGGIE_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#if !defined(CLIB_MAGGIE_PROTOS_H) && !defined(__GNUC__)
#include <clib/Maggie_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct Library *MaggieBase;
#endif

#ifdef __GNUC__
#ifdef __AROS__
#include <defines/Maggie.h>
#else
#include <inline/Maggie.h>
#endif
#elif defined(__VBCC__)
#include <inline/Maggie_protos.h>
#else
#include <pragma/Maggie_lib.h>
#endif

#endif	/*  _PROTO_MAGGIE_H  */
