// Camera.h

#pragma once

#include "../CommonFiles/Box.h"
#include "VisElem.h"
#include "VisLiftGroup.h"

class CEngine;
class CLiftGroupVis;

// camera locations
enum CAMLOC
{
	CAMLOC_LOBBY, CAMLOC_STOREY, CAMLOC_LIFTGROUP, CAMLOC_OVERHEAD, CAMLOC_LIFT, CAMLOC_SHAFT, CAMLOC_OUTSIDE, CAMLOC_BELOW, CAMLOC_ABOVE, CAMLOC_UNKNOWN
};

enum { CAMLOC_NEXT = 10000, CAMLOC_PREV = 9999 };

// camera description - for the external use
struct CAMDESC
{
	AVLONG floor;
	CAMLOC camloc;
	AVLONG index;
	bool bExact;
};

class CCamera
{
	// specifies camera params - for positioning and animation (internal use)
	struct CAMPARAMS
	{
		CAMLOC camloc;			// camera location
		AVULONG nId;			// lift id for CAMLOC_LIFT, storey id otherwise
		AVFLOAT fAspectRatio;	// aspect ratio

		AVVECTOR eye;			// camera eye position
		AVFLOAT fPan;			// camera pan angle
		AVFLOAT fTilt;			// camera tilt angle
		AVFLOAT fHFOV, fVFOV;	// fields of view (zoom): horizontal and vertical angle
		AVFLOAT fZoom;			// user-imposed zoom
		AVFLOAT fClipNear;		// clip near value
		AVFLOAT fClipFar;		// clip far value

		// Camera Eye-Reference object - Weak Reference (no need for Release)
		HBONE EyeRef(CLiftGroupVis *pLiftGroup)			{ return camloc == CAMLOC_LIFT ? pLiftGroup->GetLiftElement(nId)->GetBone() : pLiftGroup->GetStoreyElement(nId)->GetBone(); }
		AVFLOAT FOV()									{ return max(fHFOV, fVFOV) + fZoom; }
	};

	friend class CScriptEvent;

	// The Engine & Project
	CEngine *m_pEngine;
	CProjectVis *m_pProject;

	// Lift Group information
	AVULONG m_nId;					// id
	XBOX m_box;						// the lobby box (no height)
	AVFLOAT m_nTripodHeight;		// reference height (max tripod height, equal to lift door height)
	
	// Camera position & state information
	CAMLOC m_camloc;				// camera location information
	AVLONG m_camXZone, m_camYZone;	// sub-zone information for the lobby area (-1 .. 3 in each direction, 0..2 is within the lobby)
	AVLONG m_camZone;				// sub-zone information: 9 indexed areas within the lobby only
	AVLONG m_camAzim;				// azimuth information for the area
	AVLONG m_nShaftPos[2];			// x-coordinate in relation to shafts, shown as shaft index for row 0 and 1, may be out of bounds (e.g. -1) if camera to the left or 
	AVULONG m_nLiftGroup;			// lift group index
	AVLONG m_nStorey;				// storey index (-1 if in a lift)
	AVLONG m_nShaft;				// shaft index (-1 if outside the shafts)
	AVLONG m_nLift;					// lift index (-1 if outside the lifts)
	AVULONG m_nLiftStorey;			// storey index - used only when in lift (and nStorey == -1)
	CAMPARAMS m_cp;					// full camera params details
	bool m_bMoved, m_bRotated, m_bZoomed;	// all clear if the camera is as descibed in m_cp; set when moved, rotated or zoomed

	// FreeWill objects
	HBONE m_pBaseBone;				// base bone (a part of the construction on which the camera is mounted)
	HBONE m_pHandleBone;			// camera handle, used to move and pan
	HCAMERA m_pCamera;				// camera, autonomically tilts but makes no other transformations

	// data used by text description function
	CAMDESC m_desc;
	wchar_t m_buf[256];

public:
	CCamera();
	~CCamera();
	
	AVULONG GetId()											{ return m_nId; }
	void SetId(AVULONG nId)									{ m_nId = nId; }

	bool IsReady()											{ return m_pCamera != NULL; }

	HCAMERA GetCamera()										{ return m_pCamera; }
	HBONE GetHandle()										{ return m_pHandleBone; }

	// creation
	bool Create(CEngine *pEngine, CProjectVis *pProject, AVULONG nId, AVULONG nLiftGroup, AVLONG nStorey);

	// location checker
	void CheckLocation();

	// Camera Motions
	void MoveTo(CAMLOC camloc, AVULONG nId, AVFLOAT fAspect = 1)								{ MoveTo(GetDefCameraParams(camloc, nId, fAspect)); }
	void AnimateTo(CAMLOC camloc, AVULONG nId, AVFLOAT fAspect = 1, AVULONG nTime = 1000)		{ AnimateTo(GetDefCameraParams(camloc, nId, fAspect), nTime); }

	void Pan(AVFLOAT f);
	void Tilt(AVFLOAT f);
	void Zoom(AVFLOAT f);
	void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z, HBONE pRef = NULL);

	// adjust the camera after aspect ratio change
	void Adjust(AVFLOAT fAspect);

	// camera information
	CAMLOC GetDescription(CAMDESC *pDesc = NULL);
	LPTSTR GetTextDescription();
	LPTSTR GetShortTextDescription();

	AVLONG GetLiftGroup()				{ return m_nLiftGroup; }
	AVLONG GetStorey()					{ return m_camloc == CAMLOC_LIFT ? m_nLiftStorey : m_nStorey; }
	AVLONG GetShaft()					{ return m_nShaft; }
	AVLONG GetLift()					{ return m_nLift; }
	CAMLOC GetLoc()						{ return m_camloc; }
	AVLONG GetZone()					{ return m_camZone; }
	AVLONG GetXZone()					{ return m_camXZone; }
	AVLONG GetYZone()					{ return m_camYZone; }
	AVLONG GetAzimuth()					{ return m_camAzim; }
	AVLONG GetShaftPos(AVULONG nRow)	{ return m_nShaftPos[nRow]; }

private:
	// Camera Motions
	void MoveTo(CAMPARAMS &cp);
	void AnimateTo(CAMPARAMS &cp, AVULONG nTime);

	// Attachement
	void SetBaseBone(HBONE pNode, bool bKeepCoord = true);
	void SetLiftGroup(AVULONG nGroup, bool bKeepCoord = true);	// attaches camera to the lift group and the lobby box
	void SetStorey(AVLONG nStorey, bool bKeepCoord = true);		// attaches camera to the storey
	void SetLift(AVLONG nLift, bool bKeepCoord = true);			// attaches camera to the lift

	// Location services
	CAMPARAMS GetCameraParams();
	CAMPARAMS GetDefCameraParams(CAMLOC camloc, AVULONG nId, AVFLOAT fAspect);
};

