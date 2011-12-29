// Camera.h

#pragma once


#include "../CommonFiles/Box.h"
#include "Building.h"

class CBuilding;
interface IKineNode;
interface ISceneCamera;
interface IAction;
interface ISceneCamera;

// camera locations
enum CAMLOC
{
	CAMLOC_LOBBY, CAMLOC_OVERHEAD, CAMLOC_LIFT, CAMLOC_SHAFT, CAMLOC_OUTSIDE, CAMLOC_BELOW, CAMLOC_ABOVE, CAMLOC_UNKNOWN
};

// camera description - for the external use
struct CAMDESC
{
	AVLONG floor;
	CAMLOC camloc;
	AVLONG index;
	bool bExact;
};

// specifies camera params - for positioning and animation (internal use)
struct CAMPARAMS
{
	AVVECTOR eye;			// camera eye position
	IKineNode *m_pEyeRef;	// camera eye reference object (used only for lifts) - WEAK REFERENCE (no AddRef/Release)
	AVFLOAT fPan;			// camera pan angle
	AVFLOAT fTilt;			// camera tilt angle
	AVFLOAT fHFOV, fVFOV;	// fiels of view (zoom): horizontal and vertical angle
	AVFLOAT fZoom;			// user-imposed zoom
	AVFLOAT FOV()			{ return max(fHFOV, fVFOV) + fZoom; }
	AVFLOAT fClipNear;		// clip near value
	AVFLOAT fClipFar;		// clip far value
	AVFLOAT fAspectRatio;	// aspect ratio
};

class CCamera
{
	// Building information
	CBuilding *m_pBuilding;			// the building
	AVULONG m_nId;					// id
	BOX m_box;						// the lobby box (no height)
	AVFLOAT m_nTripodHeight;		// reference height (max tripod height, equal to lift door height)
	
	// Camera position & state information
	CAMLOC m_camloc;				// camera location information
	AVLONG m_camloc2, m_camloc3;	// sub-zone information for m_camloc = CAMLOC_LOBBY or CAMLOC_OUTSIDE
	AVLONG m_nStorey;				// storey index (-1 if in a lift)
	AVLONG m_nLift;					// lift index (-1 if outside the lobby)
	AVULONG m_nLiftStorey;			// storey index - used only when in lift (and nStorey == -1)
	CAMPARAMS m_cp;					// full camera params details
	bool m_bMoved, m_bRotated, m_bZoomed;	// all clear if the camera is as descibed in m_cp; set when moved, rotated or zoomed

	// FreeWill objects
	IKineNode *m_pBaseBone;			// base bone (a part of the building on which the camera is mounted)
	IKineNode *m_pHandleBone;		// camera handle, used to move and pan
	ISceneCamera *m_pCamera;		// camera, autonomically tilts but makes no other transformations

	// data used by animation callback functions
	AVFLOAT m_fFOV, m_fClipNear;

	// data used by text description function
	CAMDESC m_desc;
	wchar_t m_buf[256];

public:
	CCamera(CBuilding *pBuilding, AVULONG nId);
	~CCamera();

	CBuilding *GetBuilding()								{ return m_pBuilding; }
	void SetBuilding(CBuilding *pBuilding);					// sets building and the lobby box
	AVULONG GetId()											{ return m_nId; }
	void SetId(AVULONG nId)									{ m_nId = nId; }

	void SetBaseBone(IKineNode *pNode, bool bKeepCoord = true);

	AVLONG GetStorey()										{ return m_nStorey; }
	void SetStorey(AVLONG nStorey, bool bKeepCoord = true);	// attaches camera to the storey

	AVLONG GetLift()										{ return m_nLift; }
	void SetLift(AVLONG nLift, bool bKeepCoord = true);		// attaches camera to the lift

	ISceneCamera *GetCamera()								{ return m_pCamera; }
	IKineNode *GetHandle()									{ return m_pHandleBone; }

	bool IsReady()											{ return m_pCamera != NULL; }

	void GetCurPos(AVVECTOR &pos);
	void GetCurLocalPos(AVVECTOR &pos);
	
	void GetCameraPos_Lobby(AVULONG nSetupId, AVFLOAT fAspect, CAMPARAMS &cp);
	void GetCameraPos_Overhead(AVFLOAT fAspect, CAMPARAMS &cp);
	void GetCameraPos_Lift(AVULONG nLiftId, AVFLOAT fAspect, CAMPARAMS &cp);
	void GetCameraPos_Ext(AVULONG nPos, AVFLOAT fAspect, CAMPARAMS &cp);
	void GetCameraPos_Storey(AVULONG nStorey, AVFLOAT &fZRelMove);

	bool Create();
	bool Destroy();

	// Camera Motions
	void MoveTo(CAMPARAMS &cp);
	void MoveTo()												{ MoveTo(m_cp); }
	void MoveToLobby(AVULONG nSetupId, AVFLOAT fAspect);
	void MoveToOverhead(AVFLOAT fAspect);
	void MoveToLift(AVULONG nLiftId, AVFLOAT fAspect);
	void MoveToLiftRel(AVLONG n, AVFLOAT fAspect)				{ if (m_nLift >= 0) MoveToLift(GetLift() + n, fAspect); }
	void MoveToExt(AVULONG nPos, AVFLOAT fAspect);
	void MoveToStorey(AVULONG nStorey);
	void MoveToStoreyRel(AVLONG n)								{ if (m_nStorey >= 0) MoveToStorey(GetStorey() + n); }

	void AnimateTo(IAction *pTickSource, CAMPARAMS &cp);
	void AnimateTo(IAction *pTickSource)						{ AnimateTo(pTickSource); }
	void AnimateToLobby(IAction *pTickSource, AVULONG nSetupId, AVFLOAT fAspect);
	void AnimateToOverhead(IAction *pTickSource, AVFLOAT fAspect);
	void AnimateUndoOverhead(IAction *pTickSource, AVFLOAT fAspect);
	void AnimateToLift(IAction *pTickSource, AVULONG nLiftId, AVFLOAT fAspect);
	void AnimateToLiftRel(IAction *pTickSource, AVLONG n, AVFLOAT fAspect)	{ if (m_nLift >= 0) AnimateToLift(pTickSource, GetLift() + n, fAspect); }
	void AnimateToExt(IAction *pTickSource, AVULONG nPos, AVFLOAT fAspect);
	void AnimateToStorey(IAction *pTickSource, AVULONG nStorey);
	void AnimateToStoreyRel(IAction *pTickSource, AVULONG n)				{ AnimateToStorey(pTickSource, GetStorey() + n); }

	void Reset();
	void Pan(AVFLOAT f);
	void Tilt(AVFLOAT f);
	void Zoom(AVFLOAT f);
	void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z, IKineNode *pRef = NULL);

	// adjust the camera after aspect ratio change
	void Adjust(AVFLOAT fAspect);

	// location checker
	void CheckLocation();

	// camera information
	CAMLOC GetDescription(CAMDESC *pDesc = NULL);
	LPTSTR GetTextDescription();

	// animation callback functions
	friend int _callback_fun(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, CCamera *pCamera);
	friend int _callback_fun_oh(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, CCamera *pCamera);
};

