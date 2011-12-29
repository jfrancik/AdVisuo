/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Lift journey definition - directly used by CLift objects

// This structure has been simplified after dropping old-styled XML

struct JOURNEY
{
	struct DOOR
	{
		AVULONG m_timeOpen, m_durationOpen;
		AVULONG m_timeClose, m_durationClose;
		DOOR() { m_timeOpen = m_timeClose = UNDEF; m_durationOpen = m_durationClose = 1000; }
	};

	AVULONG		m_floorFrom, m_floorTo;		// journey floors
	AVULONG		m_numDoors[DECK_NUM];		// number of door cycles (usually 2 - open & close) per lift deck
	DOOR		*m_pDoors[DECK_NUM];		// door cycle times per lift deck
	AVULONG		m_timeGo, m_timeDest;		// journey time

	AVULONG		m_timeOpen, m_timeClose;	// summary - when first door start to open, last door start to close
	AVULONG		m_timesOpen[DECK_NUM];		// summary - on a per deck basis
	AVULONG		m_timesClose[DECK_NUM];

	bool		m_bLegacy;					// if in legacy mode, uses only m_floorFrom, m_floorTo, m_timeOpen, m_timeGo, m_timeDest, m_timeClose

	JOURNEY();
	void Consolidate(std::vector<JOURNEY::DOOR> (&doors)[DECK_NUM]);
};

/////////////////////////////////////////////////////////////
// Alternative to CSimResolver: deduces lift journeys
// on the basis of the passenger traffic data

class CBuilding;
class CSimBase;

class CSimDeducer
{
	std::vector<JOURNEY> journeys;

	AVULONG m_nLiftId;
	CBuilding *m_pBuilding;

	enum EVENT_TYPE { REQ, CALL, ARR };
	struct RECORD
	{
		EVENT_TYPE event_type;
		AVULONG nTime;
		AVULONG nStorey;
		RECORD(EVENT_TYPE et, AVULONG t, AVULONG s)	{ event_type = et; nTime = t; nStorey = s; }
		friend bool operator <(RECORD &r1, RECORD &r2) { return r1.nTime < r2.nTime; }
	};
	std::vector <RECORD> m_rec;
public:
	CSimDeducer(CSimBase &sim, AVULONG nLiftId);

	AVULONG GetJourneysCount();
	JOURNEY *GetJourneys();

private:
	void Record(AVULONG nArrivalFloor, AVULONG nDestFloor, AVULONG timeArrival, AVULONG timeLoad, AVULONG timeUnload);
	void ConsolidateRecords();
	AVULONG CalculateTypicalJourneyTime(AVULONG floorFrom, AVULONG floorTo);
};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// CSimDeducer - used to create JOURNEY's after old-style, incomplete XML data were loaded

CSimDeducer::CSimDeducer(CSimBase &sim, AVULONG nLiftId)
{
	m_pBuilding = sim.GetBuilding();
	m_nLiftId = nLiftId;

	for (AVULONG i = 0; i < sim.GetPassengerCount(); i++)
	{
		if (sim.GetPassenger(i)->GetLiftId() != nLiftId) continue;
		Record(sim.GetPassenger(i)->GetArrivalFloor(), sim.GetPassenger(i)->GetDestFloor(), sim.GetPassenger(i)->GetArrivalTime(), sim.GetPassenger(i)->GetLoadTime(), sim.GetPassenger(i)->GetUnloadTime());
	}
}

AVULONG CSimDeducer::GetJourneysCount()
{
	if (journeys.size() == 0)
		ConsolidateRecords();
	return journeys.size();
}

JOURNEY *CSimDeducer::GetJourneys()
{
	AVULONG nJourneys = GetJourneysCount();
	JOURNEY *pJourneys = new JOURNEY[nJourneys];
	int j = 0;
	for (vector<JOURNEY>::iterator i = journeys.begin(); i != journeys.end(); i++)
	{
		pJourneys[j] = *i;
		j++;
	}
	return pJourneys;
}

AVULONG CSimDeducer::CalculateTypicalJourneyTime(AVULONG floorFrom, AVULONG floorTo)
{
	AVFLOAT dist = abs(m_pBuilding->pStoreys[floorFrom].StoreyLevel - m_pBuilding->pStoreys[floorTo].StoreyLevel) / 1000.0f;
	AVFLOAT v = m_pBuilding->pLifts[m_nLiftId].Speed;
	if (dist == 0)
		return 0;
	else
		return (AVULONG)(1000.0f * dist / v) + 750;
}

void CSimDeducer::Record(AVULONG nArrivalFloor, AVULONG nDestFloor, AVULONG timeArrival, AVULONG timeLoad, AVULONG timeUnload)
{
	m_rec.push_back(RECORD(REQ, timeArrival, nArrivalFloor));
	m_rec.push_back(RECORD(CALL, timeLoad, nArrivalFloor));
	m_rec.push_back(RECORD(ARR, timeUnload, nDestFloor));
}

void CSimDeducer::ConsolidateRecords()
{
	AVULONG n = m_rec.size();
	if (n == 0) return;

	sort(m_rec.begin(), m_rec.end());

	const AVULONG TTR = 2000;		// time to run - after last passenger embarked
	const AVULONG TTC = 5000;		// time to close - after last passenger left and lift is idle
	const AVULONG TTC_MIN = 2000;	// time to close - after last passenger left, minimum time if lift is called at another place

	JOURNEY journey;
	AVULONG nStorey = 0;
	for (AVULONG i = 0; i < n; i++)
	{
		RECORD *pRecord = &(m_rec[i]);
		if (pRecord->event_type == REQ)
			continue;	// no reaction for requests at this stage

		AVULONG t = pRecord->nTime;
		AVULONG s = pRecord->nStorey;

		if (s == nStorey)
		{
			// call at the same storey
			if (journey.m_timeOpen == UNDEF && journey.m_timeDest == UNDEF)
			{											// first passenger arrived - door still closed
				ASSERT(journey.m_floorFrom == nStorey);	// open the door, plan time to go & to close
				journey.m_timeOpen = t - 1000;			// This may happen only for the initial (1st) jouney...
				journey.m_timeGo = t + TTR;
				journey.m_timeDest = UNDEF;
				journey.m_timeClose = t + TTC;
			}
			else
			if (journey.m_timeOpen != UNDEF && journey.m_timeDest == UNDEF)
			{											// another passenger arrived, dor is open but lift not started
				ASSERT(journey.m_floorFrom == nStorey);	// extend time to go & to close
				journey.m_timeGo = t + TTR;
				journey.m_timeClose = t + TTC;
				journey.m_timeDest = UNDEF;
			}
			else
			if (journey.m_timeDest != UNDEF && t < journey.m_timeClose)
			{											// the lift is already at the destination floor, but door is open
				ASSERT(journey.m_floorTo == nStorey);	// extend time to close
				journey.m_timeClose = t + TTC;
			}
			else
			if (journey.m_timeDest != UNDEF && t >= journey.m_timeClose)
			{											// the lift is already at the destination floor, and door is already closed
				ASSERT(journey.m_floorTo == nStorey);	// 1. the journey is complete -> register it
														// 2. open the door, plan time to go and close
				journey.m_bLegacy = true;
				journeys.push_back(journey);

				journey = JOURNEY();
				journey.m_floorFrom = nStorey;
				journey.m_floorTo = nStorey;
				journey.m_timeOpen = t - 1000;
				journey.m_timeGo = t + TTR;
				journey.m_timeClose = t + TTC;
				journey.m_timeDest = UNDEF;
			}
		}
		else
		{
			// arriving to a different storey
			AVULONG tjt = CalculateTypicalJourneyTime(nStorey, s);

			if (journey.m_timeOpen == UNDEF && journey.m_timeDest == UNDEF)
			{											// lift at its initial position, first call registered above the ground
				ASSERT(journey.m_floorFrom == nStorey);	// do not open door, start & arrive, plan time to close
														// This may happen only for the initial (1st) jouney...
				journey.m_floorTo = nStorey = s;
				journey.m_timeOpen = UNDEF;
				journey.m_timeGo = t - tjt;
				journey.m_timeDest = t;
				journey.m_timeClose = t + TTC;
				
				// problem: time left for journey is too short
				if (journey.m_timeGo < 0) journey.m_timeGo = 0;
			}
			else
			if (journey.m_timeOpen != UNDEF && journey.m_timeDest == UNDEF)
			{											// door is already open, lift should call now at another floor
				ASSERT(journey.m_floorFrom == nStorey);	// estimate time to go, set dest time, plan to close door
				journey.m_floorTo = nStorey = s;
				journey.m_timeGo = t - tjt;
				journey.m_timeDest = t;
				journey.m_timeClose = t + TTC;
				
				// problem: time left for journey is too short
				if (journey.m_timeGo < journey.m_timeOpen + TTR)
					journey.m_timeGo = journey.m_timeOpen + TTR;
				// problem: time left for journey is critically short
				if (journey.m_timeGo > journey.m_timeDest)
					journey.m_timeGo = journey.m_timeOpen + 1000;
				ASSERT(journey.m_timeGo <= journey.m_timeDest);
			}
			else
			if (journey.m_timeDest != UNDEF)
			{											// lift is at dest floor, but a new call registered somewhere else
				ASSERT(journey.m_floorTo == nStorey);	// 1. prev. journey is complete => register it
				AVULONG m_timeGo = t - tjt;								// time to go
				AVULONG timeEC = journey.m_timeClose - TTC + TTC_MIN;	// earliest time to close
				AVULONG timeLT = journey.m_timeClose;					// latest time to close

				// two first items mean timing problems
				if (t < timeEC) journey.m_timeClose = m_timeGo = t;
				else if (m_timeGo < timeEC) journey.m_timeClose = m_timeGo = timeEC;
				else if (m_timeGo < timeLT) journey.m_timeClose = m_timeGo;

				journey.m_bLegacy = true;
				journeys.push_back(journey);

				journey = JOURNEY();					// 2. Start a new journey
				journey.m_floorFrom = nStorey;
				journey.m_floorTo = nStorey = s;
				journey.m_timeOpen = UNDEF;				// no need to open door
				journey.m_timeGo = m_timeGo;
				journey.m_timeDest = t;
				journey.m_timeClose = t + TTC;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Lagacy XML Parsers

HRESULT CSimBase::XmlParse(CComPtr<IXmlReader> pReader)
{
	HRESULT h;
	std::vector<CPassengerBase*> vec;

	XmlNodeType nodeType;
	LPCWSTR pLocalName;
	while ((h = pReader->Read(&nodeType)) == S_OK)
	{
		switch (nodeType)
		{
		case XmlNodeType_Element:
			h = pReader->GetLocalName(&pLocalName, NULL);
			if (wcscmp(pLocalName, L"PassengerOutput") == 0)
			{
				CPassengerBase *pPsngr = CreatePassenger(0);
				pPsngr->XmlParse(pReader);

				vec.push_back(pPsngr);
			}
			break;
		case XmlNodeType_Text:
			break;
		case XmlNodeType_EndElement:
			break;
		}
	}

	CreatePassengers(vec.size());
	if (GetPassengerCount() == 0)
		return E_SIM_PASSENGERS;
	for (AVULONG i = 0; i < GetPassengerCount(); i++)
		SetPassenger(i, vec[i]);

	CreateLifts();
	for (AVULONG i = 0; i < GetLiftCount(); i++)
	{
		CLiftBase *pLift = CreateLift(i);
		pLift->Load(i);
		SetLift(i, pLift);
	}
	return S_OK;
}

// Load for XML files or with SimIter information missing
void CLiftBase::Load(AVULONG nId)
{
	SetId(nId);

	CSimDeducer sd(*GetSim(), GetId());

	// Consolidate - store JOURNEY's
	SetJourneys(sd.GetJourneysCount(), sd.GetJourneys());

	Debug(L"Lift %d: %d travels found", GetId(), GetJourneyCount());

	//for (AVULONG i = 0; i < GetJourneyCount(); i++)
	//{
	//	JOURNEY j = m_pJourneys[i];
	//	if (j.timeOpen[0] == UNDEF) j.timeOpen[0] = 0;
	//	if (j.timeDest == UNDEF) j.timeDest = 0;
	//	Debug(L"  --> from %d to %d    OPEN: %8d  GO: %8.0d  ARRIVE: %8.0d  CLOSE: %8.0d", j.floorFrom, j.floorTo, j.timeOpen[0], j.timeGo, j.timeDest, j.timeFinalClose);
	//}
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Special Processing for "legacy" JOURNEY's


void CLift::Go(JOURNEY &j)
{
	if (!j.m_bLegacy)
		for (AVULONG iDeck = 0; iDeck < DECK_NUM; iDeck++)
			for (AVULONG iCycle = 0; iCycle < j.m_numDoors[iDeck]; iCycle++)
			{
				AnimateDoor(j.m_floorFrom+iDeck, true,  j.m_pDoors[iDeck][iCycle].m_timeOpen , j.m_pDoors[iDeck][iCycle].m_durationOpen);
				AnimateDoor(j.m_floorFrom+iDeck, false, j.m_pDoors[iDeck][iCycle].m_timeClose, j.m_pDoors[iDeck][iCycle].m_durationClose);
			}
	else
	{
		// legacy code (XML)
		if (j.m_timeOpen  != UNDEF) AnimateDoor(j.m_floorFrom, true,  j.m_timeOpen, 1000);
		if (j.m_timeGo    != UNDEF) AnimateDoor(j.m_floorFrom, false, j.m_timeGo - 1000, 1000);
		if (j.m_timeDest  != UNDEF) AnimateDoor(j.m_floorTo,   true,  j.m_timeDest, 1000);
		if (j.m_timeClose != UNDEF) AnimateDoor(j.m_floorFrom, false, j.m_timeClose, 1000);
	}

	// journey
	if (j.m_timeGo != UNDEF && j.m_timeDest != UNDEF)
		AnimateJourney(j.m_floorFrom, j.m_floorTo, j.m_timeGo, j.m_timeDest - j.m_timeGo);
}
