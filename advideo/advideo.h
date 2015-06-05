// advideo.h : main header file for the advideo DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CadvideoApp
// See advideo.cpp for the implementation of this class
//

class CadvideoApp : public CWinApp
{
public:
	CadvideoApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

