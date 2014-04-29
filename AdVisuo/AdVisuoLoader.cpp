#include "StdAfx.h"
#include "_base.h"
#include "AdVisuoLoader.h"
#include "VisProject.h"
#include "xmlrequest.h"
#include "AdVisuo.h"


CAdVisuoLoader::CAdVisuoLoader(CXMLRequest *pHttp, CProjectVis *pProject)
{
	m_pHttp = pHttp;
	m_pProject = pProject;
	m_timeLoaded = 0;
}


CAdVisuoLoader::~CAdVisuoLoader(void)
{
}


void CAdVisuoLoader::Load(xmltools::CXmlReader reader)
{
	m_pProject->Load(reader);
}

void CAdVisuoLoader::Execute()
{
	std::wstring response;
	m_pHttp->get_response(response);
	m_pProject->LoadFromBuf(response.c_str());
	
	LONG prevTimeLoaded = m_timeLoaded;
	m_timeLoaded += 60000;
	
	if (m_timeLoaded < m_pProject->GetTimeSaved())
	{
		m_pHttp->take_authorisation_from(AVGetApp()->GetAuthorisationAgent());
		if (m_pHttp->AVIsAuthorised() <= 0)
			throw _prj_error(_prj_error::E_PRJ_NOT_AUTHORISED);
		m_pHttp->AVPrjData(m_pProject->GetId(), m_timeLoaded, m_timeLoaded + 60000);
	}
}
