#include "StdAfx.h"
#include "_base.h"
#include "FailureStr.h"
#include "VisProject.h"
#include "XmlRequest.h"

CString GetFailureString(_version_error &ve, CString url)
{
	return ve.ErrorMessage().c_str();
}

CString GetFailureString(_prj_error &pe, CString url)
{
	std::wstringstream str;
	str << L"AdVisuo internal error:<br />" << pe.ErrorMessage() << L"<br />while loading from " << (LPCTSTR)url << L".";
	return str.str().c_str();
}

CString GetFailureString(_com_error &ce, CString url)
{
	std::wstringstream str;
	str << "System cannot access server side service at:<br />" << (LPCTSTR)url << ".<br/>&nbsp;<br/>";

	if ((wchar_t*)ce.Description())
		str << (LPCTSTR)ce.Description();
	else
		str << ce.ErrorMessage();

	CString s = str.str().c_str();
	s.TrimRight();
	return s;
}

CString GetFailureString(_xmlreq_error &xe, CString url)
{
	std::wstringstream str;
	str << L"HTTP error " << xe.status() << L": " << xe.msg() << L"<br />at " << (LPCTSTR)url << L".";
	return str.str().c_str();
}

CString GetFailureString(dbtools::_value_error &ve, CString url)
{
	std::wstringstream str;
	str << L"AdVisuo internal error:<br />" << ve.ErrorMessage() << L"<br />while loading from " << (LPCTSTR)url << L".";
	return str.str().c_str();
}

CString GetFailureString(CString url)
{
	std::wstringstream str;
	str << L"Unidentified error while connecting to " << (LPCTSTR)url;
	return str.str().c_str();
}

////////////////////////////////////////////////////////////////////////
// _version_error

std::wstring _version_error::ErrorMessage()
{
	std::wstringstream str;
	str << L"Version of AdVisuo you are using is outdated and not compatible<br />with the server software.<br /><br />";
	str << L"Currently used version: " << VERSION_MAJOR << L"." << VERSION_MINOR  << L"." << VERSION_REV << L" (" << __DATE__ << L")<br />";
	str << L"Version required:       " << nVersionReq/10000 << L"." << (nVersionReq%10000) / 100  << L"." << nVersionReq%100 << L" (" << (LPCTSTR)strVerDate << L")<br />";
	str << L"<a href='" << (LPCTSTR)strDownloadPath << "'>Download the latest version</a>";
	return str.str();
}

