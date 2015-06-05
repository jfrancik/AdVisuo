// advidmon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include "../CommonFiles/DBTools.h"
#include "../CommonFiles/DBConnStr.h"

using namespace std;
using namespace dbtools;

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);

	wcout << L"AdVisuo Video Monitor (application)" << endl;

	// Registry-based data
	wstring strConsoleConnection = GetConnString(CONN_CONSOLE);
	if (strConsoleConnection.empty())
	{
		wcout << "Cannot access connection string, exiting" << endl;
		return 1;
	}

	// locate advidgen
	wchar_t advidgen_path[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	_wsplitpath_s(argv[0], drive, dir, fname, ext);
	_wmakepath_s(advidgen_path, drive, dir, L"advidgen", L"exe");

	// Monitoring loop...
	while (1)
	{
		wcout << endl << L"Waiting for tasks...";
		try
		{
			CDataBase db(strConsoleConnection.c_str());
			CDataBase::SELECT sel;
			do 
			{
				Sleep(1000);
				sel = db.select(L"SELECT * FROM Videos WHERE status=1");	// status=1 = task submitted by SimQ for further processing
				wcout << L".";
			} while (!sel);
			wcout << endl;

			if (sel)
			{
				// change status to 2 (sent to advidgen)
				int nVideoId = sel[L"VideoId"];
				db.execute(L"UPDATE Videos SET status=2 WHERE VideoId = %d", nVideoId);

				wcout << endl;
				wcout << "Calling advidgen.exe...";
				HINSTANCE hReturnCode = ShellExecute(NULL, L"open", advidgen_path, L"", NULL, SW_SHOWNORMAL);
			}
		}
		catch (dbtools::CValue val)
		{
			wcout << "Oops! Something went wrong! (DB)" << endl;
		}
		catch(...)
		{
			wcout << "Oops! Something went wrong! (...)" << endl;
		}
	}

	return 0;
}