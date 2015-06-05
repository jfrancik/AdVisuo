// advidgen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include "../advideo/video.h"
#include "../CommonFiles/DBTools.h"
#include "../CommonFiles/DBConnStr.h"

using namespace std;
using namespace dbtools;

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);

	wcout << L"AdVisuo Video Generator (application)" << endl;

	// Registry-based data
	wstring strConsoleConnection = GetConnString(CONN_CONSOLE);
	std::wstring strVideoRootDir;			// Video root dir

	HKEY hRegKey = NULL; 
	HRESULT h = RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", &hRegKey); if (h != 0) return -10;
	BYTE buf[1024];
	DWORD type, size = 1024;
	h = RegQueryValueEx(hRegKey, L"VideoRootDir", 0, &type, buf, &size); if (h != 0) return -10;
	strVideoRootDir = (LPCTSTR)buf;
	h = RegCloseKey(hRegKey); if (h != 0) return 2;

	if (strConsoleConnection.empty())
	{
		wcout << "Cannot access connection string, exiting" << endl;
		return 1;
	}
	if (!strVideoRootDir.empty() && strVideoRootDir[strVideoRootDir.length() - 1] != '\\')
		strVideoRootDir += '\\';


	try
	{
		CDataBase db(strConsoleConnection.c_str());
		CDataBase::SELECT sel;
		sel = db.select(L"SELECT * FROM Videos WHERE status=2");
		if (!sel)
		{
			wcout << "No task found, exiting" << endl;
			return 1;
		}

		// read params
		int nVideoId = sel[L"VideoId"];
		wstring fname = strVideoRootDir + (wstring)sel[L"Filename"];
		ULONG idSimulation = sel[L"SimulationId"];
		ULONG nLiftGroup = sel[L"LiftGroup"];
		ULONG nCamera = sel[L"Camera"];
		ULONG nLift = sel[L"LiftFloor"];
		ULONG nFloor = sel[L"LiftFloor"];
		ULONG nSize = sel[L"Size"];
		ULONG nTimeFrom = sel[L"Time"];
		ULONG nDuration = sel[L"Duration"];


		// process
		wcout << L"PROCESSING: " << fname.c_str() << L"  simulationId=" << idSimulation << " from " << nTimeFrom / 1000 << " for " << nDuration/1000 << endl;

		AVVideo(nVideoId, idSimulation, nLiftGroup, nCamera, nLift, nFloor, nSize, nTimeFrom, nDuration, (fname + L".avi").c_str());

		// external tools
		wcout << "Running external tools...";
		wstring params = L"/C " + strVideoRootDir + L"fft.bat " + fname;
		HINSTANCE hReturnCode=ShellExecute(NULL, L"open", L"cmd.exe", params.c_str(), NULL, SW_SHOWNORMAL);
		DWORD LastError = GetLastError();

		// wait for the tools to delete the avi file
		while (1) 
		{
			ifstream f(fname + L".avi");
			if (f.good())
			{
				f.close();
				Sleep(1000);
				wcout << ".";
			}
			else
			{
				f.close();
				break;
			}
		}
		db.execute(L"UPDATE Videos SET status = 3 WHERE VideoId = %d", nVideoId);
		wcout << endl << "OK." << endl;
	}
	catch (dbtools::CValue val)
	{
		wcout << "Something went wrong! (DB)" << endl;
	}
	catch(...)
	{
		wcout << "Something went wrong! (...)" << endl;
	}

	CoUninitialize();

	return 0;
}

