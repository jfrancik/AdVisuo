// dllmain.cpp - a part of the AdVisuo Server Module
// Defines the entry point for the DLL application.
#include "stdafx.h"
#include "adv.h"

TCHAR g_pMainPath[MAX_PATH];

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		AddEventSource(hModule, L"AdVisuo", 1);
		Log(INFO_STARTED);

		GetModuleFileName(hModule, g_pMainPath, MAX_PATH);

		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

