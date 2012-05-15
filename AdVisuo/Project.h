// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/BaseClasses.h"
#include "../CommonFiles/XMLTools.h"
#include "sim.h"
using namespace std;

class _prj_error
{
public:
	enum ERROR_CODES { 
		E_PRJ_NOT_FOUND = 0x80000100, 
		E_PRJ_NO_BUILDING,
		E_PRJ_PASSENGERS, 
		E_PRJ_LIFTS, 
		E_PRJ_FLOORS, 
		E_PRJ_LIFT_DECKS,
		E_PRJ_FILE_STRUCT,
		E_PRJ_INTERNAL };
	_prj_error(enum ERROR_CODES err_code)	{ _error = err_code; }
	enum ERROR_CODES Error()				{ return _error; }
	std::wstring ErrorMessage();
private:
	enum ERROR_CODES _error;
};

class CProject : public CProjectBase
{
	CSim *m_pSim;

	// Loading Phase
	enum PHASE { PHASE_NONE, PHASE_PRJ, PHASE_SIM, PHASE_BLD, PHASE_STRUCT, PHASE_SIMDATA } m_phase;

public:
	CProject(CSim *pSim);

	CSim *GetSim()					{ return m_pSim; }
	CBuilding *GetBuilding()		{ return GetSim()->GetBuilding(); }

	// XML Load/Store/Parse/Feed --- throw _com_error or _sim_errror
	void LoadFromBuf(LPCOLESTR pBuf)										{ Load(pBuf); }
	void LoadFromFile(LPCOLESTR pFileName)									{ Load((std::wstring)pFileName); }
	void Load(xmltools::CXmlReader reader);

	void StoreToFile(LPCOLESTR pFileName)									{ Store((std::wstring)pFileName); }
	void StoreToBuf(LPOLESTR pBuffer, size_t nSize)							{ ASSERT(FALSE); } // not implemented at the moment
	void Store(xmltools::CXmlWriter writer);

	static void LoadIndexFromBuf(LPCOLESTR pBuf, vector<CProject*> &prjs)		{ LoadIndex(pBuf, prjs); }
	static void LoadIndexFromFile(LPCOLESTR pFileName, vector<CProject*> &prjs)	{ LoadIndex((std::wstring)pFileName, prjs); }
	static void LoadIndex(xmltools::CXmlReader reader, vector<CProject*>&);
};
