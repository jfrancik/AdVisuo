// advideo.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include "video.h"
#include <windows.h>
#include <iostream>
#include "../CommonFiles/DBTools.h"
#include "../CommonFiles/DBConnStr.h"
#include "../AdVisuo/AdVisuoLoader.h"
#include "../AdVisuo/VisProject.h"
#include "../AdVisuo/VisSim.h"
#include "../AdVisuo/Camera.h"
#include "../AdVisuo/AdVisuoRenderer.h"
#include "Engine.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace dbtools;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::wstring _stdPathModels;

ADVIDEO_API ULONG AVVideo(ULONG idVideo, ULONG idSimulation, ULONG nLiftGroup, ULONG nCamera, ULONG nLift, ULONG nFloor, ULONG nSize, ULONG _nTimeFrom, ULONG _nDuration, LPCTSTR pFileName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int nIncrement = 0;	 // alive control

	try
	{
		// Registry-based data
		wstring strSimQueueConnection = GetConnString(CONN_SIMQUEUE);  // -- strictly to report progress!
		wstring strAdVisuoWebService;		// AdVisuo Web Service

		HKEY hRegKey = NULL; 
		HRESULT h = RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", &hRegKey); if (h != 0) return -10;
		BYTE buf[1024];
		DWORD type, size = 1024;
		h = RegQueryValueEx(hRegKey, L"AdVisuoWebService", 0, &type, buf, &size); if (h != 0) return -10;
		strAdVisuoWebService = (LPCTSTR)buf;
		h = RegCloseKey(hRegKey); if (h != 0) return -10;

		// open database
		CDataBase db(strSimQueueConnection.c_str());

		// initialise connection
		CXMLRequest Http;
		Http.create();
		Http.set_authorisation_data(L"jarekf", L"A4TosNS1iOjD4Mp1SsALDC+Q2i5=");
		Http.setURL(strAdVisuoWebService.c_str());

		// other preparation
		LONG nTimeFrom = (LONG)_nTimeFrom;
		LONG nTimeTo = nTimeFrom + (LONG)_nDuration;

		CProjectVis Prj;				// The Project
		CAdVisuoLoader Loader(&Prj);	// The Project Loader
		CEngine Engine;

		if (idVideo > 0) db.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 1, nIncrement++, idVideo);

		Engine.Create(::GetDesktopWindow(), NULL);
		Prj.SetEngine(&Engine);
		Loader.Start(strAdVisuoWebService.c_str(), &Http, idSimulation);

		if (idVideo > 0) db.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 2, nIncrement++, idVideo);

		if (!LoadStructure(Loader)) return -1;
		if (!Build(Engine, Prj)) return -2;
		CCamera *pCamera = BuildCamera(Engine, Prj, nLiftGroup, nCamera, nLift, nFloor);
		if (!pCamera) return -3;
		if (!LoadData(Loader, Engine, nTimeTo)) return -4;
		Loader.Stop();

		if (idVideo > 0) db.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 3, nIncrement++, idVideo);

		CAdVisuoRenderer renderer(&Engine, pCamera);

		// Rewind
		Engine.Stop();
		for (AVULONG i = 0; i < Prj.GetLiftGroupsCount(); i++)
		{
			Prj.GetLiftGroup(i)->GetCurSim()->Stop();
			Prj.GetLiftGroup(i)->RestoreConfig();
		}

		if (idVideo > 0) db.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 4, nIncrement++, idVideo);

		LONG t = 0;

		for (AVULONG i = 0; i < Prj.GetLiftGroupsCount(); i++)
			t = Prj.GetLiftGroup(i)->GetCurSim()->FastForward(&Engine, nTimeFrom);

		for ( ; t <= (AVLONG)nTimeFrom; t += 40)
			Engine.Proceed(t);

		t = nTimeFrom;

		Engine.ResetAccel();
		Engine.Play(t);

		Engine.StartTargetToVideo(GetStandardSize(nSize), pFileName, 25, "x264");

		Engine.Proceed(t);

		Engine.SetTargetOffScreen();
		Engine.BeginFrame();
		renderer.Render(&Prj);
		Engine.EndFrame();

		t += 1000 / 25;

		if (idVideo > 0) db.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 5, nIncrement++, idVideo);

		LONG iCnt = 0;
		for (t; t <= nTimeTo; t += Engine.GetAccel() * 1000 / 25)
		{
			// proceed the script - and provide for any pre-programmed fast forward
			Engine.SetPlayTime(t);
			Engine.Proceed(t);
			Engine.ProceedAux(t);

			//Engine.SetTargetOffScreen();
			Engine.BeginFrame();
			renderer.Render(&Prj);
			Engine.EndFrame();

			if (++iCnt % 25 == 0)
			{
				// progress reporting...
				wcout << L"#";
				int percent = 5 + 90 * (t - nTimeFrom) / (nTimeTo - nTimeFrom);
				if (idVideo > 0) db.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", percent, nIncrement++, idVideo);
			}

		}
		wcout << endl;

		//Engine.SetTargetToScreen();
		Engine.BeginFrame();
		renderer.Render(&Prj);
		Engine.EndFrame();
		for (int i = 0; i < 40; i++)
		{
			//Engine.SetTargetOffScreen();
			Engine.BeginFrame();
			renderer.Render(&Prj);
			Engine.EndFrame();
		}
		Engine.DoneTargetOffScreen();
	}
	catch (...)
	{
		return -6;
	}

	return 0;
}



bool LoadStructure(CAdVisuoLoader &Loader)
{
	CAdVisuoLoader::STATUS status = CAdVisuoLoader::NOT_STARTED;
	while ((status = Loader.GetStatus()) <= CAdVisuoLoader::LOADING_STRUCTURE)
		;
	if (status == CAdVisuoLoader::TERMINATED && Loader.GetReasonForTermination() == CAdVisuoLoader::FAILED)
	{
		wcout << L"Project loading failed!!!" << endl;
		return false;
	}
	wcout << L"Structure loaded OK" << endl;
	return true;
}

bool LoadData(CAdVisuoLoader &Loader, CEngine &Engine, LONG nTimeMax)
{
	wstring STATUS_NAMES[] = { L"NOT_STARTED", L"PREPROCESS", L"LOADING_STRUCTURE", L"LOADING_DATA", L"TERMINATED" };
	wstring REASON_NAMES[] = { L"NOT_TERMINATED", L"COMPLETE", L"STOPPED", L"FAILED" };
	CAdVisuoLoader::STATUS status = CAdVisuoLoader::NOT_STARTED;
	LONG t = 0;

	while ((status = Loader.GetStatus()) != CAdVisuoLoader::TERMINATED)
	{
		if (status !=  CAdVisuoLoader::LOADING_DATA)
			wcout << L"Status: " << STATUS_NAMES[status] << endl;

		if (t != Loader.GetTimeLoaded())
		{
			t = Loader.GetTimeLoaded();
			wcout << L"Time: " << t << endl;

			Loader.Update(&Engine);
		}

		if (t > nTimeMax)
			break;
	}
	wcout << L"Final Status: " << STATUS_NAMES[status] << endl;
	wcout << L"Reason for termination: " << REASON_NAMES[Loader.GetReasonForTermination()] << endl;
	return (status == CAdVisuoLoader::LOADING_DATA || status == CAdVisuoLoader::TERMINATED) && (Loader.GetReasonForTermination() != CAdVisuoLoader::FAILED && Loader.GetReasonForTermination() != CAdVisuoLoader::STOPPED);
}

bool Build(CEngine &Engine, CProjectVis &Prj)
{
	// initialise the simulation
	Engine.InitMats(Prj.GetMaxStoreyCount(), Prj.GetMaxBasementStoreyCount(), Prj.GetMaxShaftCount());
	Prj.Construct();
	Prj.StoreConfig();
	if (Prj.GetLiftGroupsCount() == 0)
		return false;
	wcout << L"Structure built OK." << endl;
	return true;
}

CCamera *BuildCamera(CEngine &Engine, CProjectVis &Prj, int nLiftGroup, int nCamera, int nLift, int nStorey)
{
	CCamera *pCamera = new CCamera();
	pCamera->Create(&Engine, &Prj, 0, 0, 0); 

#define ID_CAMERA_LEFTREAR              0
#define ID_CAMERA_RIGHTREAR             2
#define ID_CAMERA_RIGHTFRONT            4
#define ID_CAMERA_LEFTFRONT             6

	switch (nCamera)
	{
	case 0: 
		pCamera->MoveTo(CAMLOC_LIFTGROUP, nLiftGroup); pCamera->MoveTo(CAMLOC_STOREY, nStorey); pCamera->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT); 
		pCamera->Pan(0.12f); pCamera->Move(-15, 10, 0);
		break;
	case 1: 
		pCamera->MoveTo(CAMLOC_LIFTGROUP, nLiftGroup); pCamera->MoveTo(CAMLOC_STOREY, nStorey); pCamera->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTREAR); 
		pCamera->Pan(0.12f); pCamera->Move(-15, 10, 0);
		break;
	case 2: 
		pCamera->MoveTo(CAMLOC_LIFTGROUP, nLiftGroup); pCamera->MoveTo(CAMLOC_STOREY, nStorey); pCamera->MoveTo(CAMLOC_LOBBY, ID_CAMERA_RIGHTFRONT);
		pCamera->Pan(-0.12f); pCamera->Move(15, 10, 0);
		break;
	case 3: 
		pCamera->MoveTo(CAMLOC_LIFTGROUP, nLiftGroup); pCamera->MoveTo(CAMLOC_STOREY, nStorey); pCamera->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTREAR); 
		pCamera->Pan(-0.12f); pCamera->Move(15, 10, 0);
		break;
	case 4: 
		pCamera->MoveTo(CAMLOC_LIFTGROUP, nLiftGroup); pCamera->MoveTo(CAMLOC_STOREY, nStorey); pCamera->MoveTo(CAMLOC_OUTSIDE, 0); 
		break;
	case 5: 
		pCamera->MoveTo(CAMLOC_LIFTGROUP, nLiftGroup); pCamera->MoveTo(CAMLOC_STOREY, nStorey); pCamera->MoveTo(CAMLOC_OUTSIDE, 1); 
		break;
	case 6: 
		pCamera->MoveTo(CAMLOC_LIFTGROUP, nLiftGroup); pCamera->MoveTo(CAMLOC_STOREY, 0); pCamera->MoveTo(CAMLOC_LIFT, nLift); 
		
		break;
	case 7: 
		pCamera->MoveTo(CAMLOC_LIFTGROUP, nLiftGroup); pCamera->MoveTo(CAMLOC_STOREY, nStorey); pCamera->MoveTo(CAMLOC_LOBBY, ID_CAMERA_LEFTFRONT); 
		break;
	}
	

	wcout << L"Cameras initialised, ready for rendering." << endl;

	return pCamera;
}


CSize GetStandardSize(int nSize)
{
	if (nSize < 0 || nSize >= 8) nSize = 1;
	int STD_SIZES[][2] =
	{
		{ 854, 480},
		{ 1280, 720 },
		{ 1600, 900 },
		{ 1920, 1080 },
		{ 320, 240 },
		{ 640, 480 },
		{ 800, 600 },
		{ 1024, 768 },
	};
	return CSize(STD_SIZES[nSize][0], STD_SIZES[nSize][1]);
}
