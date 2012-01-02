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

//////////////////////////////////////////////////////////////////////////////////////////////
// CValue

CValue::CValue()			 { type = V_INT;	this->i = 0; }
CValue::CValue(bool b)		 { type = V_INT;	this->i = b;}
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
	case VT_DATE:	type = V_DATE;	d = v;	break;
	case VT_BSTR:	type = V_STRING;s = (_bstr_t)v;	break;
	default:		throw *this;
	}
}

CValue::operator BOOL()				
{
	switch (type)
	{
	case V_INT:		return i;
	default:		throw *this; 
	}
}

CValue::operator FLOAT()
{ 
	switch (type)
	{
	case V_FLOAT:	return f;
	case V_INT:		return (FLOAT)i;
	default:		throw *this; 
	}
}

CValue::operator ULONG()
{ 
	switch (type)
	{
	case V_FLOAT:	return (ULONG)f;
	case V_INT:		return (ULONG)i;
	default:		throw *this; 
	}
}

CValue::operator LONG()
{ 
	switch (type)
	{
	case V_FLOAT:	return (LONG)f;
	case V_INT:		return (LONG)i;
	default:		throw *this; 
	}
}

CValue::operator DATE()
{ 
	switch (type)
	{
	case V_DATE:	return d;
	default:		throw *this; 
	}
}

CValue::operator wstring()
{
	wstringstream str;
	switch (type)
	{
	case V_NULL:	str << "NULL";	break;
	case V_INT:		str << i;		break;
	case V_FLOAT:	str << f;		break;
	case V_DATE:	throw *this;	// not implemented
	case V_STRING:	str << s;		break;
	case V_SYMBOL:	str << s;		break;
	}
	return str.str();
}

CValue::operator string()
{
	return _wstr2str(operator wstring());
}

string CValue::escaped()
{
	if (type == V_STRING)
	{
		stringstream str;
		str << "'" << (string)(*this) << "'";
		return str.str();
	}
	else
		return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CCollection

void CCollection::replace_key(std::string old_key, std::string new_key)
{
	if (old_key == new_key) return;
	CCollection::iterator i = find(old_key);
	if (i != end())
	{
		(*this)[new_key] = (*this)[old_key];
		erase(i);
	}
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
void CDataBase::execute(const char *query, ...)
{
	char out[1024];
	va_list body; va_start(body, query); vsprintf_s(out, query, body); va_end(body);
	m_h = m_connection->Execute(out, NULL, 1);
	if FAILED(m_h) throw m_h;
}

// SQL SELECT
CDataBase::SELECT CDataBase::select(const char *query, ...)
{
	char out[1024];
	va_list body; va_start(body, query); vsprintf_s(out, query, body); va_end(body);
	return CDataBase::SELECT(*this, out);
}

// SQL INSERT
CDataBase::INSERT CDataBase::insert(const char *table)
{ 
	return CDataBase::INSERT(this, table); 
}

// SQL UPDATE
CDataBase::UPDATE CDataBase::update(const char *table, const char *where_clause, ...)
{ 
	char out[1024];
	va_list body; va_start(body, where_clause); vsprintf_s(out, where_clause, body); va_end(body);
	return CDataBase::UPDATE(this, table, out);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// CDataBase::SELECT

CDataBase::SELECT::SELECT(CDataBase &db, const char *query)
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
	for each (pair<string, CValue> p in coll)
		(*this)[p.first] = p.second;
}

string CDataBase::INSERT::query()
{
	stringstream str;
	bool bComma;

	str << "INSERT INTO " << table << "(";
	bComma = false;
	for each (pair<string, CValue> p in *this)
	{
		if (bComma) str << ", ";
		bComma = true;
		str << p.first;
	}

	str << ") VALUES (";
	bComma = false;
	for each (pair<string, CValue> p in *this)
	{
		if (bComma) str << ", ";
		bComma = true;
		str << (string)p.second.escaped();
	}

	str << ")";
	return str.str();
}

string CDataBase::UPDATE::query()
{
	stringstream str;
	str << "UPDATE " << table << " SET ";
	bool bComma = false;
	for each (pair<string, CValue> p in *this)
	{
		if (bComma) str << ", ";
		bComma = true;
		str << p.first << "=" << (string)p.second.escaped();
	}

	str << " " << where_clause;
	return str.str();
}
