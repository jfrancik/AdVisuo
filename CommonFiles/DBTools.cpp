// DBTools.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "DBTools.h"

#include <sstream>

#pragma warning (disable:4996)

using namespace dbtools;
using namespace std;

static wstring _str2wstr(string str)
{ 
	size_t sz = str.size() + 1; 
	wchar_t *p  = new wchar_t[sz]; 
	::mbstowcs(p, str.c_str(), sz); 
	wstring w = p; 
	delete [] p; 
	return w; 
}

static string _wstr2str(wstring str)
{ 
	size_t sz = str.size() + 1; 
	char *p  = new char[sz]; 
	::wcstombs(p, str.c_str(), sz); 
	string s = p; 
	delete [] p; 
	return s; 
}

std::wstring _value_error::ErrorMessage()
{
	std::wstring types[] = { L"NULL", L"int", L"bool", L"float", L"date", L"string", L"symbol" };
	wstringstream s;
	switch (_error)
	{
		case E_VALUE_BAD_VARIANT:		s << L"Cannot convert data from VARIANT (VT = " << _i1 << L")"; break;
		case E_VALUE_BAD_CONVERSION:	s << L"Cannot convert value from " << types[_i1] << L" to " << types[_i2]; break;
		case E_VALUE_BAD_DATE_FORMAT:	s << L"Format of date/time value is inconsistent with ISO 8601"; break;
		case E_VALUE_BAD_XS_TYPE:		s << L"Format of type string definition inconsistent with XML Schema"; break;
		default:						s << L"Unrecognised error (CValue related)"; break;
	}
	return s.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CValue

CValue::CValue()			 { type = V_INT;	this->i = 0; }
CValue::CValue(bool b)		 { type = V_BOOL;	this->b = b;}
CValue::CValue(FLOAT f)		 { type = V_FLOAT;	this->f = f; }
CValue::CValue(ULONG u)		 { type = V_INT;	this->i = u; }
CValue::CValue(LONG n)		 { type = V_INT;	this->i = n; }
CValue::CValue(string s)	 { type = V_STRING;	this->s = _str2wstr(s); }
CValue::CValue(char *ps)	 { type = V_STRING;	this->s = _str2wstr(ps); }
CValue::CValue(wstring s)	 { type = V_STRING;	this->s = s; }
CValue::CValue(wchar_t *ps)	 { type = V_STRING;	this->s = ps; }
CValue::CValue(DATE d)		 { type = V_DATE;	this->d = d; }
CValue::CValue(_variant_t v)
{
	switch(v.vt)
	{
	case VT_EMPTY:
	case VT_NULL:	type = V_NULL;	break;
	case VT_I1:
	case VT_UI1:
	case VT_I2:
	case VT_I4:		type = V_INT;	i = v;	break;
	case VT_R4:
	case VT_R8:
	case VT_CY:
	case VT_DECIMAL:type = V_FLOAT;	f = v;	break;
	case VT_BOOL:	type = V_BOOL;  b = v;  break;
	case VT_DATE:	type = V_DATE;	d = v;	break;
	case VT_BSTR:	type = V_STRING;s = (_bstr_t)v;	break;
	default:		throw _value_error(_value_error::E_VALUE_BAD_VARIANT, v.vt);
	}
}

CValue::operator BOOL()
{
	switch (type)
	{
	case V_BOOL:	return b;
	case V_INT:		return i;
	default:		throw _value_error(_value_error::E_VALUE_BAD_CONVERSION, type, V_BOOL); 
	}
}

CValue::operator FLOAT()
{
	switch (type)
	{
	case V_FLOAT:	return f;
	case V_BOOL:	return (FLOAT)b;
	case V_INT:		return (FLOAT)i;
	default:		throw _value_error(_value_error::E_VALUE_BAD_CONVERSION, type, V_FLOAT);
	}
}

CValue::operator ULONG()
{ 
	switch (type)
	{
	case V_FLOAT:	return (ULONG)f;
	case V_BOOL:	return (ULONG)b;
	case V_INT:		return (ULONG)i;
	default:		throw _value_error(_value_error::E_VALUE_BAD_CONVERSION, type, V_INT);
	}
}

CValue::operator LONG()
{ 
	switch (type)
	{
	case V_FLOAT:	return (LONG)f;
	case V_BOOL:	return (LONG)b;
	case V_INT:		return (LONG)i;
	default:		throw _value_error(_value_error::E_VALUE_BAD_CONVERSION, type, V_INT);
	}
}

CValue::operator DATE()
{ 
	switch (type)
	{
	case V_DATE:	return d;
	default:		throw _value_error(_value_error::E_VALUE_BAD_CONVERSION, type, V_DATE);
	}
}

CValue::operator wstring()
{
	switch (type)
	{
	case V_NULL:	return L"0";
	case V_BOOL:	return to_wstring((_LONGLONG)b);
	case V_INT:		return to_wstring((_LONGLONG)i);
	case V_FLOAT:	return to_wstring((long double)f);
	case V_DATE:
		{
			UDATE ud;
			VarUdateFromDate(d, 0, &ud);
			wchar_t buf[256];
			_snwprintf(buf, 256, L"%04d-%02d-%02dT%02d:%02d:%02d", ud.st.wYear, ud.st.wMonth, ud.st.wDay, ud.st.wHour, ud.st.wMinute, ud.st.wSecond);
			return buf;
		}
	case V_STRING:	return s;
	case V_SYMBOL:	return s;
	default:		throw _value_error(_value_error::E_VALUE_BAD_CONVERSION, type, V_STRING);
	}
}

CValue::operator string()
{
	return _wstr2str(operator wstring());
}

std::wstring CValue::as_xs_type()
{
	switch (type)
	{
	case V_INT:		return L"xs:int";
	case V_BOOL:	return L"xs:boolean";
	case V_FLOAT:	return L"xs:double";
	case V_DATE:	return L"xs:dateTime";
	case V_STRING:	return L"xs:string";
	case V_SYMBOL:	return L"xs:string";
	default:		return L"";
	}
}

std::wstring CValue::as_sql_type()
{
	switch (type)
	{
	case V_NULL:	return L"int";
	case V_INT:		return L"int";
	case V_BOOL:	return L"bit";
	case V_FLOAT:	return L"float";	// L"decimal(9,2)";
	case V_DATE:	return L"datetime";
	case V_STRING:	return L"nvarchar(255)";
	case V_SYMBOL:	return (s == L"CURRENT_TIMESTAMP") ? L"datetime" : L"nvarchar(255)";
	default:		return L"";
	}
}

void CValue::from_string(std::string sv)
{
	switch (type)
	{
	case V_NULL:	break;
	case V_BOOL:	b = (sv == "1" || sv == "T" || sv == "t" || sv == "true" || sv == "TRUE"); break;
	case V_INT:		i = stoi(sv); break;
	case V_FLOAT:	f = stof(sv); break;
	case V_DATE:	sv = sv.substr(0, 10) + " " + sv.substr(11, 8);
					if FAILED(VarDateFromStr(_str2wstr(sv).c_str(), LANG_USER_DEFAULT, 0, &d)) throw _value_error(_value_error::E_VALUE_BAD_DATE_FORMAT);
					break;
	case V_STRING:	s = _str2wstr(sv); break;
	case V_SYMBOL:	s = _str2wstr(sv); break;
	}
}

void CValue::from_wstring(std::wstring wsv)
{
	switch (type)
	{
	case V_NULL:	break;
	case V_BOOL:	b = (wsv == L"1" || wsv == L"T" || wsv == L"t" || wsv == L"true" || wsv == L"TRUE"); break;
	case V_INT:		i = stoi(wsv); break;
	case V_FLOAT:	f = stof(wsv); break;
	case V_DATE:	wsv = wsv.substr(0, 10) + L" " + wsv.substr(11, 8);
					if FAILED(VarDateFromStr(wsv.c_str(), LANG_USER_DEFAULT, 0, &d)) throw _value_error(_value_error::E_VALUE_BAD_DATE_FORMAT);
					break;
	case V_STRING:	s = wsv;		break;
	case V_SYMBOL:	s = wsv;		break;
	}
}

void CValue::from_type(std::wstring type)
{
	if (type == L"xs:string")
		*this = L"";
	else if (type == L"xs:decimal")
		*this = 0.0f;
	else if (type == L"xs:double")
		*this = 0.0f;
	else if (type == L"xs:integer")
		*this = (ULONG)0;
	else if (type == L"xs:int")
		*this = (ULONG)0;
	else if (type == L"xs:boolean")
		*this = false;
	else if (type == L"xs:date")
		*this = (DATE)0;
	else if (type == L"xs:time")
		*this = (DATE)0;
	else if (type == L"xs:dateTime")
		*this = (DATE)0;
	else
		throw _value_error(_value_error::E_VALUE_BAD_XS_TYPE);
}

wstring CValue::escaped()
{
	if (type == V_STRING)
	{
		wstringstream str;
		str << L"'" << (wstring)(*this) << L"'";
		return str.str();
	}
	else
	if (type == V_DATE)
	{
		wstringstream str;
		str << L"'" << (wstring)(*this) << L"'";
		return str.str();
	}
	else
		return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CDataBase

CDataBase::CDataBase(LPCOLESTR pConnectionString)
{
	m_connection = NULL;
	m_h = m_connection.CreateInstance(__uuidof(ADODB::Connection));
	if FAILED(m_h) throw m_h;
	m_connection->CursorLocation = ADODB::adUseClient;
	m_h = m_connection->Open(pConnectionString, L"", L"", ADODB::adConnectUnspecified);
	if FAILED(m_h) throw m_h;
}

// execute any SQL statement
void CDataBase::execute(const wchar_t *query, ...)
{
	wchar_t out[10240];
	va_list body; va_start(body, query); vswprintf_s(out, query, body); va_end(body);
	m_h = m_connection->Execute(out, NULL, 1);
	if FAILED(m_h) throw m_h;
}

// SQL SELECT
CDataBase::SELECT CDataBase::select(const wchar_t *query, ...)
{
	wchar_t out[10240];
	va_list body; va_start(body, query); vswprintf_s(out, query, body); va_end(body);
	return CDataBase::SELECT(*this, out);
}

// SQL INSERT
CDataBase::INSERT CDataBase::insert(const wchar_t *table)
{ 
	return CDataBase::INSERT(this, table); 
}

// SQL UPDATE
CDataBase::UPDATE CDataBase::update(const wchar_t *table, const wchar_t *where_clause, ...)
{ 
	wchar_t out[10240];
	va_list body; va_start(body, where_clause); vswprintf_s(out, where_clause, body); va_end(body);
	return CDataBase::UPDATE(this, table, out);
}

// SQL CREATE
CDataBase::CREATE CDataBase::create(const wchar_t *table)
{ 
	return CDataBase::CREATE(this, table); 
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CDataBase::SELECT

CDataBase::SELECT::SELECT(CDataBase &db, const wchar_t *query)
{
	m_h = m_recordset.CreateInstance(__uuidof(ADODB::Recordset));
	if FAILED(m_h) throw m_h;
	m_h = m_recordset->Open(query, ((ADODB::_ConnectionPtr&)db).GetInterfacePtr(), ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText);
	if FAILED(m_h) throw m_h;
}

void CDataBase::SELECT::operator >> (CCollection &coll)
{
	for (short i = 0; i < (short)getSize(); i++)
		coll[getName(i)] = (*this)[i];
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CDataBase::QUERY/INSERT/UPDATE

void CDataBase::QUERY::operator << (CCollection &coll)
{
	for each (pair<wstring, CValue> p in coll)
		(*this)[p.first] = p.second;
}

wstring CDataBase::INSERT::query()
{
	wstringstream str;
	bool bComma;

	str << L"INSERT INTO " << table << L"(";
	bComma = false;
	for each (pair<wstring, CValue> p in *this)
	{
		if (bComma) str << L", ";
		bComma = true;
		str << p.first;
	}

	str << L") VALUES (";
	bComma = false;
	for each (pair<wstring, CValue> p in *this)
	{
		if (bComma) str << L", ";
		bComma = true;
		str << (wstring)p.second.escaped();
	}

	str << L")";
	return str.str();
}

void CDataBase::INSERT::createTables()
{
	CDataBase::CREATE create = pDB->create(table.c_str());
	create << *this;

	create.execute();
}

void CDataBase::INSERT::execute()
{ 
	if (pDB)
	{
		createTables();
		pDB->execute(query().c_str());
	}
}

wstring CDataBase::UPDATE::query()
{
	wstringstream str;
	str << L"UPDATE " << table << L" SET ";
	bool bComma = false;
	for each (pair<wstring, CValue> p in *this)
	{
		if (bComma) str << L", ";
		bComma = true;
		str << p.first << L"=" << (wstring)p.second.escaped();
	}

	str << L" " << where_clause;
	return str.str();
}

wstring CDataBase::CREATE::query()
{
	wstringstream str;

	str << L"IF OBJECT_ID('dbo." << table << L"','U') IS NULL CREATE TABLE dbo." << table << L" (";
	str << L"ID int IDENTITY(1,1) NOT NULL";
	for each (pair<wstring, CValue> p in *this)
	{
		str << L", ";
		str << p.first << L" " << p.second.as_sql_type() << L" NOT NULL";
	}

	str << L")";
	return str.str();
}

