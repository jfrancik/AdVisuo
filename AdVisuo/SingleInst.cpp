// SingleInst.cpp : implementation file
//

#include "stdafx.h"
#include "SingleInst.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////
// CSingleInstance class

CSingleInstance::CSingleInstance(LPCTSTR lpName)
{
	m_hShare = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 
				0, sizeof(SHARE), lpName);
	ASSERT(m_hShare);
	if (!m_hShare)
		return;

	m_pShare = (SHARE*)MapViewOfFile(m_hShare, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	ASSERT(m_pShare);
}

CSingleInstance::~CSingleInstance()
{
	ReleaseInstance();
}

BOOL CSingleInstance::Verify()
{
	return m_hShare && m_pShare;
}

HWND CSingleInstance::GetPreviousInstance()
{
	if (m_pShare)
		if (IsWindow(m_pShare->hWnd))
			return m_pShare->hWnd;
		else
			return NULL;
	else
		return NULL;
}

HWND CSingleInstance::CallPreviousInstance()
{
	HWND hWnd = GetPreviousInstance();
	if (!hWnd)
		return NULL;

	HWND hChild = ::GetLastActivePopup(hWnd);	// get any pop-up
	::SetForegroundWindow(hWnd);
	if (::IsIconic(hWnd))						// restore if iconic
		::ShowWindow(hWnd, SW_RESTORE);
	if (hWnd != hChild)							// bring popup to top
		::SetForegroundWindow(hChild);

	return hWnd;
}

LRESULT CSingleInstance::SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam, PVOID xParam, UINT nLen)
{
	HWND hWnd = GetPreviousInstance();
	if (hWnd)
	{
		SetUserData(xParam, nLen);
		return ::SendMessage(hWnd, Msg, wParam, lParam);
	}
	else
		return 0;
}

BOOL CSingleInstance::PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam, PVOID xParam, UINT nLen)
{
	HWND hWnd = GetPreviousInstance();
	if (hWnd)
	{
		SetUserData(xParam, nLen);
		return ::PostMessage(hWnd, Msg, wParam, lParam);
	}
	else
		return FALSE;
}

BOOL CSingleInstance::GetXParam(int &nLen, PVOID &xParam)
{
	nLen = 0;
	if (m_pShare)
	{
		nLen = m_pShare->nUserSize;
		xParam = m_pShare->pUserData;
	}
	ResetUserData();
	return nLen;
}

void CSingleInstance::EstablishInstance(HWND hWnd)
{
	if (m_pShare)
		m_pShare->hWnd = hWnd;
}

void CSingleInstance::ReleaseInstance()
{
	if (m_pShare)
		UnmapViewOfFile((LPVOID)m_pShare);
	m_pShare = NULL;
	if (m_hShare)
		CloseHandle(m_hShare);
	m_hShare = NULL;
}

void CSingleInstance::SetUserData(PVOID xParam, UINT nLen)
{
	if (!xParam)
		return;
	if (!nLen)
		nLen = (UINT)strlen((LPCSTR)xParam) + 1;
	if (nLen > SI_MAX_USER_DATA)
		nLen = SI_MAX_USER_DATA;
	ASSERT(m_pShare);
	if (!m_pShare)
		return;

	m_pShare->nUserSize = nLen;
	memcpy(m_pShare->pUserData, xParam, nLen);
}

void CSingleInstance::ResetUserData()
{
	ASSERT(m_pShare);
	if (!m_pShare)
		return;
	m_pShare->nUserSize = 0;
}
