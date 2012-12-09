// IfcLiftGroup.h - a part of the AdVisuo Server Module

#pragma once

#include "SrvLiftGroup.h"

class CElemIfc;

class CLiftGroupIfc : public CLiftGroupSrv
{
public:
	CLiftGroupIfc(CProject *pProject, AVULONG nIndex) : CLiftGroupSrv(pProject, nIndex) { }

	CElemIfc *GetElement()																{ return (CElemIfc*)CLiftGroupSrv::GetElement(); }

	CElemIfc *GetStoreyElement(AVULONG nStorey)											{ return (CElemIfc*)CLiftGroupSrv::GetStoreyElement(nStorey); }
	CElemIfc *GetMachineRoomElement()													{ return (CElemIfc*)CLiftGroupSrv::GetMachineRoomElement(); }
	CElemIfc *GetPitElement()															{ return (CElemIfc*)CLiftGroupSrv::GetPitElement(); }
	
	CElemIfc *GetLiftElement(AVULONG nLift)												{ return (CElemIfc*)CLiftGroupSrv::GetLiftElement(nLift); }
	CElemIfc *GetLiftDeck(AVULONG nLift, AVULONG nDeck)									{ return (CElemIfc*)CLiftGroupSrv::GetLiftDeck(nLift, nDeck); }
	CElemIfc *GetLiftDoor(AVULONG nLift, AVULONG nDoor)									{ return (CElemIfc*)CLiftGroupSrv::GetLiftDoor(nLift, nDoor); }

	CElemIfc *GetShaftElement(AVULONG nStorey, AVULONG nShaft)							{ return (CElemIfc*)CLiftGroupSrv::GetShaftElement(nStorey, nShaft); }
	CElemIfc *GetShaftElementLobbySide(AVULONG nStorey, AVULONG nShaft)					{ return (CElemIfc*)CLiftGroupSrv::GetShaftElementLobbySide(nStorey, nShaft); }
	CElemIfc *GetShaftElementLeft(AVULONG nStorey, AVULONG nShaft)						{ return (CElemIfc*)CLiftGroupSrv::GetShaftElementLeft(nStorey, nShaft); }
	CElemIfc *GetShaftElementRight(AVULONG nStorey, AVULONG nShaft)						{ return (CElemIfc*)CLiftGroupSrv::GetShaftElementRight(nStorey, nShaft); }
	CElemIfc *GetShaftElementLeftOrRight(AVULONG nStorey, AVULONG nShaft, AVULONG n)	{ return (CElemIfc*)CLiftGroupSrv::GetShaftElementLeftOrRight(nStorey, nShaft, n); }
	CElemIfc *GetShaftDoor(AVULONG nStorey, AVULONG nShaft, AVULONG nDoor)				{ return (CElemIfc*)CLiftGroupSrv::GetShaftDoor(nStorey, nShaft, nDoor); }
};
