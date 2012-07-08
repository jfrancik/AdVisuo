// Building.h - a part of the AdVisuo Server Module

#pragma once

#include "SrvBuilding.h"
#include "ifc/baseIfcElement.h"

class CElemIfc;

class CBuildingIfc : public CBuildingSrv
{
public:
	CBuildingIfc(CProject *pProject, AVULONG nIndex) : CBuildingSrv(pProject, nIndex) { }

	CElemIfc *GetElement()																{ return (CElemIfc*)CBuildingSrv::GetElement(); }

	CElemIfc *GetStoreyElement(AVULONG nStorey)											{ return (CElemIfc*)CBuildingSrv::GetStoreyElement(nStorey); }
	CElemIfc *GetMachineRoomElement()													{ return (CElemIfc*)CBuildingSrv::GetMachineRoomElement(); }
	CElemIfc *GetPitElement()															{ return (CElemIfc*)CBuildingSrv::GetPitElement(); }
	
	CElemIfc *GetLiftElement(AVULONG nLift)												{ return (CElemIfc*)CBuildingSrv::GetLiftElement(nLift); }
	CElemIfc *GetLiftDeck(AVULONG nLift, AVULONG nDeck)									{ return (CElemIfc*)CBuildingSrv::GetLiftDeck(nLift, nDeck); }
	CElemIfc *GetLiftDoor(AVULONG nLift, AVULONG nDoor)									{ return (CElemIfc*)CBuildingSrv::GetLiftDoor(nLift, nDoor); }

	CElemIfc *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return (CElemIfc*)CBuildingSrv::GetShaftElement(nStorey, nShaft); }
	CElemIfc *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)					{ return (CElemIfc*)CBuildingSrv::GetShaftElementLobbySide(nStorey, nShaft); }
	CElemIfc *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return (CElemIfc*)CBuildingSrv::GetShaftElementLeft(nStorey, nShaft); }
	CElemIfc *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)						{ return (CElemIfc*)CBuildingSrv::GetShaftElementRight(nStorey, nShaft); }
	CElemIfc *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return (CElemIfc*)CBuildingSrv::GetShaftElementLeftOrRight(nStorey, nShaft, n); }
	CElemIfc *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return (CElemIfc*)CBuildingSrv::GetShaftDoor(nStorey, nShaft, nDoor); }
};
