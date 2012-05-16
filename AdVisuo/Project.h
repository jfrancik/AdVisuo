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
	// Loading Phase
	enum PHASE { PHASE_NONE, PHASE_PRJ, PHASE_SIM, PHASE_BLD, PHASE_STRUCT, PHASE_SIMDATA };

	std::vector<CSim*> m_sims;
	std::vector<CBuilding*> m_buildings;
	std::vector<PHASE> m_phases;

	int m_nDefault;

public:
	CProject();
	virtual ~CProject();

	CSim *GetSim()					{ return m_sims[m_nDefault]; }
	CBuilding *GetBuilding()		{ return m_buildings[m_nDefault]; }
	CSim *GetSim(int i)				{ return m_sims[i]; }
	CBuilding *GetBuilding(int i)	{ return m_buildings[i]; }

	int GetDefault()				{ return m_nDefault; }
	void SetDefault(int n)			{ m_nDefault = n; }
	int GetSize()					{ return m_sims.size(); }
	
	int Append();

	// XML Load/Store/Parse/Feed --- throw _com_error or _sim_errror
	void LoadFromBuf(LPCOLESTR pBuf, AVULONG nLiftGroup)					{ Load(pBuf, nLiftGroup); }
	void LoadFromFile(LPCOLESTR pFileName, AVULONG nLiftGroup)				{ Load((std::wstring)pFileName, nLiftGroup); }
	void Load(xmltools::CXmlReader reader, AVULONG nLiftGroup);

	void StoreToFile(LPCOLESTR pFileName)									{ Store((std::wstring)pFileName); }
	void StoreToBuf(LPOLESTR pBuffer, size_t nSize)							{ ASSERT(FALSE); } // not implemented at the moment
	void Store(xmltools::CXmlWriter writer);

	static void LoadIndexFromBuf(LPCOLESTR pBuf, vector<CProject*> &prjs)		{ LoadIndex(pBuf, prjs); }
	static void LoadIndexFromFile(LPCOLESTR pFileName, vector<CProject*> &prjs)	{ LoadIndex((std::wstring)pFileName, prjs); }
	static void LoadIndex(xmltools::CXmlReader reader, vector<CProject*>&);
};
