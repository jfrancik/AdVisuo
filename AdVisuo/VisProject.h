// Project.h - a part of the AdVisuo Server Module

#pragma once

#include "../CommonFiles/ConstrProject.h"
#include "../CommonFiles/XMLTools.h"

class CLiftGroupVis;
class CSimVis;
class CEngine;

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
		E_PRJ_NOT_AUTHORISED,
		E_PRJ_INTERNAL };
	_prj_error(enum ERROR_CODES err_code)	{ _error = err_code; }
	enum ERROR_CODES Error()				{ return _error; }
	std::wstring ErrorMessage();
private:
	enum ERROR_CODES _error;
};

class CProjectVis : public CProjectConstr
{
	CEngine *m_pEngine;
public:
	CProjectVis()							{ m_pEngine = NULL; }
	virtual ~CProjectVis()					{ }

	// Implementation
	virtual CLiftGroup *CreateLiftGroup(AVULONG iIndex);
	virtual CElem *CreateElement(CLiftGroup *pLiftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec);

	CLiftGroupVis *GetLiftGroup(int i)		{ return (CLiftGroupVis*)CProjectConstr::GetLiftGroup(i); }
	CLiftGroupVis *FindLiftGroup(int id)	{ return (CLiftGroupVis*)CProjectConstr::FindLiftGroup(id); }
	CLiftGroupVis *AddLiftGroup()			{ return (CLiftGroupVis*)CProjectConstr::AddLiftGroup(); }

	CSimVis *FindSim(int id)				{ return (CSimVis*)CProjectConstr::FindSim(id); }
	
	// Engine specific
	void SetEngine(CEngine *pEngine)		{ m_pEngine = pEngine; }
	CEngine *GetEngine()					{ return m_pEngine; }
	void StoreConfig();

	// XML Load/Store/Parse/Feed --- throw _com_error or _sim_errror
	void LoadFromBuf(LPCOLESTR pBuf)										{ Load(pBuf); }
	void LoadFromFile(LPCOLESTR pFileName)									{ Load((std::wstring)pFileName); }
	void Load(xmltools::CXmlReader reader);

	void LoadProject(dbtools::CDataBase::SELECT &sel);
	void LoadLiftGroup(dbtools::CDataBase::SELECT &sel);
	void LoadFloor(dbtools::CDataBase::SELECT &sel);
	void LoadShaft(dbtools::CDataBase::SELECT &sel);
	void LoadSim(dbtools::CDataBase::SELECT &sel);
	void LoadJourney(dbtools::CDataBase::SELECT &sel);
	void LoadPassenger(dbtools::CDataBase::SELECT &sel);
	

	void StoreToFile(LPCOLESTR pFileName)									{ Store((std::wstring)pFileName); }
	void StoreToBuf(LPOLESTR pBuffer, size_t nSize)							{ ASSERT(FALSE); } // not implemented at the moment
	void Store(xmltools::CXmlWriter writer);

	static void LoadIndexFromBuf(LPCOLESTR pBuf, std::vector<CProjectVis*> &prjs)		{ LoadIndex(pBuf, prjs); }
	static void LoadIndexFromFile(LPCOLESTR pFileName, std::vector<CProjectVis*> &prjs)	{ LoadIndex((std::wstring)pFileName, prjs); }
	static void LoadIndex(xmltools::CXmlReader reader, std::vector<CProjectVis*>&);

	static void LoadFoldersFromBuf(LPCOLESTR pBuf, std::vector<std::wstring> &folders)			{ LoadFolders(pBuf, folders); }
	static void LoadFoldersFromFile(LPCOLESTR pFileName, std::vector<std::wstring> &folders)	{ LoadFolders((std::wstring)pFileName, folders); }
	static void LoadFolders(xmltools::CXmlReader reader, std::vector<std::wstring> &folders);
};
