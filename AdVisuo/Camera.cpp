// Camera.cpp

#include "StdAfx.h"
#include "Camera.h"
#include "VisProject.h"
#include "VisLiftGroup.h"
#include "Engine.h"

#include <freewill.h>

#pragma warning (disable:4995)
#pragma warning (disable:4996)
#pragma warning (disable:4244)
 
#define NOBONE ((IKineNode*)NULL)

CCamera::CCamera(CLiftGroupVis *pLiftGroup, AVULONG nId)
{
	m_pBaseBone = NULL;
	m_pHandleBone = NULL;
	m_pCamera = NULL;

	SetLiftGroup(pLiftGroup);
	SetId(nId);
	SetStorey(0, false);
	m_nLift = 0;
	m_nShaft = 0;
	CheckLocation();

	m_camloc = CAMLOC_LOBBY;
	m_bMoved = m_bRotated = m_bZoomed = false;
	m_desc.camloc = CAMLOC_UNKNOWN;
}

CCamera::~CCamera()
{
	Destroy();
}

void CCamera::SetLiftGroup(CLiftGroupVis *pLiftGroup)
{
	m_pLiftGroup = pLiftGroup;
	if (!m_pLiftGroup) return;

	m_box = m_pLiftGroup->GetBox();

	AVFLOAT nLWall = 0, nRWall = 0;
	if (m_pLiftGroup->GetLobbyArrangement() == CLiftGroupVis::LOBBY_DEADEND_LEFT)  nLWall = m_pLiftGroup->GetBox().LeftThickness();
	if (m_pLiftGroup->GetLobbyArrangement() == CLiftGroupVis::LOBBY_DEADEND_RIGHT) nRWall = m_pLiftGroup->GetBox().RightThickness();

	m_box = BOX(
		m_pLiftGroup->GetBox().Left() + 2 + nLWall,
		m_pLiftGroup->GetBox().Front() + 10,
		m_pLiftGroup->GetBox().Width() - 4 - nLWall - nRWall, 
		m_pLiftGroup->GetBox().Depth() - 20);

	m_nTripodHeight = m_pLiftGroup->GetShaft(0)->GetBoxDoor().Height();
}

void CCamera::SetBaseBone(IKineNode *pNode, bool bKeepCoord)
{
	ASSERT(pNode); if (!pNode) return;

	if (bKeepCoord)
	{
		// modify coordinates so that to keep the camera intact
		ITransform *pT;
		m_pHandleBone->CreateCompatibleTransform(&pT);
		pNode->GetGlobalTransform(pT);
		m_pHandleBone->Transform(pT, KINE_LOCAL | KINE_INVERTED);
		m_pHandleBone->GetParentTransform(pT);
		m_pHandleBone->Transform(pT, KINE_LOCAL | KINE_REGULAR);
		pT->Release();
	}

	// deconstruct the camera from wherever it is now
	if (m_pBaseBone && m_pHandleBone)
		m_pBaseBone->DelChildPtr(m_pHandleBone);
	if (m_pBaseBone) 
		m_pBaseBone->Release();

	// get storey bone
	m_pBaseBone = pNode;
	m_pBaseBone->AddRef();

	// install the camera on the bone - if camera already created
	if (m_pHandleBone)
	{
		OLECHAR buf[257];
		m_pBaseBone->CreateUniqueLabel(L"CameraHandle", 256, buf);
		m_pBaseBone->AddChild(buf, m_pHandleBone);
	}
}

void CCamera::SetStorey(AVLONG nStorey, bool bKeepCoord)
{
	ASSERT (nStorey >= 0 && (ULONG)nStorey < GetLiftGroup()->GetStoreyCount());
	if (nStorey < 0 || (ULONG)nStorey >= GetLiftGroup()->GetStoreyCount())
		return;

	// storey parameters
	m_nStorey = nStorey;

	// new base bone
	SetBaseBone(m_pLiftGroup->GetStoreyElement(m_nStorey)->GetBone(), bKeepCoord);
}

void CCamera::SetLift(AVLONG nLift, bool bKeepCoord)
{
	ASSERT (nLift >= 0 && (ULONG)nLift < GetLiftGroup()->GetLiftCount());
	if (nLift < 0 || (ULONG)nLift >= GetLiftGroup()->GetLiftCount())
		return;

	// shaft/lift parameters
	m_nLift = nLift;
	m_nShaft = GetLiftGroup()->GetLift(nLift)->GetShaftId();

	// new base bone
	SetBaseBone(m_pLiftGroup->GetLiftElement(m_nLift)->GetBone(), bKeepCoord);
}

bool CCamera::Create()
{
	ASSERT(m_pHandleBone == NULL && m_pCamera == NULL);

	if (!m_pLiftGroup || !m_pLiftGroup->IsValid() || !m_pBaseBone)
		return false;

	OLECHAR buf[257];
	m_pBaseBone->CreateUniqueLabel(L"CameraHandle", 256, buf);
	m_pBaseBone->CreateChild(buf, &m_pHandleBone);

	m_pHandleBone->FWDevice()->CreateObject(L"Camera", IID_ISceneCamera, (IFWUnknown**)&m_pCamera);
	m_pHandleBone->AddChild(L"Camera", m_pCamera);

	m_pCamera->Create(__FW_Vector(0, 0, 0), __FW_Vector(0, 1, 0), __FW_Vector(0, 0, 1.0f));
	m_pCamera->PutPerspective((AVFLOAT)M_PI / 4, 20.0f, 10000.0f, 0.0f);
	m_pCamera->PutVisible(TRUE);

	IKineTargetedObj *pTO = NULL;
	m_pCamera->QueryInterface(&pTO);
	pTO->PutConfig(KINE_TARGET_ORBITING, NULL);
	pTO->Release();

	return true;
}

bool CCamera::Destroy()
{
	if (!m_pBaseBone && !m_pHandleBone && !m_pCamera)
		return true;
	if (m_pBaseBone && m_pHandleBone)
		m_pBaseBone->DelChildPtr(m_pHandleBone);
	m_pCamera->Release();
	m_pHandleBone->Release();
	m_pBaseBone->Release();
	m_pBaseBone = NULL;
	m_pHandleBone = NULL;
	m_pCamera = NULL;
	m_pLiftGroup = NULL;
	return true;
}

void CCamera::GetCurPos(AVVECTOR &pos)
{
	pos.x = pos.y = pos.z = 0;
	m_pHandleBone->LtoG((FWVECTOR*)&pos);
}

void CCamera::GetCurLocalPos(AVVECTOR &pos)
{
	pos.x = pos.y = pos.z = 0;
	m_pHandleBone->LtoG((FWVECTOR*)&pos);
	m_pBaseBone->GtoL((FWVECTOR*)&pos);
}

CAMPARAMS CCamera::GetCameraParams()
{
	CAMPARAMS cp;
	memset(&cp, 0, sizeof(cp));

	cp.camloc = m_camloc;
	cp.nId = m_camloc == CAMLOC_LIFT ? GetLift() : GetStorey();

	ITransform *pT = NULL;
	m_pHandleBone->CreateCompatibleTransform(&pT);

	FWVECTOR vecPan = { 0, 1, 0 };
	m_pHandleBone->GetLocalTransform(pT);
	pT->AsVector((FWVECTOR*)&cp.eye);
	pT->ApplyRotationTo(&vecPan);
	cp.fPan = atan2(vecPan.x, -vecPan.y);

	FWVECTOR vecTilt = { 0, 1, 0 };
	m_pCamera->GetLocalTransform(pT);
	pT->ApplyRotationTo(&vecTilt);
	cp.fTilt = atan2(vecTilt.z, vecTilt.y);

	pT->Release();

	AVFLOAT fov;
	m_pCamera->GetPerspective(&fov, &cp.fClipNear, &cp.fClipFar, &cp.fAspectRatio);
	cp.fHFOV = cp.fVFOV = fov;

	return cp;
}

AVFLOAT hypotenuse(AVFLOAT a, AVFLOAT b)	{ return sqrt(a*a+b*b); }

CAMPARAMS CCamera::GetDefCameraParams(CAMLOC camloc, AVULONG nId, AVFLOAT fAspect)
{
	CAMPARAMS cp;

	cp.fClipNear = 10;
	cp.fClipFar = 10000;
	cp.fZoom = 0;

	switch (camloc)
	{
	case CAMLOC_STOREY:
		cp = GetCameraParams();
		if (cp.camloc == CAMLOC_LIFT) break;	// cannot change floor while in lift!
		cp.nId = nId;

		if (cp.eye.z <= m_nTripodHeight * 1.1)
		{
			// camera mounted on a tripod
			if (cp.eye.z > GetLiftGroup()->GetStorey(nId)->GetHeight())
				cp.eye.z = GetLiftGroup()->GetStorey(nId)->GetHeight();
		}
		else
		{
			// if camera mounted below the ceiling (above the standard tripod height)
			cp.eye.z = GetLiftGroup()->GetStorey(nId)->GetHeight() - GetLiftGroup()->GetStorey(GetStorey())->GetHeight() + cp.eye.z;

			if (cp.eye.z > GetLiftGroup()->GetStorey(nId)->GetHeight())
				cp.eye.z = GetLiftGroup()->GetStorey(nId)->GetHeight();
			if (cp.eye.z < m_nTripodHeight)
				cp.eye.z = m_nTripodHeight;
		}
		
		break;

	case CAMLOC_LOBBY:
	{
		cp.camloc = camloc;
		cp.nId = GetStorey();
		cp.fAspectRatio = fAspect;

		if (nId > 7) nId = 0;

		AVFLOAT nStoreyHeight = min(m_pLiftGroup->GetStorey(GetStorey())->GetBox().Height(), 2 * m_nTripodHeight);
		AVFLOAT fEyeHeight = min(nStoreyHeight - 20, m_nTripodHeight);

		// eye position
		switch (nId)
		{
		case 0: cp.eye = Vector(m_box.Left(), m_box.Front(), fEyeHeight); break;		// rear left
		case 1: cp.eye = Vector(m_box.CentreX(), m_box.Front(), fEyeHeight); break;	// rear centre
		case 2: cp.eye = Vector(m_box.Right(), m_box.Front(), fEyeHeight); break;	// rear right
		case 3: cp.eye = Vector(m_box.Right(), m_box.CentreY(), fEyeHeight); break;	// right side
		case 4: cp.eye = Vector(m_box.Right(), m_box.Rear(), fEyeHeight); break;	// front right
		case 5: cp.eye = Vector(m_box.CentreX(), m_box.Rear(), fEyeHeight); break;	// front centre
		case 6: cp.eye = Vector(m_box.Left(), m_box.Rear(), fEyeHeight); break;	// front left
		case 7: cp.eye = Vector(m_box.Left(), m_box.CentreY(), fEyeHeight); break;	// left side
		}

		AVFLOAT fTargetDist;								// distance to the point the camera is looking at
		AVFLOAT nTargetHeight = m_nTripodHeight * 2 / 4;	// height of the point the camera is looking at
		AVFLOAT fHalfwayDist;		// distance to the location used to setup FOV vertically, roughly halfway to the back of the room
		AVFLOAT fRealFOV;			// real FOV

		switch (nId)
		{
		case 0:
		case 2:
		case 4:
		case 6:	// looking from the corners of the lobby
			fTargetDist = hypotenuse(m_box.Depth(), m_box.Width()/2);
			fHalfwayDist = hypotenuse(m_box.Depth()/2, m_box.Width()/2);
			fRealFOV = (AVFLOAT)M_PI/2;
			cp.fHFOV = 2 * atan(1.0f / fAspect);
			break;
		case 1:
		case 5:	// looking from the front or rear
			fTargetDist = m_box.Depth();
			fHalfwayDist = fTargetDist * 0.5f;
			fRealFOV = 2*atan2(m_box.Width()/2, m_box.Depth());
			cp.fHFOV = 2*atan2(m_box.Width()/2/fAspect, m_box.Depth());
			break;
		case 3:
		case 7:	// looking from the sides
			fTargetDist = m_box.Width();
			fHalfwayDist = fTargetDist * 0.3f;
			fRealFOV = (AVFLOAT)M_PI/2;
			cp.fHFOV = 2 * atan(1.0f / fAspect);
			break;
		}

		cp.fVFOV = 2 * atan2(nStoreyHeight / 2, fHalfwayDist);
		cp.fTilt = -atan2(fEyeHeight - nTargetHeight, fTargetDist);

		switch (nId)
		{
		case 0: cp.fPan = (AVFLOAT)M_PI - fRealFOV / 2; break;	// rear left
		case 1: cp.fPan = (AVFLOAT)M_PI; break;					// rear centre
		case 2: cp.fPan = (AVFLOAT)M_PI + fRealFOV / 2; break;	// rear right
		case 3: cp.fPan = -(AVFLOAT)M_PI/2; break;				// right side
		case 4: cp.fPan = -fRealFOV / 2; break;					// front right
		case 5: cp.fPan = 0; break;								// front centre
		case 6: cp.fPan = fRealFOV / 2; break;					// front left
		case 7: cp.fPan = (AVFLOAT)M_PI/2; break;				// left side
		}
		
		break;
	}
	case CAMLOC_OVERHEAD:
	{
		cp.camloc = camloc;
		cp.nId = GetStorey();
		cp.fAspectRatio = fAspect;

		// top centre
		cp.fPan = 0;
		cp.fTilt = -(AVFLOAT)M_PI / 2;
		cp.fHFOV = (AVFLOAT)(M_PI) / 4;
		cp.fVFOV = 0;
		AVFLOAT fEyeHeight1 = 1.25f * m_box.Depth() / 2 / tan(cp.fHFOV/2);
		AVFLOAT fEyeHeight2 = 1.10f * m_box.Width() / fAspect / 2 / tan(cp.fHFOV/2);
		AVFLOAT fEyeHeight = max(fEyeHeight1, fEyeHeight2);
		cp.eye = Vector(m_box.CentreX(), m_box.CentreY(), fEyeHeight);
		AVFLOAT nStoreyHeight = m_pLiftGroup->GetStorey(GetStorey())->GetBox().Height();
		cp.fClipNear = fEyeHeight - nStoreyHeight + 20;
		if (cp.fClipNear < 10) cp.fClipNear = 10;

		break;
	}
	case CAMLOC_LIFT:
	{
		cp.camloc = camloc;
		cp.nId = nId;
		cp.fAspectRatio = fAspect;

		cp.camloc = camloc;
		cp.nId = nId;

		BOX box = m_pLiftGroup->GetLift(nId)->GetShaft()->GetBoxCar();
		AVFLOAT nCarHeight = box.Height();
		AVFLOAT fEyeHeight = min(nCarHeight - 20, m_nTripodHeight);
		AVFLOAT fTargetDist = box.Depth();					// distance to the point the camera is looking at
		AVFLOAT nTargetHeight = m_nTripodHeight * 2 / 4;	// height of the point the camera is looking at

		AVFLOAT fLine = (m_pLiftGroup->GetLift(nId)->GetShaft()->GetShaftLine() == 0) ? 1.0f : -1.0f;
	
		cp.eye = Vector(box.Width()/2, box.Depth()+fLine*10, fEyeHeight);
		cp.fHFOV = 0;
		cp.fVFOV = (AVFLOAT)M_PI / 2;
		cp.fTilt = -atan2(fEyeHeight - nTargetHeight, abs(fTargetDist));
		cp.fPan = (fLine > 0) ? (AVFLOAT)M_PI : 0;

		break;
	}
	case CAMLOC_OUTSIDE:
	{
		cp.camloc = camloc;
		cp.nId = 0;
		cp.fAspectRatio = fAspect;

		BOX lbox = m_pLiftGroup->GetShaft(0)->GetBoxCar();

		AVFLOAT yFront = (m_pLiftGroup->GetShaftLinesCount() == 1) ? m_box.Rear() : m_pLiftGroup->GetShaft(m_pLiftGroup->GetShaftCount(0))->GetBox().RearExt();
		AVFLOAT yRear  = m_pLiftGroup->GetShaft(0)->GetBox().RearExt();
		AVFLOAT nHeight = m_pLiftGroup->GetStorey(m_pLiftGroup->GetStoreyCount() - 1)->GetLevel() + m_pLiftGroup->GetStorey(m_pLiftGroup->GetStoreyCount() - 1)->GetHeight();

		switch (nId)
		{
		case 0: 
			cp.eye = Vector(0, yFront + nHeight, nHeight/2);
			cp.fPan = 0;
			break;
		case 1: 
			cp.eye = Vector(0, yRear - nHeight, nHeight/2);
			cp.fPan = (FLOAT)M_PI;
			break;
		case 2: 
			cp.eye = Vector(m_box.Left() - nHeight, 0, nHeight/2);
			cp.fPan = (FLOAT)(M_PI/2);
			break;
		case 3: 
			cp.eye = Vector(m_box.Right() + nHeight, 0, nHeight/2);
			cp.fPan = -(FLOAT)(M_PI/2);;
			break;
		}

		cp.fHFOV = 0;
		cp.fVFOV = 0.98f;
		cp.fTilt = 0;

		cp.fClipNear = 25;
		cp.fClipFar = 50000;

		break;
	}
	
	case CAMLOC_SHAFT:
	case CAMLOC_BELOW:
	case CAMLOC_ABOVE:
	case CAMLOC_UNKNOWN:
	default:
		ASSERT(FALSE);
		break;
	}
	return cp;
}

void CCamera::MoveTo(CAMPARAMS &cp)
{
	if (m_camloc == CAMLOC_LIFT)
		SetStorey(m_nLiftStorey);
	if (cp.camloc == CAMLOC_LIFT)
		SetLift(cp.nId);
	else
		SetStorey(cp.nId);

	m_camloc = cp.camloc;
	m_cp = cp;

	Reset();
	Move(cp.eye.x, cp.eye.y, cp.eye.z, cp.EyeRef(GetLiftGroup()));
	Pan((AVFLOAT)M_PI + cp.fPan);
	Tilt(cp.fTilt);
	m_pCamera->PutPerspective(cp.FOV(), cp.fClipNear, cp.fClipFar, 0);

	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::AnimateTo(CEngine *pEngine, CAMPARAMS &cp, AVULONG nTime)
{
	pEngine->StartCameraAnimation();	
		
	if (m_camloc == CAMLOC_LIFT)
		SetStorey(m_nLiftStorey);
	if (cp.camloc == CAMLOC_LIFT)
		SetLift(cp.nId);
	else
		SetStorey(cp.nId);

	m_camloc = cp.camloc;
	m_cp = cp;

	ANIM_HANDLE a;
	if (cp.camloc == CAMLOC_OVERHEAD) 
	{
		AVVECTOR eye1 = { cp.eye.x, cp.eye.y, m_pLiftGroup->GetStorey(cp.nId)->GetLevel() + m_pLiftGroup->GetStorey(cp.nId)->GetBox().Height() };

		a = pEngine->MoveCameraTo(GetHandle(), eye1, 0, nTime * 6 / 10, NULL);
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = pEngine->PanCamera(GetHandle(), (AVFLOAT)M_PI + cp.fPan, 0, nTime);
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = pEngine->TiltCamera(m_pCamera, cp.fTilt, nTime/2, nTime/2);
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);

		// second stage: move up, change the fClipNear
		a = pEngine->MoveCameraTo(GetHandle(), cp.eye, nTime, nTime/2, cp.EyeRef(GetLiftGroup()));
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = pEngine->ZoomCamera(m_pCamera, -1, m_cp.fClipNear, m_cp.fClipFar, nTime, nTime/3);
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);

		// third step: adjust zoom
		a = pEngine->ZoomCamera(m_pCamera, m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, 3*nTime/2, nTime/4);
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
	}
	else
	{
		a = pEngine->ZoomCamera(m_pCamera, -1, m_cp.fClipNear, m_cp.fClipFar, 0, nTime/10);

		a = pEngine->MoveCameraTo(GetHandle(), cp.eye, 0, nTime * 6 / 10, cp.EyeRef(GetLiftGroup()));
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = pEngine->PanCamera(GetHandle(), (AVFLOAT)M_PI + cp.fPan, 0, nTime);
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = pEngine->TiltCamera(m_pCamera, cp.fTilt, nTime/2, nTime/2);
		pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		if (cp.FOV())
		{
			a = pEngine->ZoomCamera(m_pCamera, m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, nTime, nTime/2);
			pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		}
	}

	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::Reset()
{
	m_pHandleBone->Reset();
	m_pCamera->Reset();
}

void CCamera::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z, IKineNode *pRef)
{
	if (!m_pCamera || !m_pHandleBone) return;

	ITransform *pT = NULL;
	m_pHandleBone->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(x, y, z);
	m_pHandleBone->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
	m_bMoved = true;
}

void CCamera::Pan(AVFLOAT f)
{
	if (!m_pCamera || !m_pHandleBone) return;

	ITransform *pT = NULL;
	m_pHandleBone->CreateCompatibleTransform(&pT);
	pT->FromRotationZ(f);
	m_pHandleBone->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
	m_bRotated = true;
}

void CCamera::Tilt(AVFLOAT f)
{
	if (!m_pCamera) return;

	ITransform *pT = NULL;
	m_pCamera->CreateCompatibleTransform(&pT);
	pT->FromRotationX(f);
	m_pCamera->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
	m_bRotated = true;
}

void CCamera::Zoom(AVFLOAT f)
{
	if (!m_pCamera) return;

	//m_cp.fZoom -= f;
	m_cp.fHFOV -= f;
	m_cp.fVFOV -= f;
	m_pCamera->PutPerspective(m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, m_cp.fAspectRatio);

	m_bZoomed = true;


	//AVFLOAT fFOV, fNear, fFar, fAspect;
	//m_pCamera->GetPerspective(&fFOV, &fNear, &fFar, &fAspect);
	//fFOV -= f;
	//m_pCamera->PutPerspective(fFOV, fNear, fFar, fAspect);
}

void CCamera::Adjust(AVFLOAT fNewAspectRatio)
{
	m_cp.fHFOV = 2 * atan(tan(m_cp.fHFOV/2) * m_cp.fAspectRatio / fNewAspectRatio);
	m_cp.fAspectRatio = fNewAspectRatio;
	m_pCamera->PutPerspective(m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, m_cp.fAspectRatio);
}

void CCamera::CheckLocation()
{
	if (!m_pHandleBone) return;

	if (m_camloc == CAMLOC_OVERHEAD)
		return;

	// initial values
	enum CAMLOC camloc = CAMLOC_OUTSIDE;
	AVLONG nStorey = 0, nShaft = 0, nLift = 0, nLiftStorey = 0;

	// position of the camera
	AVVECTOR pos = { 0, 0, 0 };
	m_pHandleBone->LtoG((FWVECTOR*)&pos);

	// first thing: determine which lift group it is!
	CProjectVis *pProject = GetLiftGroup()->GetProject();
	CLiftGroupVis *pGroup = pProject->GetLiftGroup(0);
	for each (CLiftGroupVis *p in pProject->GetLiftGroups())
	{
		pGroup = p;
		if (pos.y >= p->GetTotalAreaBox().FrontExt()) break;
	}
	if (pGroup != GetLiftGroup())
		SetLiftGroup(pGroup);

	// camera azimouth and zone
	m_camAzim = ((AVLONG)((m_pLiftGroup->GetBox().InBoxAzimuth(pos, true) + M_PI + M_PI/4) / (M_PI/2))) % 4;
	if (m_pLiftGroup->GetBox().InBoxSection(pos, 3, 3, m_camXZone, m_camYZone))
		m_camZone = m_camXZone + 3 * m_camYZone;
	else
		m_camZone = 9;

	// camera position against the shafts
	for (AVLONG iRow = 0; iRow < 2; iRow++)
	{
		m_nShaftPos[iRow] = GetLiftGroup()->GetShaftBegin(iRow);
		while (m_nShaftPos[iRow] < (AVLONG)GetLiftGroup()->GetShaftEnd(iRow) && !GetLiftGroup()->GetShaft(m_nShaftPos[iRow])->InWidth(pos.x))
			m_nShaftPos[iRow]++;
		if (m_nShaftPos[iRow] == (AVLONG)GetLiftGroup()->GetShaftEnd(iRow))
			if (iRow == 0 && pos.x < 0 || iRow == 1 && pos.x > 0)
				m_nShaftPos[iRow] = (AVLONG)GetLiftGroup()->GetShaftBegin(iRow) - 1;
	}

	if (pos.z < GetLiftGroup()->GetStorey(0)->GetLevel())
	{
		camloc = CAMLOC_BELOW;
		nStorey = 0;
		nShaft = nLift = -1;
	}
	else if (pos.z >= GetLiftGroup()->GetStorey(GetLiftGroup()->GetStoreyCount() - 1)->GetRoofLevel())
	{
		camloc = CAMLOC_ABOVE;
		nStorey = GetLiftGroup()->GetStoreyCount() - 1;
		nShaft = nLift = -1;
	}
	else
	{
		// within the height of the lift group; it may be outside, lobby, shaft or lift
		// first, find the storey...
		while (nStorey < (AVLONG)GetLiftGroup()->GetStoreyCount() && !GetLiftGroup()->GetStorey(nStorey)->Within(pos))
			nStorey++;
		ASSERT (nStorey < (AVLONG)GetLiftGroup()->GetStoreyCount());
		
		if (m_pLiftGroup->GetBox().InBox(pos))
		{
			camloc = CAMLOC_LOBBY;
			nShaft = nLift = -1;
		}
		else
		{
			nShaft = 0;
			while (nShaft < (AVLONG)GetLiftGroup()->GetShaftCount() && !GetLiftGroup()->GetShaft(nShaft)->InBox(pos))
				nShaft++;
			if (nShaft < (AVLONG)GetLiftGroup()->GetShaftCount())
			{
				// we're in a shaft - check for the lifts
				nLift = 0;
				while (nLift < (AVLONG)GetLiftGroup()->GetLiftCount() && !GetLiftGroup()->GetLift(nLift)->Within(pos, GetLiftGroup()->GetLiftPos(nLift)))
					nLift++;
				if (nLift < (AVLONG)GetLiftGroup()->GetLiftCount())
				{
					camloc = CAMLOC_LIFT;
					nLiftStorey = nStorey;
					nStorey = -1;
				}
				else
				{
					camloc = CAMLOC_SHAFT;
					nLift = nShaft;
				}
			}
			else
			{
				camloc = CAMLOC_OUTSIDE;
				nShaft = nLift = -1;
			}
		}
	}

	if (camloc == CAMLOC_LIFT && nLift != m_nLift)
		SetLift(nLift);
	if (camloc != CAMLOC_LIFT && nStorey != m_nStorey)
		SetStorey(nStorey);

	m_camloc = camloc;
	m_nStorey = nStorey;
	m_nLift = nLift;
	m_nShaft = nShaft;
	m_nLiftStorey = nLiftStorey;
}

CAMLOC CCamera::GetDescription(CAMDESC *pDesc)
{
	if (!pDesc) return m_camloc;

	AVLONG ZONE[] = { 0, 1, 2, 7, -1, 3, 6, 5, 4 };
	AVLONG AZIM[] = { 1, 7, 5, 3 };
	AVLONG AZIMb[] = { 1, 2, 0, 3 };

	pDesc->floor = GetStorey() - GetLiftGroup()->GetBasementStoreyCount();
	pDesc->camloc = m_camloc;
	pDesc->index = -1;
	pDesc->bExact = !m_bMoved;
	switch (m_camloc)
	{
	case CAMLOC_LOBBY:
		if (m_camZone >= 0 && m_camZone < 9)
			pDesc->index = ZONE[m_camZone];
		if (pDesc->index == -1 && m_camAzim >= 0 && m_camAzim < 4)
			pDesc->index = AZIM[m_camAzim];
		break;
	case CAMLOC_LIFT:
		pDesc->floor = m_nLiftStorey - GetLiftGroup()->GetBasementStoreyCount();
		pDesc->index = GetLift() + 1;
		break;
	case CAMLOC_SHAFT:
		pDesc->index = GetShaft() + 1;
		break;
	case CAMLOC_OUTSIDE:
		if (m_camAzim >= 0 && m_camAzim < 4)
			pDesc->index = AZIMb[m_camAzim];
		break;
	}
	return m_camloc;
}

LPTSTR CCamera::GetTextDescription()
{
	CAMDESC desc;
	GetDescription(&desc);

	if (memcmp(&desc, &m_desc, sizeof(CAMDESC)) == 0)
		return m_buf;
	m_desc = desc;

	AVULONG nId = GetId() + 1;
	AVULONG nSize = 256;
	AVULONG nGrp = GetLiftGroup()->GetIndex() + 1;

	CString pFloorName = GetLiftGroup()->GetStorey(desc.floor + GetLiftGroup()->GetBasementStoreyCount())->GetName().c_str();
	pFloorName.Trim();

	switch (desc.camloc)
	{
	case CAMLOC_LOBBY:
		switch (desc.index)
		{
		case 0: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby, rear left corner", nId, nGrp, pFloorName); break;
		case 1: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby, rear side", nId, nGrp, pFloorName); break;
		case 2: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby, rear right corner", nId, nGrp, pFloorName); break;
		case 3: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby, right side", nId, nGrp, pFloorName); break;
		case 4: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby, front right corner", nId, nGrp, pFloorName); break;
		case 5: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby, front side", nId, nGrp, pFloorName); break;
		case 6: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby, front left corner", nId, nGrp, pFloorName); break;
		case 7: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby, left side", nId, nGrp, pFloorName); break;
		default: _snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s lobby", nId, nGrp, pFloorName); break;
		};
		break;
	case CAMLOC_OVERHEAD:	_snwprintf(m_buf, nSize, L"Cam %d: Grp %d %s overhead view", nId, nGrp, pFloorName); break;
	case CAMLOC_LIFT:		_snwprintf(m_buf, nSize, L"Cam %d: Grp %d Lift %c at %s", nId, nGrp, L'A' + desc.index - 1, pFloorName); break;
	case CAMLOC_SHAFT:		_snwprintf(m_buf, nSize, L"Cam %d: Grp %d Shaft %c at %s", nId, nGrp, L'A' + desc.index - 1, pFloorName); break;
	case CAMLOC_OUTSIDE:
		switch (desc.index)
		{
		case 0: _snwprintf(m_buf, nSize, L"Cam %d: %s, front side of the building", nId, pFloorName); break;
		case 1: _snwprintf(m_buf, nSize, L"Cam %d: %s, rear side of the building", nId, pFloorName); break;
		case 2: _snwprintf(m_buf, nSize, L"Cam %d: %s, left side of the building", nId, pFloorName); break;
		case 3: _snwprintf(m_buf, nSize, L"Cam %d: %s, right side of the building", nId, pFloorName); break;
		default: _snwprintf(m_buf, nSize, L"Cam %d: %s, outside the building", nId, pFloorName); break;
		}
		break;
	case CAMLOC_BELOW:		_snwprintf(m_buf, nSize, L"Cam %d: Below the building", nId); break;
	case CAMLOC_ABOVE:		_snwprintf(m_buf, nSize, L"Cam %d: Above the building", nId); break;
	default:				_snwprintf(m_buf, nSize, L"Cam %d: Location unknown", nId); break;
	};
	return m_buf;
}

LPTSTR CCamera::GetShortTextDescription()
{
	CAMDESC desc;
	GetDescription(&desc);

	AVULONG nId = GetId() + 1;
	AVULONG nSize = 256;

	CString pFloorName = GetLiftGroup()->GetStorey(desc.floor + GetLiftGroup()->GetBasementStoreyCount())->GetName().c_str();
	pFloorName.Trim();

	static wchar_t buf[256];

	switch (desc.camloc)
	{
	case CAMLOC_LOBBY:
		switch (desc.index)
		{
		case 0: _snwprintf(buf, nSize, L"%s lobby, rear left corner", pFloorName); break;
		case 1: _snwprintf(buf, nSize, L"%s lobby, rear side", pFloorName); break;
		case 2: _snwprintf(buf, nSize, L"%s lobby, rear right corner", pFloorName); break;
		case 3: _snwprintf(buf, nSize, L"%s lobby, right side", pFloorName); break;
		case 4: _snwprintf(buf, nSize, L"%s lobby, front right corner", pFloorName); break;
		case 5: _snwprintf(buf, nSize, L"%s lobby, front side", pFloorName); break;
		case 6: _snwprintf(buf, nSize, L"%s lobby, front left corner", pFloorName); break;
		case 7: _snwprintf(buf, nSize, L"%s lobby, left side", pFloorName); break;
		default: _snwprintf(buf, nSize, L"%s lobby", pFloorName); break;
		};
		break;
	case CAMLOC_OVERHEAD:	_snwprintf(buf, nSize, L"%s overhead view", pFloorName); break;
	case CAMLOC_LIFT:		_snwprintf(buf, nSize, L"Lift %c at %s", L'A' + desc.index - 1, pFloorName); break;
	case CAMLOC_SHAFT:		_snwprintf(buf, nSize, L"Shaft %c at %s", L'A' + desc.index - 1, pFloorName); break;
	case CAMLOC_OUTSIDE:
		switch (desc.index)
		{
		case 0: _snwprintf(buf, nSize, L"%s, front side of the building", pFloorName); break;
		case 1: _snwprintf(buf, nSize, L"%s, rear side of the building", pFloorName); break;
		case 2: _snwprintf(buf, nSize, L"%s, left side of the building", pFloorName); break;
		case 3: _snwprintf(buf, nSize, L"%s, right side of the building", pFloorName); break;
		default: _snwprintf(buf, nSize, L"%s, outside the building", pFloorName); break;
		}
		break;
	case CAMLOC_BELOW:		_snwprintf(buf, nSize, L"Below the building"); break;
	case CAMLOC_ABOVE:		_snwprintf(buf, nSize, L"Above the building"); break;
	default:				_snwprintf(buf, nSize, L"Location unknown"); break;
	};
	return buf;
}
