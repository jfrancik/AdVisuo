// advideo.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include "../CommonFiles/DBTools.h"
#include "../CommonFiles/DBConnStr.h"
#include "../AdVisuo/VisProject.h"
#include "../AdVisuo/VisSim.h"
#include "../AdVisuo/Camera.h"
#include "../AdVisuo/AdVisuoRenderer.h"
#include "Video.h"
#include "Engine.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace dbtools;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::wstring _stdPathModels;

ADVIDEO_API LONG AVVideo(ULONG idVideo, ULONG idSimulation, ULONG nLiftGroup, ULONG nCamera, ULONG nLift, ULONG nFloor, ULONG nSize, ULONG _nTimeFrom, ULONG _nDuration, LPCTSTR pFileName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int nIncrement = 0;	 // alive control

	try
	{
		// Registry-based data
		wstring strSimQueueConnection = GetConnString(CONN_SIMQUEUE);			// visualisation database connection string
		wstring strVisualisationConnection = GetConnString(CONN_VISUALISATION);	// -- strictly to report progress!

		// open databases
		CDataBase dbProgress(strSimQueueConnection.c_str());
		CDataBase db(strVisualisationConnection.c_str());

		// other preparation
		LONG nTimeFrom = (LONG)_nTimeFrom;
		LONG nTimeTo = nTimeFrom + (LONG)_nDuration;

		CProjectVis Prj;				// The Project
		CEngine Engine;					// The Engine

		// Load the Project
		std::wstring diagStr;
		if (!Load(db, &Prj, idSimulation, diagStr))
		{
			wcout << diagStr << endl;
			return -20;
		}

		if (idVideo > 0) dbProgress.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 1, nIncrement++, idVideo);

		// Create the Project and the Engine
		Engine.Create(::GetDesktopWindow(), NULL);
		Prj.SetEngine(&Engine);

		if (idVideo > 0) dbProgress.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 2, nIncrement++, idVideo);

		// Build and Update
		if (!Build(Engine, Prj)) return -2;
		CCamera *pCamera = BuildCamera(Engine, Prj, nLiftGroup, nCamera, nLift, nFloor);
		if (!pCamera) return -3;
		
		if (idVideo > 0) dbProgress.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 3, nIncrement++, idVideo);

		// Initialise rendering
		CAdVisuoRenderer renderer(&Engine, pCamera);
		Engine.StartTargetToVideo(GetStandardSize(nSize), pFileName, 25, GetStandardBitrate(nSize, 25));
		Engine.SetTargetOffScreen();

		// Rewind (required as initialisation ???)
		Engine.Stop();
		for (AVULONG i = 0; i < Prj.GetLiftGroupsCount(); i++)
		{
			Prj.GetLiftGroup(i)->GetCurSim()->Stop();
			Prj.GetLiftGroup(i)->RestoreConfig();
		}

		if (idVideo > 0) dbProgress.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 4, nIncrement++, idVideo);

		// Fast Forward to the starting position
		LONG t = 0;
		for (AVULONG i = 0; i < Prj.GetLiftGroupsCount(); i++)
			t = Prj.GetLiftGroup(i)->GetCurSim()->FastForward(&Engine, nTimeFrom);

		for ( ; t < (AVLONG)nTimeFrom; t += 40)
			Engine.Proceed(t);

		Engine.ResetAccel();
		Engine.Play(t);

		DWORD nmsec = ::GetTickCount();
		if (idVideo > 0) dbProgress.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", 5, nIncrement++, idVideo);

		LONG iCnt = 0;
		for (t; t <= nTimeTo; t += Engine.GetAccel() * 1000 / 25)
		{
			// proceed the script - and provide for any pre-programmed fast forward
			Engine.SetPlayTime(t);
			Engine.Proceed(t);
			Engine.ProceedAux(t);

			Engine.BeginFrame();
			renderer.Render(&Prj);
			Engine.EndFrame();

			if (++iCnt % 25 == 0)
			{
				// progress reporting...
				wcout << L"#";
				int percent = 5 + 90 * (t - nTimeFrom) / (nTimeTo - nTimeFrom);
				if (idVideo > 0) dbProgress.execute(L"UPDATE Queue SET Progress = %d, Increment = %d WHERE VideoId = %d", percent, nIncrement++, idVideo);
			}

		}
		Engine.DoneTargetOffScreen();
		delete pCamera;

		wcout << endl;
		wcout << "Execution time: " << ::GetTickCount() - nmsec << endl;
	}
	catch (...)
	{
		return -6;
	}

	return 0;
}

class _prj_deleted { };

bool Load(dbtools::CDataBase db, CProjectVis *pProject, AVULONG nProjectId, std::wstring &diagStr)
{
	diagStr = L"OK";

	std::wstring response;
	try
	{
		dbtools::CDataBase::SELECT sel;

		// load project
		sel = db.select(L"SELECT * FROM AVProjects WHERE SimulationId=%d", nProjectId);
		if (!sel) throw _prj_deleted();
		pProject->LoadProject(sel);

		// load lift groups
		sel = db.select(L"SELECT * FROM AVLiftGroups WHERE ProjectID=%d ORDER BY ID", pProject->GetId());
		while (sel)
		{
			pProject->LoadLiftGroup(sel);
			sel++;
		}

		// load floors, shafts and sims for each lift group
		for each (CLiftGroupVis *pGroup in pProject->GetLiftGroups())
		{
			sel = db.select(L"SELECT * FROM AVFloors WHERE LiftGroupId=%d ORDER BY ID", pGroup->GetId());
			while (sel)
			{
				pProject->LoadFloor(sel);
				sel++;
			}

			sel = db.select(L"SELECT * FROM AVShafts WHERE LiftGroupId=%d ORDER BY ID", pGroup->GetId());
			while (sel)
			{
				pProject->LoadShaft(sel);
				sel++;
			}

			sel = db.select(L"SELECT * FROM AVSims WHERE LiftGroupId=%d ORDER BY ID", pGroup->GetId());
			while (sel)
			{
				pProject->LoadSim(sel);
				sel++;
			}
		}

		sel = db.select(L"SELECT * FROM AVJourneys WHERE SimID IN (SELECT s.ID FROM AVSims s, AVLiftGroups g WHERE s.LiftGroupId = g.ID AND g.ProjectId = %d) AND TimeGo < %d ORDER BY ID", pProject->GetId(), pProject->GetMaxTime());
		while (sel)
		{
			pProject->LoadJourney(sel);
			sel++;
		}
		sel = db.select(L"SELECT * FROM AVPassengers WHERE SimID IN (SELECT s.ID FROM AVSims s, AVLiftGroups g WHERE s.LiftGroupId = g.ID AND g.ProjectId = %d) AND TimeBorn < %d ORDER BY ID", pProject->GetId(), pProject->GetMaxTime());
		while (sel)
		{
			pProject->LoadPassenger(sel);
			sel++;
		}

		// Completed!
		return true;
	}
	catch (_prj_deleted)
	{
		diagStr = L"Project cannot be accessed and could possibly have been deleted";
	}
	catch (_prj_error pe)
	{
		diagStr = L"AdVisuo internal data error: " + pe.ErrorMessage();
	}
	catch (_com_error ce)
	{
		if ((wchar_t*)ce.Description())
			diagStr = L"Database connection or structure error: " + ce.Description();
		else
			diagStr = L"Database connection or structure error";
	}
	catch (dbtools::_value_error ve)
	{
		diagStr = L"Internal data format error: " + ve.ErrorMessage();
	}
	catch(...)
	{
		diagStr = L"Unidentified error";
	}
	return false;
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

// https://support.google.com/youtube/answer/1722171?hl=en-GB
AVULONG GetStandardBitrate(int nSize, int nFramerate)
{
	if (nSize < 0 || nSize >= 8) nSize = 1;
	int STD_BITRATES[][2] =
	{
		{ 2500000, 4000000 },
		{ 5000000, 7500000 },
		{ 8000000, 1200000 },
		{ 8000000, 1200000 },
		{ 1000000, 1500000 },
		{ 2500000, 4000000 },
		{ 5000000, 7500000 },
		{ 5000000, 7500000 },
	};
	if (nFramerate < 40)
		return STD_BITRATES[nSize][0];
	else
		return STD_BITRATES[nSize][1];
}