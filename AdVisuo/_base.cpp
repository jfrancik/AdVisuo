// _base.cpp - a part of the AdVisuo Client Software

#include "stdafx.h"
#include <list>

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

void RegisterOutTextSink(IOutTextSink *p)
{
	g_OutTextSinks.push_back(p);
}

void UnRegisterOutTextSink(IOutTextSink *p)
{
	g_OutTextSinks.remove(p);
}
