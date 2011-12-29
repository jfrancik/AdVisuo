// BaseData.h - AdVisuo Common Source File

#pragma once

#include <string>
#include <sstream>

/////////////////////////////////////////////////////////////
// Unddefined Value

enum { UNDEF = 0xffffffff };

/////////////////////////////////////////////////////////////
// Lift journey definition - directly used by CLift(Base) objects

struct JOURNEY
{
	struct DOOR
	{
		AVULONG m_timeOpen, m_durationOpen;
		AVULONG m_timeClose, m_durationClose;
		DOOR()					{ reset(); }

		void reset()			{ m_timeOpen = m_timeClose = UNDEF; m_durationOpen = m_durationClose = 1000; }

		AVULONG timeOpened()	{ return m_timeOpen + m_durationOpen; }
		AVULONG timeClosed()	{ return m_timeClose + m_durationClose; }

		friend std::wstringstream &operator << (std::wstringstream &s, DOOR &d);
		friend std::wstringstream &operator >> (std::wstringstream &s, DOOR &d);
	};

	std::vector<DOOR> m_doorcycles[DECK_NUM];

	AVULONG		m_shaftFrom, m_shaftTo;		// shaft id
	AVULONG		m_floorFrom, m_floorTo;		// journey floors
	AVULONG		m_timeGo, m_timeDest;		// journey time

	AVULONG FirstOpenTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? UNDEF : m_doorcycles[iDeck][0].m_timeOpen; }
	AVULONG FirstOpenedTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? UNDEF : m_doorcycles[iDeck][0].timeOpened(); }
	AVULONG LastCloseTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? 0     : m_doorcycles[iDeck][N-1].m_timeClose; }
	AVULONG LastClosedTime(AVULONG iDeck)	{ AVULONG N = m_doorcycles[iDeck].size(); return (N == 0) ? 0     : m_doorcycles[iDeck][N-1].timeClosed(); }

	AVULONG FirstOpenTime()					{ AVULONG n = UNDEF; for (AVULONG i = 0; i < DECK_NUM; i++) n = min(n, FirstOpenTime(i)); return n; }
	AVULONG FirstOpenedTime()				{ AVULONG n = UNDEF; for (AVULONG i = 0; i < DECK_NUM; i++) n = min(n, FirstOpenedTime(i)); return n; }
	AVULONG LastCloseTime()					{ AVULONG n = 0; for (AVULONG i = 0; i < DECK_NUM; i++) n = max(n, LastCloseTime(i)); return n; }
	AVULONG LastClosedTime()				{ AVULONG n = 0; for (AVULONG i = 0; i < DECK_NUM; i++) n = max(n, LastClosedTime(i)); return n; }

	std::wstring StringifyDoorCycles();
	void ParseDoorCycles(std::wstring);

	JOURNEY()								{ m_shaftFrom = m_shaftTo = 0; m_floorFrom = m_floorTo = m_timeGo = m_timeDest = UNDEF; }
};

/////////////////////////////////////////////////////////////
// Passenger Waypoint definition - directly used by CPassenger(Base) objects

enum ENUM_ACTION { MOVE, WAIT, WALK, TURN, ENTER_ARR_FLOOR, ENTER_LIFT, ENTER_DEST_FLOOR };
struct WAYPOINT
{
	ENUM_ACTION nAction;
	AVVECTOR vector;
	AVLONG nTime;
	std::wstring wstrStyle;

	friend std::wstringstream &operator << (std::wstringstream &s, WAYPOINT &w);
	friend std::wstringstream &operator >> (std::wstringstream &s, WAYPOINT &w);
};
