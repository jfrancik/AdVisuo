#pragma once

#include "../CommonFiles/XMLTools.h"

class CXMLRequest;
class CProjectVis;

class CAdVisuoLoader
{
	CXMLRequest *m_pHttp;
	CProjectVis *m_pProject;
	AVLONG m_timeLoaded;

public:
	CAdVisuoLoader(CXMLRequest *pHttp, CProjectVis *pProject);
	~CAdVisuoLoader(void);

	void Load(xmltools::CXmlReader reader);
	void Load(LPCOLESTR pBuf)						{ Load(pBuf); }
	void Execute();
};

