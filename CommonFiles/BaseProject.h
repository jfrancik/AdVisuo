// BaseProject.h - AdVisuo Common Source File

#pragma once

#include "DBTools.h"

class CLiftGroup;
class CScenario;
class CSim;

class CProject : public dbtools::CCollection
{
	AVULONG m_nSimulationId;		// original (console) simulation id

	AVULONG m_nId;					// project id
	AVULONG m_nAVVersionId;			// AV Version id

	// simulation time
	AVLONG m_nMinSimulationTime;	// simulation minimum time - may be less than zero but not greater than zero
	AVLONG m_nMaxSimulationTime;	// simulation maximum time
	AVLONG m_nTimeSaved;			// simulation saved time

	std::vector<CLiftGroup*> m_groups;
	std::vector<CScenario*> m_scenarios;

public:
	CProject();
	virtual ~CProject();

	AVULONG GetSimulationId()					{ return m_nSimulationId; }
	AVULONG GetId()								{ return m_nId; }
	AVULONG GetAVVersionId()					{ return m_nAVVersionId; }
	static AVULONG GetAVNativeVersionId()		{ return 10900; }
	
	void SetSimulationId(AVULONG n)				{ m_nSimulationId = n; }
	void SetId(AVULONG n)						{ m_nId = n; }
	void SetAVVersionId(AVULONG n)				{ m_nAVVersionId = n; }

	// Time-related functions
	AVLONG GetMinSimulationTime()				{ return m_nMinSimulationTime; }	// simulation minimum time - may be less than zero but not greater than zero
	AVLONG GetMaxSimulationTime()				{ return m_nMaxSimulationTime; }	// simulation maximum time
	AVLONG GetTimeSaved()						{ return m_nTimeSaved; }			// simulation saved time

	void ReportMinSimulationTime(AVLONG n)		{ if (n < m_nMinSimulationTime) m_nMinSimulationTime = n; }		// used to collect time min info
	void ReportMaxSimulationTime(AVLONG n)		{ if (n > m_nMaxSimulationTime) m_nMaxSimulationTime = n; }		// used to collect time max info

	AVULONG GetMaxStoreyCount();
	AVULONG GetMaxBasementStoreyCount();
	AVULONG GetMaxShaftCount();
	
	enum PRJ_INFO { PRJ_PROJECT_NAME, PRJ_BUILDING_NAME, PRJ_LANGUAGE, PRJ_UNITS, PRJ_COMPANY, PRJ_CITY, PRJ_LB_RGN, PRJ_COUNTY, PRJ_DESIGNER, PRJ_COUNTRY, PRJ_CHECKED_BY, PRJ_POST_CODE };
	std::wstring GetProjectInfo(PRJ_INFO what);

	// Lift Groups
	CLiftGroup *AddLiftGroup();
	CLiftGroup *GetLiftGroup(int i)				{ return m_groups[i]; }		// get lift group by index
	CLiftGroup *FindLiftGroup(int i);										// find lift group by id
	AVULONG GetLiftGroupsCount()				{ return m_groups.size(); }
	std::vector<CLiftGroup*> &GetLiftGroups()	{ return m_groups; }		// get lift group collection

	// Scenarios
	CScenario *AddScenario();
	CScenario *GetScenario(int i)				{ return m_scenarios[i]; }	// get scenario by index
	CScenario *FindScenario(int i);											// find scenario by id
	AVULONG GetScenariosCount()					{ return m_scenarios.size(); }
	std::vector<CScenario*> &GetScenarios()		{ return m_scenarios; }		// get scenario collection

	// Sims
	CSim *FindSim(int id);													// find lift group by id

	// toolkit
	void ResolveMe();

	void Scale(AVFLOAT fScale);
	void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);

protected:
	virtual CLiftGroup *CreateLiftGroup(AVULONG iIndex) = 0;
	virtual CScenario *CreateScenario(AVULONG iIndex) = 0;
};

