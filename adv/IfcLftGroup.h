// IfcLftGroup.h - a part of the AdVisuo Server Module

#pragma once

#include "SrvLftGroup.h"

class CElemIfc;

class CLftGroupIfc : public CLftGroupSrv
{
public:
	CLftGroupIfc(CProject *pProject, AVULONG nIndex) : CLftGroupSrv(pProject, nIndex) { }

	CElemIfc *GetElement()																{ return (CElemIfc*)CLftGroupSrv::GetElement(); }

	CElemIfc *GetStoreyElement(AVULONG nStorey)											{ return (CElemIfc*)CLftGroupSrv::GetStoreyElement(nStorey); }
	CElemIfc *GetMachineRoomElement()													{ return (CElemIfc*)CLftGroupSrv::GetMachineRoomElement(); }
	CElemIfc *GetPitElement()															{ return (CElemIfc*)CLftGroupSrv::GetPitElement(); }
	
	CElemIfc *GetLiftElement(AVULONG nLift)												{ return (CElemIfc*)CLftGroupSrv::GetLiftElement(nLift); }
	CElemIfc *GetLiftDeck(AVULONG nLift, AVULONG nDeck)									{ return (CElemIfc*)CLftGroupSrv::GetLiftDeck(nLift, nDeck); }
	CElemIfc *GetLiftDoor(AVULONG nLift, AVULONG nDoor)									{ return (CElemIfc*)CLftGroupSrv::GetLiftDoor(nLift, nDoor); }

	CElemIfc *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return (CElemIfc*)CLftGroupSrv::GetShaftElement(nStorey, nShaft); }
	CElemIfc *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)					{ return (CElemIfc*)CLftGroupSrv::GetShaftElementLobbySide(nStorey, nShaft); }
	CElemIfc *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return (CElemIfc*)CLftGroupSrv::GetShaftElementLeft(nStorey, nShaft); }
	CElemIfc *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)						{ return (CElemIfc*)CLftGroupSrv::GetShaftElementRight(nStorey, nShaft); }
	CElemIfc *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return (CElemIfc*)CLftGroupSrv::GetShaftElementLeftOrRight(nStorey, nShaft, n); }
	CElemIfc *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return (CElemIfc*)CLftGroupSrv::GetShaftDoor(nStorey, nShaft, nDoor); }
};
