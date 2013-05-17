// stdafx.h - a part of the AdVisuo Server Module
// include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define __ADV_DLL

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(1, __FILE__, __LINE__)


#define VER200


#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

// TODO: reference additional headers your program requires here

// obsolete - shouldn't be used any more...
#define IDENTITY


#include <math.h>
#include <time.h>
#include <string>
#include <list>
#include <vector>

#include "log.h"

#define AVULONG ULONG
#define AVLONG LONG
#define AVFLOAT FLOAT
#define AVSTRING LPOLESTR

struct AVVECTOR
{
	AVFLOAT x;
	AVFLOAT y;
	AVFLOAT z;
};
struct AVSIZE
{
	AVSIZE(AVFLOAT x = 0, AVFLOAT y = 0) : x(x), y(y) { }
	AVFLOAT x;
	AVFLOAT y;
};

#define _USE_MATH_DEFINES
#include <math.h>

//#include <stdio.h>
//#include <stdarg.h>
//#include <windows.h>

/////////////////////////////////////////////////////////////
// Debug & Trace

#ifdef _DEBUG
inline void _trace(LPCWSTR fmt, ...)
{
	WCHAR out[1024];
	va_list body;
	va_start(body, fmt);
	vswprintf(out, 1024, fmt, body);
	va_end(body);
	OutputDebugString(out);
}
inline void Debug(LPCTSTR fmt, ...)
{
	WCHAR out[1024];
	va_list body;
	va_start(body, fmt);
	vswprintf(out, 1024, fmt, body);
	va_end(body);
//	OutputDebugString(out);
//	OutputDebugString(L"\n");
//	printf("%ls\n", out);
}

#define ASSERT(x) {if(!(x)) _asm{int 0x03}}
#define VERIFY(x) {if(!(x)) _asm{int 0x03}}
#define TRACE _trace
#else
#define ASSERT(x)
#define VERIFY(x) x
inline void _trace(LPCTSTR fmt, ...) { }
inline void Debug(LPCTSTR fmt, ...) { }
#define TRACE  1 ? (void)0 : _trace
#endif

/////////////////////////////////////////////////////////////
// Max Number of Lift Decks and Doors

#define DECK_NUM	2
#define MAX_DOORS	6

/////////////////////////////////////////////////////////////
// AdVisuo specific includes

#include "../CommonFiles/Vector.h"
#include "../CommonFiles/Box.h"




#include <assert.h>

