// AdVisuoDoc.cpp - a part of the AdVisuo Client Software

// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// AdVisuoDoc.cpp : implementation of the CAdVisuoDoc class
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "AdVisuoDoc.h"
#include "MainFrm.h"

#include "freewill.c"			// #FreeWill: Obligatory!
#include "freewilltools.h"		// #FreeWill: Some usefull tools

#include "Block.h"

#include <iostream>
#include <fstream>
#import "msxml.dll"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Conversion Macros
#define DEG2RAD(d)	( (d) * (FWFLOAT)3.14159265358979323846 / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (FWFLOAT)3.14159265358979323846 )

// CAdVisuoDoc

IMPLEMENT_DYNCREATE(CAdVisuoDoc, CDocument)

BEGIN_MESSAGE_MAP(CAdVisuoDoc, CDocument)
//	ON_COMMAND(ID_ACTION_PLAY, &CAdVisuoDoc::OnActionPlay)
	ON_UPDATE_COMMAND_UI(ID_ACTION_PLAY, &CAdVisuoDoc::OnUpdateActionPlay)
	ON_COMMAND(ID_ACTION_PAUSE, &CAdVisuoDoc::OnActionPause)
	ON_UPDATE_COMMAND_UI(ID_ACTION_PAUSE, &CAdVisuoDoc::OnUpdateActionPause)
	ON_COMMAND(ID_ACTION_STOP, &CAdVisuoDoc::OnActionStop)
	ON_UPDATE_COMMAND_UI(ID_ACTION_STOP, &CAdVisuoDoc::OnUpdateActionStop)
	ON_COMMAND(ID_ACTION_SLOWDOWN, &CAdVisuoDoc::OnActionSlowdown)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SLOWDOWN, &CAdVisuoDoc::OnUpdateActionSlowdown)
	ON_COMMAND(ID_ACTION_SPEEDUP, &CAdVisuoDoc::OnActionSpeedup)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SPEEDUP, &CAdVisuoDoc::OnUpdateActionSpeedup)
	ON_COMMAND(ID_ACTION_NORMALPACE, &CAdVisuoDoc::OnActionNormalpace)
	ON_UPDATE_COMMAND_UI(ID_ACTION_NORMALPACE, &CAdVisuoDoc::OnUpdateActionNormalpace)
	
//	ON_COMMAND(ID_FILE_OPENSIM, &CAdVisuoDoc::OnFileOpensim)
//	ON_COMMAND(ID_FILE_SAVEIFC, &CAdVisuoDoc::OnFileSaveifc)
	ON_UPDATE_COMMAND_UI(ID_STATUSBAR_PANE2, &CAdVisuoDoc::OnUpdateStatusbarPane2)
ON_UPDATE_COMMAND_UI(ID_ACTION_RENDER, &CAdVisuoDoc::OnUpdateActionRender)
END_MESSAGE_MAP()


// CAdVisuoDoc construction/destruction

DWORD CAdVisuoDoc::c_fpsNUM = 21;

CAdVisuoDoc::CAdVisuoDoc()
{
	m_h = S_FALSE;

	m_pFWDevice = NULL;
	m_pRenderer = NULL;
	m_pScene = NULL;
	m_pBody = NULL;
	m_pActionTick = NULL;
	m_pLight1 = NULL;
	m_pLight2 = NULL;

	m_pObjPlatePlay = NULL;
	m_pObjPlatePlayDark = NULL;
	m_pObjPlateText = NULL;

	m_pfps = new DWORD[c_fpsNUM];
	memset(m_pfps, 0, sizeof(DWORD) * c_fpsNUM);
	m_nfps = 0;
}

CAdVisuoDoc::~CAdVisuoDoc()
{
	if (m_pFWDevice) m_pFWDevice->Release();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pScene) m_pScene->Release();
	if (m_pBody) m_pBody->Release();
	if (m_pActionTick) m_pActionTick->Release();
	if (m_pLight1) m_pLight1->Release();
	if (m_pLight2) m_pLight2->Release();

	if (m_pObjPlatePlay) m_pObjPlatePlay->Release();
	if (m_pObjPlatePlayDark) m_pObjPlatePlayDark->Release();
	if (m_pObjPlateText) m_pObjPlateText->Release();

	delete [] m_pfps;
}

	#define MB_CANCELTRYCONTINUE        0x00000006L
	#define IDTRYAGAIN      10
	#define IDCONTINUE      11
	HRESULT __stdcall HandleErrors(struct FWERROR *p, BOOL bRaised)
	{
		if (!bRaised)
		{
			TRACE("Last error recovered\n");
			return S_OK;
		}

		FWSTRING pLabel = NULL;
		if (p->pSender)
		{
			IKineChild *pChild;
			if (SUCCEEDED(p->pSender->QueryInterface(&pChild)) && pChild)
			{
				pChild->GetLabel(&pLabel);
				pChild->Release();
			}
		}

		CString str;
		if (pLabel)
			str.Format(L"%ls(%d): Error 0x%x (class %ls, object %ls), %ls\n", p->pSrcFile, p->nSrcLine, p->nCode & 0xffff, p->pClassName, pLabel, p->pMessage);
		else
			str.Format(L"%ls(%d): Error 0x%x (class %ls), %ls\n", p->pSrcFile, p->nSrcLine, p->nCode & 0xffff, p->pClassName, p->pMessage);
		switch (AfxMessageBox(str, MB_CANCELTRYCONTINUE | MB_DEFBUTTON3))
		{
		case IDCANCEL: FatalAppExit(0, L"Application stopped"); break;
		case IDTRYAGAIN: DebugBreak(); break;
		case IDCONTINUE: break;
		}
		return p->nCode;
	}

bool CAdVisuoDoc::CreateFreeWill(HWND hWnd)
{
	if (IsEngineReady())
		return true;

	// #FreeWill: create the FreeWill device
	HRESULT h;
	h = CoCreateInstance(CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)&m_pFWDevice);
	if (FAILED(h)) return false;
	Debug(L"FreeWill+ system initialised successfully.");

	// #FreeWill: set-up the error handler
	m_pFWDevice->SetUserErrorHandler(HandleErrors);

	// #FreeWill: create & initialise the renderer
	h = m_pFWDevice->CreateObject(L"Renderer", IID_IRenderer, (IFWUnknown**)&m_pRenderer);
	if (FAILED(h)) return false;
	Debug(L"Renderer started successfully.");
	h = m_pRenderer->InitDisplay(hWnd, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	if (FAILED(h)) return false;
//	FWCOLOR back = { 0.6f, 0, 0.7f };
//	FWCOLOR back = { 0.82f, 0.88f, 0.94f };
	FWCOLOR back = { 0.0f, 0.0f, 0.0f };
	m_pRenderer->PutBackColor(back);

	// #FreeWill: create & initialise the buffers - determine hardware factors
	IMeshVertexBuffer *pVertexBuffer;
	IMeshFaceBuffer *pFaceBuffer;
	h = m_pRenderer->GetBuffers(&pVertexBuffer, &pFaceBuffer); if (FAILED(h)) return false;
	h = pVertexBuffer->Create(300000, MESH_VERTEX_XYZ | MESH_VERTEX_NORMAL | MESH_VERTEX_BONEWEIGHT | MESH_VERTEX_TEXTURE, 4, 1);
	if (FAILED(h)) return false;
	h = pFaceBuffer->Create(300000); if (FAILED(h)) return false;
	pVertexBuffer->Release();
	pFaceBuffer->Release();

	// #FreeWill: create & initialise the animation scene
	h = m_pFWDevice->CreateObject(L"Scene", IID_IScene, (IFWUnknown**)&m_pScene);
	if (FAILED(h)) return false;

	// #FreeWill: create & initialise the character body
	h = m_pFWDevice->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&m_pBody);
	if (FAILED(h)) return false;

	// #FreeWill: initialise the Tick Action
	m_pActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);
	m_pActionTick->SetEnvelopeEx(ACTION_ENV_NONE, 0.4f, 0.4f, 0.4f, 0.4f);

	m_pScene->PutRenderer(m_pRenderer);

	// #Load the Scene
	IFileLoader *pLoader;
	m_pFWDevice->CreateObject(L"FileLoader", IID_IFileLoader, (IFWUnknown**)&pLoader);
	ISceneObject *pBip01 = NULL;
	m_pScene->NewObject(L"Bip01", &pBip01);
	pLoader->LoadObject((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"lobby.3D"), L"Bip01", pBip01);


	IKineChild *pFootsteps = NULL;
	pBip01->GetChild(L"Bip01.Footsteps", &pFootsteps);
	if (pFootsteps)
	{
		ITransform *pT = NULL;
		pFootsteps->GetLocalTransformRef(&pT);
		pT->MulScale(0, 0, 0);
		pT->Release();
		pFootsteps->Release();
	}


	pBip01->Release();
	pLoader->Release();
	Debug(L"Biped model loaded.");

	// Reset Character Position
	IKineNode *pBody = NULL;
	if (SUCCEEDED(m_pScene->GetChild(L"Bip01.Bip01", (IKineChild**)&pBody)) && pBody)
	{
		ITransform *pT = NULL;
		pBody->GetBaseTransformRef(&pT);
		pT->Reset(FALSE, TRUE);					// reset translation stored in file
		pT->MulTranslationXYZ(0, 160, 37.15f);	// stand on the floor surface
		pT->Release();
		pBody->Invalidate();
		pBody->Release();
	}

	// Notify the simulation engine about the scene (fully loaded now)
	m_sim.SetScene(m_pScene);

	// Load Body Object
	pBody = NULL;
	if (SUCCEEDED(m_pScene->GetChild(L"Bip01", (IKineChild**)&pBody)) && pBody)
	{
		m_pBody->LoadBody(pBody, BODY_SCHEMA_DISCREET);
		pBody->Release();
	}

	// setup lights
	m_pFWDevice->CreateObject(L"DirLight", IID_ISceneLightDir, (IFWUnknown**)&m_pLight1);
	m_pScene->AddChild(L"DirLight1", m_pLight1);
	FWCOLOR cWhite1 = { 0.7f, 0.7f, 0.7f };
	m_pLight1->PutDiffuseColor(cWhite1);
	m_pLight1->Create(__FW_Vector(0.1f, -0.3f, -0.4f));

	m_pFWDevice->CreateObject(L"DirLight", IID_ISceneLightDir, (IFWUnknown**)&m_pLight2);
	m_pScene->AddChild(L"DirLight2", m_pLight2);
	FWCOLOR cWhite2 = { 0.6f, 0.6f, 0.6f };
	m_pLight2->PutDiffuseColor(cWhite2);
	m_pLight2->Create(__FW_Vector(0, 1, 3));

	FWCOLOR cAmb = { 0.35f, 0.35f, 0.35f };
	m_pRenderer->SetAmbientLight(cAmb);

	return true;
}

bool CAdVisuoDoc::CreatePlates()
{
	IKineNode *pBone;
	CBlock block;
	IMaterial *pMaterial = NULL;
	ITexture *pTexture = NULL;
	AVVECTOR vec = { 0, 0, 0 };
	FWFLOAT fPlaySize = 0.4f;
	FWFLOAT fPlateW = 10.0f, fPlateH = 0.07f;
	FWCOLOR colPlate = { 1, 1, 1, 0.5f };

	// Play Plate
	m_pScene->NewObject(L"PlatePlay", &m_pObjPlatePlay);
	m_pObjPlatePlay->PutVisible(FALSE);
	m_pObjPlatePlay->CreateChild(L"myplate", &pBone);

	block.Open(m_pObjPlatePlay, pBone, L"myplate", 150, 10, 10, vec, DEG2RAD(45));
	block.BuildPlane(CBlock::LT, -fPlaySize/2, 1, -fPlaySize/2, 1, fPlaySize, 0, 0, 1, 0, 0, fPlaySize);
	m_pFWDevice->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	m_pRenderer->CreateTexture(&pTexture);
	pTexture->LoadFromFile((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"play.png"));
	pTexture->SetUVTile(256, 256);
	pMaterial->SetTexture(0, pTexture);
	pTexture->Release();
	pMaterial->SetSelfIlluminationOff(); pMaterial->SetTwoSided(FALSE);
	pMaterial->SetAlphaMode(MAT_ALPHA_TEXTURE);
	block.SetMaterial(pMaterial);
	pMaterial->Release();
	block.Close();
	pBone->Release();

	// Play Plate - Dark
	m_pScene->NewObject(L"PlatePlayDark", &m_pObjPlatePlayDark);
	m_pObjPlatePlayDark->PutVisible(FALSE);
	m_pObjPlatePlayDark->CreateChild(L"myplate", &pBone);

	block.Open(m_pObjPlatePlayDark, pBone, L"myplate", 150, 10, 10, vec, DEG2RAD(45));
	block.BuildPlane(CBlock::LT, -fPlaySize/2, 1, -fPlaySize/2, 1, fPlaySize, 0, 0, 1, 0, 0, fPlaySize);
	m_pFWDevice->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	FWCOLOR colDark = { 0.75f, 0.75f, 0.75f, 1.0f };
	pMaterial->SetDiffuseColor(colDark); pMaterial->SetAmbientColor(colDark); pMaterial->SetSpecularColor(colDark);
	m_pRenderer->CreateTexture(&pTexture);
	pTexture->LoadFromFile((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"play.png"));
	pTexture->SetUVTile(256, 256);
	pMaterial->SetTexture(0, pTexture);
	pTexture->Release();
	pMaterial->SetSelfIlluminationOff(); pMaterial->SetTwoSided(FALSE);
	pMaterial->SetAlphaMode(MAT_ALPHA_TEXTURE);
	block.SetMaterial(pMaterial);
	pMaterial->Release();
	block.Close();
	pBone->Release();

	// Text Plate
	m_pScene->NewObject(L"PlateDesc", &m_pObjPlateText);
	m_pObjPlateText->PutVisible(FALSE);
	m_pObjPlateText->CreateChild(L"myplate", &pBone);

	block.Open(m_pObjPlateText, pBone, L"myplate", 150, 10, 10, vec, DEG2RAD(45));
	block.BuildPlane(CBlock::LT, -fPlateW/2, 1, -fPlateH/2, 1, fPlateW, 0, 0, 1, 0, 0, fPlateH);
	m_pFWDevice->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	pMaterial->SetDiffuseColor(colPlate); pMaterial->SetAmbientColor(colPlate); pMaterial->SetSpecularColor(colPlate);
	pMaterial->SetSelfIlluminationOff(); pMaterial->SetTwoSided(FALSE);
	pMaterial->SetAlphaMode(MAT_ALPHA_MATERIAL);
	block.SetMaterial(pMaterial);
	pMaterial->Release();
	block.Close();
	pBone->Release();

	// Font
	m_pRenderer->SetFont(0, 16, 6, true, false, L"Arial");

	return true;
}

bool CAdVisuoDoc::CreateBuilding(ULONG nOptions)
{
	// set-up materials
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FRONT, 0.0f, 1.0f, 0.0f, 0.5f);	// transparency set
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_REAR, 0.0f, 1.0f, 0.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FRONT+1, 1.0f, 0.0f, 0.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_REAR+1,  1.0f, 0.0f, 0.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FRONT+2, 1.0f, 1.0f, 0.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_REAR+2,  1.0f, 1.0f, 0.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FRONT+3, 0.0f, 1.0f, 1.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_REAR+3,  0.0f, 1.0f, 1.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FRONT+4, 1.0f, 0.7f, 0.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_REAR+4,  1.0f, 0.7f, 0.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FRONT+5, 0.8f, 0.0f, 1.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_REAR+5,  0.8f, 0.0f, 1.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FRONT+6, 1.0f, 1.0f, 0.6f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_REAR+6,  1.0f, 1.0f, 0.6f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FRONT+7, 0.0f, 0.3f, 1.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_REAR+7,  0.0f, 0.3f, 1.0f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_SIDE, _stdPathModels + L"yellobrk.jpg", 1.0f, 1.0f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_SHAFT1, 0.5f, 0.5f, 0.5f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_SHAFT2, 0.4f, 0.2f, 0.0f, 0.3f);	// 0.4f, 0.2f, 0.0f
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_CEILING, _stdPathModels + L"ceiling.jpg", 2.0f, 2.0f, 1.0f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_FLOOR, _stdPathModels + L"floor3.jpg", 1.0f, 1.0f, 1.0f);
	
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_BLACK,  0.1f, 0.1f, 0.1f);
//	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_DOOR, _stdPathModels + L"metal1.jpg", 1.0f, 1.0f, 0.75f);
//	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_OPENING, _stdPathModels + L"metal2.jpg", 1.0f, 1.0f, 0.8f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_DOOR,	0.4f, 0.3f, 0.3f, 0.8f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_OPENING, 0.6f, 0.6f, 0.6f, 0.8f);

	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_LIFT, _stdPathModels + L"oak.jpg", 1.0f, 0.75f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_LIFT_FLOOR, _stdPathModels + L"metalplate.jpg", 3.0f, 3.0f);
	m_building.SetMaterial(m_pRenderer, CBuilding::MAT_LIFT_CEILING, _stdPathModels + L"metal3.jpg", 2.0f, 2.0f);
	
	m_building.Deconstruct(m_pScene);
	m_building.SetScaledDims(0.04f);
	m_building.Construct(m_pScene, L"Building", Vector(0, 0, 0));
	m_building.StoreConfig();

	Debug(L"Building created, ready for rendering.");
	TRACE("Building created, ready for rendering.\n");
	ASSERT(m_building.GetStoreyObj(0));

	return true;
}

bool CAdVisuoDoc::PrepareSim()
{
	m_pActionTick->UnSubscribeAll();
	m_sim.Play(m_pActionTick);
	if (m_sim.GetTimeLowerBound() < 0)
		for (AVLONG i = 0; i <= -m_sim.GetTimeLowerBound(); i += 40)
			Proceed(i);
	return true;
}

//bool CAdVisuoDoc::LoadSim(LPCOLESTR pXMLFileName)
//{
//	if (!IsBuildingLoaded())
//	{
//		Debug(L"Simulation data not loaded because no building data found");
//		return false;
//	}
//
//	m_sim.SetBuilding(&m_building);
//	m_sim.SetScene(m_pScene);
//
//	// obsolete code - no XML now...
//	// test file...
//	//const int N = 5;
//	//char buf[N+1];
//	//memset(buf, 0, sizeof(char)*(N+1));
//	//ifstream myFile (pXMLFileName, ios::in | ios::binary);
//	//if (myFile) myFile.read((char*)&buf, sizeof(char)*(N));
//	//myFile.close();
//	//bool bXml = strcmp(buf, "<?xml") == 0;
//	//buf[4] = 0;
//	//bool bSim = strcmp(buf, "ASKZ") == 0;
//	//
//	//HRESULT h;
//	//if (bXml)
//	//	SetSimReady((h = m_sim.LoadAsXML(pXMLFileName)) == S_OK);
//	//else if (bSim)
//	//	SetSimReady((h = m_sim.LoadAsSIM(pXMLFileName)) == S_OK);
//
//	HRESULT h;
//	SetSimReady((h = m_sim.LoadTmp(pXMLFileName)) == S_OK);
//
//	if (IsSimReady())
//	{
//		Debug(L"Simulation data loaded from %s", pXMLFileName);
//		Debug(L"%d passenger records loaded", m_sim.GetPassengerCount());
//	}
//	else
//	{
//		Debug(L"Error while loading simulation data from %s", pXMLFileName);
//		switch (h)
//		{
//		case CSim::E_SIM_PASSENGERS: Debug(L"No hall calls found"); break;
//		case CSim::E_SIM_LIFTS: Debug(L"Simulation data not compatible with the building structure: too many lifts!"); break;
//		case CSim::E_SIM_FLOORS: Debug(L"Simulation data not compatible with the building structure: too many floors!"); break;
//		}
//		
//	}
//	return IsSimReady();
//}

//bool CAdVisuoDoc::SaveIFC(LPCOLESTR pIFCFileName)
//{
//	if (!IsBuildingLoaded())
//	{
//		Debug(L"No building data found!!!");
//		return false;
//	}
//
//	HRESULT h = m_building.SaveAsIFC(pIFCFileName);
//	if (h == S_OK)
//	{
//		Debug(L"Building data saved to %s", pIFCFileName);
//		return true;
//	}
//	else
//	{
//		Debug(L"Error while exporting to IFC");
//		AfxGetMainWnd()->MessageBox(L"Error while exporting to IFC");
//		return false;
//	}	
//}

void CAdVisuoDoc::Play()
{
	PutAccel(1);
	m_pRenderer->Play();
	m_pRenderer->PutPlayTime(-m_sim.GetTimeLowerBound());
}

bool CAdVisuoDoc::Proceed(FWULONG nMSec)
{
	// SPECIAL ADDITION FOR DUBAI/ADRIAN
	static bool b1 = false;
	if (!b1 && nMSec >= 170000)
	{
		PutAccel(GetAccel() * 8);
		b1 = true;
	}
	
	static bool b2 = false;
	if (!b2 && nMSec >= 300000)
	{
		PutAccel(1.0f);
		b2 = true;
	}

	static bool b3 = false;
	if (!b3 && nMSec >= 540000)
	{
		PutAccel(GetAccel() * 8);
		b3 = true;
	}
	
	m_sim.SetColouringMode(((CAdVisuoApp*)AfxGetApp())->GetColouringMode());
	m_sim.SetTime(nMSec);

	m_pActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, NULL);
	return (m_pActionTick->AnySubscriptionsLeft() == TRUE);
}

bool CAdVisuoDoc::Proceed()
{
	return Proceed(GetPlayTime());
}

void CAdVisuoDoc::Rewind(FWULONG nMSec)
{
	ASSERT(FALSE);		// not implemented yet!
}

FWULONG CAdVisuoDoc::GetFPS()
{
	if (m_pfps[m_nfps] == 0)
		return 0;
	else
		return 1000 * (c_fpsNUM-1) / (m_pfps[(m_nfps+c_fpsNUM-1)%c_fpsNUM] - m_pfps[m_nfps]);
}



BOOL CAdVisuoDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

HRESULT CAdVisuoDoc::LoadDocument(CComPtr<IXmlReader> pReader)
{
	m_sim.SetBuilding(&m_building);
	return m_sim.Load(pReader);
}

HRESULT CAdVisuoDoc::LoadDocument(LPCTSTR lpszPathName)
{
	m_sim.SetBuilding(&m_building);
	return m_sim.Load(lpszPathName);
}

HRESULT CAdVisuoDoc::DownloadDocument(LPCTSTR lpszMethod, LPCTSTR lpszUrl, LPCTSTR lpszRequest)
{
	try
	{
		HRESULT h;

		MSXML::IXMLHttpRequestPtr HttpRequest;
		h = HttpRequest.CreateInstance("Microsoft.XMLHTTP");
		if (FAILED(h)) return h;

		VARIANT vAsync; vAsync.vt = VT_BOOL; vAsync.boolVal = FALSE;
		VARIANT vUser; vUser.vt = VT_BSTR; vUser.bstrVal = NULL;
		VARIANT vPassword; vPassword.vt = VT_BSTR; vPassword.bstrVal = NULL;
		VARIANT vRequest; vRequest.vt = VT_BSTR; bstr_t bstrRequest = lpszRequest; vRequest.bstrVal = bstrRequest;

		// Send Http Request
		HttpRequest->open(lpszMethod, lpszUrl, vAsync, vUser, vPassword);
		HttpRequest->setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
		HttpRequest->send(vRequest);

		// Read response and status
		long status = HttpRequest->status;
		CString strStatus = HttpRequest->statusText;
		CString strResponse = HttpRequest->responseText;
		m_strStatus = strStatus;
		m_strResponse = strResponse;

		// End http session
		HttpRequest->abort();
		long lReadyState = HttpRequest->readyState;


		DoSave(L"temp.xml");
		Debug(L"File temporarily saved to: temp.xml");



		// read data...
		m_sim.SetBuilding(&m_building);
		return m_sim.LoadFromBuf(strResponse);


		//MSXML::IXMLDOMDocumentPtr XMLDom;
		//h = XMLDom.CreateInstance("Microsoft.XMLDOM");

		//if (FAILED(h))
		//{
		//	HttpRequest->abort();
		//	return -1;
		//}

		////Load response in to XML DOM Tree
		//if (!XMLDom->loadXML(bsResponse))
		//{
		//	HttpRequest->abort();
		//	return -1;
		//}

		//VARIANT vSave;
		//vSave.vt = VT_BSTR;
		//bstr_t bstrSave = "dupa.xml";
		//vSave.bstrVal = bstrSave;
		//XMLDom->save(vSave);

	}
	catch(...)
	{
		return E_FAIL;
	}
	return S_OK;
}

BOOL CAdVisuoDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// only reads m_strResponse - to make Save operation possible
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	Debug(L""); Debug(L"Loading simulation from file: %s", lpszPathName);

	CWaitCursor wait;
	DeleteContents();

	HRESULT h = LoadDocument(lpszPathName);
	SetModifiedFlag(FALSE);
	return DisplayLoadRep(h);
}

BOOL CAdVisuoDoc::OnDownloadDocument(CString url)
{
	CString strMethod = L"GET";
	CString strRequest = L"";
	CString strName = L"";
	CString strUrl = url;

	int curpos = url.Find(L'?');
	if (curpos >= 0)
	{
		strUrl = url.Left(curpos);
		curpos++;

		CString name, val;
		while (1)
		{
			name = url.Tokenize(L"=", curpos);
			if (name.IsEmpty()) break;
			val = url.Tokenize(L"&", curpos);
			if (val.IsEmpty()) break;

			if (name.Compare(L"request") == 0)
			{
				strRequest.Format(L"id=%ls", val);
				strMethod = L"POST";
			}
			if (name.Compare(L"name") == 0)
				strName = val;
		}
	}

	if (!strName.IsEmpty())
		SetTitle(strName);

	Debug(L""); Debug(L"Downloading project from URL: %s, request %s", strUrl, strRequest);

	HRESULT h = DownloadDocument(strMethod, strUrl, strRequest);
	SetModifiedFlag(FALSE);
//	SetModifiedFlag(TRUE);
	return DisplayLoadRep(h);
}

BOOL CAdVisuoDoc::DisplayLoadRep(HRESULT h)
{
	m_h = h;

	LPCTSTR pMsg;
	switch (m_h)
	{
		case S_OK:						pMsg = L"Simulation loaded successfully."; break;
		case S_FALSE:					pMsg = L"Simulation not loaded because of unknown reasons."; break;
		case CSim::E_SIM_NO_BUILDING:	pMsg = L"Corrupt or missing building structure."; break;
		case CSim::E_SIM_PASSENGERS:	pMsg = L"No hall calls found."; break;
		case CSim::E_SIM_LIFTS:			pMsg = L"Inconsistent building structure: too many or too few lifts."; break;
		case CSim::E_SIM_FLOORS:		pMsg = L"Inconsistent building structure: too many or too few floors."; break;
		case CSim::E_SIM_LIFT_DECKS:	pMsg = L"Inconsistent building structure: wrong number of lift decks."; break;
		default:						pMsg = L"Unidentified error occured loading simulation data.";
	}
	if (m_h != S_OK)
		Debug(L"*** ERROR(S):");
	Debug(pMsg);
	return m_h == S_OK;
}

void CAdVisuoDoc::Serialize(CArchive& ar)
{
	USES_CONVERSION;
	
	if (ar.IsStoring())
	{
		CW2A cdata(m_strResponse);
		ar.GetFile()->Write(cdata, strlen(cdata));
	}
	else
	{
		// only loads m_strResponse to make it compatible with Save/Save As operations
		// regular load is made in OnOpenDocument
		ULONGLONG nSize = ar.GetFile()->GetLength();
		ASSERT(nSize <= 0xffffffff);
		char *pBuf = new char[(UINT)nSize+1];
		ar.GetFile()->Read(pBuf, (UINT)nSize);
		ar.GetFile()->SeekToBegin();
		pBuf[nSize] = '\0';
		m_strResponse = pBuf;
		delete [] pBuf;
	}
}

// CAdVisuoDoc diagnostics

#ifdef _DEBUG
void CAdVisuoDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAdVisuoDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CAdVisuoDoc commands

void CAdVisuoDoc::OnTimer()
{
	m_pfps[m_nfps] = GetTickCount();
	m_nfps = (m_nfps + 1) % c_fpsNUM;

	// #FreeWill: Push the time info into the engine
	if (IsPlaying())
		if (!Proceed())
			OnActionStop();

	UpdateAllViews(NULL, 0, 0);
}

void CAdVisuoDoc::OnActionPlay()
{
	if (IsSimReady() && !IsPlaying())
	{
		Debug(L"Processing visualisation data. Please wait...");
		Play();
		Debug(L"Visualisation started...");
	}
}

void CAdVisuoDoc::OnUpdateActionPlay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsEngineReady() && IsSimReady() && !IsPlaying());
	pCmdUI->SetCheck(IsPlaying());
}

void CAdVisuoDoc::OnActionPause()
{
	Pause();
}

void CAdVisuoDoc::OnUpdateActionPause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPlaying());
	pCmdUI->SetCheck(IsPaused());
}

void CAdVisuoDoc::OnActionStop()
{
	m_pRenderer->Stop();
	m_sim.Stop();
	m_building.RestoreConfig();
	PrepareSim();
}

void CAdVisuoDoc::OnUpdateActionStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPlaying());
}

void CAdVisuoDoc::OnActionSlowdown()
{
	PutAccel(GetAccel() / 2);
}

void CAdVisuoDoc::OnUpdateActionSlowdown(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPlaying());
}

void CAdVisuoDoc::OnActionSpeedup()
{
	PutAccel(GetAccel() * 2);
}

void CAdVisuoDoc::OnUpdateActionSpeedup(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPlaying());
}

void CAdVisuoDoc::OnActionNormalpace()
{
	PutAccel(1);
}

void CAdVisuoDoc::OnUpdateActionNormalpace(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsPlaying());
}

void CAdVisuoDoc::OnUpdateActionRender(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsSimReady() && !IsPlaying());
	pCmdUI->SetCheck(IsPlaying());
}

//void CAdVisuoDoc::OnFileOpensim()
//{
//	CFileDialog dlg(TRUE, L"*.sim", 0, 4|2, L"SIM files (*.sim)|*.sim|XML files (*.xml)|*.xml|All Files (*.*)|*.*||");
//	if (dlg.DoModal() == IDOK)
//	{
//		CWaitCursor wait_cursor;
//		LoadSim(dlg.GetPathName());
//	}
//}

//void CAdVisuoDoc::OnFileSaveifc()
//{
//	CFileDialog dlg(FALSE, L"*.ifc", 0, 4|2, L"Industry Foundation Class files (*.ifc)|*.ifc|All Files (*.*)|*.*||");
//	if (dlg.DoModal() == IDOK)
//	{
//		CWaitCursor wait_cursor;
//		SaveIFC(dlg.GetPathName());
//	}
//}

void CAdVisuoDoc::OnUpdateStatusbarPane2(CCmdUI *pCmdUI)
{
	FWULONG nTime = GetPlayTime();
	CString str;
	FWULONG fps = GetFPS();
	if (fps)
		str.Format(L"%d fps", fps);
	else
		str = L"??? fps";
	pCmdUI->SetText(str);
}

