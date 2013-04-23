// ifctest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../adv/adv.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);
	AVSetupDiagnosticOutput(true, true, false, true);
//	AVIFC8(1204, "c:\\users\\jarek\\desktop\\test.ifc");
//	AVIFC8(1204, "c:\\users\\jarek\\desktop\\test.ifc");
//	AVIFC8(1202, "c:\\users\\jarek\\desktop\\warsaw tower.ifc");
//	AVIFC8(1196, "c:\\users\\jarek\\desktop\\elevcon.ifc");

	AVIFC8(50, "c:\\users\\jarek\\desktop\\test.ifc");
//	AVIFC8(1238, "c:\\users\\jarek\\desktop\\test_variety.ifc");
//	AVIFC8(3, "c:\\users\\jarek\\desktop\\test.ifc");


	CoUninitialize();
	return 0;
}

