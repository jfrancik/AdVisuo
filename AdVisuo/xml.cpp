// Xml.cpp - a part of the AdVisuo Client Software

#include "StdAfx.h"
#include "Building.h"
#include "Sim.h"
#include "Lift.h"
#include "Passenger.h"
#include <list>

using namespace std;

//////////////////////////////////////////////////////////////////////////////////
// Protected Static Operations:
// Get XML Reader/Writer from File or Buffer

CComPtr<IXmlReader> CSim::GetReaderFromBuf(LPCOLESTR pBuf)
{
    HRESULT h;
    CComPtr<IStream> pStream;
    CComPtr<IXmlReader> pReader;
	CComPtr<IXmlReaderInput> pReaderInput;

	size_t nSize = sizeof(pBuf[0]) * wcslen(pBuf);
	HGLOBAL	hMem = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
	LPVOID pImage = ::GlobalLock(hMem);
	memcpy(pImage, pBuf, nSize);
	::GlobalUnlock(hMem);

	h = CreateStreamOnHGlobal(hMem, TRUE, &pStream); if (FAILED(h)) throw _com_error(h);
	h = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL); if (FAILED(h)) throw _com_error(h);
	h = CreateXmlReaderInputWithEncodingName(pStream, NULL, L"utf-16", FALSE, L"", &pReaderInput);
    h = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit); if (FAILED(h)) throw _com_error(h);
    h = pReader->SetInput(pReaderInput); if (FAILED(h)) throw _com_error(h);

	return pReader;
}

CComPtr<IXmlWriter> CSim::GetWriterToBuf(LPCOLESTR pBuf, size_t nSize)
{
	ASSERT(FALSE);
	// this function does not work well - needs to be fixed

    HRESULT h;
    CComPtr<IStream> pStream;
    CComPtr<IXmlWriter> pWriter;
	CComPtr<IXmlReaderInput> pReaderInput;

	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, nSize);

	h = CreateStreamOnHGlobal(hMem, FALSE, &pStream); if (FAILED(h)) throw _com_error(h);
	h = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL); if (FAILED(h)) throw _com_error(h);
    h = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE); if (FAILED(h)) throw _com_error(h);
    h = pWriter->SetOutput(pStream); if (FAILED(h)) throw _com_error(h);

	return pWriter;
}

CComPtr<IXmlWriter> CSim::GetWriterToFile(LPCOLESTR pFileName)
{
    HRESULT h;
    CComPtr<IStream> pFileStream;
    CComPtr<IXmlWriter> pWriter;

	h = SHCreateStreamOnFile(pFileName, STGM_CREATE | STGM_WRITE, &pFileStream); if (FAILED(h)) throw _com_error(h);
    h = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL); if (FAILED(h)) throw _com_error(h);
    h = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE); if (FAILED(h)) throw _com_error(h);
    h = pWriter->SetOutput(pFileStream); if (FAILED(h)) throw _com_error(h);

	return pWriter;
}

CComPtr<IXmlReader> CSim::GetReaderFromFile(LPCOLESTR pFileName)
{
    HRESULT h;
    CComPtr<IStream> pFileStream;
    CComPtr<IXmlReader> pReader;

	h = SHCreateStreamOnFile(pFileName, STGM_READ, &pFileStream); if (FAILED(h)) throw _com_error(h);
    h = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL); if (FAILED(h)) throw _com_error(h);
    h = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit); if (FAILED(h)) throw _com_error(h);
    h = pReader->SetInput(pFileStream); if (FAILED(h)) throw _com_error(h);

	return pReader;
}

//////////////////////////////////////////////////////////////////////////////////
// Load from XML

// in Ruby it would be a class extension
void Journey_Parse(JOURNEY &j, CComPtr<IXmlReader> pReader, LPCWSTR pTagName, AVULONG &nLiftID);

static void _parse(AVULONG &var, LPCWSTR pVal)			{ var = _wtoi(pVal); }
static void _parse(AVLONG &var, LPCWSTR pVal)			{ var = _wtoi(pVal); }
static void _parse(AVFLOAT &var, LPCWSTR pVal)			{ var = (AVFLOAT)_wtof(pVal); }
static void _parse(bool &var, LPCWSTR pVal)				{ var = (wcscmp(pVal, L"true") == 0); }
static void _parse(wstring &var, LPCWSTR pVal)			{ var = pVal; }

#define DEFINE_PARSER(TYPE) void _parse(TYPE &var, LPCWSTR pVal)	{ var = (TYPE)_wtoi(pVal); }
DEFINE_PARSER(CBuildingBase::SHAFT_ARRANGEMENT)
DEFINE_PARSER(CBuildingBase::LOBBY_ARRANGEMENT)
DEFINE_PARSER(CBuildingBase::DOOR_TYPE)
DEFINE_PARSER(CBuildingBase::TYPE_OF_LIFT)
DEFINE_PARSER(CBuildingBase::CAR_ENTRANCES)
DEFINE_PARSER(CBuildingBase::CNTRWEIGHT_POS)
DEFINE_PARSER(CBuildingBase::LIFT_STRUCTURE)
DEFINE_PARSER(CSimBase::ALGORITHM)
void _parse(enum ENUM_ACTION &var, LPCWSTR pVal)	{ var = (enum ENUM_ACTION)_wtoi(pVal); }

#define BEGIN_PARSE_LIST(READER, NAME)						\
{															\
	CComPtr<IXmlReader> __pReader = (READER);				\
	LPCWSTR pMyName = (NAME);								\
	HRESULT h;												\
	XmlNodeType nodeType;									\
	LPCWSTR pLocalName, pValue;								\
	bool bDone = false;										\
	while (!bDone && (h = __pReader->Read(&nodeType)) == S_OK)		\
	{														\
		switch (nodeType)									\
		{													\
		case XmlNodeType_Element:							\
			h = __pReader->GetLocalName(&pLocalName, NULL); \
			break;											\
		case XmlNodeType_Text:								\
			h = __pReader->GetValue(&pValue, NULL);			\
			if (0) ;

#define PARSE_LIST_ITEM(STR, VAR)	else if (wcscmp(pLocalName, STR) == 0) _parse(VAR, pValue);

#define END_PARSE_LIST										\
			break;											\
		case XmlNodeType_EndElement:						\
			h = __pReader->GetLocalName(&pLocalName, NULL);	\
			if (wcscmp(pLocalName, pMyName) == 0)			\
				bDone = true;								\
			break;											\
		}													\
	}														\
}

void CSim::Load(CComPtr<IXmlReader> pReader)
{
	XmlNodeType nodeType;
	LPCWSTR pLocalName;
	HRESULT h;

	CBuilding *pBuilding = GetBuilding();
	CBuilding::SHAFT *pShaft = NULL;
	CBuilding::STOREY *pStorey = NULL;
	if (!pBuilding) throw _sim_error(_sim_error::E_SIM_INTERNAL);

	AVULONG iLift = 0, iShaft = 0, iStorey = 0;//StoreysBelowGround;

	while ((h = pReader->Read(&nodeType)) == S_OK)
	{
		switch (nodeType)
		{
		case XmlNodeType_Element:
			h = pReader->GetLocalName(&pLocalName, NULL);
			if (wcscmp(pLocalName, L"AVProject") == 0) 
			{
				if (m_phase != PHASE_NONE) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
				m_phase = PHASE_PRJ;

				XParse(pReader, L"AVProject");
			}
			else if (wcscmp(pLocalName, L"AVBuilding") == 0) 
			{
				if (m_phase != PHASE_PRJ) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
				m_phase = PHASE_BLD;

				pBuilding->XParse(pReader, L"AVBuilding");
				pBuilding->CreateShafts();
				pBuilding->CreateStoreys();
			}
			else if (wcscmp(pLocalName, L"AVShaft") == 0)
			{
				if (m_phase != PHASE_BLD && m_phase != PHASE_STRUCT) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
				m_phase = PHASE_STRUCT;

				if (iShaft >= GetBuilding()->GetShaftCount()) throw _sim_error(_sim_error::E_SIM_LIFTS);
				CBuilding::SHAFT *pPrevShaft = pShaft;
				pShaft = pBuilding->GetShaft(iShaft);
				pShaft->XParse(pReader, L"AVShaft", GetBuilding());
				iShaft++;

				for (AVULONG i = 0; i < pShaft->NumberOfLifts; i++)
				{
					CLiftBase *pLift = CreateLift(iLift);
					AddLift(pLift);
					iLift++;
				}
			}
			else if (wcscmp(pLocalName, L"AVFloor") == 0) 
			{
				if (m_phase != PHASE_BLD && m_phase != PHASE_STRUCT) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
				m_phase = PHASE_STRUCT;

				if (iStorey >= GetBuilding()->GetStoreyCount()) throw _sim_error(_sim_error::E_SIM_FLOORS);
				CBuilding::STOREY *pPrevStorey = pStorey;
				pStorey = pBuilding->GetStorey(iStorey);
				pStorey->XParse(pReader, L"AVFloor");
				iStorey++;
			}
			else if (wcscmp(pLocalName, L"AVJourney") == 0) 
			{
				if (m_phase != PHASE_STRUCT && m_phase != PHASE_SIM) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
				m_phase = PHASE_SIM;

				JOURNEY journey;
				AVULONG nLiftID;
				Journey_Parse(journey, pReader, L"AVJourney", nLiftID);
				  
				if (nLiftID >= pBuilding->GetLiftCount() || nLiftID >= LIFT_MAXNUM || journey.m_shaftFrom >= pBuilding->GetShaftCount() || journey.m_shaftTo >= pBuilding->GetShaftCount()) 
					throw _sim_error(_sim_error::E_SIM_LIFTS);

				GetLift(nLiftID)->AddJourney(journey);
			}
			else if (wcscmp(pLocalName, L"AVPassenger") == 0) 
			{
				if (m_phase != PHASE_STRUCT && m_phase != PHASE_SIM) throw _sim_error(_sim_error::E_SIM_FILE_STRUCT);
				m_phase = PHASE_SIM;

				CPassenger *pPassenger = (CPassenger*)CreatePassenger(0);
				pPassenger->XParse(pReader, L"AVPassenger");
				AddPassenger(pPassenger);
			}
			break;
		case XmlNodeType_Text:
			break;
		case XmlNodeType_EndElement:
			break;
		}
	}
	// Some tests
	if (m_nProjectID == 0)
		throw _sim_error(_sim_error::E_SIM_NOT_FOUND);
	if (m_phase == PHASE_STRUCT && iShaft != GetBuilding()->GetShaftCount())
		throw _sim_error(_sim_error::E_SIM_LIFTS);
	if (m_phase == PHASE_STRUCT && iStorey != GetBuilding()->GetStoreyCount())
		throw _sim_error(_sim_error::E_SIM_FLOORS);

	if (m_phase >= PHASE_STRUCT) 
		GetBuilding()->Resolve();

	if (m_phase >= PHASE_STRUCT && !GetBuilding()->IsValid())
		throw _sim_error(_sim_error::E_SIM_NO_BUILDING);
}

void CSim::LoadIndex(CComPtr<IXmlReader> pReader, vector<CSim*> &sims)
{
	XmlNodeType nodeType;
	LPCWSTR pLocalName;
	HRESULT h;

	while ((h = pReader->Read(&nodeType)) == S_OK)
	{
		switch (nodeType)
		{
		case XmlNodeType_Element:
			h = pReader->GetLocalName(&pLocalName, NULL);
			if (wcscmp(pLocalName, L"AVProject") == 0) 
			{
				CSim *pSim = new CSim(NULL);
				pSim->XParse(pReader, L"AVProject");
				sims.push_back(pSim);
			}
			break;
		case XmlNodeType_Text:
			break;
		case XmlNodeType_EndElement:
			break;
		}
	}
	// Some tests
//	if (m_nProjectID == 0)
//		throw _sim_error(_sim_error::E_SIM_NOT_FOUND);
}

void CSim::XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName)
{
	BEGIN_PARSE_LIST(pReader, pTagName)
	PARSE_LIST_ITEM(L"ID",				m_nProjectID)
	PARSE_LIST_ITEM(L"SimulationID",	m_nSimulationID)
	PARSE_LIST_ITEM(L"SIMVersionID",	m_nSIMVersionID)
	PARSE_LIST_ITEM(L"AVVersionId",		m_nAVVersionID)
	PARSE_LIST_ITEM(L"Floors",			m_nBldFloors)
	PARSE_LIST_ITEM(L"Shafts",			m_nBldShafts)
	PARSE_LIST_ITEM(L"Lifts",			m_nBldLifts)
	PARSE_LIST_ITEM(L"Passengers",		m_nPassengers)
	PARSE_LIST_ITEM(L"SimulationTime",	m_nSimulationTime)

	PARSE_LIST_ITEM(L"JourneysSaved",	m_nJourneysSaved)
	PARSE_LIST_ITEM(L"PassengersSaved",	m_nPassengersSaved)
	PARSE_LIST_ITEM(L"TimeSaved",		m_nTimeSaved)
	PARSE_LIST_ITEM(L"SavedAll",		m_bSavedAll)

	PARSE_LIST_ITEM(L"SIMFileName",		m_strSIMFileName)
	PARSE_LIST_ITEM(L"IFCFileName",		m_strIFCFileName)
	PARSE_LIST_ITEM(L"ProjectName",		m_strProjectName)
	PARSE_LIST_ITEM(L"Language",		m_strLanguage)
	PARSE_LIST_ITEM(L"MeasurementUnits",m_strMeasurementUnits)
	PARSE_LIST_ITEM(L"BuildingName",	m_strBuildingName)
	PARSE_LIST_ITEM(L"ClientCompany",	m_strClientCompanyName)
	PARSE_LIST_ITEM(L"City",			m_strCity)
	PARSE_LIST_ITEM(L"LBRegionDistrict",m_strLBRegionDistrict)
	PARSE_LIST_ITEM(L"StateCounty",		m_strStateCounty)
	PARSE_LIST_ITEM(L"LiftDesigner",	m_strLiftDesigner)
	PARSE_LIST_ITEM(L"Country",			m_strCountry)
	PARSE_LIST_ITEM(L"CheckedBy",		m_strCheckedBy)
	PARSE_LIST_ITEM(L"PostalZipCode",	m_strPostalZipCode)
	PARSE_LIST_ITEM(L"Algorithm",		m_nAlgorithm)
	END_PARSE_LIST
}

void CBuilding::SHAFT::XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName, CBuilding *pBuilding)
{
	BEGIN_PARSE_LIST(pReader, pTagName)
	PARSE_LIST_ITEM(L"ShaftID",					ShaftID)
	//PARSE_LIST_ITEM(L"BuildingID",			nBuildingID)
	PARSE_LIST_ITEM(L"Capacity",				Capacity)
	PARSE_LIST_ITEM(L"Speed",					Speed)
	PARSE_LIST_ITEM(L"Acceleration",			Acceleration)
	PARSE_LIST_ITEM(L"Jerk",					Jerk)
	PARSE_LIST_ITEM(L"DoorType",				DoorType)
	PARSE_LIST_ITEM(L"LiftDoorHeight",			LiftDoorHeight)
	PARSE_LIST_ITEM(L"LiftDoorWidth",			LiftDoorWidth)
	PARSE_LIST_ITEM(L"CarHeight",				CarHeight)
	PARSE_LIST_ITEM(L"TypeOfLift",				TypeOfLift)
	PARSE_LIST_ITEM(L"NumberOfLifts",			NumberOfLifts)
	PARSE_LIST_ITEM(L"MachRoomSlab",			pBuilding->MachRoomSlab)		// kept for compatibility - now loaded with the Building
	PARSE_LIST_ITEM(L"LiftBeamHeight",			pBuilding->LiftBeamHeight)		// kept for compatibility - now loaded with the Building
	PARSE_LIST_ITEM(L"NoOfCarEntrances",		NoOfCarEntrances)
	PARSE_LIST_ITEM(L"CounterWeightPosition",	CounterWeightPosition)
	PARSE_LIST_ITEM(L"Structure",				pBuilding->Structure)			// kept for compatibility - now loaded with the Building
	PARSE_LIST_ITEM(L"CarWidth",				CarWidth)
	PARSE_LIST_ITEM(L"CarDepth",				CarDepth)
	PARSE_LIST_ITEM(L"ShaftWidth",				ShaftWidth)
	PARSE_LIST_ITEM(L"ShaftDepth",				ShaftDepth)
	PARSE_LIST_ITEM(L"IntDivBeam",				pBuilding->IntDivBeamWidth)		// kept for compatibility - now loaded with the Building
	PARSE_LIST_ITEM(L"PitDepth",				PitDepth)
	PARSE_LIST_ITEM(L"OverallHeight",			OverallHeight)
	PARSE_LIST_ITEM(L"HeadRoom",				HeadRoom)
	PARSE_LIST_ITEM(L"MachRoomHeight",			MachRoomHeight)
	PARSE_LIST_ITEM(L"MachRoomExt",				MachRoomExt)
    
	PARSE_LIST_ITEM(L"PreOperTime",				PreOperTime)
    PARSE_LIST_ITEM(L"OpeningTime",				OpeningTime)
    PARSE_LIST_ITEM(L"ClosingTime",				ClosingTime)
    PARSE_LIST_ITEM(L"MotorStartDelay",			MotorStartDelay)
    PARSE_LIST_ITEM(L"LoadingTime",				LoadingTime)
    PARSE_LIST_ITEM(L"UnloadingTime",			UnloadingTime)
    PARSE_LIST_ITEM(L"FloorsServed",			FloorsServed)
	END_PARSE_LIST

	if (pBuilding->IntDivBeamHeight < 0)
		pBuilding->IntDivBeamHeight = pBuilding->IntDivBeamWidth * 250.0f / 150.0f;
}

void CBuilding::STOREY::XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName)
{
	BEGIN_PARSE_LIST(pReader, pTagName)
	PARSE_LIST_ITEM(L"FloorID",				StoreyID)
	PARSE_LIST_ITEM(L"HeightValue",			HeightValue)

    PARSE_LIST_ITEM(L"Area",				Area)
    PARSE_LIST_ITEM(L"PopDensity",			PopDensity)
    PARSE_LIST_ITEM(L"Absentee",			Absentee)
    PARSE_LIST_ITEM(L"StairFactor",			StairFactor)
    PARSE_LIST_ITEM(L"Escalator",			Escalator)
    PARSE_LIST_ITEM(L"Name",				Name)
	END_PARSE_LIST
}

void CBuilding::XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName)
{
	BEGIN_PARSE_LIST(pReader, pTagName)
	PARSE_LIST_ITEM(L"BuildingName",			BuildingName)
	PARSE_LIST_ITEM(L"ID",						nBuildingID)
	PARSE_LIST_ITEM(L"NoOfShafts",				NoOfShafts)
	PARSE_LIST_ITEM(L"PosLiftBookM",			PosLiftBookM)
	PARSE_LIST_ITEM(L"NoOfBook",				NoOfBook)
	PARSE_LIST_ITEM(L"LobbyDepth",				LobbyDepth)
	PARSE_LIST_ITEM(L"LobbyWidth",				LobbyWidth)
	PARSE_LIST_ITEM(L"FrontWallThickness",		FrontWallThickness)
	PARSE_LIST_ITEM(L"LobbyCeilingSlabHeight",	LobbyCeilingSlabHeight)
	PARSE_LIST_ITEM(L"FloorsAboveGround",		StoreysAboveGround)
	PARSE_LIST_ITEM(L"FloorsBelowGround",		StoreysBelowGround)
	PARSE_LIST_ITEM(L"LiftShaftArrangement",	LiftShaftArrang)
	PARSE_LIST_ITEM(L"LobbyArrangement",		LobbyArrangement)

	// seven new variables
	PARSE_LIST_ITEM(L"SideWallThickness",		SideWallThickness)
	PARSE_LIST_ITEM(L"ShaftWallThickness",		ShaftWallThickness)
	PARSE_LIST_ITEM(L"IntDivBeamWidth",			IntDivBeamWidth)
	PARSE_LIST_ITEM(L"IntDivBeamHeight",		IntDivBeamHeight)
	PARSE_LIST_ITEM(L"MachRoomSlab",			MachRoomSlab)
	PARSE_LIST_ITEM(L"LiftBeamHeight",			LiftBeamHeight)
	PARSE_LIST_ITEM(L"Structure",				Structure)

	END_PARSE_LIST
}

void Journey_Parse(JOURNEY &j, CComPtr<IXmlReader> pReader, LPCWSTR pTagName, AVULONG &nLiftID)
{
	std::wstring DC;
	BEGIN_PARSE_LIST(pReader, pTagName)
	PARSE_LIST_ITEM(L"LiftID",		nLiftID)
	PARSE_LIST_ITEM(L"ShaftFrom",	j.m_shaftFrom)
	PARSE_LIST_ITEM(L"ShaftTo",		j.m_shaftTo)
	PARSE_LIST_ITEM(L"FloorFrom",	j.m_floorFrom)
	PARSE_LIST_ITEM(L"FloorTo",		j.m_floorTo)
	PARSE_LIST_ITEM(L"TimeGo",		j.m_timeGo)
	PARSE_LIST_ITEM(L"TimeDest",	j.m_timeDest)
	PARSE_LIST_ITEM(L"DC",			DC)
	END_PARSE_LIST

	j.ParseDoorCycles(DC);
}

void CPassenger::XParse(CComPtr<IXmlReader> pReader, LPCWSTR pTagName)
{
	AVULONG ProjectId, PassengerId, ShaftId, LiftId, DeckId, FloorArrival, FloorDest, TimeBorn, TimeArrival, TimeGo, TimeLoad, TimeUnload, SpanWait;
	std::wstring WP;

	BEGIN_PARSE_LIST(pReader, pTagName)
	PARSE_LIST_ITEM(L"ProjectId",		ProjectId)
	PARSE_LIST_ITEM(L"PassengerId",		PassengerId)
	PARSE_LIST_ITEM(L"ShaftId",			ShaftId)
	PARSE_LIST_ITEM(L"LiftId",			LiftId)
	PARSE_LIST_ITEM(L"DeckId",			DeckId)
	PARSE_LIST_ITEM(L"FloorArrival",	FloorArrival)
	PARSE_LIST_ITEM(L"FloorDest",		FloorDest)
	PARSE_LIST_ITEM(L"TimeBorn",		TimeBorn)
	PARSE_LIST_ITEM(L"TimeArrival",		TimeArrival)
	PARSE_LIST_ITEM(L"TimeGo",			TimeGo)
	PARSE_LIST_ITEM(L"TimeLoad",		TimeLoad)
	PARSE_LIST_ITEM(L"TimeUnload",		TimeUnload)
	PARSE_LIST_ITEM(L"SpanWait",		SpanWait)
	PARSE_LIST_ITEM(L"WP",				WP)
	END_PARSE_LIST

	SetId(PassengerId);
	SetShaftId(ShaftId);
	SetLiftId(LiftId);
	SetDeck(DeckId);
	SetArrivalFloor(FloorArrival);
	SetDestFloor(FloorDest);
	SetBornTime(TimeBorn);
	SetArrivalTime(TimeArrival);
	SetGoTime(TimeGo);
	SetLoadTime(TimeLoad);
	SetUnloadTime(TimeUnload);
	SetWaitSpan(SpanWait);

	ParseWayPoints(WP);
}


////////////////////////////////////////////////////////////////////////////////////////////////
// Store as XML

static HRESULT _write(CComPtr<IXmlWriter> pWriter, AVULONG val)			{ wchar_t buf[80]; _snwprintf_s(buf, 80, L"%d", val); return pWriter->WriteString(buf); }
static HRESULT _write(CComPtr<IXmlWriter> pWriter, AVLONG val)			{ wchar_t buf[80]; _snwprintf_s(buf, 80, L"%d", val); return pWriter->WriteString(buf); }
static HRESULT _write(CComPtr<IXmlWriter> pWriter, AVFLOAT val)			{ wchar_t buf[80]; _snwprintf_s(buf, 80, L"%f", val); return pWriter->WriteString(buf); }
static HRESULT _write(CComPtr<IXmlWriter> pWriter, bool val)			{ return pWriter->WriteString(val ? L"true" : L"false"); }
static HRESULT _write(CComPtr<IXmlWriter> pWriter, std::wstring val)	{ return pWriter->WriteString(val.c_str()); }

#define BEGIN_GROUP(id)		h = pWriter->WriteStartElement(NULL, (id), NULL); if (FAILED(h)) throw _com_error(h);
#define STORE_ITEM(id, val)	h = pWriter->WriteStartElement(NULL, (id), NULL); if (FAILED(h)) throw _com_error(h); \
							h = _write(pWriter, val); if (FAILED(h)) throw _com_error(h); \
							h = pWriter->WriteFullEndElement(); if (FAILED(h)) throw _com_error(h);
#define END_GROUP			h = pWriter->WriteFullEndElement(); if (FAILED(h)) throw _com_error(h);

// in Ruby it would be a class extension
void Journey_Feed(JOURNEY &j, CComPtr<IXmlWriter> pWriter, LPCWSTR pTagName, AVULONG nLiftID);

void CSim::Store(CComPtr<IXmlWriter> pWriter)
{
	HRESULT h;

	CBuilding *pBuilding = GetBuilding();
	if (!pBuilding) throw _sim_error(_sim_error::E_SIM_INTERNAL);

    h = pWriter->WriteStartDocument(XmlStandalone_Omit); if (FAILED(h)) throw _com_error(h);
	BEGIN_GROUP(L"AdVisuo-Saved-Project")

		STORE_ITEM(L"AdVisuo-Client-Version", std::wstring(L"1.00"))

		XFeed(pWriter, L"AVProject");
		
		pBuilding->XFeed(pWriter, L"AVBuilding");

		BEGIN_GROUP(L"AVShafts")
			for (ULONG i = 0; i < pBuilding->GetShaftCount(); i++)
				pBuilding->GetShaft(i)->XFeed(pWriter, pBuilding->GetId(), L"AVShaft");
		END_GROUP
	
		BEGIN_GROUP(L"AVFloors")
			for (ULONG i = 0; i < pBuilding->GetStoreyCount(); i++)
				pBuilding->GetStorey(i)->XFeed(pWriter, pBuilding->GetId(), L"AVFloor");
		END_GROUP

		
		BEGIN_GROUP(L"AVJourneys")
			for (ULONG i = 0; i < GetLiftCount(); i++)
				for (ULONG j = 0; j < GetLift(i)->GetJourneyCount(); j++)
				{
					JOURNEY *pJ = GetLift(i)->GetJourney(j);
					Journey_Feed(*pJ, pWriter, L"AVJourney", GetLift(i)->GetId());
				}
		END_GROUP

		BEGIN_GROUP(L"AVPassengers")
		for (ULONG i = 0; i < GetPassengerCount(); i++)
			GetPassenger(i)->XFeed(pWriter, L"AVPassenger");
		END_GROUP

	END_GROUP
	
	h = pWriter->Flush(); if (FAILED(h)) throw _com_error(h);
}

void CSim::XFeed(CComPtr<IXmlWriter> pWriter, LPCWSTR pTagName)
{
	HRESULT h;
	BEGIN_GROUP(pTagName)
		STORE_ITEM(L"ID",				m_nProjectID)
		STORE_ITEM(L"SimulationID",		m_nSimulationID)
		STORE_ITEM(L"SIMVersionID",		m_nSIMVersionID)
		STORE_ITEM(L"AVVersionId",		m_nAVVersionID)
		STORE_ITEM(L"Floors",			m_nBldFloors)
		STORE_ITEM(L"Shafts",			m_nBldShafts)
		STORE_ITEM(L"Lifts",			m_nBldLifts)
		STORE_ITEM(L"Passengers",		m_nPassengers)
		STORE_ITEM(L"SimulationTime",	m_nSimulationTime)

		STORE_ITEM(L"JourneysSaved",	m_nJourneysSaved)
		STORE_ITEM(L"PassengersSaved",	m_nPassengersSaved)
		STORE_ITEM(L"TimeSaved",		m_nTimeSaved)
		STORE_ITEM(L"SavedAll",			m_bSavedAll)

		STORE_ITEM(L"SIMFileName",		m_strSIMFileName)
		STORE_ITEM(L"IFCFileName",		m_strIFCFileName)
		STORE_ITEM(L"ProjectName",		m_strProjectName)
		STORE_ITEM(L"Language",			m_strLanguage)
		STORE_ITEM(L"MeasurementUnits",	m_strMeasurementUnits)
		STORE_ITEM(L"BuildingName",		m_strBuildingName)
		STORE_ITEM(L"ClientCompany",	m_strClientCompanyName)
		STORE_ITEM(L"City",				m_strCity)
		STORE_ITEM(L"LBRegionDistrict",	m_strLBRegionDistrict)
		STORE_ITEM(L"StateCounty",		m_strStateCounty)
		STORE_ITEM(L"LiftDesigner",		m_strLiftDesigner)
		STORE_ITEM(L"Country",			m_strCountry)
		STORE_ITEM(L"CheckedBy",		m_strCheckedBy)
		STORE_ITEM(L"PostalZipCode",	m_strPostalZipCode)
		STORE_ITEM(L"Algorithm",		(AVULONG)m_nAlgorithm)
	END_GROUP
}

void CBuilding::SHAFT::XFeed(CComPtr<IXmlWriter> pWriter, AVULONG nIdBuilding, LPCWSTR pTagName)
{
	HRESULT h;
	BEGIN_GROUP(pTagName)
		STORE_ITEM(L"ShaftID",				ShaftID)
		STORE_ITEM(L"BuildingID",			nIdBuilding)
		STORE_ITEM(L"Capacity",				Capacity)
		STORE_ITEM(L"Speed",				Speed)
		STORE_ITEM(L"Acceleration",			Acceleration)
		STORE_ITEM(L"Jerk",					Jerk)
		STORE_ITEM(L"DoorType",				(AVULONG)DoorType)
		STORE_ITEM(L"LiftDoorHeight",		LiftDoorHeight)
		STORE_ITEM(L"LiftDoorWidth",		LiftDoorWidth)
		STORE_ITEM(L"CarHeight",			CarHeight)
		STORE_ITEM(L"TypeOfLift",			(AVULONG)TypeOfLift)
		STORE_ITEM(L"NumberOfLifts",		NumberOfLifts)
//		STORE_ITEM(L"MachRoomSlab",			MachRoomSlab)						// obsolete, now fed with the building
//		STORE_ITEM(L"LiftBeamHeight",		LiftBeamHeight)						// obsolete, now fed with the building
		STORE_ITEM(L"NoOfCarEntrances",		(AVULONG)NoOfCarEntrances)
		STORE_ITEM(L"CounterWeightPosition",(AVULONG)CounterWeightPosition)
//		STORE_ITEM(L"Structure",			(AVULONG)Structure)					// obsolete, now fed with the building
		STORE_ITEM(L"CarWidth",				CarWidth)
		STORE_ITEM(L"CarDepth",				CarDepth)
		STORE_ITEM(L"ShaftWidth",			ShaftWidth)
		STORE_ITEM(L"ShaftDepth",			ShaftDepth)
//		STORE_ITEM(L"IntDivBeam",			IntDivBeam)							// obsolete, now fed with the building
		STORE_ITEM(L"PitDepth",				PitDepth)
		STORE_ITEM(L"OverallHeight",		OverallHeight)
		STORE_ITEM(L"HeadRoom",				HeadRoom)
		STORE_ITEM(L"MachRoomHeight",		MachRoomHeight)
		STORE_ITEM(L"MachRoomExt",			MachRoomExt)
    
		STORE_ITEM(L"PreOperTime",			PreOperTime)
		STORE_ITEM(L"OpeningTime",			OpeningTime)
		STORE_ITEM(L"ClosingTime",			ClosingTime)
		STORE_ITEM(L"MotorStartDelay",		MotorStartDelay)
		STORE_ITEM(L"LoadingTime",			LoadingTime)
		STORE_ITEM(L"UnloadingTime",		UnloadingTime)
		STORE_ITEM(L"FloorsServed",			FloorsServed)
	END_GROUP
}

void CBuilding::STOREY::XFeed(CComPtr<IXmlWriter> pWriter, AVULONG nIdBuilding, LPCWSTR pTagName)
{
	HRESULT h;
	BEGIN_GROUP(pTagName)
		STORE_ITEM(L"FloorID",			StoreyID)
		STORE_ITEM(L"BuildingID",		nIdBuilding)
		STORE_ITEM(L"HeightValue",		HeightValue)

		STORE_ITEM(L"Area",				Area)
		STORE_ITEM(L"PopDensity",		PopDensity)
		STORE_ITEM(L"Absentee",			Absentee)
		STORE_ITEM(L"StairFactor",		StairFactor)
		STORE_ITEM(L"Escalator",		Escalator)
		STORE_ITEM(L"Name",				Name)
	END_GROUP
}

void CBuilding::XFeed(CComPtr<IXmlWriter> pWriter, LPCWSTR pTagName)
{
	HRESULT h;
	BEGIN_GROUP(pTagName)
		STORE_ITEM(L"BuildingName",			BuildingName)
		STORE_ITEM(L"ID",					nBuildingID)
		STORE_ITEM(L"NoOfShafts",			NoOfShafts)
		STORE_ITEM(L"PosLiftBookM",			PosLiftBookM)
		STORE_ITEM(L"NoOfBook",				NoOfBook)
		STORE_ITEM(L"LobbyDepth",			LobbyDepth)
		STORE_ITEM(L"LobbyWidth",			LobbyWidth)
		STORE_ITEM(L"FrontWallThickness",	FrontWallThickness)
		STORE_ITEM(L"LobbyCeilingSlabHeight",	LobbyCeilingSlabHeight)
		STORE_ITEM(L"FloorsAboveGround",	StoreysAboveGround)
		STORE_ITEM(L"FloorsBelowGround",	StoreysBelowGround)
		STORE_ITEM(L"LiftShaftArrangement",	(AVULONG)LiftShaftArrang)
		STORE_ITEM(L"LobbyArrangement",		(AVULONG)LobbyArrangement)

		// seven new variables
		STORE_ITEM(L"SideWallThickness",	SideWallThickness)
		STORE_ITEM(L"ShaftWallThickness",	ShaftWallThickness)
		STORE_ITEM(L"IntDivBeamWidth",		IntDivBeamWidth)
		STORE_ITEM(L"IntDivBeamHeight",		IntDivBeamHeight)
		STORE_ITEM(L"MachRoomSlab",			MachRoomSlab)
		STORE_ITEM(L"LiftBeamHeight",		LiftBeamHeight)
		STORE_ITEM(L"Structure",			(AVULONG)Structure)
	END_GROUP
}

void Journey_Feed(JOURNEY &j, CComPtr<IXmlWriter> pWriter, LPCWSTR pTagName, AVULONG nLiftID)
{
	HRESULT h;
	BEGIN_GROUP(pTagName)
		//STORE_ITEM(L"ID",			ID)
		STORE_ITEM(L"LiftID",		nLiftID)
		STORE_ITEM(L"ShaftFrom",	j.m_shaftFrom)
		STORE_ITEM(L"ShaftTo",		j.m_shaftTo)
		STORE_ITEM(L"FloorFrom",	j.m_floorFrom)
		STORE_ITEM(L"FloorTo",		j.m_floorTo)
		STORE_ITEM(L"TimeGo",		j.m_timeGo)
		STORE_ITEM(L"TimeDest",		j.m_timeDest)
		STORE_ITEM(L"DC",			j.StringifyDoorCycles())
	END_GROUP
}

void CPassenger::XFeed(CComPtr<IXmlWriter> pWriter, LPCWSTR pTagName)
{
	HRESULT h;
	BEGIN_GROUP(pTagName)
		STORE_ITEM(L"ProjectId",	GetSim()->m_nProjectID)
		STORE_ITEM(L"PassengerId",	GetId())
		STORE_ITEM(L"ShaftId",		GetShaftId())
		STORE_ITEM(L"LiftId",		GetLiftId())
		STORE_ITEM(L"DeckId",		GetDeck())
		STORE_ITEM(L"FloorArrival", GetArrivalFloor())
		STORE_ITEM(L"FloorDest",	GetDestFloor())
		STORE_ITEM(L"TimeBorn",		GetBornTime())
		STORE_ITEM(L"TimeArrival",	GetArrivalTime())
		STORE_ITEM(L"TimeGo",		GetGoTime())
		STORE_ITEM(L"TimeLoad",		GetLoadTime())
		STORE_ITEM(L"TimeUnload",	GetUnloadTime())
		STORE_ITEM(L"SpanWait",		GetWaitSpan())
		STORE_ITEM(L"WP",			StringifyWayPoints())
	END_GROUP
}
