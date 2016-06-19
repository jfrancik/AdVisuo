class _version_error;
class _prj_error;
class _com_error;
class _xmlreq_error;
namespace dbtools
{
	class _value_error;
}


inline CString GetFailureTitle(_version_error &ve)			{ return L"VERSION MISMATCH"; }
inline CString GetFailureTitle(_prj_error &pe)				{ return L"INTERNAL ERROR"; }
inline CString GetFailureTitle(_com_error &ce)				{ return L"CONNECTION OR DATABASE ERROR"; }
inline CString GetFailureTitle(_xmlreq_error &xe)			{ return L"HTTP ERROR"; }
inline CString GetFailureTitle(dbtools::_value_error &ve)	{ return L"INTERNAL ERROR"; }
inline CString GetFailureTitle()							{ return L"ERROR"; }

CString GetFailureString(_version_error &ve, CString url);
CString GetFailureString(_prj_error &pe, CString url);
CString GetFailureString(_com_error &ce, CString url);
CString GetFailureString(_xmlreq_error &xe, CString url);
CString GetFailureString(dbtools::_value_error &ve, CString url);
CString GetFailureString(CString url);
