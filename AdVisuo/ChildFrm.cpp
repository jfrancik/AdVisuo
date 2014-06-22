// ChildFrm.cpp - a part of the AdVisuo Client Software

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

// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "AdVisuo.h"
#include "AdVisuoView.h"
#include "AdVisuoDoc.h"

#include "ChildFrm.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
//	ON_WM_SIZE()
//	ON_COMMAND_RANGE(ID_LAYOUT_SINGLE, ID_LAYOUT_QUADRUPLE, &CChildFrame::OnLayout)
//	ON_UPDATE_COMMAND_UI_RANGE(ID_LAYOUT_SINGLE, ID_LAYOUT_QUADRUPLE, &CChildFrame::OnUpdateLayout)
//	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
}

CChildFrame::~CChildFrame()
{
}

/*
BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	if (pContext && pContext->m_pCurrentFrame)
		m_nLayoutId = ((CChildFrame*)pContext->m_pCurrentFrame)->m_nLayoutId;

	CRect rect;
	if (pContext && pContext->m_pCurrentFrame)
		pContext->m_pCurrentFrame->GetClientRect(&rect);
	else
		GetClientRect(&rect);
	rect.right /= 2; rect.bottom /= 2;

	CAdVisuoDoc *pDoc = (CAdVisuoDoc*)pContext->m_pCurrentDoc;

	CAdVisuoView *pView;
m_nLayoutId = ID_LAYOUT_SINGLE;
	switch (m_nLayoutId)
	{
	case ID_LAYOUT_SINGLE - ID_LAYOUT_SINGLE:
		pView = (CAdVisuoView*)CreateView(pContext, AFX_IDW_PANE_FIRST);
		if (!pView) return FALSE;
		return TRUE;
	case ID_LAYOUT_HORIZONTAL - ID_LAYOUT_SINGLE:
		if (!m_wndSplitter.CreateStatic(this, 2, 1)) 
			return FALSE;
		if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext)
		 || !m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext))
		{
			m_wndSplitter.DestroyWindow();
			return FALSE;
		}
		return TRUE;
	case ID_LAYOUT_VERTICAL - ID_LAYOUT_SINGLE:
		if (!m_wndSplitter.CreateStatic(this, 1, 2)) 
			return FALSE;
		if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext)
		 || !m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext))
		{
			m_wndSplitter.DestroyWindow();
			return FALSE;
		}
		return TRUE;
	case ID_LAYOUT_QUADRUPLE - ID_LAYOUT_SINGLE:
		if (!m_wndSplitter.CreateStatic(this, 2, 2)) 
			return FALSE;
		if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext)
		 || !m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext)
		 || !m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext)
		 || !m_wndSplitter.CreateView(1, 1, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext))
		{
			m_wndSplitter.DestroyWindow();
			return FALSE;
		}
		return TRUE;
	case ID_LAYOUT_TRIPLE - ID_LAYOUT_SINGLE:
		if (!m_wndSplitter.CreateStatic(this, 1, 2)) 
			return FALSE;
		if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CAdVisuoView), CSize(300, 300), pContext))
		{
			m_wndSplitter.DestroyWindow();
			return FALSE;
		}
		m_wndSplitter.SetColumnInfo(0, 300, 0);
		m_wndSplitter2.CreateStatic(&m_wndSplitter, 2, 1, WS_CHILD | WS_VISIBLE, m_wndSplitter.IdFromRowCol(0, 1));
		if (!m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext)
		 || !m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext))
		{
			m_wndSplitter2.DestroyWindow();
			return FALSE;
		}
		return TRUE;
	case ID_LAYOUT_TRIPLE32778 - ID_LAYOUT_SINGLE:
		if (!m_wndSplitter.CreateStatic(this, 1, 2)) 
			return FALSE;
		if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CAdVisuoView), CSize(300, 300), pContext))
		{
			m_wndSplitter.DestroyWindow();
			return FALSE;
		}
		m_wndSplitter.SetColumnInfo(0, 900, 0);
		m_wndSplitter2.CreateStatic(&m_wndSplitter, 2, 1, WS_CHILD | WS_VISIBLE, m_wndSplitter.IdFromRowCol(0, 0));
		if (!m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext)
		 || !m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CAdVisuoView), rect.Size(), pContext))
		{
			m_wndSplitter2.DestroyWindow();
			return FALSE;
		}
		return TRUE;
	default:
		return CreateView(pContext, AFX_IDW_PANE_FIRST) != NULL;
	}
}
*/

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame message handlers

/*void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChildWndEx::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(&rect);
}

void CChildFrame::OnLayout(UINT nCmd)
{
	AfxGetMainWnd()->LockWindowUpdate();

	bool bNewFrame = GetKeyState(VK_SHIFT) < 0;

	ULONG nMyLayoutId = m_nLayoutId;
	m_nLayoutId = nCmd - ID_LAYOUT_SINGLE;

	CAdVisuoDoc *pDoc = ((CAdVisuoDoc*)GetActiveDocument());
	CFrameWnd* pFrame = pDoc->GetDocTemplate()->CreateNewFrame(pDoc, this);
	pFrame->InitialUpdateFrame(pDoc, TRUE);

	if (!bNewFrame)
		DestroyWindow();
	else
		m_nLayoutId = nMyLayoutId;

	AfxGetMainWnd()->UnlockWindowUpdate();
}

void CChildFrame::OnUpdateLayout(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
	pCmdUI->SetCheck(m_nLayoutId == pCmdUI->m_nID - ID_LAYOUT_SINGLE);
}

void CChildFrame::OnDestroy()
{
	CMDIChildWndEx::OnDestroy();

	// TODO: Add your message handler code here
}
*/