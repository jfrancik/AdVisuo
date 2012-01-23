// XMLTools.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "XMLTools.h"

#pragma warning (disable:4996)

using namespace dbtools;
using namespace xmltools;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CXmlReader

CXmlReader::CXmlReader(LPCOLESTR pBuf) : CCollection()
{
    HRESULT h;
    CComPtr<IStream> pStream;
	CComPtr<IXmlReaderInput> pReaderInput;

	size_t nSize = sizeof(pBuf[0]) * wcslen(pBuf);
	HGLOBAL	hMem = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
	LPVOID pImage = ::GlobalLock(hMem);
	memcpy(pImage, pBuf, nSize);
	::GlobalUnlock(hMem);

	h = CreateStreamOnHGlobal(hMem, TRUE, &pStream); if (FAILED(h)) throw _com_error(h);
	h = CreateXmlReader(__uuidof(IXmlReader), (void**) &m_pReader, NULL); if (FAILED(h)) throw _com_error(h);
	h = CreateXmlReaderInputWithEncodingName(pStream, NULL, L"utf-16", FALSE, L"", &pReaderInput);
    h = m_pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit); if (FAILED(h)) throw _com_error(h);
    h = m_pReader->SetInput(pReaderInput); if (FAILED(h)) throw _com_error(h);
}

CXmlReader::CXmlReader(std::wstring strFName) : CCollection()
{
    HRESULT h;
    CComPtr<IStream> pFileStream;

	h = SHCreateStreamOnFile(strFName.c_str(), STGM_READ, &pFileStream); if (FAILED(h)) throw _com_error(h);
    h = CreateXmlReader(__uuidof(IXmlReader), (void**) &m_pReader, NULL); if (FAILED(h)) throw _com_error(h);
    h = m_pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit); if (FAILED(h)) throw _com_error(h);
    h = m_pReader->SetInput(pFileStream); if (FAILED(h)) throw _com_error(h);
}

void CXmlReader::operator >> (CCollection &coll)
{
	for each (pair<wstring, CValue> p in *this)
		coll[p.first] = p.second;
}

bool CXmlReader::read()
{
	XmlNodeType nodeType;
	LPCWSTR pLocalName;
	enum { NONE, SCHEMA, DATA } state = NONE;

	while (m_pReader->Read(&nodeType) == S_OK)
	{
		switch (nodeType)
		{
		case XmlNodeType_Element:
			m_pReader->GetLocalName(&pLocalName, NULL);

			if (state == NONE && wcscmp(pLocalName, L"schema") == 0)
			{
				state = SCHEMA;
				reset();
			}
			else
			if (state == NONE && wcscmp(pLocalName, getName().c_str()) == 0)
				state = DATA;
			else
			if (state == SCHEMA && wcscmp(pLocalName, L"DataSet") == 0)
				reset();
			else
			if (state == SCHEMA && wcscmp(pLocalName, L"element") == 0)
			{
				// scheme element
				LPCWSTR pName, pType;
				if (m_pReader->MoveToAttributeByName(L"name", L"") == S_OK)
				{
					m_pReader->GetValue(&pName, NULL);
					if ((wstring)pName == L"NewDataSet")
						break;
					else
					if (getName().empty())
						m_name = pName;
					else
					if (m_pReader->MoveToAttributeByName(L"type", L"") == S_OK)
					{
						m_pReader->GetValue (&pType, NULL);
						m_schema[pName].from_type(pType);
					}
				}
			}
			break;
		case XmlNodeType_Text:
			if (state == DATA)
			{
				LPCWSTR pValue;
				m_pReader->GetValue(&pValue, NULL);
				ME[pLocalName] = m_schema[pLocalName];
				ME[pLocalName].from_wstring(pValue);
			}
			break;
		case XmlNodeType_EndElement:
			m_pReader->GetLocalName(&pLocalName, NULL);
			if (state == SCHEMA && wcscmp(pLocalName, L"schema") == 0)
				state = NONE;
			else
			if (state == DATA && wcscmp(pLocalName, getName().c_str()) == 0)
				return true;
			break;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CXmlWriter

CXmlWriter::CXmlWriter(LPCOLESTR pBuf, size_t nSize) : CCollection()
{
	ASSERT(FALSE);
	// this function does not work well - needs to be fixed

    HRESULT h;
    CComPtr<IStream> pStream;
	CComPtr<IXmlReaderInput> pReaderInput;

	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, nSize);

	h = CreateStreamOnHGlobal(hMem, FALSE, &pStream); if (FAILED(h)) throw _com_error(h);
	h = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &m_pWriter, NULL); if (FAILED(h)) throw _com_error(h);
    h = m_pWriter->SetProperty(XmlWriterProperty_Indent, TRUE); if (FAILED(h)) throw _com_error(h);
    h = m_pWriter->SetOutput(pStream); if (FAILED(h)) throw _com_error(h);

	m_nLevel = 0;
	writeHeaders();
}

CXmlWriter::CXmlWriter(std::wstring strFName) : CCollection()
{
    HRESULT h;
    CComPtr<IStream> pFileStream;

	h = SHCreateStreamOnFile(strFName.c_str(), STGM_CREATE | STGM_WRITE, &pFileStream); if (FAILED(h)) throw _com_error(h);
    h = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &m_pWriter, NULL); if (FAILED(h)) throw _com_error(h);
    h = m_pWriter->SetProperty(XmlWriterProperty_Indent, TRUE); if (FAILED(h)) throw _com_error(h);
    h = m_pWriter->SetOutput(pFileStream); if (FAILED(h)) throw _com_error(h);

	m_nLevel = 0;
	writeHeaders();
}

CXmlWriter::~CXmlWriter()
{
	HRESULT h = m_pWriter->Flush(); if (FAILED(h)) throw _com_error(h);
}

void CXmlWriter::writeHeaders()
{
	HRESULT h = m_pWriter->WriteStartDocument(XmlStandalone_Omit); if (FAILED(h)) throw _com_error(h);
	writeStart(L"AdVisuo-Saved-Project");
	writeElement(L"AdVisuo-Client-Version", L"1.00");
}

void CXmlWriter::operator << (CCollection &coll)
{
	for each (pair<wstring, CValue> p in coll)
		ME[p.first] = p.second;
}

void CXmlWriter::write(wstring name)
{
	if (name != getName())
	{
		writeEnd(1);
		m_name = name;
		writeStart(L"DataSet");
		int nLevel = writeStart(L"schema", L"xs", L"http://www.w3.org/2001/XMLSchema");
		writeStart(L"element", L"xs"); writeAttr(L"name", L"NewDataSet");
		writeStart(L"choice", L"xs");
		writeStart(L"element", L"xs"); writeAttr(L"name", getName().c_str());
		writeStart(L"complexType", L"xs");
		writeStart(L"sequence", L"xs");

		for each (pair<wstring, CValue> p in *this)
		{
			writeStart(L"element", L"xs");
			writeAttr(L"name", p.first.c_str());
			writeAttr(L"type", p.second.as_xs_type().c_str());
			writeEnd();
		}
		writeEnd(nLevel);
		writeStart(L"NewDataSet");
	}
	writeStart(getName().c_str());
	for each (pair<wstring, CValue> p in *this)
		writeElement(p.first.c_str(), p.second.as_wstring().c_str());
	writeEnd();
}

