#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "DlgHtBase.h"
class CProjectVis;
class CXMLRequest;
namespace xmltools { class CXmlWriter; }

class CDlgHtSelect : public CDlgHtBase
{
	DECLARE_DYNCREATE(CDlgHtSelect)

	std::vector<std::wstring> m_folders;
	std::vector<CProjectVis*> m_prjs;

	std::wstring m_url;

	AVULONG m_nSortModePrj, m_nSortModeSim;
	bool m_bSortAscPrj, m_bSortAscSim;

	AVULONG m_nSimulationToShow;

public:
	ULONG m_nProjectId;			// selection (project id)
	ULONG m_nSimulationId;		// selection (simulation id)

public:
	CDlgHtSelect(std::wstring url = L"(unknown URL)", AVULONG nProjectId = 0, AVULONG nSimulationId = 0, bool bGotoSimulations = false, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgHtSelect();

	void Load(CXMLRequest &http);

	ULONG GetProjectId()			{ return m_nProjectId; }
	ULONG GetSimulationId()			{ return m_nSimulationId; }

	// generates content for a list of projects
	CString GenContent();
	CString GenContent(AVULONG nProjectId);
	CString GenContent(xmltools::CXmlWriter &writer, AVULONG nXslRes);

	// Functions
	void GotoProjects(AVULONG nProjectId);
	void GotoSimulations(AVULONG nProjectId, AVULONG nSimulationId);

	void SelectProject(AVULONG nProjectId);
	void SelectSimulation(AVULONG nSimulationId);

	// Button Handlers
	virtual void OnOK();
	virtual void OnCancel();
	HRESULT OnButtonCancel(IHTMLElement *pElement);
	HRESULT OnButtonOpenProject(IHTMLElement *pElement);
	HRESULT OnButtonOpenSimulation(IHTMLElement *pElement);
	HRESULT OnButtonBack(IHTMLElement *pElement);

	HRESULT OnButtonSortPrjDate(IHTMLElement *pElement);
	HRESULT OnButtonSortPrjName(IHTMLElement *pElement);
	HRESULT OnButtonSortPrjNo(IHTMLElement *pElement);
	HRESULT OnButtonSortSimDate(IHTMLElement *pElement);
	HRESULT OnButtonSortSimName(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_ADV_SELECT, IDH = IDR_HTML_SELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	DECLARE_DISPATCH_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
