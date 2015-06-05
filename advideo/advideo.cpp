// advideo.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "advideo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CadvideoApp

BEGIN_MESSAGE_MAP(CadvideoApp, CWinApp)
END_MESSAGE_MAP()

// CadvideoApp construction

CadvideoApp::CadvideoApp()
{
}


// The one and only CadvideoApp object
CadvideoApp theApp;


// CadvideoApp initialization
BOOL CadvideoApp::InitInstance()
{
	CWinApp::InitInstance();

	// Resolve default paths
	// chdir
	wchar_t path_buffer[_MAX_PATH];
	wchar_t drive1[_MAX_DRIVE];
	wchar_t drive2[_MAX_DRIVE];
	wchar_t dir1[_MAX_DIR];
	wchar_t dir2[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	/* modified from deprecated _splitpath & _makepath in VS2008 */
	_wsplitpath_s(m_pszHelpFilePath, drive1, dir1, fname, ext);
	if (wcslen(dir1) > 0 && (dir1[wcslen(dir1)-1] == '\\' || dir1[wcslen(dir1)-1] == '/'))
	{
		dir1[wcslen(dir1)-1] = '\0';
		_wsplitpath_s(dir1, drive2, dir2, fname, ext);
	}
	else
		wcscpy_s(dir2, _MAX_DIR, dir1);
	_wmakepath_s(path_buffer, drive1, dir2, L"", L"");
	_stdPathModels = CString(path_buffer) + L"models\\";

	return TRUE;
}

