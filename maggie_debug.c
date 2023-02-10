#include "maggie_vec.h"
#include "maggie_vertex.h"
#include "maggie_debug.h"
#include "maggie_internal.h"

#include "font8x8.h"
#include <stdarg.h>
#include <float.h>

static int xpos;
static int ypos;

static unsigned int HexToStr(char *dest, unsigned int integer)
{
	if(!integer)
	{
		dest[0] = '0';
		return 1;
	}

    char destBuffer[256];
	char *hexnums = "0123456789abcdef";
	int dPos = 0;

	for(int i = 0; i < 8; ++i)
	{
		int digit = (integer >> ((7 - i) * 4)) & 0xf;
		if(digit || dPos)
        	dest[dPos++] = hexnums[digit];
	}
	dest[dPos] = 0;

    return dPos;
}


static int IntToStr(char *dest, int i)
{
    char destBuffer[256];
    if(!i)
    {
        dest[0] = '0';
        return 1;
    }
    int rPos = 0;
    int dPos = 0;
	if(i < 0)
	{
		i = -i;
		dest[dPos++] = '-';
	}
    while(i)
    {
        destBuffer[rPos++] = i % 10 + '0';
        i /= 10;
    }
    for(rPos--; rPos >= 0; --rPos)
    {
        dest[dPos++] = destBuffer[rPos];
    }
	dest[dPos] = 0;
    return dPos;
}

static int FloatToStr(char *dest, float f)
{
	int strPos = 0;

	if(f < 0)
	{
		f = -f;
		dest[strPos++] = '-';
	}

    int integer = (int)f;

    f = f - integer;

    strPos += IntToStr(&dest[strPos], integer);
    dest[strPos++] = '.';
    for(int i = 0; i < 2; ++i)
    {
        f = f * 10.0f;
        integer = (int)f;
		f = f - integer;
        dest[strPos++] = integer + '0';
    }
    dest[strPos] = 0;
    return strPos;
}

void TextOut16(const char *textBuffer, MaggieBase *lib)
{
	int org_xpos = xpos;
	UWORD *screen = (UWORD *)lib->screen;

	for(int i = 0; textBuffer[i]; ++i)
	{
		switch(textBuffer[i])
		{
			case '\n':
				xpos = org_xpos;
				ypos++;
				break;
			case ' ':
				xpos++;
				break;
			case '\t':
				xpos = (xpos + 8) & ~7;
				break;
			default: 
			{
				int dpos = textBuffer[i] * 64;
				int y = ypos * 8 * lib->xres;

				for(int j = 0; j < 8; ++j)
				{
					for(int k = 0; k < 8; ++k)
					{
						if(font8x8[dpos])
						{
							screen[y + (xpos * 8 + k)] = ~0;
						}
						dpos++;
					}
					y += lib->xres;
				}
				xpos++;
			} break;
		}
		if(xpos >= (lib->xres / 8))
			return;

	}
	xpos = org_xpos;
	ypos++;
}

void TextOut32(const char *textBuffer, MaggieBase *lib)
{
	int org_xpos = xpos;
	ULONG *screen = lib->screen;

	for(int i = 0; textBuffer[i]; ++i)
	{
		switch(textBuffer[i])
		{
			case '\n':
				xpos = org_xpos;
				ypos++;
				break;
			case ' ':
				xpos++;
				break;
			case '\t':
				xpos = (xpos + 8) & ~7;
				break;
			default: 
			{
				int dpos = textBuffer[i] * 64;
				int y = ypos * 8 * lib->xres;

				for(int j = 0; j < 8; ++j)
				{
					for(int k = 0; k < 8; ++k)
					{
						if(font8x8[dpos])
						{
							screen[y + (xpos * 8 + k)] = ~0;
						}
						dpos++;
					}
					y += lib->xres;
				}
				xpos++;
			} break;
		}
		if(xpos >= (lib->xres / 8))
			return;

	}
	xpos = org_xpos;
	ypos++;
}

void TextOut(MaggieBase *lib, char *fmt, ...)
{
	char destBuffer[1024];

    va_list vl;
	va_start(vl, fmt);

    int arg = 0;
    int dstPos = 0;

	if(ypos >= (lib->yres / 8))
		return;

    for(int i = 0; fmt[i]; ++i)
    {
        switch(fmt[i])
        {
            case '%' :
            {
                i++;
                switch(fmt[i])
                {
                    case 'f' :
                    {
						double dd = va_arg(vl, double);
                        dstPos += FloatToStr(&destBuffer[dstPos], (float)dd);
						dd = va_arg(vl, double);
                    } break;
                    case 'd' :
                    {
                        dstPos += IntToStr(&destBuffer[dstPos], va_arg(vl, int));
                    } break;
					case 'x' :
                    {
						dstPos += HexToStr(&destBuffer[dstPos], va_arg(vl, unsigned int));
					} break;
                    default :
                    {
                        destBuffer[dstPos++] = fmt[i];
                    } break;
                }
            } break;
            default :
            {
                destBuffer[dstPos++] = fmt[i];
            } break;
        }
    }
	va_end(vl);
	destBuffer[dstPos++] = 0;
	if(lib->drawMode & MAG_DRAWMODE_32BIT)
	{
		TextOut32(destBuffer, lib);
	}
	else
	{
		TextOut16(destBuffer, lib);
	}
}

void DebugReset()
{
	xpos = 0;
	ypos = 0;
}

