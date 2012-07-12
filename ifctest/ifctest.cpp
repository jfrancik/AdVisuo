// ifctest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../adv/adv.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);
	AVSetupDiagnosticOutput(true, true, true);
	HRESULT h = AVIFC8(1199, "c:\\users\\jarek\\desktop\\test.ifc");
	CoUninitialize();
	return 0;
}

