// Lift.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include <comdef.h>

using namespace std;

HMODULE g_hModule = NULL;

bool g_bRegEvents = true;
bool g_bOnScreen = false;
bool g_bBenchmark = false;

void AddEventSource(HMODULE hModule, PCTSTR pszName, DWORD dwCategoryCount /* =0 */ )
{
	g_hModule = hModule;

	HKEY hRegKey = NULL; 
	TCHAR szPath[MAX_PATH];
        
	_stprintf_s(szPath, _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s"), pszName);

	// Create the event source registry key
	RegCreateKey(HKEY_LOCAL_MACHINE, szPath, &hRegKey);

	// Name of the PE module that contains the message resource
	GetModuleFileName(hModule, szPath, MAX_PATH);

	// Register EventMessageFile
	RegSetValueEx(hRegKey, L"EventMessageFile", 0, REG_EXPAND_SZ, (PBYTE)szPath, (_tcslen(szPath) + 1) * sizeof TCHAR); 

	// Register supported event types
	DWORD dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 
	RegSetValueEx(hRegKey, _T("TypesSupported"), 0, REG_DWORD, (LPBYTE)&dwTypes, sizeof dwTypes);

	// If we want to support event categories,
	// we have also to register the CategoryMessageFile.
	// and set CategoryCount. Note that categories
	// need to have the message ids 1 to CategoryCount!

	if (dwCategoryCount > 0) 
	{
		RegSetValueEx(hRegKey, _T("CategoryMessageFile"), 0, REG_EXPAND_SZ, (PBYTE)szPath, (_tcslen(szPath) + 1) * sizeof TCHAR);
		RegSetValueEx(hRegKey, _T("CategoryCount"), 0, REG_DWORD, (PBYTE)&dwCategoryCount, sizeof dwCategoryCount);
	}
            
	RegCloseKey( hRegKey );
} 

static void _PrintMessage(DWORD dwEventID, ... )
{
	wchar_t buf[1024];
	va_list args;
	va_start(args, dwEventID);
	FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, g_hModule, dwEventID, LANG_NEUTRAL, buf, 1024, &args);
	wstring str = buf;
	size_t i;
	while ((i = str.find(L"\r\n\r\n")) != str.npos)
		str.replace(i, 4, L"\r\n");
	
	if (dwEventID >= 0x80000000)
		wprintf(L"(Error %d) %ls", dwEventID & 0x3fffffff, str.c_str());
	else
	if (dwEventID >= 0x40000000)
		wprintf(L"(Warning %d) %ls", dwEventID & 0x3fffffff, str.c_str());
	else
		wprintf(L"(%d) %ls", dwEventID & 0x3fffffff, str.c_str());
}

DWORD Logf(DWORD dwEventID, LPCTSTR fmt, ...)
{
	WCHAR out[1024];
	va_list body;
	va_start(body, fmt);
	vswprintf(out, 1024, fmt, body);
	va_end(body);

	WORD wType;
	if (dwEventID >= 0x80000000L) wType = EVENTLOG_ERROR_TYPE;
	else if (dwEventID >= 0x40000000L) wType = EVENTLOG_WARNING_TYPE;
	else wType = EVENTLOG_SUCCESS;

	const wchar_t* msg = out;
	HANDLE hSrc = RegisterEventSource(NULL, L"AdVisuo");
	if (g_bRegEvents) ReportEvent(hSrc, wType, CAT_ADV_DLL, dwEventID, NULL, 1, 0, &LPCTSTR(msg), NULL);
	DeregisterEventSource(hSrc);

	if (g_bOnScreen) _PrintMessage(dwEventID, out);

	return OLE_S_FIRST + dwEventID;
}

DWORD Log(DWORD dwEventID, LPCTSTR pStr1, LPCTSTR pStr2, LPCTSTR pStr3, LPCTSTR pStr4)
{
	WORD wType;
	if (dwEventID >= 0x80000000L) wType = EVENTLOG_ERROR_TYPE;
	else if (dwEventID >= 0x40000000L) wType = EVENTLOG_WARNING_TYPE;
	else wType = EVENTLOG_SUCCESS;

	const wchar_t* msgs[4];
	msgs[0] = pStr1;
	msgs[1] = pStr2;
	msgs[2] = pStr3;
	msgs[3] = pStr4;
	HANDLE hSrc = RegisterEventSource(NULL, L"AdVisuo");
	if (g_bRegEvents) ReportEvent(hSrc, wType, CAT_ADV_DLL, dwEventID, NULL, 4, 0, msgs, NULL);
	DeregisterEventSource(hSrc);

	if (g_bOnScreen) _PrintMessage(dwEventID, pStr1, pStr2, pStr3, pStr4);

	return OLE_S_FIRST + dwEventID;
}

DWORD Logf(DWORD dwEventID, HRESULT h, LPCTSTR fmt, ...)
{
	WCHAR out[1024];
	va_list body;
	va_start(body, fmt);
	vswprintf(out, 1024, fmt, body);
	va_end(body);

	WORD wType;
	if (dwEventID >= 0x80000000L) wType = EVENTLOG_ERROR_TYPE;
	else if (dwEventID >= 0x40000000L) wType = EVENTLOG_WARNING_TYPE;
	else wType = EVENTLOG_SUCCESS;

	const wchar_t* msgs[2];
	msgs[0] = _wcsdup(_com_error(h).ErrorMessage());
	msgs[1] = out;
	HANDLE hSrc = RegisterEventSource(NULL, L"AdVisuo");
	if (g_bRegEvents) ReportEvent(hSrc, wType, CAT_ADV_DLL, dwEventID, NULL, 2, 0, msgs, NULL);
	DeregisterEventSource(hSrc);

	if (g_bOnScreen) _PrintMessage(dwEventID, msgs[0], msgs[1]);

	return OLE_S_FIRST + dwEventID;
}

DWORD Logf(DWORD dwEventID, _com_error &ce, LPCTSTR fmt, ...)
{
	WCHAR out[1024];
	va_list body;
	va_start(body, fmt);
	vswprintf(out, 1024, fmt, body);
	va_end(body);

	WORD wType;
	if (dwEventID >= 0x80000000L) wType = EVENTLOG_ERROR_TYPE;
	else if (dwEventID >= 0x40000000L) wType = EVENTLOG_WARNING_TYPE;
	else wType = EVENTLOG_SUCCESS;

	WCHAR num[16];
	wsprintf(num, L"%d", ce.Error());
	wstring msg = ce.ErrorMessage();
	wstring src = ce.Source().length() ? ce.Source() : L"unspecified";
	wstring desc = ce.Description().length() ? ce.Description() : L"description not provided";

	const wchar_t* msgs[5];
	msgs[0] = num;
	msgs[1] = msg.c_str();
	msgs[2] = src.c_str();
	msgs[3] = desc.c_str();
	msgs[4] = out;
	HANDLE hSrc = RegisterEventSource(NULL, L"AdVisuo");
	if (g_bRegEvents) ReportEvent(hSrc, wType, CAT_ADV_DLL, dwEventID, NULL, 5, 0, msgs, NULL);
	DeregisterEventSource(hSrc);

	if (g_bOnScreen) _PrintMessage(dwEventID, msgs[0], msgs[1], msgs[2], msgs[3], msgs[4]);

	return OLE_S_FIRST + dwEventID;
}

DWORD Logf(DWORD dwEventID, bool bGetOn, DWORD PassengerId, DWORD Time, DWORD Floor, DWORD Lift, DWORD Deck, LPCTSTR fmt, ...)
{
	WCHAR out[1024];
	va_list body;
	va_start(body, fmt);
	vswprintf(out, 1024, fmt, body);
	va_end(body);

	WORD wType;
	if (dwEventID >= 0x80000000L) wType = EVENTLOG_ERROR_TYPE;
	else if (dwEventID >= 0x40000000L) wType = EVENTLOG_WARNING_TYPE;
	else wType = EVENTLOG_SUCCESS;
	
	WCHAR num[16][5];
	wsprintf(num[0], L"%d", PassengerId);
	wsprintf(num[1], L"%d", Time);
	wsprintf(num[2], L"%d", Floor);
	wsprintf(num[3], L"%d", Lift);
	wsprintf(num[4], L"%d", Deck);

	const wchar_t* msgs[6];
	msgs[0] = bGetOn ? L"on" : L"off";
	msgs[1] = num[0];
	msgs[2] = num[1];
	msgs[3] = num[2];
	msgs[4] = num[3];
	msgs[5] = num[4];
	HANDLE hSrc = RegisterEventSource(NULL, L"AdVisuo");
	if (g_bRegEvents) ReportEvent(hSrc, wType, CAT_ADV_DLL, dwEventID, NULL, 6, 0, msgs, NULL);
	DeregisterEventSource(hSrc);

	if (g_bOnScreen) _PrintMessage(dwEventID, msgs[0], msgs[1], msgs[2], msgs[4], msgs[4], msgs[5]);

	return OLE_S_FIRST + dwEventID;
}

CLogTime::CLogTime()
{
	m_nTime = ::GetTickCount();
}

DWORD CLogTime::Log(LPCTSTR pStr)
{
	WCHAR strTime[116];
	if (g_bBenchmark) 
	{
		_snwprintf_s(strTime, 16, L"%.3f", (GetTickCount() - m_nTime) / 1000.0f);
		Reset();
		return ::Log(INFO_TIME, pStr, strTime);
	}
	else
		return S_OK;
}

void CLogTime::Reset()
{
	m_nTime = ::GetTickCount();
}
