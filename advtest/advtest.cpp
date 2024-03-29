﻿// advtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <conio.h>
#include "../adv/adv.h"

using namespace std;

////////////////////////////////////////
// main function

bool version()
{
	try
	{
		AVULONG v = AVGetVersion();
		AVSTRING s = AVGetRelease();
		wcout << L"ADV AdVisuo Server Ver " << v / 10000 << L"." << (v % 10000) / 100 << L"."  << v % 100 << L" (" << s << L"). Copyright (C) 2011-14 Lerch Bates" << endl;
		return true;
	}
	catch (...)
	{
		wcout << L"ADV AdVisuo Server. Copyright (C) 2011-14 Lerch Bates" << endl;
		wcout << L"Version number not found: use adv -c to configure the connection string." << endl;
		return false;
	}
}

void usage()
{
	wcout << L"Usage:" << endl
			<< L"  adv [ID]      reads simulation data and prepares for visualisation" << endl
			<< L"  adv -d [ID]   deletes visualisation data for the specified simulation" << endl
			<< L"  adv -dall     deletes all available visualisation data" << endl
			<< L"  adv -drop     drops all table structure - the next storage will re-initialise" << endl
			<< L"  adv -r        generates a BAT file to drop and then reinit all data" << endl
			<< L"  adv -f [ID]   creates IFC file for the specified simulation" << endl
			<< L"  adv -t [ID]   calls AVTest for the specified simulation" << endl
			<< L"  adv -i [ID]   calls AVInit for the specified simulation" << endl
			<< L"  adv -p [ID]   calls AVProcess for the specified project" << endl
			<< L"  adv -h [ID]   displays this information" << endl
			<< L"  adv -ver      displays version information" << endl
			<< L"  adv -c conn   configures the connection string; use %s for the db name" << endl
			<< L"  adv -u userid creates a ticket for the user id" << endl
			<< L"[ID] is optional and may be omitted to enter interactive mode." << endl
			<< L"More options:" << endl
			<< L"-q for quiet mode    -qp for not showing progress   -v for verbose mode" << endl
			<< L"-b to display benchmarks    -l to generate detailed log" << endl
			<< L"-y to proceed, -n to discard when sim up to date (avoiding a query to continue)" << endl
			<< L"-w to wait for a key pressed at the end of execution" << endl
			<< endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);
	AVSetupDiagnosticOutput(false, false, false, false, false, false, false);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	enum OPTION { DEFAULT, IFC, DEL, DEL_ALL, DROP_TABLES, REINIT, TEST, INIT, PROCESS, USER_TICKET, CONN, CONN_ELEVATED, HELP, VERSION, WRONG, WRONG_2 } option = DEFAULT;
	AVULONG nSimulationID = 0;
	AVULONG nProjectID;
	_TCHAR *pParam = NULL;
	bool bQuiet = false, bPercentage = true, bVerbose = false, bYes = false, bNo = false, bWait = false, bBenchmark = false, bLog = false;
	HRESULT h = S_OK;

	// read params...
	for (int i = 1; i < argc; i++)
		if (_wtoi(argv[i]) > 0)
			nSimulationID = _wtoi(argv[i]);
		else
		if (argv[i][0] == L'-' || argv[i][0] == L'/')
			switch (argv[i][1])
			{
			case 'q':
				bPercentage = false;
				if (_wcsicmp(argv[i], L"-q") == 0)
					bQuiet = true;
				break;
			case 'v':
				if (_wcsicmp(argv[i], L"-ver") == 0)
					option = (option == DEFAULT) ? VERSION : WRONG;
				else
					bVerbose = true;
				break;
			case 'w':
				bWait = true;
				break;
			case 'n':
				bNo = true;
				break;
			case 'y':
				bYes = true;
				break;
			case 'b':
				bBenchmark = true;
				break;
			case 'l':
				bLog = true;
				break;
			case 'f':
				option = (option == DEFAULT) ? IFC : WRONG;
				break;
			case 'd':
				if (_wcsicmp(argv[i], L"-dall") == 0)
					option = (option == DEFAULT) ? DEL_ALL : WRONG;
				else if (_wcsicmp(argv[i], L"-drop") == 0)
					option = (option == DEFAULT) ? DROP_TABLES : WRONG;
				else
					option = (option == DEFAULT) ? DEL : WRONG;
				break;

			case 'r':
				option = (option == DEFAULT) ? REINIT : WRONG;
				break;
			case 't':
				option = (option == DEFAULT) ? TEST : WRONG;
				break;
			case 'i':
				option = (option == DEFAULT) ? INIT : WRONG;
				break;
			case 'p':
				option = (option == DEFAULT) ? PROCESS : WRONG;
				break;
			case 'c':
				if (_wcsicmp(argv[i], L"-c-elevated") == 0)
					option = (option == DEFAULT) ? CONN_ELEVATED : WRONG;
				else
					option = (option == DEFAULT) ? CONN : WRONG;
				break;
			case 'u':
				option = (option == DEFAULT) ? USER_TICKET : WRONG;
				break;
			case 'h':
			case '?':
				option = (option == DEFAULT) ? HELP : WRONG;
				break;
			default:
				option = WRONG;
			}
		else
			pParam = argv[i];

	if ((option == DEL_ALL || option == DROP_TABLES || option == REINIT || option == CONN || option == CONN_ELEVATED) && nSimulationID > 0)
		option = WRONG;

	if ((option == USER_TICKET || option == CONN || option == CONN_ELEVATED) && pParam == NULL)
		option = WRONG;

	if ((option == DEFAULT || option == IFC || option == DEL || option == TEST || option == INIT || option == PROCESS) && nSimulationID <= 0)
	{
		if (!bQuiet && !version()) { CoUninitialize(); return 0; }
		if (!bQuiet) wcout << (option == PROCESS ? L"ProjectID: " : L"SimulationID: ");
		wcin >> nSimulationID;
		if (nSimulationID <= 0) option = WRONG_2;
	}

	AVSetupDiagnosticOutput(true, !bQuiet, bVerbose, bBenchmark, bPercentage, false, bLog);

	switch (option)
	{
	case DEFAULT:
		//h = AVTest(nSimulationID);
		//if FAILED(h)
		//	break;
		//if (h == S_OK)
		//{
		//	if (bQuiet)
		//	{
		//		if (!bYes) break;
		//	}
		//	else if (bNo)
		//	{
		//		wcout << L"Project " << nSimulationID << " is up to date." << endl;
		//		break;
		//	}
		//	else if (bYes)
		//		wcout << L"Project " << nSimulationID << " is up to date but will be proceeded." << endl;
		//	else
		//	{
		//		wcout << L"Project " << nSimulationID << " is up to date. Proceed anyway (Y/N)? " << endl;
		//		int ch = _getch();
		//		if (ch != 'y' && ch != 'Y')
		//			break;
		//	}
		//}
		h = AVRun(nSimulationID);
		break;
	case IFC:
		h = AVIFC(nSimulationID, L"out.ifc");
		break;
	case DEL:
		h = AVDelete(nSimulationID);
		break;
	case DEL_ALL: 
		h = AVDeleteAll();
		break;
	case REINIT:
		AVWriteReinitBATFile();
		break;
	case TEST:
		h = AVTest(nSimulationID);
		switch (h)
		{
		case S_OK: wcout << L"Simulation data for id = " << nSimulationID << L": up to date." << endl; break;
		case S_FALSE: wcout << L"Simulation data for id = " << nSimulationID << L": not found." << endl; break;
		case S_FALSE+1: wcout << L"Simulation data for id = " << nSimulationID << L": outdated." << endl; break;
		}
		break;
	case DROP_TABLES: 
		h = AVDropTables();
		break;
	case INIT: 
		h = AVInit(nSimulationID, nProjectID);
		wcout << L"Project ID = " << nProjectID << endl;
		break;
	case PROCESS: 
		h = AVProcess(nSimulationID);
		break;
	case USER_TICKET:
		wchar_t pBufTicket[64];
		h = AVCreateTicket(pParam, pBufTicket);
		wcout << L"Ticket generated: " << pBufTicket << endl;
		break;
	case CONN:
		wchar_t buf[512];
		_snwprintf_s(buf, 512, L"-c-elevated \"%s\"", pParam);
		SHELLEXECUTEINFO shExecInfo;
		shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shExecInfo.fMask = NULL;
		shExecInfo.hwnd = NULL;
		shExecInfo.lpVerb = L"runas";
		shExecInfo.lpFile = argv[0];
		shExecInfo.lpParameters = buf;
		shExecInfo.lpDirectory = NULL;
		shExecInfo.nShow = SW_HIDE;
		shExecInfo.hInstApp = NULL;
		ShellExecuteEx(&shExecInfo);
		break;
	case CONN_ELEVATED:
		AVSetConnString(pParam);
		break;
	case WRONG:
		version();
	case WRONG_2:
		wcerr << L"Wrong parameters." << endl;
		usage();
		break;
	case HELP:
		version();
		usage();
		break;
	case VERSION:
		version();
		break;
	}
	if (bWait)
	{
		wcout << L"Press any key to finish..." << endl;
		_getch();
	}
	CoUninitialize();
	return 0;
}

