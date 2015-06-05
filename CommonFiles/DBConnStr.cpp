// DBConnStr.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "DBConnStr.h"

AVSTRING dbtools::GetConnString(enum CONNECTIONS connId)
{
	static WCHAR buf[CONN_RESERVED][1024];
	HKEY hRegKey = NULL; 

	DWORD type, size;
	HRESULT h;
	h = RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\LerchBates\\AdVisuo\\ServerModule", &hRegKey); if (h != 0) return NULL;
	size = 1024; 

	switch (connId)
	{
	case CONN_CONSOLE:			h = RegQueryValueEx(hRegKey, L"ConsoleConnectionString", 0, &type, (PBYTE)buf[connId], &size); break;
	case CONN_VISUALISATION:	h = RegQueryValueEx(hRegKey, L"VisualisationConnectionString", 0, &type, (PBYTE)buf[connId], &size); break;
	case CONN_REPORT:			h = RegQueryValueEx(hRegKey, L"ReportsConnectionString", 0, &type, (PBYTE)buf[connId], &size); break;
	case CONN_USERS:			h = RegQueryValueEx(hRegKey, L"UsersConnectionString", 0, &type, (PBYTE)buf[connId], &size); break;
	case CONN_SIMQUEUE:			h = RegQueryValueEx(hRegKey, L"SimQueueConnectionString", 0, &type, (PBYTE)buf[connId], &size); break;
	}
	if (h != 0) return NULL;

	h = RegCloseKey(hRegKey); if (h != 0) return NULL;

	return buf[connId];
}


