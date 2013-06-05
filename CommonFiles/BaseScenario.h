// BaseScenario.h - AdVisuo Common Source File

#pragma once

#include "DBTools.h"

class CProject;
class CSim;

class CScenario : public dbtools::CCollection
{
	AVULONG m_nScenarioId;			// original (console) traffic scenario id
	AVULONG m_nId;					// scenario id
	CProject *m_pProject;
	std::wstring m_name;			// scenario name

	std::vector<CSim*> m_sims;		// collections of sims with this scenario - across all lift groups

public:
	CScenario(CProject *pProject);
	virtual ~CScenario();

	AVULONG GetScenarioId()						{ return m_nScenarioId; }
	AVULONG GetId()								{ return m_nId; }
	std::wstring GetName()						{ return m_name; }

	void SetScenarioId(AVULONG n)				{ m_nScenarioId = n; }
	void SetId(AVULONG n)						{ m_nId = n; }
	void SetName(std::wstring name)				{ m_name = name; }

	AVULONG GetSimCount()						{ return m_sims.size(); }
	std::vector<CSim*> &GetSims()				{ return m_sims; }

	void AddSim(CSim *pSim)						{ m_sims.push_back(pSim); }
	void ResolveMe();
};

