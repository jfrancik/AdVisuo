// _base.h

#pragma once

///////////////////////////////////////////////////////////////
// Versioning

#define VERSION 30614	// 3.06.14

#define VERSION_MAJOR	(VERSION / 10000)
#define VERSION_MINOR	((VERSION % 10000) / 100)
#define VERSION_REV		(VERSION % 100)

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define VERSION_DATE WIDEN(__DATE__)


// revisions:
// 00 - 09 - alpha or work in progress
// 10 - 19 - beta
// 20 - 29 - release candidate or early release
// 30 - 39 - stable release

///////////////////////////////////////////////////////////////
// OutText

// Text Output Function
void OutText(LPCTSTR fmt, ...);

// Special Wait Message display.
// nWaitStage: -1 for no wait, normally use count down like 3, 2, 1, 0, -1 (end of wait state)
// nMSec: if not zero, sleeps either statically or actively for a number of milliseconds provided
// result: false if STOP request generated, true otherwise
bool OutWaitMessage(LONG nWaitStage, ULONG nMsecs);


	// see also: IOutTextSink interface below and additional functions

///////////////////////////////////////////////////////////////
// AV Data Types

#define AVULONG ULONG
#define AVLONG LONG
#define AVFLOAT FLOAT
#define AVSTRING LPOLESTR

struct AVVECTOR
{
	AVFLOAT x;
	AVFLOAT y;
	AVFLOAT z;
};

struct AVSIZE
{
	AVSIZE(AVFLOAT x = 0, AVFLOAT y = 0) : x(x), y(y) { }
	AVFLOAT x;
	AVFLOAT y;
};

struct AVCOLOR
{
	AVFLOAT r;
	AVFLOAT g;
	AVFLOAT b;
	AVFLOAT a;
};

class _version_error
{
	int nVersionReq;
	CString strVerDate;
	CString strDownloadPath;
public:
	_version_error(int _nVersionReq, CString _strVerDate, CString _strDownloadPath)	: nVersionReq(_nVersionReq), strVerDate(_strVerDate), strDownloadPath(_strDownloadPath) { }
	std::wstring ErrorMessage();
};

#define _FWCOLOR(c) (*(FWCOLOR*)&(c))
#define AVCOLOR2COLORREF(c)	RGB((unsigned)((c).r*255.0f), (unsigned)((c).g*255.0f), (unsigned)((c).b*255.0f))

///////////////////////////////////////////////////////////////
// Namespaces and Typedefs

namespace fw
{
	interface IAction;
	interface IBody;
	interface IKineNode;
	interface IMaterial;
	interface ISceneObject;
	interface ISceneCamera;
	interface IMesh;
};

typedef fw::IAction		* HACTION;
typedef fw::IBody		* HBODY;
typedef fw::IKineNode	* HBONE;
typedef fw::IMaterial	* HMATERIAL;
typedef fw::ISceneObject* HOBJECT;
typedef fw::ISceneCamera* HCAMERA;
typedef fw::IMesh		* HMESH;

///////////////////////////////////////////////////////////////
// Interfaces

interface ILostDeviceObserver
{
	virtual void OnLostDevice() = 0;
	virtual void OnResetDevice() = 0;
};

interface IAnimationListener
{
	virtual int OnAnimationBegin(AVULONG nParam) = 0;
	virtual int OnAnimationTick(AVULONG nParam) = 0;
	virtual int OnAnimationEnd(AVULONG nParam) = 0;
};

interface IOutTextSink
{
	virtual void OutText(LPCTSTR lpszItem) = 0;
	virtual bool OutWaitMessage(AVLONG nWaitStage, AVULONG &nMsecs) = 0;	// params & result as in global OutWaitMessage
																			// Implementation is not required to sleep for nMsecs; 
};																			// if it does, nMsecs value should be decreased to reflect sleeping time

void RegisterOutTextSink(IOutTextSink*);
void UnRegisterOutTextSink(IOutTextSink*);

///////////////////////////////////////////////////////////////
// Global Access

// AdVisuo app
class CAdVisuoApp;
inline CAdVisuoApp *AVGetApp()					{ return (CAdVisuoApp*)AfxGetApp(); }

// AdVisuo mainframe
class CMainFrame;
inline CMainFrame *AVGetMainWnd()				{ return (CMainFrame*)AfxGetMainWnd(); }

// Standard Models Path
extern std::wstring _stdPathModels;
#define STD_PATH(str)	((AVSTRING)(_stdPathModels + (str)).c_str())

///////////////////////////////////////////////////////////////
// Parameters and Constants

// Max Number of Lifts
#define LIFT_MAXNUM	100

// Max Number of Lift Decks and Doors
#define DECK_NUM	2
#define MAX_DOORS	6

/////////////////////////////////////////////////////////////
// Additional includes

#include "../CommonFiles/Vector.h"
#include "../CommonFiles/Box.h"

#pragma warning (disable:4800)

