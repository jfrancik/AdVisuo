// BaseProject.h - AdVisuo Common Source File

#pragma once

#include "DBTools.h"

class CLiftGroup;
class CSim;

class CProject : public dbtools::CCollection
{
	AVULONG m_nSimulationId;		// original (console) simulation id

	AVULONG m_nId;					// project id
	AVULONG m_nAVVersionId;			// AV Version id

	// simulation time
	AVLONG m_nMaxTime;				// simulation maximum time
	AVLONG m_nTimeSaved;			// simulation saved time

	std::vector<CLiftGroup*> m_groups;

public:
	CProject();
	virtual ~CProject();

	AVULONG GetSimulationId()					{ return m_nSimulationId; }
	AVULONG GetId()								{ return m_nId; }
	AVULONG GetAVVersionId()					{ return m_nAVVersionId; }
	
	void SetSimulationId(AVULONG n)				{ m_nSimulationId = n; }
	void SetId(AVULONG n)						{ m_nId = n; }
	void SetAVVersionId(AVULONG n)				{ m_nAVVersionId = n; }

	// Time-related functions
	AVLONG GetMaxTime()							{ return m_nMaxTime; }	// simulation maximum time
	void SetMaxTime(AVLONG t)					{ m_nMaxTime = t; }
	AVLONG GetTimeSaved()						{ return m_nTimeSaved; }			// simulation saved time
	void SetTimeSaved(AVLONG t)					{ m_nTimeSaved = t; }

	AVLONG GetSimulationStartTime();

	AVULONG GetMaxStoreyCount();
	AVULONG GetMaxBasementStoreyCount();
	AVULONG GetMaxShaftCount();
	
	enum PRJ_INFO { PRJ_NAME, PRJ_NUMBER, PRJ_LIFT_DESIGNER, PRJ_CHECKED_BY, PRJ_CLIENT_NAME, PRJ_BUILDING_NAME, PRJ_CITY, PRJ_COUNTY, PRJ_COUNTRY, PRJ_LB_RGN, PRJ_POST_CODE,
					PRJ_FOLDER_NAME,
					PRJ_CREATED_BY, PRJ_CREATED_DATE, PRJ_MODIFIED_BY, PRJ_MODIFIED_DATE,
					SIM_NAME, SIM_COMMENTS,
					SIM_CREATED_BY, SIM_CREATED_DATE, SIM_MODIFIED_BY, SIM_MODIFIED_DATE,
					};

	std::wstring GetProjectInfo(PRJ_INFO what);

	// Lift Groups
	CLiftGroup *AddLiftGroup();
	CLiftGroup *GetLiftGroup(int i)				{ return m_groups[i]; }		// get lift group by index
	CLiftGroup *FindLiftGroup(int i);										// find lift group by id
	AVULONG GetLiftGroupsCount()				{ return m_groups.size(); }
	std::vector<CLiftGroup*> &GetLiftGroups()	{ return m_groups; }		// get lift group collection

	// Sims
	CSim *FindSim(int id);													// find lift group by id

	// toolkit
	void ResolveMe();

	void Scale(AVFLOAT fScale);
	void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);

protected:
	virtual CLiftGroup *CreateLiftGroup(AVULONG iIndex) = 0;
};

