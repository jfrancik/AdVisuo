#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "../advideo/video.h"
#include "../CommonFiles/DBTools.h"
#include "../CommonFiles/DBConnStr.h"

using namespace std;
using namespace dbtools;

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);

	wcout << L"AdVisuo Video Service (application)" << endl;

	AVSTRING pConsoleConn = GetConnString(CONN_CONSOLE);
	if (pConsoleConn)
		wcout << "Connection string: " << pConsoleConn << endl;
	else
	{
		wcout << "Cannot access connection string, exiting" << endl;
		return 1;
	}

	while (1)
	{
		wcout << endl << L"Waiting for tasks...";
		try
		{
			CDataBase db(pConsoleConn);
			CDataBase::SELECT sel;
			do 
			{
				sel = db.select(L"SELECT * FROM Videos WHERE status=0");
				Sleep(1000);
				wcout << L".";
			} while (!sel);
			wcout << endl;

			if (sel)
			{
				// read params
				int nVideoId = sel[L"VideoId"];
				wstring fname = sel[L"Filename"];
				ULONG idSimulation = sel[L"SimulationId"];
				ULONG nLiftGroup = sel[L"LiftGroup"];
				ULONG nCamera = sel[L"Camera"];
				ULONG nLift = sel[L"LiftFloor"];
				ULONG nFloor = sel[L"LiftFloor"];
				ULONG nSize = sel[L"Size"];
				ULONG nTimeFrom = sel[L"Time"];
				ULONG nDuration = sel[L"Duration"]; 

				// process
				db.execute(L"UPDATE Videos SET status = %d WHERE VideoId = %d", 1, nVideoId);
				wcout << L"PROCESSING: " << fname.c_str() << L"  simulationId=" << idSimulation << " from " << nTimeFrom / 1000 << " for " << nDuration/1000 << endl;

				AVVideo(nVideoId, idSimulation, nLiftGroup, nCamera, nLift, nFloor, nSize, nTimeFrom, nDuration, fname.c_str());

				// external tools
				wcout << "Running external tools...";
				wstring params(L"/C fft.bat ");
				params += fname;
				HINSTANCE hReturnCode=ShellExecute(NULL, L"open", L"cmd.exe", params.c_str(), NULL, SW_SHOWNORMAL);
				DWORD LastError = GetLastError();

				// wait for the tools to delete the avi file
				fname += L".avi";
				while (1) 
				{
					ifstream f(fname);
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
				db.execute(L"UPDATE Videos SET status = %d WHERE VideoId = %d", 2, nVideoId);
				wcout << endl << "OK." << endl;
			}
		}
		catch (dbtools::CValue val)
		{
			wcout << "Something went wrong! (DB)" << endl;
		}
		catch(...)
		{
			wcout << "Something went wrong! (...)" << endl;
		}
	}




	

	CoUninitialize();

	return 0;
}

