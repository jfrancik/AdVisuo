// DBConnStr.h - AdVisuo Common Source File

/***********************************************************************************
Easy access to database connection strings 

***********************************************************************************/

#pragma once

namespace dbtools
{

	enum CONNECTIONS { CONN_CONSOLE, CONN_VISUALISATION, CONN_REPORT, CONN_USERS, CONN_SIMQUEUE, CONN_RESERVED };
	AVSTRING GetConnString(enum CONNECTIONS connId);

}	// namespace DBTools