// adv.cpp - a part of the AdVisuo Server Module
// Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "adv.h"
#include "Building.h"
#include "Sim.h"
#include "ATLComTime.h"
#include "../CommonFiles/DBTools.h"

#define WARNED(hr) (((DWORD)(hr)) >= 0x40000000L)

#define STD_CATCH(str, num)					\
	catch(_com_error &ce)					\
	{										\
		Log(ERROR_DB, ce);					\
		Logf(STATUS_FAIL, (str), (num));	\
		return ce.Error();					\
	}										\
	catch (HRESULT h)						\
	{										\
		Log(ERROR_COM, h);					\
		Logf(STATUS_FAIL, (str), (num));	\
		return h;							\
	}										\
	catch (DWORD dw)						\
	{										\
		Log(dw);							\
		Logf(STATUS_FAIL, (str), (num));	\
		return dw;							\
	}										\
	catch (dbtools::CValue val)				\
	{										\
		Log(ERROR_CONVERSION);				\
		Logf(STATUS_FAIL, (str), (num));	\
		return ERROR_CONVERSION;			\
	}										\
	catch(...)								\
	{										\
		return Logf(ERROR_UNKNOWN, (str), (num));	\
	}


ADV_API AVULONG AVGetVersion()
{
	return CSimBase::GetAVNativeVersionId();
}

ADV_API HRESULT AVSetConnStrings(AVSTRING pConnConsole, AVSTRING pConnVisualisation)
{
	std::wstring str1;
	if (!pConnConsole)
	{
		str1 = pConnVisualisation;
		size_t i = str1.find(L"Adsimulo_Visualisation");
		if (i != str1.npos) str1.replace(i, wcslen(L"Adsimulo_Visualisation"), L"Adsimulo_Console");
		pConnConsole = (LPOLESTR)str1.c_str();
	}

	HKEY hRegKey = NULL; 
	HRESULT h;
	h = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hRegKey, NULL); if (h != 0) return Log(ERROR_COM, h);
	h = RegSetValueEx(hRegKey, L"ConsoleConnectionString", 0, REG_EXPAND_SZ, (PBYTE)pConnConsole, (_tcslen(pConnConsole) + 1) * sizeof TCHAR); if (h != 0) return Log(ERROR_COM, h);
	h = RegSetValueEx(hRegKey, L"VisualisationConnectionString", 0, REG_EXPAND_SZ, (PBYTE)pConnVisualisation, (_tcslen(pConnVisualisation) + 1) * sizeof TCHAR); if (h != 0) return Log(ERROR_COM, h);
	h = RegCloseKey(hRegKey); if (h != 0) return Log(ERROR_COM, h);
	Logf(INFO_CONFIG_SET, L"\n\rConsole = %s\n\rVisualisation = %s", pConnConsole, pConnVisualisation);
	return S_OK;
}

ADV_API HRESULT AVSetConnStrings8(char *pConnConsole, char *pConnVisualisation)
{
	USES_CONVERSION;
	CA2W wConnConsole(pConnConsole);
	CA2W wConnVisualisation(pConnVisualisation);
	return AVSetConnStrings(wConnConsole, wConnVisualisation);
}

ADV_API HRESULT AVSetConnString(AVSTRING pConn)
{
	static wchar_t pConnConsole[1024];
	static wchar_t pConnVisualisation[1024];
	if (wcschr(pConn, L'=') == NULL)
	{
		_snwprintf_s(pConnConsole, 1024,	   L"Provider=SQLOLEDB;Data Source=%s\\SQLEXPRESS;Initial Catalog=Adsimulo_Console;Integrated Security=SSPI;", pConn);
		_snwprintf_s(pConnVisualisation, 1024, L"Provider=SQLOLEDB;Data Source=%s\\SQLEXPRESS;Initial Catalog=Adsimulo_Visualisation;Integrated Security=SSPI;", pConn);
	}
	else
	{
		_snwprintf_s(pConnConsole, 1024, pConn, L"Adsimulo_Console");
		_snwprintf_s(pConnVisualisation, 1024, pConn, L"Adsimulo_Visualisation");
	}
	return AVSetConnStrings(pConnConsole, pConnVisualisation);
}

ADV_API HRESULT AVSetConnString8(char *pConn)
{
	USES_CONVERSION;
	CA2W wConn(pConn);
	return AVSetConnString(wConn);
}


ADV_API HRESULT AVSetScalingFactor(AVFLOAT fScale)
{
	HKEY hRegKey = NULL; 
	HRESULT h;
	h = RegCreateKey(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", &hRegKey); if (h != 0) return Log(ERROR_COM, h);
	h = RegSetValueEx(hRegKey, L"Scale", 0, REG_BINARY, (PBYTE)&fScale, sizeof AVFLOAT); if (h != 0) return Log(ERROR_COM, h);
	h = RegCloseKey(hRegKey); if (h != 0) return Log(ERROR_COM, h);
	Logf(INFO_CONFIG_SET, L"\nScale = %f", fScale);
	return S_OK;
}

HRESULT GetConnStrings(AVSTRING *ppConnConsole, AVSTRING *ppConnVisualisation)
{
	static WCHAR bufConnConsole[1024];
	static WCHAR bufConnVisualisation[1024];
	HKEY hRegKey = NULL; 

	DWORD type, size;
	HRESULT h;
	h = RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", &hRegKey); if (h != 0) return Log(ERROR_COM, h);
	size = 1024; h = RegQueryValueEx(hRegKey, L"ConsoleConnectionString", 0, &type, (PBYTE)bufConnConsole, &size); if (h != 0) return Log(ERROR_COM, h);
	size = 1024; h = RegQueryValueEx(hRegKey, L"VisualisationConnectionString", 0, &type, (PBYTE)bufConnVisualisation, &size); if (h != 0) return Log(ERROR_COM, h);
	h = RegCloseKey(hRegKey); if (h != 0) return Log(ERROR_COM, h);

	if (ppConnConsole) *ppConnConsole = bufConnConsole;
	if (ppConnVisualisation) *ppConnVisualisation = bufConnVisualisation;

	return S_OK;
}

HRESULT GetScale(AVFLOAT *pScale)
{

	HKEY hRegKey = NULL; 

	DWORD type, size = sizeof AVFLOAT;
	LONG r1 = RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", &hRegKey);
	LONG r2 = RegQueryValueEx(hRegKey, L"Scale", 0, &type, (PBYTE)pScale, &size);
	if (r1 != 0 || r2 != 0)
		*pScale = 0.04f;
	RegCloseKey(hRegKey);
	return S_OK;
}

extern bool g_bRegEvents;
extern bool g_bOnScreen;
extern bool g_bBenchmark;

ADV_API HRESULT AVSetupDiagnosticOutput(bool bRegisterEventLog, bool bPrintOnScreen, bool bBenchmark)
{
	g_bRegEvents = bRegisterEventLog;
	g_bOnScreen = bPrintOnScreen;
	g_bBenchmark = bBenchmark;
	return S_OK;
}

// Tests for presence of the simulation data in the visualisation database.
// Input:  - nSimulationID - simulation id as uded in CONSOLE database
// Return values:
// S_OK			if the visualisation data found, processed, up-to-date and ready to start
// S_FALSE		if the visualisation data not found
// S_FALSE+1	if the visualisation data found but outdated
// Standard error code in case of errors - check with FAILED(...) macro
ADV_API HRESULT AVTest(AVULONG nSimulationID)
{
	CLogTime lt;
	AVSTRING pConnStr;
	GetConnStrings(NULL, &pConnStr);
	try
	{
		HRESULT h;
		DWORD dwStatus = STATUS_OK;

		AVULONG nProjectID;
		
		CSim sim(NULL);
		h = sim.FindProjectID(pConnStr, nSimulationID, nProjectID);
		if (h == S_FALSE) return S_FALSE;

		h = sim.LoadFromVisualisation(pConnStr, nProjectID);

		FILETIME ft;
		HANDLE hFile = CreateFileW(sim.GetSIMFileName().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
		if (hFile == INVALID_HANDLE_VALUE) return S_FALSE;
		GetFileTime(hFile, NULL, NULL, &ft);
		CloseHandle(hFile);

		COleDateTime timeSim(sim.GetTimeStamp());
		COleDateTime timeFile(ft);

		lt.Log(L"AVTest");
		if (timeSim >= timeFile)
			return S_OK;
		else
			return S_FALSE+1;
	}
	STD_CATCH(L"AVTest(%d)", nSimulationID);
}

ADV_API HRESULT AVDelete(AVULONG nSimulationId)
{
	CLogTime lt;
	AVSTRING pConnStr;
	GetConnStrings(NULL, &pConnStr);
	try
	{
		DWORD dwStatus = STATUS_OK;

		HRESULT h = CSim::CleanUp(pConnStr, nSimulationId);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Log(L"AVDelete");
		return Logf(dwStatus, L"AVDelete(%d)", nSimulationId);
	}
	STD_CATCH(L"AVDelete(%d)", nSimulationId);
}

ADV_API HRESULT AVDeleteAll()
{
	CLogTime lt;
	AVSTRING pConnStr;
	GetConnStrings(NULL, &pConnStr);
	try
	{
		DWORD dwStatus = STATUS_OK;

		HRESULT h = CSim::CleanUpAll(pConnStr);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Log(L"AVDeleteAll");
		return Logf(dwStatus, L"AVDeleteAll()");
	}
	STD_CATCH(L"AVDeleteAll()", 0);
}

ADV_API HRESULT AVDropTables()
{
	CLogTime lt;
	AVSTRING pConnStr;
	GetConnStrings(NULL, &pConnStr);
	try
	{
		DWORD dwStatus = STATUS_OK;

		HRESULT h = CSim::DropTables(pConnStr);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Log(L"AVDropTables");
		return Logf(dwStatus, L"AVDropTables()");
	}
	STD_CATCH(L"AVDropTables()", 0);
}

ADV_API HRESULT AVInit(AVULONG nSimulationId, AVULONG &nProjectID)
{
	CLogTime lt;
	AVSTRING pConsoleConn, pVisConn;
	GetConnStrings(&pConsoleConn, &pVisConn);
	
	try
	{
		HRESULT h = S_OK;
		DWORD dwStatus = STATUS_OK;

		h = CSim::CleanUp(pVisConn, nSimulationId); 
		if WARNED(h) dwStatus = STATUS_WARNING;
		
		CBuilding building;
		h = building.LoadFromConsole(pConsoleConn, nSimulationId); 
		if WARNED(h) dwStatus = STATUS_WARNING;

		CSim sim(&building);
		h = sim.LoadFromConsole(pConsoleConn, nSimulationId); 
		if WARNED(h) dwStatus = STATUS_WARNING;
		
		h = sim.Store(pVisConn, nSimulationId); 
		if WARNED(h) dwStatus = STATUS_WARNING;

		nProjectID = sim.GetProjectId();

		lt.Log(L"AVInit");
		return Logf(dwStatus, L"AVInit(%d)", nSimulationId);
	}
	STD_CATCH(L"AVInit(%d)", nSimulationId);
}

ADV_API HRESULT AVProcess(AVULONG nProjectID)
{
	CLogTime ltall, lt;
	AVSTRING pVisConn;
	GetConnStrings(NULL, &pVisConn);
	
	try
	{
		HRESULT h;
		DWORD dwStatus = STATUS_OK;

		AVFLOAT fScale;
		GetScale(&fScale);

		CBuilding building;
		h = building.LoadFromVisualisation(pVisConn, nProjectID);
		building.Scale(fScale);
		if WARNED(h) dwStatus = STATUS_WARNING;

		CSim sim(&building);
		h = sim.LoadFromVisualisation(pVisConn, nProjectID);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Reset(); 
		h = sim.LoadSim(); lt.Log(L"LoadSim");
		if WARNED(h) dwStatus = STATUS_WARNING;

		sim.Play(); lt.Log(L"Play");

		h = sim.Update(pVisConn); lt.Log(L"Update");
		if WARNED(h) dwStatus = STATUS_WARNING;

		ltall.Log(L"AVProcess");
		return Logf(dwStatus, L"AVProcess(%d)", nProjectID);
	}
	STD_CATCH(L"AVProcess(%d)", nProjectID);
}

ADV_API HRESULT AVIFC(AVULONG nSimulationId)
{
	CLogTime lt;
	AVSTRING pConsoleConn;
	GetConnStrings(&pConsoleConn, NULL);
	
	try
	{
		HRESULT h;
		DWORD dwStatus = STATUS_OK;

		CBuilding building;
		h = building.LoadFromConsole(pConsoleConn, nSimulationId);
		if WARNED(h) dwStatus = STATUS_WARNING;

		CSim sim(&building);
		h = sim.LoadFromConsole(pConsoleConn, nSimulationId);
		if WARNED(h) dwStatus = STATUS_WARNING;

		h = sim.GetBuilding()->SaveAsIFC(sim.GetIFCFileName().c_str());
		if WARNED(h) dwStatus = STATUS_WARNING;
		
		lt.Log(L"AVIFC");
		Logf(STATUS_GENERIC, L"IFC file saved to: %s", sim.GetIFCFileName().c_str());
		return Logf(dwStatus, L"AVIFC(%d)", nSimulationId);
	}
	STD_CATCH(L"AVIFC(%d)", nSimulationId);
}

