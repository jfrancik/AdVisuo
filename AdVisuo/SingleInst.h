// SingleInstance.h : header file
// Implements single instance management object 
// uses shared memory to store window handle of the 1st instance

#ifndef __SINGLEINSTANCE_H
#define __SINGLEINSTANCE_H

/////////////////////////////////////////////////////////////////////////////
// CSingleInstance class

// Verify				verifies if CSingleInstance created ok
// GetPreviousInstance	returns handle of the main window of previous instance or NULL if none
// CallPreviousInstance	activates previous instance and brings it to front - return as GetPreviousInstance
//						does nothing if there is no previous instance
// SendMessage			sends message to main window of the previous instance
// PostMessage			posts message to main window of the previous instance
//						Both functions can pass an extra parameter xParam, up to 256 (SI_MAX_USER_DATA) bytes
//						nLen is the size of such param, if 0 param is supposed to be 0-terminated
// GetXParam			retrieves an xParam sent from second instance by Send/PostMessage
// EstablishInstance	first instance must call this function passing its main window

// internal structure of shared memory
#define SI_MAX_USER_DATA	256
struct SHARE
{
	HWND hWnd;
	UINT nUserSize;
	char pUserData[SI_MAX_USER_DATA];
};

class CSingleInstance : public CObject
{
// Construction
public:
	CSingleInstance(LPCTSTR lpName);
	virtual ~CSingleInstance();

	BOOL Verify();
	HWND GetPreviousInstance();
	HWND CallPreviousInstance();
	LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam, PVOID xParam, UINT nLen = 0);
	BOOL PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam, PVOID xParam, UINT nLen = 0);
	BOOL GetXParam(int &nLen, PVOID &xParam);
	void EstablishInstance(HWND hWnd);
	void ReleaseInstance();
protected:
	void CSingleInstance::SetUserData(PVOID xParam, UINT nLen);	// used by Send/PostMessage
	void CSingleInstance::ResetUserData();						// called by GetXParam

protected:
	HANDLE m_hShare;
	SHARE *m_pShare;
};

#endif