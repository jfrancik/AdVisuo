// adv.cpp - a part of the AdVisuo Server Module
// Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "../CommonFiles/DBTools.h"
#include "../CommonFiles/DBConnStr.h"
#include "adv.h"
#include "SrvProject.h"
#include "SrvLiftGroup.h"
#include "SrvSim.h"
#include "SrvLift.h"
#include "SrvPassenger.h"
#include "IfcProject.h"
#include "IfcLiftGroup.h"
#include "ATLComTime.h"
#include "IFCBuilder.h"

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
	AVSTRING pConnStr = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	return CProjectSrv::QueryVerInt(pConnStr);
}

ADV_API AVSTRING AVGetRelease()
{
	AVSTRING pConnStr = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	auto str = CProjectSrv::QueryVerStr(pConnStr);
	static wchar_t buf[128];
	wcscpy_s(buf, str.c_str());
	return buf;
}

ADV_API HRESULT AVSetConnStrings(AVSTRING pConnConsole, AVSTRING pConnReports, AVSTRING pConnVisualisation, AVSTRING pConnUsers, AVSTRING pConnSimQueue)
{
	HKEY hRegKey = NULL; 
	HRESULT h;
	h = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hRegKey, NULL); if (h != 0) return Log(ERROR_COM, h);
	h = RegSetValueEx(hRegKey, L"ConsoleConnectionString", 0, REG_EXPAND_SZ, (PBYTE)pConnConsole, (_tcslen(pConnConsole) + 1) * sizeof TCHAR); if (h != 0) return Log(ERROR_COM, h);
	h = RegSetValueEx(hRegKey, L"ReportsConnectionString", 0, REG_EXPAND_SZ, (PBYTE)pConnReports, (_tcslen(pConnReports) + 1) * sizeof TCHAR); if (h != 0) return Log(ERROR_COM, h);
	h = RegSetValueEx(hRegKey, L"VisualisationConnectionString", 0, REG_EXPAND_SZ, (PBYTE)pConnVisualisation, (_tcslen(pConnVisualisation) + 1) * sizeof TCHAR); if (h != 0) return Log(ERROR_COM, h);
	h = RegSetValueEx(hRegKey, L"UsersConnectionString", 0, REG_EXPAND_SZ, (PBYTE)pConnUsers, (_tcslen(pConnUsers) + 1) * sizeof TCHAR); if (h != 0) return Log(ERROR_COM, h);
	h = RegSetValueEx(hRegKey, L"SimQueueConnectionString", 0, REG_EXPAND_SZ, (PBYTE)pConnSimQueue, (_tcslen(pConnSimQueue) + 1) * sizeof TCHAR); if (h != 0) return Log(ERROR_COM, h);
	h = RegCloseKey(hRegKey); if (h != 0) return Log(ERROR_COM, h);
	Logf(INFO_CONFIG_SET, L"\n\rConsole = %s\n\rReports = %s\n\rVisualisation = %s\n\rUsers = %s\n\rSimQueue = %s", pConnConsole, pConnReports, pConnVisualisation, pConnUsers, pConnSimQueue);
	return S_OK;
}

ADV_API HRESULT AVSetConnStrings8(char *pConnConsole, char *pConnReports, char *pConnVisualisation, char *pConnUsers, char *pConnSimQueue)
{
	USES_CONVERSION;
	CA2W wConnConsole(pConnConsole);
	CA2W wConnReports(pConnReports);
	CA2W wConnVisualisation(pConnVisualisation);
	CA2W wConnUsers(pConnUsers);
	CA2W wConnSimQueue(pConnSimQueue);
	return AVSetConnStrings(wConnConsole, wConnReports, wConnVisualisation, wConnUsers, wConnSimQueue);
}

ADV_API HRESULT AVSetConnString(AVSTRING pConn)
{
	static wchar_t pConnConsole[1024];
	static wchar_t pConnReport[1024];
	static wchar_t pConnVisualisation[1024];
	static wchar_t pConnUsers[1024];
	static wchar_t pConnSimQueue[1024];
	if (wcschr(pConn, L'=') == NULL)
	{
		_snwprintf_s(pConnConsole, 1024,	   L"Provider=SQLOLEDB;Data Source=%s\\SQLEXPRESS;Initial Catalog=Adsimulo_Console;Integrated Security=SSPI;", pConn);
		_snwprintf_s(pConnReport, 1024,	       L"Provider=SQLOLEDB;Data Source=%s\\SQLEXPRESS;Initial Catalog=Adsimulo_Reports;Integrated Security=SSPI;", pConn);
		_snwprintf_s(pConnVisualisation, 1024, L"Provider=SQLOLEDB;Data Source=%s\\SQLEXPRESS;Initial Catalog=Adsimulo_Visualisation;Integrated Security=SSPI;", pConn);
		_snwprintf_s(pConnUsers, 1024,         L"Provider=SQLOLEDB;Data Source=%s\\SQLEXPRESS;Initial Catalog=AD_AspNetProviders;Integrated Security=SSPI;", pConn);
		_snwprintf_s(pConnSimQueue, 1024,      L"Provider=SQLOLEDB;Data Source=%s\\SQLEXPRESS;Initial Catalog=AD_AspNetProviders;Integrated Security=SSPI;", pConn);
	}
	else
	{
		_snwprintf_s(pConnConsole, 1024, pConn, L"Adsimulo_Console");
		_snwprintf_s(pConnReport, 1024, pConn, L"Adsimulo_Reports");
		_snwprintf_s(pConnVisualisation, 1024, pConn, L"Adsimulo_Visualisation");
		_snwprintf_s(pConnUsers, 1024, pConn, L"AD_AspNetProviders");
		_snwprintf_s(pConnSimQueue, 1024, pConn, L"SimQueue");
	}
	return AVSetConnStrings(pConnConsole, pConnReport, pConnVisualisation, pConnUsers, pConnSimQueue);
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
extern bool g_bVerbose;
extern bool g_bBenchmark;
extern bool g_bProgressOnScreen;
extern bool g_bProgressDB;
extern bool g_bLog;

ADV_API HRESULT AVSetupDiagnosticOutput(bool bRegisterEventLog, bool bPrintOnScreen, bool bVerbose, bool bBenchmark, bool bProgressOnScreen, bool bProgressDB, bool bLog)
{
	g_bRegEvents = bRegisterEventLog;
	g_bOnScreen = bPrintOnScreen;
	g_bVerbose = bVerbose;
	g_bBenchmark = bBenchmark;
	g_bProgressOnScreen = bProgressOnScreen;
	g_bProgressDB = bProgressDB;
	g_bLog = bLog;

	return S_OK;
}

// Tests for presence of the simulation data in the visualisation database.
// Input:  - nSimulationId - simulation id as uded in CONSOLE database
// Return values:
// S_OK			if the visualisation data found, processed, up-to-date and ready to start
// S_FALSE		if the visualisation data not found
// S_FALSE+1	if the visualisation data found but outdated
// Standard error code in case of errors - check with FAILED(...) macro
ADV_API HRESULT AVTest(AVULONG nSimulationId)
{
	CLogTime lt;
	AVSTRING pConnStr = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	try
	{
		HRESULT h;
		DWORD dwStatus = STATUS_OK;

		AVULONG nProjectID;
		
		h = CProjectSrv::FindProjectID(pConnStr, nSimulationId, nProjectID);
		if (h == S_FALSE) return S_FALSE;

		CProjectSrv prj;
		h = prj.LoadFromVisualisation(pConnStr, nProjectID);

		lt.Log(L"AVTest");

		// should retuen S_OK if data up to date
		return S_FALSE+1;
	}
	catch (...)
	{
		return S_FALSE;
	}
}

ADV_API HRESULT AVDelete(AVULONG nSimulationId)
{
	CLogTime lt;
	AVSTRING pConnStr = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	try
	{
		DWORD dwStatus = STATUS_OK;

		HRESULT h = CProjectSrv::CleanUp(pConnStr, nSimulationId);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Log(L"AVDelete");
		return Logf(dwStatus, L"AVDelete(%d)", nSimulationId);
	}
	STD_CATCH(L"AVDelete(%d)", nSimulationId);
}

ADV_API HRESULT AVDeleteAll()
{
	CLogTime lt;
	AVSTRING pConnStr = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	try
	{
		DWORD dwStatus = STATUS_OK;

		HRESULT h = CProjectSrv::CleanUpAll(pConnStr);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Log(L"AVDeleteAll");
		return Logf(dwStatus, L"AVDeleteAll()");
	}
	STD_CATCH(L"AVDeleteAll()", 0);
}

ADV_API HRESULT AVDropTables()
{
	CLogTime lt;
	AVSTRING pConnStr = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	try
	{
		DWORD dwStatus = STATUS_OK;

		HRESULT h = CProjectSrv::DropTables(pConnStr);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Log(L"AVDropTables");
		return Logf(dwStatus, L"AVDropTables()");
	}
	STD_CATCH(L"AVDropTables()", 0);
}

ADV_API HRESULT AVWriteReinitBATFile()
{
	AVSTRING pVisConn = dbtools::GetConnString(dbtools::CONN_VISUALISATION);

	std::vector<AVULONG> ids;
	CProjectSrv::QueryAvailIds(pVisConn, ids);

	std::wcout << L"@echo off" << std::endl;
	std::wcout << L"adv -drop" << std::endl;
	for each (AVULONG id in ids)
		std::wcout << L"adv " << id << std::endl;

	return S_OK;
}

ADV_API HRESULT AVInit(AVULONG nSimulationId, AVULONG &nProjectID)
{
	CLogTime lt;
	AVSTRING pConsoleConn = dbtools::GetConnString(dbtools::CONN_CONSOLE);
	AVSTRING pVisConn = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	AVSTRING pRepConn = dbtools::GetConnString(dbtools::CONN_REPORT);
	AVSTRING pSimQueueConn = dbtools::GetConnString(dbtools::CONN_SIMQUEUE);
		
	try
	{
		AVULONG nMilestones = 4;
		dbtools::CDataBase db(pSimQueueConn);
		InitProgress(&db, nSimulationId, nMilestones);

		HRESULT h = S_OK;
		DWORD dwStatus = STATUS_OK;
		LogProgressStep();

		h = CProjectSrv::CleanUp(pVisConn, nSimulationId); 
		if WARNED(h) dwStatus = STATUS_WARNING;
		LogProgressStep();

		CProjectSrv prj;
		h = prj.LoadFromConsole(pConsoleConn, pRepConn, nSimulationId);
		if WARNED(h) dwStatus = STATUS_WARNING;

		h = prj.Store(pVisConn); 
		if WARNED(h) dwStatus = STATUS_WARNING;

		h = prj.Update(pVisConn); 
		if WARNED(h) dwStatus = STATUS_WARNING;
		LogProgressStep();

		ASSERT(prj.GetId() == prj.GetLiftGroup(0)->GetProjectId());
		nProjectID = prj.GetId();

		lt.Log(L"AVInit");
		return Logf(dwStatus, L"AVInit(%d)", nSimulationId);
	}
	STD_CATCH(L"AVInit(%d)", nSimulationId);
}

ADV_API HRESULT AVProcess(AVULONG nProjectID)
{
	CLogTime ltall, lt;
	AVSTRING pVisConn = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	AVSTRING pRepConn = dbtools::GetConnString(dbtools::CONN_REPORT);
	AVSTRING pSimQueueConn = dbtools::GetConnString(dbtools::CONN_SIMQUEUE);
	
	try
	{
		AVULONG nMilestones = 3 + 2 * CProjectSrv::QuerySimCountFromVisualisation(pVisConn, nProjectID);
		dbtools::CDataBase db(pSimQueueConn);
		InitProgress(&db, -1, nMilestones);

		HRESULT h;
		DWORD dwStatus = STATUS_OK;

		AVFLOAT fScale;
		GetScale(&fScale);

		h = CProjectSrv::CleanUpSim(pVisConn, nProjectID); 
		if WARNED(h) dwStatus = STATUS_WARNING;
		LogProgressStep();

		CProjectSrv prj;
		h = prj.LoadFromVisualisation(pVisConn, nProjectID);
		if WARNED(h) dwStatus = STATUS_WARNING;
		LogProgressStep();
		LogProgressStep();

		prj.Scale(fScale);

		lt.Reset();
		h = prj.FastLoadPlayUpdate(pVisConn, pRepConn); lt.Log(L"FastLoadFromReports");
		if WARNED(h) dwStatus = STATUS_WARNING;

		// Old, "slow" version
		//lt.Reset();
		//h = prj.LoadFromReports(pRepConn); lt.Log(L"LoadFromReports");
		//if WARNED(h) dwStatus = STATUS_WARNING;
		//LogProgressStep();

		// //prj.Play(); lt.Log(L"Play");
		// //h = prj.Update(pVisConn); lt.Log(L"Update");
		//h = prj.PlayAndUpdate(pVisConn); lt.Log(L"PlayAndUpdate");
		//if WARNED(h) dwStatus = STATUS_WARNING;

		ltall.Log(L"AVProcess");
		return Logf(dwStatus, L"AVProcess(%d)", nProjectID);
	}
	STD_CATCH(L"AVProcess(%d)", nProjectID);
}

ADV_API HRESULT AVRun(AVULONG nSimulationId)
{
	CLogTime ltall, lt;
	AVSTRING pConsoleConn = dbtools::GetConnString(dbtools::CONN_CONSOLE);
	AVSTRING pVisConn = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	AVSTRING pRepConn = dbtools::GetConnString(dbtools::CONN_REPORT);
	AVSTRING pSimQueueConn = dbtools::GetConnString(dbtools::CONN_SIMQUEUE);
	
	try
	{
		AVULONG nMilestones = 3 + CProjectSrv::QuerySimCountFromConsole(pConsoleConn, nSimulationId);
		dbtools::CDataBase db(pSimQueueConn);
		InitProgress(&db, nSimulationId, nMilestones);

		HRESULT h = S_OK;
		DWORD dwStatus = STATUS_OK;

		lt.Reset();
		h = CProjectSrv::CleanUp(pVisConn, nSimulationId); lt.Log(L"CleanUp");
		if WARNED(h) dwStatus = STATUS_WARNING;

		LogProgressStep();

		CProjectSrv prj;
		lt.Reset();
		h = prj.LoadFromConsole(pConsoleConn, pRepConn, nSimulationId); lt.Log(L"LoadFromConsole");
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Reset();
		h = prj.Store(pVisConn); lt.Log(L"Store");
		if WARNED(h) dwStatus = STATUS_WARNING;

		LogProgressStep();

//		h = prj.Update(pVisConn); 
//		if WARNED(h) dwStatus = STATUS_WARNING;

		AVFLOAT fScale;
		GetScale(&fScale);
		prj.Scale(fScale);

		lt.Reset();
		h = prj.FastLoadPlayUpdate(pVisConn, pRepConn); lt.Log(L"FastLoadFromReports");
		if WARNED(h) dwStatus = STATUS_WARNING;

		// Old, "slow" version
		//lt.Reset();
		//h = prj.LoadFromReports(pRepConn); lt.Log(L"LoadFromReports");
		//if WARNED(h) dwStatus = STATUS_WARNING;
		//LogProgressStep();

		// //prj.Play(); lt.Log(L"Play");
		// //h = prj.Update(pVisConn); lt.Log(L"Update");
		//h = prj.PlayAndUpdate(pVisConn); lt.Log(L"PlayAndUpdate");
		//if WARNED(h) dwStatus = STATUS_WARNING;

		ltall.Log(L"AVRun");

		// Log Output
		if (g_bLog)
		{
			std::wofstream myfile;
			myfile.open ("advisuo.log");
			for each (CLiftGroupSrv *pGroup in prj.GetLiftGroups())
			{
				myfile << L"LIFT GROUP: " << pGroup->GetName() << endl;
				for each (CSimSrv *pSim in pGroup->GetSims())
				{
					myfile << L"  SCENARIO: " << pSim->GetScenarioName() << endl;
					for each (CLiftSrv *pLift in pSim->GetLifts())
					{
						myfile << L"    LIFT: " << pLift->GetId() << endl;
						for each (JOURNEY j in pLift->GetJourneys())
							if (j.m_floorTo != (AVULONG)-1)
								myfile << L"      JOURNEY: from " << j.m_floorFrom << L" to " << j.m_floorTo << L", " << j.m_timeGo/1000 << L" => " << j.m_timeDest/1000 << L"; doorcycles: " << j.StringifyDoorCycles() << endl;
					}
					myfile << L"  - PASSENGERS:" << endl;
					std::sort(pSim->GetPassengers().begin(), pSim->GetPassengers().end(), [](CPassenger *p1, CPassenger *p2) -> bool 
					{ 
						if (p1->GetLiftId() == p2->GetLiftId())
							return p1->GetLoadTime() < p2->GetLoadTime(); 
						else
							return p1->GetLiftId() < p2->GetLiftId(); 
					});
					for each (CPassengerSrv *pPassenger in pSim->GetPassengers())
						myfile << L"    PASSENGER: lift: " << pPassenger->GetLiftId() << L" from " << pPassenger->GetArrivalFloor() << L" to " << pPassenger->GetDestFloor() << L", " << pPassenger->GetLoadTime() << L" => " << pPassenger->GetUnloadTime() << endl;
				}
			}
			myfile.close();
		}

		_CrtDumpMemoryLeaks();
		return Logf(dwStatus, L"AVRun(%d)", nSimulationId);
	}
	STD_CATCH(L"AVRun(%d)", nSimulationId);
}

ADV_API HRESULT AVIFC(AVULONG nSimulationId, AVSTRING strIFCPathName)
{
	CLogTime lt;
	AVSTRING pConsoleConn = dbtools::GetConnString(dbtools::CONN_CONSOLE);
	AVSTRING pRepConn = dbtools::GetConnString(dbtools::CONN_REPORT);

	try
	{
		HRESULT h;
		DWORD dwStatus = STATUS_OK;

		CProjectIfc prj;
		h = prj.LoadFromConsole(pConsoleConn, nSimulationId);
		if WARNED(h) dwStatus = STATUS_WARNING;

		prj.SetRevitCompatibilityMode_(true);
		prj.Construct();

		h = prj.SaveAsIFC(strIFCPathName);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Log(L"AVIFC");
		Logf(STATUS_GENERIC, L"IFC file saved to: %s", strIFCPathName);
		return Logf(dwStatus, L"AVIFC(%d)", nSimulationId);
	}
	STD_CATCH(L"AVIFC(%d)", nSimulationId);

	//_CrtDumpMemoryLeaks();
}

ADV_API HRESULT AVIFC8(AVULONG nSimulationId, char *pIFCPathName)
{
	USES_CONVERSION;
	CA2W wIFCPathName(pIFCPathName);
	return AVIFC(nSimulationId, wIFCPathName);
}

ADV_API HRESULT AVSaveIFCMesh(AVSTRING strIFCPathName, AVSTRING strMeshPathName)
{
	CIfcBuilder *pBuilder = new CIfcBuilder(strIFCPathName);
	if (pBuilder) pBuilder->SaveAsMesh(strMeshPathName);
	return S_OK;
}

ADV_API HRESULT AVSaveIFCMesh8(char *strIFCPathName, char *strMeshPathName)
{
	return S_OK;
}

#pragma warning (disable: 4996)
ADV_API HRESULT AVCreateTicket(AVSTRING strUserName, AVSTRING strBuf)
{
	CLogTime lt;
	AVSTRING pUsersConn = dbtools::GetConnString(dbtools::CONN_USERS);
	AVSTRING pVisConn = dbtools::GetConnString(dbtools::CONN_VISUALISATION);
	try
	{
		DWORD dwStatus = STATUS_OK;

		HRESULT h = CProjectSrv::CreateTicket(pVisConn, strUserName, strBuf);
		if WARNED(h) dwStatus = STATUS_WARNING;

		lt.Log(L"AVCreateTicket");
		return Logf(dwStatus, L"AVCreateTicket(\"%s\")", strUserName);
	}
	STD_CATCH(L"AVCreateTicket()", 0);
}

