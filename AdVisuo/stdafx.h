// stdafx.h - a part of the AdVisuo Client Software

// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#define __ADVISUO_EXE

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars


#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <afxdhtml.h>

///////////////////////////////////////////////////////////////
// AdVisuo Specials

// Debug Output Function
void Debug(LPCTSTR fmt, ...);

#define AVULONG ULONG
#define AVLONG LONG
#define AVFLOAT FLOAT
#define AVSTRING LPOLESTR
//#define AVCOLOR FWCOLOR
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
struct AVCOLOR
{
	AVFLOAT r;
	AVFLOAT g;
	AVFLOAT b;
	AVFLOAT a;
};
#define _FWCOLOR(c) (*(FWCOLOR*)&(c))

// Colours
#define AVCOLOR2COLORREF(c)	RGB((unsigned)((c).r*255.0f), (unsigned)((c).g*255.0f), (unsigned)((c).b*255.0f))

// Standard Models Path
extern std::wstring _stdPathModels;
#define STD_PATH(str)	((AVSTRING)(_stdPathModels + (str)).c_str())

// Max Number of Lifts
#define LIFT_MAXNUM	100

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

/////////////////////////////////////////////////////////////
// Max Number of Lift Decks and Doors

#define DECK_NUM	2
#define MAX_DOORS	6

/////////////////////////////////////////////////////////////
// AdVisuo specific includes

#include "../CommonFiles/Vector.h"
#include "../CommonFiles/Box.h"


#pragma warning (disable:4800)
