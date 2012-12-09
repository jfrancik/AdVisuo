// Camera.h

#pragma once


#include "../CommonFiles/Box.h"
#include "VisProject.h"
#include "VisLiftGroup.h"

class CLiftGroupVis;
interface IKineNode;
interface ISceneCamera;
interface IAction;
interface ISceneCamera;

// camera locations
enum CAMLOC
{
	CAMLOC_LOBBY, CAMLOC_STOREY, CAMLOC_OVERHEAD, CAMLOC_LIFT, CAMLOC_SHAFT, CAMLOC_OUTSIDE, CAMLOC_BELOW, CAMLOC_ABOVE, CAMLOC_UNKNOWN
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
	CAMLOC camloc;			// camera location
	AVULONG nId;			// lift id for CAMLOC_LIFT, storey id otherwise
	AVFLOAT fAspectRatio;	// aspect ratio

	AVVECTOR eye;			// camera eye position
	AVFLOAT fPan;			// camera pan angle
	AVFLOAT fTilt;			// camera tilt angle
	AVFLOAT fHFOV, fVFOV;	// fiels of view (zoom): horizontal and vertical angle
	AVFLOAT fZoom;			// user-imposed zoom
	AVFLOAT fClipNear;		// clip near value
	AVFLOAT fClipFar;		// clip far value

	// Camera Eye-Reference object - Weak Reference (no need for Release)
	IKineNode *EyeRef(CLiftGroupVis *pLiftGroup)	{ return camloc == CAMLOC_LIFT ? pLiftGroup->GetLiftElement(nId)->GetBone() : pLiftGroup->GetStoreyElement(nId)->GetBone(); }
	AVFLOAT FOV()									{ return max(fHFOV, fVFOV) + fZoom; }
};

class CCamera
{
	// Lift Group information
	CLiftGroupVis *m_pLiftGroup;	// the lift group
	AVULONG m_nId;					// id
	BOX m_box;						// the lobby box (no height)
	AVFLOAT m_nTripodHeight;		// reference height (max tripod height, equal to lift door height)
	
	// Camera position & state information
	CAMLOC m_camloc;				// camera location information
	AVLONG m_camXZone, m_camYZone;	// sub-zone information for the lobby area (-1 .. 3 in each direction, 0..2 is within the lobby)
	AVLONG m_camZone;				// sub-zone information: 9 indexed areas within the lobby only
	AVLONG m_camAzim;				// azimuth information for the area
	AVLONG m_nShaftPos[2];			// x-coordinate in relation to shafts, shown as shaft index for row 0 and 1, may be out of bounds (e.g. -1) if camera to the left or 
	AVLONG m_nStorey;				// storey index (-1 if in a lift)
	AVLONG m_nShaft;				// shaft index (-1 if outside the shafts)
	AVLONG m_nLift;					// lift index (-1 if outside the lifts)
	AVULONG m_nLiftStorey;			// storey index - used only when in lift (and nStorey == -1)
	CAMPARAMS m_cp;					// full camera params details
	bool m_bMoved, m_bRotated, m_bZoomed;	// all clear if the camera is as descibed in m_cp; set when moved, rotated or zoomed

	// FreeWill objects
	IKineNode *m_pBaseBone;			// base bone (a part of the construction on which the camera is mounted)
	IKineNode *m_pHandleBone;		// camera handle, used to move and pan
	ISceneCamera *m_pCamera;		// camera, autonomically tilts but makes no other transformations

	// data used by animation callback functions
	AVFLOAT m_fFOV, m_fClipNear;

	// data used by text description function
	CAMDESC m_desc;
	wchar_t m_buf[256];

public:
	CCamera(CLiftGroupVis *pLiftGroup, AVULONG nId);
	~CCamera();

	CLiftGroupVis *GetLiftGroup()							{ return m_pLiftGroup; }
	void SetLiftGroup(CLiftGroupVis *pLiftGroup);			// sets lift group and the lobby box
	AVULONG GetId()											{ return m_nId; }
	void SetId(AVULONG nId)									{ m_nId = nId; }

	void SetBaseBone(IKineNode *pNode, bool bKeepCoord = true);

	void SetStorey(AVLONG nStorey, bool bKeepCoord = true);	// attaches camera to the storey
	void SetLift(AVLONG nLift, bool bKeepCoord = true);		// attaches camera to the lift

	ISceneCamera *GetCamera()								{ return m_pCamera; }
	IKineNode *GetHandle()									{ return m_pHandleBone; }

	bool IsReady()											{ return m_pCamera != NULL; }

	void GetCurPos(AVVECTOR &pos);
	void GetCurLocalPos(AVVECTOR &pos);
	
	CAMPARAMS GetCameraParams();
	CAMPARAMS GetDefCameraParams(CAMLOC camloc, AVULONG nId, AVFLOAT fAspect);

	bool Create();
	bool Destroy();

	// Camera Motions
	void MoveTo(CAMPARAMS &cp);
	void MoveTo(CAMLOC camloc, AVULONG nId, AVFLOAT fAspect = 1)		{ MoveTo(GetDefCameraParams(camloc, nId, fAspect)); }

	void AnimateTo(IAction *pTickSource, CAMPARAMS &cp, AVULONG nTime);
	void AnimateTo(CAMLOC camloc, IAction *pTickSource, AVULONG nId, AVFLOAT fAspect = 1, AVULONG nTime = 1000)	{ AnimateTo(pTickSource, GetDefCameraParams(camloc, nId, fAspect), nTime); }

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
	LPTSTR GetShortTextDescription();

	AVLONG GetStorey()				{ return m_camloc == CAMLOC_LIFT ? m_nLiftStorey : m_nStorey; }
	AVLONG GetShaft()				{ return m_nShaft; }
	AVLONG GetLift()				{ return m_nLift; }
	CAMLOC GetLoc()					{ return m_camloc; }
	AVLONG GetZone()				{ return m_camZone; }
	AVLONG GetXZone()				{ return m_camXZone; }
	AVLONG GetYZone()				{ return m_camYZone; }
	AVLONG GetAzimuth()				{ return m_camAzim; }
	AVLONG GetShaftPos(AVULONG nRow)	{ return m_nShaftPos[nRow]; }

	// animation callback functions
	friend int _callback_fun(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, CCamera *pCamera);
	friend int _callback_fun_oh(struct ACTION_EVENT *pEvent, IAction *pAction, AVULONG nParam, CCamera *pCamera);
};

