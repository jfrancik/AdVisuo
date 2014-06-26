// _base.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include <list>

int SETTINGS::nAspectRatio		= 10000;
int SETTINGS::nColouringMode	= 2;
int SETTINGS::nThresholdGreen	= 0;
int SETTINGS::nThresholdRed		= 50;
int SETTINGS::nNavMode			= 2;

std::list<IOutTextSink*> g_OutTextSinks;

void OutText(LPCTSTR fmt, ...)
{
	WCHAR out[1024];
	va_list body;
	va_start(body, fmt);
	vswprintf(out, 1024, fmt, body);
	va_end(body);
	TRACE(out); TRACE("\n");

	for each (IOutTextSink *pSink in g_OutTextSinks)
		pSink->OutText(out);
}

bool OutWaitMessage(AVLONG nWaitStage, AVULONG nMsecs)
{
	TRACE("Wait Message: %d\n", nWaitStage);
	for each (IOutTextSink *pSink in g_OutTextSinks)
		if (!pSink->OutWaitMessage(nWaitStage, nMsecs))
			return false;
	if (nMsecs > 0)
		Sleep(nMsecs);
	return true;
}

void RegisterOutTextSink(IOutTextSink *p)
{
	g_OutTextSinks.push_back(p);
}

void UnRegisterOutTextSink(IOutTextSink *p)
{
	g_OutTextSinks.remove(p);
}
