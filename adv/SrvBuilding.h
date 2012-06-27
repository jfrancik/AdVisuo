// Building.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrBuilding.h"
#include "../CommonFiles/DBTools.h"

class CBuildingSrv : public CBuildingConstr
{
public:
	CBuildingSrv(CProject *pProject, AVULONG nIndex) : CBuildingConstr(pProject, nIndex)	{ }

public:
	// IO
	HRESULT Store(dbtools::CDataBase db);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nSimID);
};
