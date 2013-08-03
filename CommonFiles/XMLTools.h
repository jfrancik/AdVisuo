// XMLTools.h - AdVisuo Common Source File

#pragma once

#include "DBTools.h"
#include <xmllite.h>

namespace xmltools
{

class CXmlReader : public dbtools::CCollection
{
	std::wstring m_name;
	dbtools::CCollection m_schema;	
    CComPtr<IXmlReader> m_pReader;
public:
	CXmlReader(std::wstring strFName);
	CXmlReader(LPCOLESTR pBuf);

	std::wstring getName()					{ return m_name; }
	operator CComPtr<IXmlReader>()			{ return m_pReader; }

	void reset()							{ clear(); m_schema.clear(); m_name = L""; }

	void operator >> (dbtools::CCollection &coll);
	bool read();
	bool read(dbtools::CCollection &coll)	{ read(); *this >> coll; }

	bool read_simple_type(LPCOLESTR pType);
};

class CXmlWriter : public dbtools::CCollection
{
	std::wstring m_name;
	dbtools::CCollection m_schema;
    CComPtr<IXmlWriter> m_pWriter;
	int m_nLevel;

	HGLOBAL m_hMem;
	LPVOID m_pLockedData;

protected:
	void writeHeaders();
 	void writeElement(LPCTSTR name, LPCTSTR val){ HRESULT h = m_pWriter->WriteElementString(NULL, name, NULL, val); if (FAILED(h)) throw _com_error(h); }
 	int  writeStart(LPCTSTR name, LPCTSTR ns = NULL, LPCTSTR url = NULL)
												{ HRESULT h = m_pWriter->WriteStartElement(ns, name, url); if (FAILED(h)) throw _com_error(h); return m_nLevel++; }
	void writeAttr(LPCTSTR name, LPCTSTR val)	{ HRESULT h = m_pWriter->WriteAttributeString(NULL, name, NULL, val); if (FAILED(h)) throw _com_error(h); }
	void writeString(LPCTSTR name)				{ HRESULT h = m_pWriter->WriteString(name); if (FAILED(h)) throw _com_error(h); }
	void writeEnd()								{ HRESULT h = m_pWriter->WriteEndElement(); if (FAILED(h)) throw _com_error(h); m_nLevel--; }
	void writeEnd(int nLevel)					{ while (nLevel < m_nLevel) writeEnd(); }

public:
	CXmlWriter(LPTSTR encoding, size_t nSize);	// write to mem buffer
	CXmlWriter(std::wstring strFName);			// write to a file
	~CXmlWriter();

	std::wstring getName()					{ return m_name; }
	operator CComPtr<IXmlWriter>()			{ return m_pWriter; }

	void reset()							{ clear(); m_schema.clear(); m_name = L""; }

	void operator << (dbtools::CCollection &coll);
	void write(LPCTSTR name, LPCTSTR val)						{ writeElement(name, val); }
	void write(LPCTSTR name, int val)							{ std::wstringstream s; s << val; writeElement(name, s.str().c_str()); }
	void write(std::wstring name);
	void write(std::wstring name, dbtools::CCollection &coll)	{ clear(); *this << coll; write(name); }

	LPVOID LockBuffer();
	void UnlockBuffer();
};

}	// namespace XMLTools