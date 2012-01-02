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
	sel = db.select(("SELECT * FROM AVBuildings WHERE ProjectID=%d", nProjectID);
	bldName = sel[L"BuildingName"];

All objects are reusable - and there is no need to close explicitly:

	sel = db.select("SELECT * FROM AVShafts WHERE BuildingID=%d ORDER BY ShaftID", nBuildingID);

to read entire table:

	while (sel)							// operator bool() checks for the EOF condition
	{
		capacity = sel[L"Capacity"];
		sel++;							// operator ++ executes MoveNext
	}

SQL INSERT

	CDataBase::INSERT ins = db.insert("AVBuildings")
	ins["BuildingName"] = "Front Tower";
	ins["LobbyWidth"] = 2500;
	ins["TimeStamp"] = "CURRENT_TIMESTAMP";
	ins["TimeStamp"].esc();					//	to remove quotation marks
	ins.execute();

SQL UPADTE

	CDataBase::UPDATE upd = db.insert("AVBuildings", "WHERE BuildingId=%d", id)
	upd["BuildingName"] = "Front Tower";
	upd["LobbyWidth"] = 2500;
	upd.execute();

***********************************************************************************/

#pragma once

#include <map>
#import "C:\Program files\Common Files\System\Ado\msado15.dll" rename("EOF", "ADOEOF")

#define ME	(*this)

namespace dbtools
{

struct CValue
{
private:
	enum { V_NULL, V_INT, V_FLOAT, V_DATE, V_STRING, V_SYMBOL } type;
	union
	{
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

	std::string escaped();
};

class CCollection : public std::map<std::string, CValue>
{
public:
	CCollection() : std::map<std::string, CValue>()		{ }

	void replace_key(std::string old_key, std::string new_key);
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
	
	// execute any SQL statement
	void execute(const char *query, ...);

	// SQL SELECT
	SELECT select(const char *query, ...);

	// SQL INSERT
	INSERT insert(const char *table);

	// SQL UPDATE
	UPDATE update(const char *table, const char *where_clause, ...);

	struct SELECT
	{
	private:
		ADODB::_RecordsetPtr m_recordset;
		HRESULT m_h;

		// private constructor: use CDataBase::select to create new objects
		SELECT(CDataBase &db, const char *query);

	public:
		SELECT()							{ m_h = E_POINTER; }

		operator ADODB::_RecordsetPtr()		{ return m_recordset; }

		operator HRESULT()					{ return m_h; }
		operator bool()						{ return SUCCEEDED(m_h) && !m_recordset->ADOEOF; }
		CValue operator[](short i)			{ _variant_t vIndex = i; return m_recordset->Fields->GetItem(&vIndex)->GetValue(); }
		CValue operator[](LPCOLESTR p)		{ return m_recordset->Fields->GetItem(p)->GetValue(); }
		LONG getSize()						{ return m_recordset->Fields->GetCount(); }
		std::string getName(short i)		{ _variant_t vIndex = i; return std::string(m_recordset->Fields->GetItem(&vIndex)->GetName()); }
		void operator ++()					{ m_recordset->MoveNext(); }
		void operator ++(int)				{ m_recordset->MoveNext(); }
		void operator --()					{ m_recordset->MovePrevious(); }
		void operator --(int)				{ m_recordset->MovePrevious(); }

		void operator >> (CCollection &coll);

		friend SELECT CDataBase::select(const char *query, ...);
	};

	struct QUERY : public CCollection
	{
	protected:
		CDataBase *pDB;
		std::string table;

		// private constructor: use CDataBase::SELECT::operator[] to create new objects
		QUERY(CDataBase *pDB, std::string table)		 { this->pDB = pDB, this->table = table; }

	public:
		QUERY()								{ pDB = NULL; }
		virtual std::string query() = 0;
		void execute()						{ if (pDB) pDB->execute(query().c_str()); }

		void operator << (CCollection &coll);
	};

	struct INSERT : public QUERY
	{
	private:
		INSERT(CDataBase *pDB, std::string table) : QUERY(pDB, table)	{ }
		friend INSERT CDataBase::insert(const char *table);
	public:
		INSERT()							{ }
		virtual std::string query();
	};

	struct UPDATE : public QUERY
	{
		std::string where_clause;
		UPDATE(CDataBase *pDB, std::string table, std::string where_clause) : QUERY(pDB, table)	{ this->where_clause = where_clause; }
		friend UPDATE CDataBase::update(const char *table, const char *where_clause, ...);
	public:
		UPDATE()							{ }
		virtual std::string query();
	};
};

}	// namespace DBTools