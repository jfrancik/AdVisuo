// Building.h - a part of the AdVisuo Server Module

#pragma once

#include "SrvBuilding.h"
#include "ifc/baseIfcElement.h"

class CElemIfc;
class CBoneIfc;

class CBuildingIfc : public CBuildingSrv
{
public:
	CBuildingIfc(CProject *pProject, AVULONG nIndex) : CBuildingSrv(pProject, nIndex) { }

	CElemIfc *GetElement()																{ return (CElemIfc*)CBuildingSrv::GetElement(); }

	CElemIfc *GetStoreyElement(AVULONG nStorey)											{ return (CElemIfc*)CBuildingSrv::GetStoreyElement(nStorey); }
	CBoneIfc *GetStoreyBone(AVULONG nStorey)											{ return (CBoneIfc*)CBuildingSrv::GetStoreyBone(nStorey); }
	
	CElemIfc *GetLiftElement(AVULONG nLift)												{ return (CElemIfc*)CBuildingSrv::GetLiftElement(nLift); }
	CBoneIfc *GetLiftBone(AVULONG nLift)												{ return (CBoneIfc*)CBuildingSrv::GetLiftBone(nLift); }
	CBoneIfc *GetLiftDeck(AVULONG nLift, AVULONG nDeck)									{ return (CBoneIfc*)CBuildingSrv::GetLiftDeck(nLift, nDeck); }
	CBoneIfc *GetLiftDoor(AVULONG nLift, AVULONG nDoor)									{ return (CBoneIfc*)CBuildingSrv::GetLiftDoor(nLift, nDoor); }

	CElemIfc *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return (CElemIfc*)CBuildingSrv::GetShaftElement(nStorey, nShaft); }
	CElemIfc *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)					{ return (CElemIfc*)CBuildingSrv::GetShaftElementLobbySide(nStorey, nShaft); }
	CElemIfc *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return (CElemIfc*)CBuildingSrv::GetShaftElementLeft(nStorey, nShaft); }
	CElemIfc *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)						{ return (CElemIfc*)CBuildingSrv::GetShaftElementRight(nStorey, nShaft); }
	CElemIfc *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return (CElemIfc*)CBuildingSrv::GetShaftElementLeftOrRight(nStorey, nShaft, n); }
	CBoneIfc *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return (CBoneIfc*)CBuildingSrv::GetShaftDoor(nStorey, nShaft, nDoor); }
};
