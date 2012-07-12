// Camera.cpp

#include "StdAfx.h"
#include "Camera.h"
#include "VisBuilding.h"

#include <freewill.h>
#include <fwaction.h>
#include <fwrender.h>

#include "freewilltools.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)
#pragma warning (disable:4244)
 
#define NOBONE ((IKineNode*)NULL)

CCamera::CCamera(CBuildingVis *pBuilding, AVULONG nId)
{
	m_pBaseBone = NULL;
	m_pHandleBone = NULL;
	m_pCamera = NULL;

	SetBuilding(pBuilding);
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

void CCamera::SetBuilding(CBuildingVis *pBuilding)
{
	m_pBuilding = pBuilding;
	if (!m_pBuilding) return;

	m_box = m_pBuilding->GetBox();

	AVFLOAT nLWall = 0, nRWall = 0;
	if (m_pBuilding->GetLobbyArrangement() == CBuildingVis::LOBBY_DEADEND_LEFT)  nLWall = m_pBuilding->GetBox().LeftThickness();
	if (m_pBuilding->GetLobbyArrangement() == CBuildingVis::LOBBY_DEADEND_RIGHT) nRWall = m_pBuilding->GetBox().RightThickness();

	m_box = BOX(
		m_pBuilding->GetBox().Left() + 2 + nLWall,
		m_pBuilding->GetBox().Front() + 10,
		m_pBuilding->GetBox().Width() - 4 - nLWall - nRWall, 
		m_pBuilding->GetBox().Depth() - 20);

	m_nTripodHeight = m_pBuilding->GetShaft(0)->GetBoxDoor().Height();
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
	ASSERT (nStorey >= 0 && (ULONG)nStorey < GetBuilding()->GetStoreyCount());
	if (nStorey < 0 || (ULONG)nStorey >= GetBuilding()->GetStoreyCount())
		return;

	// storey parameters
	m_nStorey = nStorey;

	// new base bone
	SetBaseBone(m_pBuilding->GetStoreyElement(m_nStorey)->GetBone(), bKeepCoord);
}

void CCamera::SetLift(AVLONG nLift, bool bKeepCoord)
{
	ASSERT (nLift >= 0 && (ULONG)nLift < GetBuilding()->GetLiftCount());
	if (nLift < 0 || (ULONG)nLift >= GetBuilding()->GetLiftCount())
		return;

	// shaft/lift parameters
	m_nLift = nLift;
	m_nShaft = GetBuilding()->GetLift(nLift)->GetShaftId();

	// new base bone
	SetBaseBone(m_pBuilding->GetLiftElement(m_nLift)->GetBone(), bKeepCoord);
}

bool CCamera::Create()
{
	ASSERT(m_pHandleBone == NULL && m_pCamera == NULL);

	if (!m_pBuilding || !m_pBuilding->IsValid() || !m_pBaseBone)
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
	m_pBuilding = NULL;
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
			if (cp.eye.z > GetBuilding()->GetStorey(nId)->GetHeight())
				cp.eye.z = GetBuilding()->GetStorey(nId)->GetHeight();
		}
		else
		{
			// if camera mounted below the ceiling (above the standard tripod height)
			cp.eye.z = GetBuilding()->GetStorey(nId)->GetHeight() - GetBuilding()->GetStorey(GetStorey())->GetHeight() + cp.eye.z;

			if (cp.eye.z > GetBuilding()->GetStorey(nId)->GetHeight())
				cp.eye.z = GetBuilding()->GetStorey(nId)->GetHeight();
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

		AVFLOAT nStoreyHeight = min(m_pBuilding->GetStorey(GetStorey())->GetBox().Height(), 2 * m_nTripodHeight);
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
		AVFLOAT nStoreyHeight = m_pBuilding->GetStorey(GetStorey())->GetBox().Height();
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

		BOX box = m_pBuilding->GetLift(nId)->GetShaft()->GetBoxCar();
		AVFLOAT nCarHeight = box.Height();
		AVFLOAT fEyeHeight = min(nCarHeight - 20, m_nTripodHeight);
		AVFLOAT fTargetDist = box.Depth();					// distance to the point the camera is looking at
		AVFLOAT nTargetHeight = m_nTripodHeight * 2 / 4;	// height of the point the camera is looking at

		AVFLOAT fLine = (m_pBuilding->GetLift(nId)->GetShaft()->GetShaftLine() == 0) ? 1.0f : -1.0f;
	
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

		BOX lbox = m_pBuilding->GetShaft(0)->GetBoxCar();

		AVFLOAT yFront = (m_pBuilding->GetShaftLinesCount() == 1) ? m_box.Rear() : m_pBuilding->GetShaft(m_pBuilding->GetShaftCount(0))->GetBox().RearExt();
		AVFLOAT yRear  = m_pBuilding->GetShaft(0)->GetBox().RearExt();
		AVFLOAT nHeight = m_pBuilding->GetStorey(m_pBuilding->GetStoreyCount() - 1)->GetLevel() + m_pBuilding->GetStorey(m_pBuilding->GetStoreyCount() - 1)->GetHeight();

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
	Move(cp.eye.x, cp.eye.y, cp.eye.z, cp.EyeRef(GetBuilding()));
	Pan((AVFLOAT)M_PI + cp.fPan);
	Tilt(cp.fTilt);
	m_pCamera->PutPerspective(cp.FOV(), cp.fClipNear, cp.fClipFar, 0);

	m_bMoved = m_bRotated = m_bZoomed = false;
}

	int _callback_fun(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, CCamera *pCamera)
	{
		if (pEvent->nEvent != EVENT_TICK) return S_OK;
		FWFLOAT t;
		pAction->GetPhase(pEvent, &t);
		pCamera->GetCamera()->PutPerspective(pCamera->m_fFOV + t * (pCamera->m_cp.FOV() - pCamera->m_fFOV), pCamera->m_cp.fClipNear, pCamera->m_cp.fClipFar, 0);
		return S_OK;
	}

	int _callback_fun_oh(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, CCamera *pCamera)
	{
		if (pEvent->nEvent != EVENT_TICK) return S_OK;
		FWFLOAT t;
		pAction->GetPhase(pEvent, &t);
		pCamera->GetCamera()->PutPerspective(pCamera->m_fFOV, pCamera->m_fClipNear + t * (pCamera->m_cp.fClipNear - pCamera->m_fClipNear), pCamera->m_cp.fClipFar, 0);

		return S_OK;
	}

void CCamera::AnimateTo(IAction *pTickSource, CAMPARAMS &cp, AVULONG nTime)
{
	if (m_camloc == CAMLOC_LIFT)
		SetStorey(m_nLiftStorey);
	if (cp.camloc == CAMLOC_LIFT)
		SetLift(cp.nId);
	else
		SetStorey(cp.nId);

	m_camloc = cp.camloc;
	m_cp = cp;

	IAction *pAction;

	if (cp.camloc == CAMLOC_OVERHEAD) 
	{
		AVVECTOR eye1 = { cp.eye.x, cp.eye.y, m_pBuilding->GetStorey(cp.nId)->GetLevel() + m_pBuilding->GetStorey(cp.nId)->GetBox().Height() };
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"MoveTo", pTickSource, 0, nTime * 6 / 10, GetHandle(), *(FWVECTOR*)&eye1, NULL);
	}
	else
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"MoveTo", pTickSource, 0, nTime * 6 / 10, GetHandle(), *(FWVECTOR*)&cp.eye, cp.EyeRef(GetBuilding()));
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	ITransform *pT = NULL;
	GetHandle()->CreateCompatibleTransform(&pT);

	pT->FromRotationZ((AVFLOAT)M_PI + cp.fPan);
	pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"RotateTo", pTickSource, 0, nTime, GetHandle(), pT, NOBONE);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	pT->FromRotationX(cp.fTilt);
	pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"RotateTo", pTickSource, nTime/2, nTime/2, m_pCamera, pT, NOBONE);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	if (cp.camloc == CAMLOC_OVERHEAD) 
	{
		// second stage: move up, change the fClipNear
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"MoveTo", pTickSource, nTime, nTime, GetHandle(), *(FWVECTOR*)&cp.eye, cp.EyeRef(GetBuilding()));
		m_pCamera->GetPerspective(&m_fFOV, &m_fClipNear, NULL, NULL); 
		pAction->SetHandleEventHook((HANDLE_EVENT_HOOK_FUNC)_callback_fun_oh, 0, this);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

		// third step: adjust zoom
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"Generic", pTickSource, pAction, nTime/2);
		pAction->SetHandleEventHook((HANDLE_EVENT_HOOK_FUNC)_callback_fun, 0, this);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
	}
	else
	if (cp.FOV())
	{
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"Generic", pTickSource, nTime, nTime/2, m_pCamera, pT, GetHandle());
		m_pCamera->GetPerspective(&m_fFOV, NULL, NULL, NULL);
		GetCamera()->PutPerspective(m_fFOV, cp.fClipNear, cp.fClipFar, 0);
		pAction->SetHandleEventHook((HANDLE_EVENT_HOOK_FUNC)_callback_fun, 0, this);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
	}
	
	pT->Release();

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

	// camera azimouth and zone
	m_camAzim = ((AVLONG)((m_pBuilding->GetBox().InBoxAzimuth(pos, true) + M_PI + M_PI/4) / (M_PI/2))) % 4;
	if (m_pBuilding->GetBox().InBoxSection(pos, 3, 3, m_camXZone, m_camYZone))
		m_camZone = m_camXZone + 3 * m_camYZone;
	else
		m_camZone = 9;

	// camera position against the shafts
	for (AVLONG iRow = 0; iRow < 2; iRow++)
	{
		m_nShaftPos[iRow] = GetBuilding()->GetShaftBegin(iRow);
		while (m_nShaftPos[iRow] < (AVLONG)GetBuilding()->GetShaftEnd(iRow) && !GetBuilding()->GetShaft(m_nShaftPos[iRow])->InWidth(pos.x))
			m_nShaftPos[iRow]++;
		if (m_nShaftPos[iRow] == (AVLONG)GetBuilding()->GetShaftEnd(iRow))
			if (iRow == 0 && pos.x < 0 || iRow == 1 && pos.x > 0)
				m_nShaftPos[iRow] = (AVLONG)GetBuilding()->GetShaftBegin(iRow) - 1;
	}

	if (pos.z < GetBuilding()->GetStorey(0)->GetLevel())
	{
		camloc = CAMLOC_BELOW;
		nStorey = 0;
		nShaft = nLift = -1;
	}
	else if (pos.z >= GetBuilding()->GetStorey(GetBuilding()->GetStoreyCount() - 1)->GetRoofLevel())
	{
		camloc = CAMLOC_ABOVE;
		nStorey = GetBuilding()->GetStoreyCount() - 1;
		nShaft = nLift = -1;
	}
	else
	{
		// within the height of the building; it may be outside, lobby, shaft or lift
		// first, find the storey...
		while (nStorey < (AVLONG)GetBuilding()->GetStoreyCount() && !GetBuilding()->GetStorey(nStorey)->Within(pos))
			nStorey++;
		ASSERT (nStorey < (AVLONG)GetBuilding()->GetStoreyCount());
		
		if (m_pBuilding->GetBox().InBox(pos))
		{
			camloc = CAMLOC_LOBBY;
			nShaft = nLift = -1;
		}
		else
		{
			nShaft = 0;
			while (nShaft < (AVLONG)GetBuilding()->GetShaftCount() && !GetBuilding()->GetShaft(nShaft)->InBox(pos))
				nShaft++;
			if (nShaft < (AVLONG)GetBuilding()->GetShaftCount())
			{
				// we're in a shaft - check for the lifts
				nLift = 0;
				while (nLift < (AVLONG)GetBuilding()->GetLiftCount() && !GetBuilding()->GetLift(nLift)->Within(pos, GetBuilding()->GetLiftPos(nLift)))
					nLift++;
				if (nLift < (AVLONG)GetBuilding()->GetLiftCount())
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

	pDesc->floor = GetStorey() - GetBuilding()->GetBasementStoreyCount();
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
		pDesc->floor = m_nLiftStorey - GetBuilding()->GetBasementStoreyCount();
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

	CString pFloorName = GetBuilding()->GetStorey(desc.floor + GetBuilding()->GetBasementStoreyCount())->GetName().c_str();
	pFloorName.Trim();

	switch (desc.camloc)
	{
	case CAMLOC_LOBBY:
		switch (desc.index)
		{
		case 0: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby, rear left corner", nId, pFloorName); break;
		case 1: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby, rear side", nId, pFloorName); break;
		case 2: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby, rear right corner", nId, pFloorName); break;
		case 3: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby, right side", nId, pFloorName); break;
		case 4: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby, front right corner", nId, pFloorName); break;
		case 5: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby, front side", nId, pFloorName); break;
		case 6: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby, front left corner", nId, pFloorName); break;
		case 7: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby, left side", nId, pFloorName); break;
		default: _snwprintf(m_buf, nSize, L"Camera %d: %s lobby", nId, pFloorName); break;
		};
		break;
	case CAMLOC_OVERHEAD:	_snwprintf(m_buf, nSize, L"Camera %d: %s overhead view", nId, pFloorName); break;
	case CAMLOC_LIFT:		_snwprintf(m_buf, nSize, L"Camera %d: Lift %c at %s", nId, L'A' + desc.index - 1, pFloorName); break;
	case CAMLOC_SHAFT:		_snwprintf(m_buf, nSize, L"Camera %d: Shaft %c at %s", nId, L'A' + desc.index - 1, pFloorName); break;
	case CAMLOC_OUTSIDE:
		switch (desc.index)
		{
		case 0: _snwprintf(m_buf, nSize, L"Camera %d: %s, front side of the building", nId, pFloorName); break;
		case 1: _snwprintf(m_buf, nSize, L"Camera %d: %s, rear side of the building", nId, pFloorName); break;
		case 2: _snwprintf(m_buf, nSize, L"Camera %d: %s, left side of the building", nId, pFloorName); break;
		case 3: _snwprintf(m_buf, nSize, L"Camera %d: %s, right side of the building", nId, pFloorName); break;
		default: _snwprintf(m_buf, nSize, L"Camera %d: %s, outside the building", nId, pFloorName); break;
		}
		break;
	case CAMLOC_BELOW:		_snwprintf(m_buf, nSize, L"Camera %d: Below the building", nId); break;
	case CAMLOC_ABOVE:		_snwprintf(m_buf, nSize, L"Camera %d: Above the building", nId); break;
	default:				_snwprintf(m_buf, nSize, L"Camera %d: Location unknown", nId); break;
	};
	return m_buf;
}

LPTSTR CCamera::GetShortTextDescription()
{
	CAMDESC desc;
	GetDescription(&desc);

	AVULONG nId = GetId() + 1;
	AVULONG nSize = 256;

	CString pFloorName = GetBuilding()->GetStorey(desc.floor + GetBuilding()->GetBasementStoreyCount())->GetName().c_str();
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
