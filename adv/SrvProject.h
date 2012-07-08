// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrProject.h"
#include "SrvBuilding.h"
#include "SrvSim.h"

class CProjectSrv : public CProjectConstr
{
public:
	CProjectSrv();

	virtual CElem *CreateElement(CBuilding *pBuilding, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec)
											{ return NULL; }

	CSimSrv *GetSim()						{ return (CSimSrv*)CProjectConstr::GetSim(); }
	CBuildingSrv *GetBuilding()				{ return (CBuildingSrv*)CProjectConstr::GetBuilding(); }
	CSimSrv *GetSim(int i)					{ return (CSimSrv*)CProjectConstr::GetSim(i); }
	CBuildingSrv *GetBuilding(int i)		{ return (CBuildingSrv*)CProjectConstr::GetBuilding(i); }

	// Database operations
	HRESULT FindProjectID(dbtools::CDataBase db, ULONG nSimulationId, ULONG &nProjectID);
	HRESULT LoadFromConsole(dbtools::CDataBase db, ULONG nSimulationId);
	HRESULT LoadFromVisualisation(dbtools::CDataBase db, ULONG nProjectId);
	HRESULT Store(dbtools::CDataBase db);
	HRESULT Update(dbtools::CDataBase db, AVLONG nTime = -1);

	static HRESULT CleanUp(dbtools::CDataBase db, ULONG nSimulationId);
	static HRESULT CleanUpAll(dbtools::CDataBase db);
	static HRESULT DropTables(dbtools::CDataBase db);

	HRESULT LoadSim(dbtools::CDataBase db, AVULONG nSimulationId);
	
	void Play()										{ for each (CSimSrv *pSim in m_sims) pSim->Play(); }

	void Scale(AVFLOAT fScale)						{ for each (CSimSrv *pSim in m_sims) pSim->GetBuilding()->Scale(fScale); }
	void Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z)		{ for each (CSimSrv *pSim in m_sims) pSim->GetBuilding()->Scale(x, y, z); }
	void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)		{ for each (CSimSrv *pSim in m_sims) pSim->GetBuilding()->Move(x, y, z); }

protected:
	virtual CBuilding *CreateBuilding(AVULONG nIndex)					{ return new CBuildingSrv(this, nIndex); }
	virtual CSim *CreateSim(CBuilding *pBuilding, AVULONG nIndex)		{ return new CSimSrv(pBuilding, nIndex); }
};
