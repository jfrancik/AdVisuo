// DBTools.h - AdVisuo Common Source File

/***********************************************************************************
Easy access to databases via SQL SELECT, INSERT and UPADTE commands
Usage:

To connect to the database:

	CDataBase db(connection);

To execute any SQL command

	db.execute("DROP TABLE test");

SQL SELECT:

	CDataBase::SELECT sel;
	sel = db.select(("SELECT * FROM AVLftGroups WHERE ProjectID=%d", nProjectID);
	bldName = sel[L"LftGroupName"];

All objects are reusable - and there is no need to close explicitly:

	sel = db.select("SELECT * FROM AVShafts WHERE LiftGroupId=%d ORDER BY ShaftID", nLiftGroupId);

to read entire table:

	while (sel)							// operator bool() checks for the EOF condition
	{
		capacity = sel[L"Capacity"];
		sel++;							// operator ++ executes MoveNext
	}

SQL INSERT

	CDataBase::INSERT ins = db.insert("AVLftGroups")
	ins["LftGroupName"] = "Front Tower";
	ins["LobbyWidth"] = 2500;
	ins["TimeStamp"] = "CURRENT_TIMESTAMP";
	ins["TimeStamp"].esc();					//	to remove quotation marks
	ins.execute();

SQL UPADTE

	CDataBase::UPDATE upd = db.insert("AVLftGroups", "WHERE LiftGrpId=%d", id)
	upd["LftGroupName"] = "Front Tower";
	upd["LobbyWidth"] = 2500;
	upd.execute();

***********************************************************************************/

#pragma once

#include <map>
#import "C:\Program files\Common Files\System\Ado\msado15.dll" rename("EOF", "ADOEOF")

#define ME	(*this)

namespace dbtools
{

struct CValue;
typedef std::map<std::wstring, CValue> CCollection;

class _value_error
{
public:
	enum ERROR_CODES {
		E_VALUE_BAD_VARIANT = 0x80000100,
		E_VALUE_BAD_CONVERSION,
		E_VALUE_BAD_DATE_FORMAT,
		E_VALUE_BAD_XS_TYPE };
	_value_error(enum ERROR_CODES err_code, ULONG i1 = 0, ULONG i2 = 0)	{ _error = err_code; _i1 = i1; _i2 = i2; }
	enum ERROR_CODES Error()											{ return _error; }
	std::wstring ErrorMessage();
private:
	enum ERROR_CODES _error;
	ULONG _i1, _i2;
};

struct CValue
{
private:
	enum { V_NULL, V_INT, V_BOOL, V_FLOAT, V_DATE, V_STRING, V_SYMBOL } type;
	union
	{
		bool b;
		int i;
		float f;
		DATE d;
	};
	std::wstring s;

public:
	CValue();
	CValue(_variant_t v);
	CValue(bool b);
	CValue(FLOAT f);
	CValue(ULONG u);
	CValue(LONG n);
	CValue(std::string s);
	CValue(char *ps);
	CValue(std::wstring s);
	CValue(wchar_t *ps);
	CValue(DATE d);

	bool isNull()						{ return type == V_NULL; }
	void act_as_symbol(bool b = true)	{ if (b) type = V_SYMBOL; else type = V_STRING; } 

	operator bool()						{ return (operator BOOL() != FALSE); }
	operator BOOL();
	operator FLOAT();
	operator ULONG();
	operator LONG();
	operator std::string();
	operator std::wstring();
	operator DATE();

	BOOL as_bool()						{ return operator BOOL(); }
	FLOAT as_float()					{ return operator FLOAT(); }
	ULONG as_ulong()					{ return operator ULONG(); }
	LONG as_long()						{ return operator LONG(); }
	std::string as_string()				{ return operator std::string(); }
	std::wstring as_wstring()			{ return operator std::wstring(); }
	DATE as_date()						{ return operator DATE(); }

	std::wstring as_xs_type();
	std::wstring as_sql_type();

	void from_string(std::string);
	void from_wstring(std::wstring);

	void from_type(std::string);
	void from_type(std::wstring);

	std::wstring escaped();
};

class CDataBase
{
	ADODB::_ConnectionPtr m_connection;
	HRESULT m_h;

public:
	// Constructor: simply use the connection string to establish a database connection instance
	CDataBase(LPCOLESTR pConnectionString);

	operator ADODB::_ConnectionPtr()		{ return m_connection; }
	operator HRESULT()						{ return m_h; }
	operator bool()							{ return SUCCEEDED(m_h); }

	struct SELECT;
	struct INSERT;
	struct UPDATE;
	struct CREATE;
	
	// execute any SQL statement
	void execute(const wchar_t *query, ...);

	// SQL SELECT
	SELECT select(const wchar_t *query, ...);

	// SQL INSERT
	INSERT insert(const wchar_t *table);

	// SQL UPDATE
	UPDATE update(const wchar_t *table, const wchar_t *where_clause, ...);

	// SQL CREATE TABLE
	CREATE create(const wchar_t *table);

	struct SELECT
	{
	private:
		ADODB::_RecordsetPtr m_recordset;
		HRESULT m_h;

		// private constructor: use CDataBase::select to create new objects
		SELECT(CDataBase &db, const wchar_t *query);

	public:
		SELECT()							{ m_h = E_POINTER; }

		operator ADODB::_RecordsetPtr()		{ return m_recordset; }

		operator HRESULT()					{ return m_h; }
		operator bool()						{ return SUCCEEDED(m_h) && !m_recordset->ADOEOF; }
		CValue operator[](short i)			{ _variant_t vIndex = i; return m_recordset->Fields->GetItem(&vIndex)->GetValue(); }
		CValue operator[](LPCOLESTR p)		{ return m_recordset->Fields->GetItem(p)->GetValue(); }
		LONG getSize()						{ return m_recordset->Fields->GetCount(); }
		std::wstring getName(short i)		{ _variant_t vIndex = i; return std::wstring(m_recordset->Fields->GetItem(&vIndex)->GetName()); }
		void operator ++()					{ m_recordset->MoveNext(); }
		void operator ++(int)				{ m_recordset->MoveNext(); }
		void operator --()					{ m_recordset->MovePrevious(); }
		void operator --(int)				{ m_recordset->MovePrevious(); }

		void operator >> (CCollection &coll);

		friend SELECT CDataBase::select(const wchar_t *query, ...);
	};

	struct QUERY : public CCollection
	{
	protected:
		CDataBase *pDB;
		std::wstring table;

		// private constructor: use CDataBase::SELECT::operator[] to create new objects
		QUERY(CDataBase *pDB, std::wstring table)		 { this->pDB = pDB, this->table = table; }

	public:
		QUERY()								{ pDB = NULL; }
		virtual std::wstring query() = 0;
		virtual void execute()				{ if (pDB) pDB->execute(query().c_str()); }

		void operator << (CCollection &coll);
	};

	struct INSERT : public QUERY
	{
	private:
		INSERT(CDataBase *pDB, std::wstring table) : QUERY(pDB, table)	{ }
		friend INSERT CDataBase::insert(const wchar_t *table);
	public:
		INSERT()							{ }
		virtual std::wstring query();

		void createTables();
		virtual void execute();
	};

	struct UPDATE : public QUERY
	{
		std::wstring where_clause;
		UPDATE(CDataBase *pDB, std::wstring table, std::wstring where_clause) : QUERY(pDB, table)	{ this->where_clause = where_clause; }
		friend UPDATE CDataBase::update(const wchar_t *table, const wchar_t *where_clause, ...);
	public:
		UPDATE()							{ }
		virtual std::wstring query();
	};

	struct CREATE : public QUERY
	{
	private:
		CREATE(CDataBase *pDB, std::wstring table) : QUERY(pDB, table)	{ }
		friend CREATE CDataBase::create(const wchar_t *table);
	public:
		CREATE()							{ }
		virtual std::wstring query();
	};
};

}	// namespace DBTools