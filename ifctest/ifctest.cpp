// ifctest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../adv/adv.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);
	AVSetupDiagnosticOutput(true, true, false, true, false, false, false);

	//AVIFC8(1532, "c:\\users\\jarek\\desktop\\test.ifc");
	AVIFC8(51, "c:\\users\\jarek\\desktop\\test.ifc");
	//AVIFC8(51, "test.ifc");

	//AVSaveIFCMesh(L"buffer.ifc");
	//AVSaveIFCMesh(L"cwt.ifc");
	//AVSaveIFCMesh(L"ladder.ifc");
	//AVSaveIFCMesh(L"light.ifc");
	//AVSaveIFCMesh(L"machine138.ifc");
	//AVSaveIFCMesh(L"machine30t.ifc");
	//AVSaveIFCMesh(L"machine40t.ifc");
	//AVSaveIFCMesh(L"machine70t.ifc");
	//AVSaveIFCMesh(L"overspeed.ifc");
	//AVSaveIFCMesh(L"panel1000.ifc");
	//AVSaveIFCMesh(L"panel1250.ifc");
	//AVSaveIFCMesh(L"panel1500.ifc");
	//AVSaveIFCMesh(L"panel1600.ifc");
	//AVSaveIFCMesh(L"panel1700.ifc");
	//AVSaveIFCMesh(L"panel700.ifc");
	//AVSaveIFCMesh(L"pulley.ifc");
	//AVSaveIFCMesh(L"rail.ifc");

	CoUninitialize();
	return 0;
}

