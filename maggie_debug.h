#ifndef MAGGIE_DEBUG_H_INCLUDED
#define MAGGIE_DEBUG_H_INCLUDED

struct MaggieBase;

void TextOut(struct MaggieBase *lib, char *fmt, ...);
void DebugReset();

#endif // MAGGIE_DEBUG_H_INCLUDED