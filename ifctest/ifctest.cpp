// ifctest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../adv/adv.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);
	AVSetupDiagnosticOutput(true, true, true);
	HRESULT h = AVIFC(1196);
	CoUninitialize();
	return 0;
}

