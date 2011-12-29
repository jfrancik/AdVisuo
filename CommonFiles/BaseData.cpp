// BaseData.cpp - AdVisuo Common Source File

#include "StdAfx.h"
#include "BaseData.h"

#include <sstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// JOURNEY

std::wstringstream &operator << (std::wstringstream &s, JOURNEY::DOOR &d)
{
	s << d.m_timeOpen << L" " << d.m_durationOpen << L" " << d.m_timeClose << L" " << d.m_durationClose << L" ";
	return s;
}

std::wstringstream &operator >> (std::wstringstream &s, JOURNEY::DOOR &d)
{
	s >> d.m_timeOpen >> d.m_durationOpen >> d.m_timeClose >> d.m_durationClose;
	return s;
}

std::wstring JOURNEY::StringifyDoorCycles()
{
	std::wstringstream s;
	for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
	{
		s << m_doorcycles[iDeck].size() << L" ";
		for (AVULONG iCycle = 0; iCycle < m_doorcycles[iDeck].size(); iCycle++)
			s << m_doorcycles[iDeck][iCycle];
	}
	return s.str();
}

void JOURNEY::ParseDoorCycles(std::wstring dc)
{
	std::wstringstream s(dc);
	for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
	{
		AVULONG n;
		s >> n;
		m_doorcycles[iDeck].resize(n);
		for (AVULONG iCycle = 0; iCycle < n; iCycle++)
			s >> m_doorcycles[iDeck][iCycle];
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WAYPOINT

std::wstringstream &operator << (std::wstringstream &s, WAYPOINT &w)
{
	s << w.nAction << L" ";
	switch (w.nAction)
	{
	case MOVE:
		s << w.vector.x << L" " << w.vector.y << L" ";
		break;
	case WAIT:
		s << w.nTime << L" ";
		break;
	case WALK:
		s << w.vector.x << L" " << w.vector.y << L" ";
		break;
	case TURN:
		break;
	case ENTER_ARR_FLOOR:
	case ENTER_LIFT:
	case ENTER_DEST_FLOOR:
		break;
	}

	if (!w.wstrStyle.empty())
		s << w.wstrStyle << L" ";
	else
		s << L"(null)" << L" ";
	
	return s;
}

std::wstringstream &operator >> (std::wstringstream &s, WAYPOINT &w)
{
	s >> (ULONG&)w.nAction;
	switch (w.nAction)
	{
	case MOVE:
		s >> w.vector.x >> w.vector.y;
		break;
	case WAIT:
		s >> w.nTime;
		break;
	case WALK:
		s >> w.vector.x >> w.vector.y;
		break;
	case TURN:
		break;
	case ENTER_ARR_FLOOR:
	case ENTER_LIFT:
	case ENTER_DEST_FLOOR:
		break;
	}

	s >> w.wstrStyle;

	return s;
}

