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

#include "stdafx.h"
#include "RibbonScenarioButton.h"
#include "../CommonFiles/BaseProject.h"
#include "../CommonFiles/BaseLiftGroup.h"
#include "../CommonFiles/BaseSimClasses.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable:4100)

const int nImageMargin = 4;
const int nCheckMargin = 2;
const int nTextOffsetX = 11;
const int nTextOffsetY = 2;

const int nBorderMarginX = 1;
const int nBorderMarginY = 3;

bool CRibbonScenarioButton::c_bDirty = false;

/////////////////////////////////////////////////////////////////////////////
// CRibbonScenarioButton

IMPLEMENT_DYNCREATE(CRibbonScenarioButton, CMFCRibbonGallery)

CRibbonScenarioButton::CRibbonScenarioButton()
{
	m_sizeMargins = CSize(2, 2);
}

CRibbonScenarioButton::CRibbonScenarioButton(UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, UINT uiImagesPaletteResID, int cxPaletteImage) :
	CMFCRibbonGallery(nID, lpszText, nSmallImageIndex, nLargeImageIndex, CSize(0, 0), 0, TRUE)
{
	SetButtonMode(TRUE);

	m_sizeMargins = CSize(2, 2);
	m_sizeMaxText = CSize(0, 0);

	
	
	m_listImages.Load(uiImagesPaletteResID);

	BITMAP bmp;
	GetObject(m_listImages.GetImageWell(), sizeof(BITMAP), &bmp);

	m_listImages.SetImageSize(CSize(cxPaletteImage, bmp.bmHeight), TRUE);
	m_imagesPalette.SetImageSize(m_listImages.GetImageSize());
}

CRibbonScenarioButton::~CRibbonScenarioButton()
{
}

void CRibbonScenarioButton::SetProject(CProject *pProject)
{
	m_pProject = pProject;

	Clear();
	
	AVULONG nIconsInRow = 0;
	AVULONG i = 0;
	bool bMultiGroup = m_pProject->GetLiftGroupsCount() > 1;
	for each (CLiftGroup *pGroup in m_pProject->GetLiftGroups())
	{
		AVULONG nCount = pGroup->GetSimCount();
		CMFCRibbonGallery::AddGroup(bMultiGroup ? pGroup->GetName().c_str() : L"", nCount);
		if (nCount > nIconsInRow) nIconsInRow = nCount;

		for each (CSim *pSim in pGroup->GetSims())
		{
			m_sims.push_back(pSim);
			CMFCRibbonGallery::SetItemToolTip(i++, pSim->GetScenarioDesc().c_str());
		}
	}

	SetIconsInRow(min(nIconsInRow, 8));

	RemoveAll();
	m_sizeMaxText = CSize(0, 0);
}

void CRibbonScenarioButton::Clear()
{
	CMFCRibbonGallery::Clear();
	m_sizeMaxText = CSize(0, 0);
	m_sims.clear();
	m_groups.clear();
}

void CRibbonScenarioButton::OnShowPopupMenu()
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnShowPopupMenu();

	// initialise m_groups
	m_groups.clear();
	for each (CLiftGroup *pGroup in m_pProject->GetLiftGroups())
		m_groups[pGroup->GetId()] = false;
	

	if (m_sizeMaxText == CSize(0, 0))
	{
		CMFCRibbonBar* pRibbonBar = GetTopLevelRibbonBar();
		ASSERT_VALID(pRibbonBar);

		CClientDC dc(pRibbonBar);

		CFont* pOldFont = dc.SelectObject(pRibbonBar->GetFont());
		ASSERT(pOldFont != NULL);

		int i = 0;

		for each (CSim *pSim in m_sims)
		{
			CSize szText = dc.GetTextExtent(pSim->GetScenarioName().c_str());

			m_sizeMaxText.cx = max(m_sizeMaxText.cx, szText.cx);
			m_sizeMaxText.cy = max(m_sizeMaxText.cy, szText.cy);
		}

		const int cxImage = m_listImages.GetImageSize().cx;

		for (i = 0; i < m_arSubItems.GetSize(); i++)
		{
			CMFCRibbonBaseElement* pButton = m_arSubItems [i];
			ASSERT_VALID(pButton);

			CString strText = pButton->GetText();
			CSize szText = dc.GetTextExtent(strText);

			m_sizeMaxText.cx = max(m_sizeMaxText.cx, szText.cx - cxImage);
		}

		dc.SelectObject(pOldFont);
	}

	CMFCRibbonGallery::OnShowPopupMenu();
}

CSize CRibbonScenarioButton::GetIconSize() const
{
	CSize size = CMFCRibbonGallery::GetIconSize();

	if (size.cx > 0)
		size.cx += 2 *(m_sizeMargins.cx + nCheckMargin) - nImageMargin + nTextOffsetX + m_sizeMaxText.cx + 10;
	else
		size.cx = m_sizeMaxText.cx + 2 *(m_sizeMargins.cy + nCheckMargin + nTextOffsetX);

	if (size.cy > 0)
		size.cy += 2 *(m_sizeMargins.cy - nImageMargin + nCheckMargin);
	else
		size.cy = m_sizeMaxText.cy + 2 * m_sizeMargins.cy;

	return size;
}

void CRibbonScenarioButton::OnDrawPaletteIcon( CDC* pDC, CRect rectIcon, int nIconIndex, CMFCRibbonGalleryIcon* pIcon, COLORREF clrText)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(pIcon);
	ASSERT(nIconIndex >= 0);

	if (nIconIndex >= (int)m_sims.size()) 
		return;
	if (!m_bSmallIcons)
		rectIcon.DeflateRect(nCheckMargin, nCheckMargin, nImageMargin, nCheckMargin);
	
	CSim *pSim = m_sims[nIconIndex];
	bool bChecked = pSim->IsCur();

	CRect rect(rectIcon);
	rect.right  = rect.left + m_listImages.GetImageSize().cx + m_sizeMargins.cx * 2;

	if (bChecked)
		CMFCVisualManager::GetInstance()->OnDrawRibbonMenuCheckFrame(pDC, this, rectIcon);

	m_listImages.DrawEx(pDC, rect, pSim->GetScenarioTypeId(), CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);

	CRect rectText(rect);
	rectText.left  = rectText.right + nTextOffsetX;
	rectText.right = rectIcon.right;

	if (!rectText.IsRectEmpty())
	{
		COLORREF clrOld = pDC->SetTextColor(clrText);
		pDC->DrawText(pSim->GetScenarioName().c_str(), rectText, DT_VCENTER | DT_SINGLELINE | DT_LEFT);
		pDC->SetTextColor(clrOld);
	}
}

void CRibbonScenarioButton::OnClickPaletteIcon(CMFCRibbonGalleryIcon* pIcon)
{
	CMFCRibbonGallery::OnClickPaletteIcon(pIcon);

	int nIndex = pIcon->GetIndex();
	if (nIndex >= 0 && nIndex < (int)m_sims.size())
	{
		SetDirtyFlag();
		CSim *pSim = m_sims[nIndex];
		pSim->SetCur();
		m_groups[pSim->GetLiftGroup()->GetId()] = true;
	}
}

BOOL CRibbonScenarioButton::OnClickPaletteSubItem(CMFCRibbonButton* /*pButton*/, CMFCRibbonPanelMenuBar *pMenuBar) 
{
	pMenuBar->InvalidateRect(NULL, 0);

	// count un-cliked groups:
	int iGroups = std::count_if(m_groups.begin(), m_groups.end(), [] (const std::pair<AVULONG, bool> &p) -> bool { return p.second == false; } );

	if (iGroups == 0)
		return FALSE;	//ClosePopupMenu();

	return TRUE; 
}

