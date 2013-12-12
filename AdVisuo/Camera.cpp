// Camera.cpp

#include "StdAfx.h"
#include "Camera.h"
#include "VisProject.h"
#include "VisLiftGroup.h"
#include "Engine.h"

namespace fw
{
	#include <freewill.h>
};

#pragma warning (disable:4995)
#pragma warning (disable:4996)
#pragma warning (disable:4244)
 
#define NOBONE ((HBONE)NULL)

CCamera::CCamera()
{
	m_nId = 0;

	m_pBaseBone = NULL;
	m_pHandleBone = NULL;
	m_pCamera = NULL;

	m_nLiftGroup = 0x7ffffff;
	m_nStorey = 0;
	m_nLift = 0;
	m_nShaft = 0;

	m_camloc = CAMLOC_LOBBY;
	m_bMoved = m_bRotated = m_bZoomed = false;
	m_desc.camloc = CAMLOC_UNKNOWN;
}

CCamera::~CCamera()
{
	m_pEngine->DeleteChild(m_pBaseBone, m_pHandleBone);	// not understood why this is necessary!
	if (m_pCamera) ((IUnknown*)m_pCamera)->Release();
	if (m_pBaseBone) ((IUnknown*)m_pBaseBone)->Release();
	if (m_pHandleBone) ((IUnknown*)m_pHandleBone)->Release();
}

bool CCamera::Create(CEngine *pEngine, CProjectVis *pProject, AVULONG nId, AVULONG nLiftGroup, AVLONG nStorey)
{
	m_pEngine = pEngine;
	m_pProject = pProject;

	if (nLiftGroup >= pProject->GetLiftGroupsCount())
		return false;

	SetLiftGroup(nLiftGroup, false);
	SetId(nId);
	SetStorey(nStorey, false);

	ASSERT(m_pHandleBone == NULL && m_pCamera == NULL && pEngine);


	if (!m_pBaseBone)
		return false;

	m_pHandleBone = m_pEngine->CreateChild(m_pBaseBone, L"CameraHandle");
	m_pCamera = m_pEngine->CreateCamera(m_pHandleBone, L"Camera");

	CheckLocation();

	return true;
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
	AVVECTOR pos = CEngine::GetBonePos(m_pHandleBone);

	// first thing: determine which lift group it is!
	AVULONG i = 0;
	while (i < m_pProject->GetLiftGroupsCount() && pos.y < m_pProject->GetLiftGroup(i)->GetTotalAreaBox().FrontExt())
		i++;
	i = min(i, m_pProject->GetLiftGroupsCount()-1);
	SetLiftGroup(i);

	CLiftGroupVis *pLiftGroup = m_pProject->GetLiftGroup(m_nLiftGroup);

	// camera azimouth and zone
	m_camAzim = ((AVLONG)((pLiftGroup->GetBox().InBoxAzimuth(pos, true) + M_PI + M_PI/4) / (M_PI/2))) % 4;
	if (pLiftGroup->GetBox().InBoxSection(pos, 3, 3, m_camXZone, m_camYZone))
		m_camZone = m_camXZone + 3 * m_camYZone;
	else
		m_camZone = 9;

	// camera position against the shafts
	for (AVLONG iRow = 0; iRow < 2; iRow++)
	{
		m_nShaftPos[iRow] = pLiftGroup->GetShaftBegin(iRow);
		while (m_nShaftPos[iRow] < (AVLONG)pLiftGroup->GetShaftEnd(iRow) && !pLiftGroup->GetShaft(m_nShaftPos[iRow])->InWidth(pos.x))
			m_nShaftPos[iRow]++;
		if (m_nShaftPos[iRow] == (AVLONG)pLiftGroup->GetShaftEnd(iRow))
			if (iRow == 0 && pos.x < 0 || iRow == 1 && pos.x > 0)
				m_nShaftPos[iRow] = (AVLONG)pLiftGroup->GetShaftBegin(iRow) - 1;
	}

	if (pos.z < pLiftGroup->GetStorey(0)->GetLevel())
	{
		camloc = CAMLOC_BELOW;
		nStorey = 0;
		nShaft = nLift = -1;
	}
	else if (pos.z >= pLiftGroup->GetStorey(pLiftGroup->GetStoreyCount() - 1)->GetRoofLevel())
	{
		camloc = CAMLOC_ABOVE;
		nStorey = pLiftGroup->GetStoreyCount() - 1;
		nShaft = nLift = -1;
	}
	else
	{
		// within the height of the lift group; it may be outside, lobby, shaft or lift
		// first, find the storey...
		while (nStorey < (AVLONG)pLiftGroup->GetStoreyCount() && !pLiftGroup->GetStorey(nStorey)->Within(pos))
			nStorey++;
		ASSERT (nStorey < (AVLONG)pLiftGroup->GetStoreyCount());
		
		if (pLiftGroup->GetBox().InBox(pos))
		{
			camloc = CAMLOC_LOBBY;
			nShaft = nLift = -1;
		}
		else
		{
			nShaft = 0;
			while (nShaft < (AVLONG)pLiftGroup->GetShaftCount() && !pLiftGroup->GetShaft(nShaft)->InBox(pos))
				nShaft++;
			if (nShaft < (AVLONG)pLiftGroup->GetShaftCount())
			{
				// we're in a shaft - check for the lifts
				nLift = 0;
				while (nLift < (AVLONG)pLiftGroup->GetLiftCount() && !pLiftGroup->GetLift(nLift)->Within(pos, pLiftGroup->GetLiftPos(nLift)))
					nLift++;
				if (nLift < (AVLONG)pLiftGroup->GetLiftCount())
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

	if (camloc == CAMLOC_LIFT && (m_camloc != CAMLOC_LIFT || nLift != m_nLift))
		SetLift(nLift);
	if (camloc != CAMLOC_LIFT && (m_camloc == CAMLOC_LIFT || nStorey != m_nStorey))
		SetStorey(nStorey);

	m_camloc = camloc;
	m_nStorey = nStorey;
	m_nLift = nLift;
	m_nShaft = nShaft;
	m_nLiftStorey = nLiftStorey;
}

void CCamera::Pan(AVFLOAT f)
{
	if (!m_pCamera || !m_pHandleBone) return;
	CEngine::RotateBoneZ(m_pHandleBone, f);
	m_bRotated = true;
}

void CCamera::Tilt(AVFLOAT f)
{
	if (!m_pCamera) return;
	CEngine::RotateBoneX((HBONE)m_pCamera, f);
	m_bRotated = true;
}

void CCamera::Zoom(AVFLOAT f)
{
	if (!m_pCamera) return;

	//m_cp.fZoom -= f;
	m_cp.fHFOV -= f;
	m_cp.fVFOV -= f;

	CEngine::SetCameraPerspective(m_pCamera, m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, m_cp.fAspectRatio);

	m_bZoomed = true;
}

void CCamera::Move(AVFLOAT x, AVFLOAT y, AVFLOAT z, HBONE pRef)
{
	if (!m_pCamera || !m_pHandleBone) return;
	CEngine::MoveBone(m_pHandleBone, x, y, z);
	m_bMoved = true;
}

void CCamera::Adjust(AVFLOAT fNewAspectRatio)
{
	m_cp.fHFOV = 2 * atan(tan(m_cp.fHFOV/2) * m_cp.fAspectRatio / fNewAspectRatio);
	m_cp.fAspectRatio = fNewAspectRatio;
	CEngine::SetCameraPerspective(m_pCamera, m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, m_cp.fAspectRatio);
}

CAMLOC CCamera::GetDescription(CAMDESC *pDesc)
{
	if (!pDesc) return m_camloc;

	AVLONG ZONE[] = { 0, 1, 2, 7, -1, 3, 6, 5, 4 };
	AVLONG AZIM[] = { 1, 7, 5, 3 };
	AVLONG AZIMb[] = { 1, 2, 0, 3 };

	pDesc->floor = GetStorey() - m_pProject->GetLiftGroup(m_nLiftGroup)->GetBasementStoreyCount();
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
		pDesc->floor = m_nLiftStorey - m_pProject->GetLiftGroup(m_nLiftGroup)->GetBasementStoreyCount();
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

	CString strFloor = m_pProject->GetLiftGroup(m_nLiftGroup)->GetStorey(desc.floor + m_pProject->GetLiftGroup(m_nLiftGroup)->GetBasementStoreyCount())->GetName().c_str();
	strFloor.Trim();
	if (strFloor.GetLength() >= 1 && strFloor.GetLength() <= 3 && strFloor[0] >= L'0' && strFloor[0] <= L'9')
		strFloor = CString(L"Floor ") + strFloor;
	CString strFloor1 = strFloor;
	if (!strFloor.IsEmpty())
		strFloor += L", ";

	CString strGroup;
	if (m_pProject->GetLiftGroupsCount() > 1) strGroup = m_pProject->GetLiftGroup(m_nLiftGroup)->GetName().c_str();
	strGroup.Trim();
	if (!strGroup.IsEmpty())
		strGroup += L", ";

	switch (desc.camloc)
	{
	case CAMLOC_LOBBY:
		switch (desc.index)
		{
		case 0: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby, rear left corner", nId, strGroup, strFloor); break;
		case 1: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby, rear side", nId, strGroup, strFloor); break;
		case 2: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby, rear right corner", nId, strGroup, strFloor); break;
		case 3: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby, right side", nId, strGroup, strFloor); break;
		case 4: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby, front right corner", nId, strGroup, strFloor); break;
		case 5: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby, front side", nId, strGroup, strFloor); break;
		case 6: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby, front left corner", nId, strGroup, strFloor); break;
		case 7: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby, left side", nId, strGroup, strFloor); break;
		default: _snwprintf(m_buf, nSize, L"CAM%d: %s%slobby", nId, strGroup, strFloor); break;
		};
		break;
	case CAMLOC_OVERHEAD:	_snwprintf(m_buf, nSize, L"CAM%d: %s%soverhead view", nId, strGroup, strFloor); break;
	case CAMLOC_LIFT:		_snwprintf(m_buf, nSize, L"CAM%d: %sLift %c at %s", nId, strGroup, L'A' + desc.index - 1, strFloor1); break;
	case CAMLOC_SHAFT:		_snwprintf(m_buf, nSize, L"CAM%d: %sShaft %c at %s", nId, strGroup, L'A' + desc.index - 1, strFloor1); break;
	case CAMLOC_OUTSIDE:
		switch (desc.index)
		{
		case 0: _snwprintf(m_buf, nSize, L"CAM%d: %sfront side of the building", nId, strFloor); break;
		case 1: _snwprintf(m_buf, nSize, L"CAM%d: %srear side of the building", nId, strFloor); break;
		case 2: _snwprintf(m_buf, nSize, L"CAM%d: %sleft side of the building", nId, strFloor); break;
		case 3: _snwprintf(m_buf, nSize, L"CAM%d: %sright side of the building", nId, strFloor); break;
		default: _snwprintf(m_buf, nSize, L"CAM%d: %soutside the building", nId, strFloor); break;
		}
		break;
	case CAMLOC_BELOW:		_snwprintf(m_buf, nSize, L"CAM%d: Below the building", nId); break;
	case CAMLOC_ABOVE:		_snwprintf(m_buf, nSize, L"CAM%d: Above the building", nId); break;
	default:				_snwprintf(m_buf, nSize, L"CAM%d: Location unknown", nId); break;
	};
	return m_buf;
}

LPTSTR CCamera::GetShortTextDescription()
{
	CAMDESC desc;
	GetDescription(&desc);

	AVULONG nId = GetId() + 1;
	AVULONG nSize = 256;

	CString pFloorName = m_pProject->GetLiftGroup(m_nLiftGroup)->GetStorey(desc.floor + m_pProject->GetLiftGroup(m_nLiftGroup)->GetBasementStoreyCount())->GetName().c_str();
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

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions

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

	CEngine::ResetBone(m_pHandleBone);
	CEngine::ResetBone((HBONE)m_pCamera);
	Move(cp.eye.x, cp.eye.y, cp.eye.z, cp.EyeRef(m_pProject->GetLiftGroup(m_nLiftGroup)));
	Pan((AVFLOAT)M_PI + cp.fPan);
	Tilt(cp.fTilt);
	CEngine::SetCameraPerspective(m_pCamera, cp.FOV(), cp.fClipNear, cp.fClipFar, 0);

	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::AnimateTo(CAMPARAMS &cp, AVULONG nTime)
{
	m_pEngine->StartCameraAnimation();	
		
	if (m_camloc == CAMLOC_LIFT)
		SetStorey(m_nLiftStorey);
	if (cp.camloc == CAMLOC_LIFT)
		SetLift(cp.nId);
	else
		SetStorey(cp.nId);

	m_camloc = cp.camloc;
	m_cp = cp;

	HACTION a;
	if (cp.camloc == CAMLOC_OVERHEAD) 
	{
		AVVECTOR eye1 = { cp.eye.x, cp.eye.y, m_pProject->GetLiftGroup(m_nLiftGroup)->GetStorey(cp.nId)->GetLevel() + m_pProject->GetLiftGroup(m_nLiftGroup)->GetStorey(cp.nId)->GetBox().Height() };

		a = m_pEngine->MoveCameraTo(GetHandle(), eye1, 0, nTime * 6 / 10, NULL);
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = m_pEngine->PanCamera(GetHandle(), (AVFLOAT)M_PI + cp.fPan, 0, nTime);
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = m_pEngine->TiltCamera((HBONE)m_pCamera, cp.fTilt, nTime/2, nTime/2);
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);

		// second stage: move up, change the fClipNear
		a = m_pEngine->MoveCameraTo(GetHandle(), cp.eye, nTime, nTime/2, cp.EyeRef(m_pProject->GetLiftGroup(m_nLiftGroup)));
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = m_pEngine->ZoomCamera(m_pCamera, -1, m_cp.fClipNear, m_cp.fClipFar, nTime, nTime/3);
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);

		// third step: adjust zoom
		a = m_pEngine->ZoomCamera(m_pCamera, m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, 3*nTime/2, nTime/4);
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
	}
	else
	{
		a = m_pEngine->ZoomCamera(m_pCamera, -1, m_cp.fClipNear, m_cp.fClipFar, 0, nTime/10);

		a = m_pEngine->MoveCameraTo(GetHandle(), cp.eye, 0, nTime * 6 / 10, cp.EyeRef(m_pProject->GetLiftGroup(m_nLiftGroup)));
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = m_pEngine->PanCamera(GetHandle(), (AVFLOAT)M_PI + cp.fPan, 0, nTime);
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		a = m_pEngine->TiltCamera((HBONE)m_pCamera, cp.fTilt, nTime/2, nTime/2);
		m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		if (cp.FOV())
		{
			a = m_pEngine->ZoomCamera(m_pCamera, m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, nTime, nTime/2);
			m_pEngine->SetCameraEnvelope(a, 0.3f, 0.3f);
		}
	}

	m_bMoved = m_bRotated = m_bZoomed = false;
}

void CCamera::SetLiftGroup(AVULONG nGroup, bool bKeepCoord)
{
	if (nGroup == m_nLiftGroup) return;
	m_nLiftGroup = nGroup;
	
	CLiftGroupVis *pLiftGroup = m_pProject->GetLiftGroup(m_nLiftGroup);
	if (!pLiftGroup) return;

	m_box = pLiftGroup->GetBox();

	AVFLOAT nLWall = 0, nRWall = 0;
	if (pLiftGroup->GetLobbyArrangement() == CLiftGroupVis::LOBBY_DEADEND_LEFT)  nLWall = pLiftGroup->GetBox().LeftThickness();
	if (pLiftGroup->GetLobbyArrangement() == CLiftGroupVis::LOBBY_DEADEND_RIGHT) nRWall = pLiftGroup->GetBox().RightThickness();

	m_box = BOX(
		pLiftGroup->GetBox().Left() + 2 + nLWall,
		pLiftGroup->GetBox().Front() + 10,
		pLiftGroup->GetBox().Width() - 4 - nLWall - nRWall, 
		pLiftGroup->GetBox().Depth() - 20);

	m_nTripodHeight = pLiftGroup->GetShaft(0)->GetBoxDoor().Height();

	// new base bone
	SetBaseBone(pLiftGroup->GetStoreyElement(m_nStorey)->GetBone(), bKeepCoord);
}

void CCamera::SetStorey(AVLONG nStorey, bool bKeepCoord)
{
	CLiftGroupVis *pLiftGroup = m_pProject->GetLiftGroup(m_nLiftGroup);
	ASSERT (nStorey >= 0 && (ULONG)nStorey < pLiftGroup->GetStoreyCount());
	if (nStorey < 0 || (ULONG)nStorey >= pLiftGroup->GetStoreyCount())
		return;

	// storey parameters
	m_nStorey = nStorey;

	// new base bone
	SetBaseBone(pLiftGroup->GetStoreyElement(m_nStorey)->GetBone(), bKeepCoord);
}

void CCamera::SetLift(AVLONG nLift, bool bKeepCoord)
{
	CLiftGroupVis *pLiftGroup = m_pProject->GetLiftGroup(m_nLiftGroup);
	ASSERT (nLift >= 0 && (ULONG)nLift < pLiftGroup->GetLiftCount());
	if (nLift < 0 || (ULONG)nLift >= pLiftGroup->GetLiftCount())
		return;

	// shaft/lift parameters
	m_nLift = nLift;
	m_nShaft = pLiftGroup->GetLift(nLift)->GetShaftId();

	// new base bone
	SetBaseBone(pLiftGroup->GetLiftElement(m_nLift)->GetBone(), bKeepCoord);
}

AVFLOAT hypotenuse(AVFLOAT a, AVFLOAT b)	{ return sqrt(a*a+b*b); }

CCamera::CAMPARAMS CCamera::GetDefCameraParams(CAMLOC camloc, AVULONG nId, AVFLOAT fAspect)
{
	CAMPARAMS cp;

	cp.fClipNear = 10;
	cp.fClipFar = 10000;
	cp.fZoom = 0;

	CLiftGroupVis *pLiftGroup = m_pProject->GetLiftGroup(m_nLiftGroup);

	switch (camloc)
	{
	case CAMLOC_STOREY:
		cp = GetCameraParams();
		if (cp.camloc == CAMLOC_LIFT) break;	// cannot change floor while in lift!

		if (nId == CAMLOC_NEXT)
			nId = pLiftGroup->GetFloorUp(GetStorey());
		else if (nId == CAMLOC_PREV)
			nId = pLiftGroup->GetFloorDown(GetStorey());

		cp.nId = nId;

		if (cp.eye.z <= m_nTripodHeight * 1.1)
		{
			// camera mounted on a tripod
			if (cp.eye.z > pLiftGroup->GetStorey(nId)->GetHeight())
				cp.eye.z = pLiftGroup->GetStorey(nId)->GetHeight();
		}
		else
		{
			// if camera mounted below the ceiling (above the standard tripod height)
			cp.eye.z = pLiftGroup->GetStorey(nId)->GetHeight() - pLiftGroup->GetStorey(GetStorey())->GetHeight() + cp.eye.z;

			if (cp.eye.z > pLiftGroup->GetStorey(nId)->GetHeight())
				cp.eye.z = pLiftGroup->GetStorey(nId)->GetHeight();
			if (cp.eye.z < m_nTripodHeight)
				cp.eye.z = m_nTripodHeight;
		}
		
		break;

	case CAMLOC_LIFTGROUP:
		cp = GetCameraParams();

		if (nId == CAMLOC_NEXT)
			nId = GetLiftGroup() + 1;
		else if (nId == CAMLOC_PREV)
			nId = GetLiftGroup() - 1;

		if (cp.camloc != CAMLOC_LOBBY)
		{
			SetLiftGroup(nId);
			cp = GetDefCameraParams(CAMLOC_LOBBY, 6, fAspect);
		}
		else
		{
			// if within the lobby, try to retain the position

			//AVFLOAT fPan;			// camera pan angle
			//AVFLOAT fTilt;			// camera tilt angle
			//AVFLOAT fHFOV, fVFOV;	// fields of view (zoom): horizontal and vertical angle

			float eye_x =  (cp.eye.x - m_box.Left()) / m_box.Width();
			float eye_y = -(cp.eye.y - m_box.Rear()) / m_box.Depth();

			SetLiftGroup(nId);

			cp.eye.x =  eye_x * m_box.Width() + m_box.Left();
			cp.eye.y = -eye_y * m_box.Depth() + m_box.Rear();
		}

		// change to valid floor, if necessary
		pLiftGroup = m_pProject->GetLiftGroup(m_nLiftGroup);
		if (!pLiftGroup->IsStoreyServed(GetStorey()))
		{
			nId = pLiftGroup->GetValidFloor(GetStorey());
			cp.nId = nId;

			if (cp.eye.z <= m_nTripodHeight * 1.1)
			{
				// camera mounted on a tripod
				if (cp.eye.z > pLiftGroup->GetStorey(nId)->GetHeight())
					cp.eye.z = pLiftGroup->GetStorey(nId)->GetHeight();
			}
			else
			{
				// if camera mounted below the ceiling (above the standard tripod height)
				cp.eye.z = pLiftGroup->GetStorey(nId)->GetHeight() - pLiftGroup->GetStorey(GetStorey())->GetHeight() + cp.eye.z;

				if (cp.eye.z > pLiftGroup->GetStorey(nId)->GetHeight())
					cp.eye.z = pLiftGroup->GetStorey(nId)->GetHeight();
				if (cp.eye.z < m_nTripodHeight)
					cp.eye.z = m_nTripodHeight;
			}
		}

		break;

	case CAMLOC_LOBBY:
	{
		cp.camloc = camloc;
		cp.nId = GetStorey();
		cp.fAspectRatio = fAspect;

		if (nId > 7) nId = 0;

		AVFLOAT nStoreyHeight = min(pLiftGroup->GetStorey(GetStorey())->GetBox().Height(), 2 * m_nTripodHeight);
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
		AVFLOAT nStoreyHeight = pLiftGroup->GetStorey(GetStorey())->GetBox().Height();
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

		BOX box = pLiftGroup->GetLift(nId)->GetShaft()->GetBoxCar();
		AVFLOAT nCarHeight = box.Height();
		AVFLOAT fEyeHeight = min(nCarHeight - 20, m_nTripodHeight);
		AVFLOAT fTargetDist = box.Depth();					// distance to the point the camera is looking at
		AVFLOAT nTargetHeight = m_nTripodHeight * 2 / 4;	// height of the point the camera is looking at

		AVFLOAT fLine = (pLiftGroup->GetLift(nId)->GetShaft()->GetShaftLine() == 0) ? 1.0f : -1.0f;
	
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

		BOX lbox = pLiftGroup->GetShaft(0)->GetBoxCar();

		AVFLOAT yFront = (pLiftGroup->GetShaftLinesCount() == 1) ? m_box.Rear() : pLiftGroup->GetShaft(pLiftGroup->GetShaftCount(0))->GetBox().RearExt();
		AVFLOAT yRear  = pLiftGroup->GetShaft(0)->GetBox().RearExt();
		AVFLOAT nHeight = pLiftGroup->GetStorey(pLiftGroup->GetStoreyCount() - 1)->GetLevel() + pLiftGroup->GetStorey(pLiftGroup->GetStoreyCount() - 1)->GetHeight();

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

void CCamera::SetBaseBone(HBONE pNode, bool bKeepCoord)
{
	ASSERT(pNode); if (!pNode) return;

	if (bKeepCoord)
	{
		// modify coordinates so that to keep the camera intact
		fw::ITransform *pT;
		m_pHandleBone->CreateCompatibleTransform(&pT);
		pNode->GetGlobalTransform(pT);
		m_pHandleBone->Transform(pT, fw::KINE_LOCAL | fw::KINE_INVERTED);
		m_pHandleBone->GetParentTransform(pT);
		m_pHandleBone->Transform(pT, fw::KINE_LOCAL | fw::KINE_REGULAR);
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

CCamera::CAMPARAMS CCamera::GetCameraParams()
{
	CAMPARAMS cp;
	memset(&cp, 0, sizeof(cp));

	cp.camloc = m_camloc;
	cp.nId = m_camloc == CAMLOC_LIFT ? GetLift() : GetStorey();

	fw::ITransform *pT = NULL;
	m_pHandleBone->CreateCompatibleTransform(&pT);

	fw::FWVECTOR vecPan = { 0, 1, 0 };
	m_pHandleBone->GetLocalTransform(pT);
	pT->AsVector((fw::FWVECTOR*)&cp.eye);
	pT->ApplyRotationTo(&vecPan);
	cp.fPan = atan2(vecPan.x, -vecPan.y);

	fw::FWVECTOR vecTilt = { 0, 1, 0 };
	m_pCamera->GetLocalTransform(pT);
	pT->ApplyRotationTo(&vecTilt);
	cp.fTilt = atan2(vecTilt.z, vecTilt.y);

	pT->Release();

	AVFLOAT fov;
	m_pCamera->GetPerspective(&fov, &cp.fClipNear, &cp.fClipFar, &cp.fAspectRatio);
	cp.fHFOV = cp.fVFOV = fov;

	return cp;
}

