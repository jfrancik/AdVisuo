// Reporting Events to the System Logs
// Based on http://www.codeproject.com/KB/system/mctutorial.aspx

#pragma once

#include "messages.h"
#include <comdef.h>

extern HMODULE g_hModule;

void AddEventSource(HMODULE hModule, PCTSTR pszName, DWORD dwCategoryCount = 0);

// generic formatted version
DWORD Logf(DWORD dwEventID, LPCTSTR fmt, ...);

// just error code
inline DWORD Log(DWORD dwEventID)					{ return Logf(dwEventID, L""); }

// up to 4 strings (no formatted version!)
DWORD Log(DWORD dwEventID, LPCTSTR pStr1, LPCTSTR pStr2 = NULL, LPCTSTR pStr3 = NULL, LPCTSTR pStr4 = NULL);

// COM HRESULT code
DWORD Logf(DWORD dwEventID, HRESULT h, LPCTSTR fmt, ...);
inline DWORD Log(DWORD dwEventID, HRESULT h)		{ return Logf(dwEventID, h, L""); }

// Database error
DWORD Logf(DWORD dwEventID, _com_error &ce, LPCTSTR fmt, ...);
inline DWORD Log(DWORD dwEventID, _com_error &ce)	{ return Logf(dwEventID, ce, L""); }

// Passenger-traffic specific errors and warnings
DWORD Logf(DWORD dwEventID, bool bGetOn, DWORD PassengerId, DWORD Time, DWORD Floor, DWORD Lift, DWORD Deck, LPCTSTR fmt, ...);
inline DWORD Log(DWORD dwEventID, bool bGetOn, DWORD PassengerId, DWORD Time, DWORD Floor, DWORD Lift, DWORD Deck)
													{ return Logf(dwEventID, bGetOn, PassengerId, Time, Floor, Lift, Deck, L""); }
// Reopenings specific warning
DWORD Logf(DWORD dwEventID, DWORD nReopenings, DWORD Time, DWORD nTruncatedVal, LPCTSTR fmt, ...);
inline DWORD Log(DWORD dwEventID, DWORD nReopenings, DWORD Time, DWORD nTruncatedVal)
													{ return Logf(dwEventID, nReopenings, Time, nTruncatedVal, L""); }

class CLogTime
{
	DWORD m_nTime;
public:
	CLogTime();
	DWORD Log(LPCTSTR pStr);
	void Reset();
};
