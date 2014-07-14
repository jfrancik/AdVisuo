// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#pragma once

#include <vector>
#include <map>

class CProject;
class CSim;

class CRibbonScenarioButton : public CMFCRibbonGallery
{
	CProject *m_pProject;

	std::vector<CSim*> m_sims;	

	CMFCToolBarImages m_listImages;
	CSize m_sizeMargins;
	CSize m_sizeMaxText;

	DECLARE_DYNCREATE(CRibbonScenarioButton)

	std::map<AVULONG, bool> m_groups;	// maps lift group ids to boolean showing if the sim has been chosen for the given group

	static bool c_bDirty;

// Construction:
public:
	CRibbonScenarioButton();
	CRibbonScenarioButton(UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, UINT uiImagesPaletteResID, int cxPaletteImage);
	virtual ~CRibbonScenarioButton();

	static bool IsDirty()			{ return c_bDirty; }
	static void ClearDirtyFlag()	{ c_bDirty = false; }
	static void SetDirtyFlag()		{ c_bDirty = true; }

	void SetProject(CProject *pProject);
	CProject *GetProject()					{ return m_pProject; }

	virtual void Clear();
	//void dupa()		{ OnShowPopupMenu(); }//this->OnP

protected:
	virtual CSize GetIconSize() const;
	virtual void OnDrawPaletteIcon(CDC* pDC, CRect rectIcon, int nIconIndex, CMFCRibbonGalleryIcon* pIcon, COLORREF clrText);
	virtual void OnShowPopupMenu();

	virtual BOOL IsItemMenuLook() const		{ return TRUE; }

	virtual void OnClickPaletteIcon(CMFCRibbonGalleryIcon* pIcon);
	virtual BOOL OnClickPaletteSubItem(CMFCRibbonButton* /*pButton*/, CMFCRibbonPanelMenuBar* /*pMenuBar*/);

	void RecalcTextSizes(CDC* pDC);
};

