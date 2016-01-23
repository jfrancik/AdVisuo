#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include "../advideo/video.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
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


	try
	{
		// read params
		int nVideoId = -1;
		wstring fname = strVideoRootDir + L"video_test";
		ULONG idSimulation = 51;
		ULONG nLiftGroup = 0;
		ULONG nCamera = 0;
		ULONG nLift = 0;
		ULONG nFloor = 0;
		ULONG nSize = 0;
		ULONG nTimeFrom = 0;
		ULONG nDuration = 2000;


		// process
		wcout << L"PROCESSING: " << fname.c_str() << L"  simulationId=" << idSimulation << " from " << nTimeFrom / 1000 << " for " << nDuration/1000 << endl;

		AVVideo(nVideoId, idSimulation, nLiftGroup, nCamera, nLift, nFloor, nSize, nTimeFrom, nDuration, (fname + L".avi").c_str());

		// external tools
/*		wcout << "Running external tools...";
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
		}*/
		wcout << endl << "OK." << endl;
	}
	catch(...)
	{
		wcout << "Something went wrong! (...)" << endl;
	}

	CoUninitialize();

	return 0;
}

