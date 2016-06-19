#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../advideo/video.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);

	CoInitialize(NULL);
	wcout << L"AdVisuo Video Generator (application)" << endl;

	// Registry-based data
	std::wstring strVideoRootDir;			// Video root dir

	HKEY hRegKey = NULL; 
	HRESULT h = RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", &hRegKey); if (h != 0) return -10;
	BYTE buf[1024];
	DWORD type, size = 1024;
	h = RegQueryValueEx(hRegKey, L"VideoRootDir", 0, &type, buf, &size); if (h != 0) return -10;
	strVideoRootDir = (LPCTSTR)buf;
	h = RegCloseKey(hRegKey); if (h != 0) return 2;

	if (!strVideoRootDir.empty() && strVideoRootDir[strVideoRootDir.length() - 1] != '\\')
		strVideoRootDir += '\\';


	for (int i = 0; i < 60; i++)
	{
		try
		{
			// read params
			int nVideoId = -1;
			std::wstringstream ss;
			ss << L"1_minute_" << i;
			wstring fname = strVideoRootDir + ss.str().c_str();
			ULONG idSimulation = 51;
			ULONG nLiftGroup = 0;
			ULONG nCamera = 0;
			ULONG nLift = 0;
			ULONG nFloor = 0;
			ULONG nSize = 1;
			ULONG nTimeFrom = i * 60 * 1000;
			ULONG nDuration = 60 * 1000;

			// process
			wcout << L"PROCESSING: " << fname.c_str() << L"  simuId=" << idSimulation << " from " << nTimeFrom / 1000 << " for " << nDuration/1000 << endl;

			AVVideo(nVideoId, idSimulation, nLiftGroup, nCamera, nLift, nFloor, nSize, nTimeFrom, nDuration, (fname + L".mp4").c_str());

			wcout << endl << "OK?" << endl;
		}
		catch(...)
		{
			wcout << "Something went wrong! (...)" << endl;
		}
	}

	for (int i = 0; i < 180; i++)
	{
		try
		{
			// read params
			int nVideoId = -1;
			std::wstringstream ss;
			ss << L"20_second_" << i;
			wstring fname = strVideoRootDir + ss.str().c_str();
			ULONG idSimulation = 51;
			ULONG nLiftGroup = 0;
			ULONG nCamera = 0;
			ULONG nLift = 0;
			ULONG nFloor = 0;
			ULONG nSize = 1;
			ULONG nTimeFrom = i * 20 * 1000;
			ULONG nDuration = 20 * 1000;

			// process
			wcout << L"PROCESSING: " << fname.c_str() << L"  simuId=" << idSimulation << " from " << nTimeFrom / 1000 << " for " << nDuration/1000 << endl;

			AVVideo(nVideoId, idSimulation, nLiftGroup, nCamera, nLift, nFloor, nSize, nTimeFrom, nDuration, (fname + L".mp4").c_str());

			wcout << endl << "OK?" << endl;
		}
		catch(...)
		{
			wcout << "Something went wrong! (...)" << endl;
		}
	}

	for (int i = 0; i < 20; i++)
	{
		try
		{
			// read params
			int nVideoId = -1;
			std::wstringstream ss;
			ss << L"10_minutes_" << i;
			wstring fname = strVideoRootDir + ss.str().c_str();
			ULONG idSimulation = 51;
			ULONG nLiftGroup = 0;
			ULONG nCamera = 0;
			ULONG nLift = 0;
			ULONG nFloor = 0;
			ULONG nSize = 1;
			ULONG nTimeFrom = i * 60 * 1000;
			ULONG nDuration = 10 * 60 * 1000;

			// process
			wcout << L"PROCESSING: " << fname.c_str() << L"  simuId=" << idSimulation << " from " << nTimeFrom / 1000 << " for " << nDuration/1000 << endl;

			AVVideo(nVideoId, idSimulation, nLiftGroup, nCamera, nLift, nFloor, nSize, nTimeFrom, nDuration, (fname + L".mp4").c_str());

			wcout << endl << "OK?" << endl;
		}
		catch(...)
		{
			wcout << "Something went wrong! (...)" << endl;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		try
		{
			// read params
			int nVideoId = -1;
			std::wstringstream ss;
			ss << L"1_hour_" << i;
			wstring fname = strVideoRootDir + ss.str().c_str();
			ULONG idSimulation = 51;
			ULONG nLiftGroup = 0;
			ULONG nCamera = 0;
			ULONG nLift = 0;
			ULONG nFloor = 0;
			ULONG nSize = 1;
			ULONG nTimeFrom = 0;
			ULONG nDuration = 60 * 60 * 1000;

			// process
			wcout << L"PROCESSING: " << fname.c_str() << L"  simuId=" << idSimulation << " from " << nTimeFrom / 1000 << " for " << nDuration/1000 << endl;

			AVVideo(nVideoId, idSimulation, nLiftGroup, nCamera, nLift, nFloor, nSize, nTimeFrom, nDuration, (fname + L".mp4").c_str());

			wcout << endl << "OK?" << endl;
		}
		catch(...)
		{
			wcout << "Something went wrong! (...)" << endl;
		}
	}

	CoUninitialize();
	return 0;
}

