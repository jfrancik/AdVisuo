// Camera.cpp

#include "StdAfx.h"
#include "Camera.h"
#include "Building.h"

#include <freewill.h>
#include <fwaction.h>
#include <fwrender.h>

#include "freewilltools.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)
#pragma warning (disable:4244)
 
#define NOBONE ((IKineNode*)NULL)

CCamera::CCamera(CBuilding *pBuilding, AVULONG nId)
{
	m_pBaseBone = NULL;
	m_pHandleBone = NULL;
	m_pCamera = NULL;

	SetBuilding(pBuilding);
	SetId(nId);
	SetStorey(0, false);
	m_nLift = 0;
	CheckLocation();

	m_camloc = CAMLOC_LOBBY;
	m_bMoved = m_bRotated = m_bZoomed = false;
	m_desc.camloc = CAMLOC_UNKNOWN;
}

CCamera::~CCamera()
{
	Destroy();
}

void CCamera::SetBuilding(CBuilding *pBuilding)
{
	m_pBuilding = pBuilding;
	if (!m_pBuilding) return;

	m_box = m_pBuilding->GetBox();

	AVFLOAT nLWall = 0, nRWall = 0;
	if (m_pBuilding->GetLobbyArrangement() == CBuilding::LOBBY_DEADEND_LEFT)  nLWall = m_pBuilding->GetBox().LeftThickness();
	if (m_pBuilding->GetLobbyArrangement() == CBuilding::LOBBY_DEADEND_RIGHT) nRWall = m_pBuilding->GetBox().RightThickness();

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
	SetBaseBone(m_pBuilding->GetStoreyBone(m_nStorey), bKeepCoord);
}

void CCamera::SetLift(AVLONG nLift, bool bKeepCoord)
{
	ASSERT (nLift >= 0 && (ULONG)nLift < GetBuilding()->GetShaftCount());
	if (nLift < 0 || (ULONG)nLift >= GetBuilding()->GetShaftCount())
		return;

	// storey parameters
	m_nLift = nLift;

	// new base bone
	SetBaseBone(m_pBuilding->GetLiftBone(m_nLift));
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

AVFLOAT hypotenuse(AVFLOAT a, AVFLOAT b)	{ return sqrt(a*a+b*b); }

void CCamera::GetCameraPos_Lobby(AVULONG nSetupId, AVFLOAT fAspect, CAMPARAMS &cp)
{
	if (nSetupId > 7) nSetupId = 0;

	AVFLOAT nStoreyHeight = min(m_pBuilding->GetStorey(m_nStorey)->GetBox().Height(), 2 * m_nTripodHeight);
	AVFLOAT fEyeHeight = min(nStoreyHeight - 20, m_nTripodHeight);

	// eye position
	cp.m_pEyeRef = m_pBaseBone;
	switch (nSetupId)
	{
	case 0: cp.eye = Vector(m_box.Left(), m_box.Front(), fEyeHeight); break;		// rear left
	case 1: cp.eye = Vector(m_box.CenterX(), m_box.Front(), fEyeHeight); break;	// rear centre
	case 2: cp.eye = Vector(m_box.Right(), m_box.Front(), fEyeHeight); break;	// rear right
	case 3: cp.eye = Vector(m_box.Right(), m_box.CenterY(), fEyeHeight); break;	// right side
	case 4: cp.eye = Vector(m_box.Right(), m_box.Rear(), fEyeHeight); break;	// front right
	case 5: cp.eye = Vector(m_box.CenterX(), m_box.Rear(), fEyeHeight); break;	// front centre
	case 6: cp.eye = Vector(m_box.Left(), m_box.Rear(), fEyeHeight); break;	// front left
	case 7: cp.eye = Vector(m_box.Left(), m_box.CenterY(), fEyeHeight); break;	// left side
	}

	AVFLOAT fTargetDist;								// distance to the point the camera is looking at
	AVFLOAT nTargetHeight = m_nTripodHeight * 2 / 4;	// height of the point the camera is looking at
	AVFLOAT fHalfwayDist;		// distance to the location used to setup FOV vertically, roughly halfway to the back of the room
	AVFLOAT fRealFOV;			// real FOV

	switch (nSetupId)
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

	switch (nSetupId)
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
		
	cp.fClipNear = 10;
	cp.fClipFar = 10000;
	cp.fAspectRatio = fAspect;
	cp.fZoom = 0;
}

void CCamera::GetCameraPos_Overhead(AVFLOAT fAspect, CAMPARAMS &cp)
{
	// top centre
	cp.fPan = 0;
	cp.fTilt = -(AVFLOAT)M_PI / 2;
	cp.fHFOV = (AVFLOAT)(M_PI) / 4;
	cp.fVFOV = 0;
	AVFLOAT fEyeHeight1 = 1.25f * m_box.Depth() / 2 / tan(cp.fHFOV/2);
	AVFLOAT fEyeHeight2 = 1.10f * m_box.Width() / fAspect / 2 / tan(cp.fHFOV/2);
	AVFLOAT fEyeHeight = max(fEyeHeight1, fEyeHeight2);
	cp.m_pEyeRef = m_pBaseBone;
	cp.eye = Vector(m_box.CenterX(), m_box.CenterY(), fEyeHeight);
	AVFLOAT nStoreyHeight = m_pBuilding->GetStorey(m_nStorey)->GetBox().Height();
	cp.fClipNear = fEyeHeight - nStoreyHeight + 20;
	if (cp.fClipNear < 10) cp.fClipNear = 10;
	cp.fClipFar = 10000;
	cp.fAspectRatio = fAspect;
	cp.fZoom = 0;
}


void CCamera::GetCameraPos_Lift(AVULONG nLiftId, AVFLOAT fAspect, CAMPARAMS &cp)
{
	BOX box = m_pBuilding->GetShaft(nLiftId)->GetBoxCar();
	AVFLOAT nCarHeight = box.Height();
	AVFLOAT fEyeHeight = min(nCarHeight - 20, m_nTripodHeight);
	AVFLOAT fTargetDist = box.Depth();					// distance to the point the camera is looking at
	AVFLOAT nTargetHeight = m_nTripodHeight * 2 / 4;	// height of the point the camera is looking at

	AVFLOAT fLine = (m_pBuilding->GetShaft(nLiftId)->GetShaftLine() == 0) ? 1.0f : -1.0f;
	
	cp.m_pEyeRef = m_pBaseBone;
	cp.eye = Vector(box.Width()/2, box.Depth()+fLine*10, fEyeHeight);
	cp.fHFOV = 0;
	cp.fVFOV = (AVFLOAT)M_PI / 2;
	cp.fTilt = -atan2(fEyeHeight - nTargetHeight, abs(fTargetDist));
	cp.fPan = (fLine > 0) ? (AVFLOAT)M_PI : 0;

	cp.fClipNear = 10;
	cp.fClipFar = 10000;
	cp.fAspectRatio = fAspect;
	cp.fZoom = 0;
}

void CCamera::GetCameraPos_Ext(AVULONG nPos, AVFLOAT fAspect, CAMPARAMS &cp)
{
	BOX lbox = m_pBuilding->GetShaft(0)->GetBoxCar();

	AVFLOAT yFront = (m_pBuilding->GetShaftLinesCount() == 1) ? m_box.Rear() : m_pBuilding->GetShaft(m_pBuilding->GetShaftCount(0))->GetBox().RearExt();
	AVFLOAT yRear  = m_pBuilding->GetShaft(0)->GetBox().RearExt();
	AVFLOAT nHeight = m_pBuilding->GetStorey(m_pBuilding->GetStoreyCount() - 1)->GetLevel() + m_pBuilding->GetStorey(m_pBuilding->GetStoreyCount() - 1)->GetHeight();

	cp.m_pEyeRef = m_pBaseBone;
	switch (nPos)
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

	cp.fClipNear = 10;
	cp.fClipFar = 10000;
	cp.fAspectRatio = fAspect;
	cp.fZoom = 0;
}

void CCamera::GetCameraPos_Storey(AVULONG nStorey, AVFLOAT &fZRelMove)
{
	CBuilding::STOREY *pStorey = GetBuilding()->GetStorey(m_nStorey);

	AVVECTOR pos;
	GetCurLocalPos(pos);
	bool bOnTripod = pos.z <= m_nTripodHeight;
	AVFLOAT h = bOnTripod ? pos.z : pStorey->GetHeight() - pos.z;

	fZRelMove = -pos.z - pStorey->GetLevel();

	pStorey = GetBuilding()->GetStorey(nStorey);
	if (!bOnTripod) h = max(pStorey->GetHeight() - h, m_nTripodHeight);

	fZRelMove += pStorey->GetLevel() + h;
}

void CCamera::MoveTo(CAMPARAMS &cp)
{
	Reset();
	Move(cp.eye.x, cp.eye.y, cp.eye.z, cp.m_pEyeRef);
	Pan((AVFLOAT)M_PI + cp.fPan);
	Tilt(cp.fTilt);
	m_pCamera->PutPerspective(cp.FOV(), cp.fClipNear, cp.fClipFar, 0);
}

void CCamera::MoveToLobby(AVULONG nSetupId, AVFLOAT fAspect)
{
	if (m_camloc == CAMLOC_LIFT) 
		SetStorey(m_nLiftStorey);

	GetCameraPos_Lobby(nSetupId, fAspect, m_cp);
	m_camloc = CAMLOC_LOBBY;

	MoveTo(m_cp);
	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::MoveToOverhead(AVFLOAT fAspect)
{
	if (m_camloc == CAMLOC_LIFT) 
		SetStorey(m_nLiftStorey);

	GetCameraPos_Overhead(fAspect, m_cp);
	m_camloc = CAMLOC_OVERHEAD;

	MoveTo(m_cp);
	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::MoveToLift(AVULONG nLiftId, AVFLOAT fAspect)
{
	SetLift(nLiftId);

	GetCameraPos_Lift(nLiftId, fAspect, m_cp);
	m_camloc = CAMLOC_LIFT;

	MoveTo(m_cp);
	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::MoveToExt(AVULONG nPos, AVFLOAT fAspect)
{
	SetStorey(0);

	GetCameraPos_Ext(nPos, fAspect, m_cp);
	m_camloc = CAMLOC_OUTSIDE;

	MoveTo(m_cp);
	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::MoveToStorey(AVULONG nStorey)
{
	AVFLOAT z;
	GetCameraPos_Storey(nStorey, z);
	SetStorey(nStorey);
	Move(0, 0, z);
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

void CCamera::AnimateTo(IAction *pTickSource, CAMPARAMS &cp)
{
	IAction *pAction;

	pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"MoveTo", pTickSource, 0, 600, GetHandle(), *(FWVECTOR*)&cp.eye, cp.m_pEyeRef);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	ITransform *pT = NULL;
	GetHandle()->CreateCompatibleTransform(&pT);

	pT->FromRotationZ((AVFLOAT)M_PI + cp.fPan);
	pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"RotateTo", pTickSource, 0, 1000, GetHandle(), pT, NOBONE);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	pT->FromRotationX(cp.fTilt);
	pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"RotateTo", pTickSource, 500, 500, m_pCamera, pT, NOBONE);
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

	if (cp.FOV())
	{
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"Generic", pTickSource, 1000, 500, m_pCamera, pT, GetHandle());
		m_pCamera->GetPerspective(&m_fFOV, NULL, NULL, NULL);
		GetCamera()->PutPerspective(m_fFOV, cp.fClipNear, cp.fClipFar, 0);
		pAction->SetHandleEventHook((HANDLE_EVENT_HOOK_FUNC)_callback_fun, 0, this);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
	}
	
	pT->Release();
}

void CCamera::AnimateToLobby(IAction *pTickSource, AVULONG nSetupId, AVFLOAT fAspect)
{
	if (m_camloc == CAMLOC_LIFT) 
		SetStorey(m_nLiftStorey);

	if (m_camloc == CAMLOC_OVERHEAD)
		AnimateUndoOverhead(pTickSource, fAspect);

	GetCameraPos_Lobby(nSetupId, fAspect, m_cp);
	m_camloc = CAMLOC_LOBBY;

	AnimateTo(pTickSource, m_cp);
	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::AnimateToOverhead(IAction *pTickSource, AVFLOAT fAspect)
{
	if (m_camloc == CAMLOC_LIFT) 
		SetStorey(m_nLiftStorey);

	GetCameraPos_Overhead(fAspect, m_cp);

	if (m_camloc == CAMLOC_OVERHEAD) 
	{
		AnimateTo(pTickSource, m_cp);
		m_bMoved = m_bRotated = m_bZoomed = false;
	}
	else
	{
		m_camloc = CAMLOC_OVERHEAD;

		IAction *pAction = NULL;

		// first stage - move under the ceiling and move down
		AVVECTOR eye1 = { m_cp.eye.x, m_cp.eye.y, m_pBuilding->GetStorey(m_nStorey)->GetLevel() + m_pBuilding->GetStorey(m_nStorey)->GetBox().Height() };

		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"MoveTo", pTickSource, 0, 600, GetHandle(), *(FWVECTOR*)&eye1, NULL);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

		ITransform *pT = NULL;
		GetHandle()->CreateCompatibleTransform(&pT);

		pT->FromRotationZ((AVFLOAT)M_PI + m_cp.fPan);
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"RotateTo", pTickSource, 0, 1000, GetHandle(), pT, NOBONE);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

		pT->FromRotationX(m_cp.fTilt);
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"RotateTo", pTickSource, 500, 500, m_pCamera, pT, NOBONE);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

		pT->Release();

		// second stage: move up, change the fClipNear
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"MoveTo", pTickSource, 1000, 1000, GetHandle(), *(FWVECTOR*)&m_cp.eye, m_cp.m_pEyeRef);
		m_pCamera->GetPerspective(&m_fFOV, &m_fClipNear, NULL, NULL); 
		pAction->SetHandleEventHook((HANDLE_EVENT_HOOK_FUNC)_callback_fun_oh, 0, this);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

		// third step: adjust zoom
		pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"Generic", pTickSource, pAction, 500);
		pAction->SetHandleEventHook((HANDLE_EVENT_HOOK_FUNC)_callback_fun, 0, this);
		pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);

		m_bMoved = m_bRotated = m_bZoomed = false;
	}
}

void CCamera::AnimateUndoOverhead(IAction *pTickSource, AVFLOAT fAspect)
{
}

void CCamera::AnimateToLift(IAction *pTickSource, AVULONG nLiftId, AVFLOAT fAspect)
{
	SetLift(nLiftId);

	if (m_camloc == CAMLOC_OVERHEAD)
		AnimateUndoOverhead(pTickSource, fAspect);

	GetCameraPos_Lift(nLiftId, fAspect, m_cp);
	m_camloc = CAMLOC_LIFT;

//	AVVECTOR eye1 = m_cp.eye;
//	m_pBaseBone->LtoG((FWVECTOR*)&eye1);
//	m_cp.eye.z = eye1.z;

	AnimateTo(pTickSource, m_cp);
	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::AnimateToExt(IAction *pTickSource, AVULONG nPos, AVFLOAT fAspect)
{
	SetStorey(0);

	if (m_camloc == CAMLOC_OVERHEAD)
		AnimateUndoOverhead(pTickSource, fAspect);

	GetCameraPos_Ext(nPos, fAspect, m_cp);

	AnimateTo(pTickSource, m_cp);
	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::AnimateToStorey(IAction *pTickSource, AVULONG nStorey)
{
	AVFLOAT z;
	GetCameraPos_Storey(nStorey, z);
	SetStorey(nStorey);

	IAction *pAction = (IAction*)FWCreateObjWeakPtr(pTickSource->FWDevice(), L"Action", L"Move", pTickSource, 0, 1000, GetHandle(), 0, 0, z );
	pAction->SetEnvelope(ACTION_ENV_PARA, 0.3f, 0.3f);
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
	AVLONG nStorey = 0, nLift = 0, nLiftStorey = 0;

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
		m_nLiftPos[iRow] = GetBuilding()->GetShaftIndex(iRow);
		while (m_nLiftPos[iRow] < (AVLONG)GetBuilding()->GetShaftIndex(iRow) + (AVLONG)GetBuilding()->GetShaftCount(iRow) && !GetBuilding()->GetShaft(m_nLiftPos[iRow])->InWidth(pos.x))
			m_nLiftPos[iRow]++;
		if (m_nLiftPos[iRow] == (AVLONG)GetBuilding()->GetShaftIndex(iRow) + (AVLONG)GetBuilding()->GetShaftCount(iRow))
			if (iRow == 0 && pos.x < 0 || iRow == 1 && pos.x > 0)
				m_nLiftPos[iRow] = (AVLONG)GetBuilding()->GetShaftIndex(iRow) - 1;
	}

	if (pos.z < GetBuilding()->GetStorey(0)->GetLevel())
	{
		camloc = CAMLOC_BELOW;
		nStorey = 0;
		nLift = -1;
	}
	else if (pos.z >= GetBuilding()->GetStorey(GetBuilding()->GetStoreyCount() - 1)->GetRoofLevel())
	{
		camloc = CAMLOC_ABOVE;
		nStorey = GetBuilding()->GetStoreyCount() - 1;
		nLift = -1;
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
			nLift = -1;
		}
		else
		{
			while (nLift < (AVLONG)GetBuilding()->GetShaftCount() && !GetBuilding()->GetShaft(nLift)->InBox(pos))
				nLift++;
			if (nLift < (AVLONG)GetBuilding()->GetShaftCount())
			{
				AVFLOAT H = GetBuilding()->GetShaft(nLift)->GetBoxCar().Height();
				AVFLOAT Z = GetBuilding()->GetLiftZPos(nLift);
				if (GetBuilding()->GetShaft(nLift)->Within(pos.z, GetBuilding()->GetLiftZPos(nLift)))
				{
					camloc = CAMLOC_LIFT;
					nLiftStorey = nStorey;
					nStorey = -1;
				}
				else
				{
					camloc = CAMLOC_SHAFT;
//					nLift = -1;
				}
			}
			else
			{
				camloc = CAMLOC_OUTSIDE;
				nLift = -1;
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
	m_nLiftStorey = nLiftStorey;
}

CAMLOC CCamera::GetDescription(CAMDESC *pDesc)
{
	if (!pDesc) return m_camloc;

	AVLONG ZONE[] = { 0, 1, 2, 7, -1, 3, 6, 5, 4 };
	AVLONG AZIM[] = { 1, 7, 5, 3 };
	AVLONG AZIMb[] = { 1, 2, 0, 3 };

	pDesc->floor = m_nStorey - GetBuilding()->GetBasementStoreyCount();
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
		pDesc->index = m_nLift + 1;
		break;
	case CAMLOC_SHAFT:
		pDesc->index = m_nLift + 1;
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
	case CAMLOC_LIFT:		_snwprintf(m_buf, nSize, L"Camera %d: Lift %d at %s", nId, desc.index, pFloorName); break;
	case CAMLOC_SHAFT:		_snwprintf(m_buf, nSize, L"Camera %d: Lift shafts at %s", nId, pFloorName); break;
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
