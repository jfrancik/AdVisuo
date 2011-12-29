#include "StdAfx.h"
#include "Building.h"
#include "Vector.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)
 
CBuilding::CBuilding(void)
{
	pLifts = NULL;
	pStoreys = NULL;

	NoOfLifts = 4;
	PosLiftBookM = 0;
	NoOfBook = 3;
	LobbyDepth = 0;
	FrontWallThickness = 200;
	LobbyCeilingSlabHeight = 1100;
	StoreysAboveGround = 8;
	StoreysBelowGround = 0;
	HomeStorey = 0;
	LiftShaftArrang = SHAFT_INLINE;
	LobbyArrangement = LOBBY_OPENPLAN;

	NoOfStoreys = StoreysBelowGround + StoreysAboveGround;
	LobbyWidth = 0;		// unspecified at this stage
	NoOfLiftLines = (LiftShaftArrang == SHAFT_OPPOSITE) ? 2 : 1;
	NoOfLiftsInLine = (LiftShaftArrang == SHAFT_OPPOSITE) ? (NoOfLifts+1) / 2 : NoOfLifts;

	fScale = 0.0;
}

CBuilding::~CBuilding(void)
{
	if (pStoreys) delete [] pStoreys;
	if (pLifts) delete [] pLifts;
}

CBuilding::LIFT::LIFT()
{
	LiftID = 0;				// unspecified at this stage
	Capacity = 1275;
	Speed = 2.5f;
	DoorType = DOOR_CENTRE;
	UnOc = true;
	LiftDoorHeight = 2200;
	LiftDoorWidth = 1100;
	CarHeight = 2400;
	TypeOfLift = LIFT_SINGLE_DECK;
	MachRoomSlab = 300;
	LiftBeamHeight = 250;
	NoOfCarEntrances = CAR_FRONT;
	CounterWeightPosition = CNTRWEIGHT_REAR;
	Structure = STRUCT_CONCRETE;
	CarWidth = 2000;
	CarDepth = 1400;
	ShaftWidth = 2600;
	ShaftDepth = 2400;
	IntDivBeam = 150;
	PitDepth = 1800;
	OverallHeight = 3000;
	HeadRoom = 4950;
	MachRoomHeight = 2450;
}

CBuilding::STOREY::STOREY()
{
	StoreyID = 0;
	HeightValue = 3800;
	ExpressZone = false;
}


	// _helperParse(ANY_TYPE &var, LPCWSTR pLocalName, LPCWSTR pVal)
	wchar_t *LOOKUP_BOOL[]				= { L"false", L"true" };
	wchar_t *LOOKUP_SHAFT_ARRANGEMENT[]	= { L"Inline", L"Opposite" };
	wchar_t *LOOKUP_LOBBY_ARRANGEMENT[]	= { L"Through", L"Open Plan", L"Dead End on the Left", L"Dead End on the Right", };
	wchar_t *LOOKUP_DOOR_TYPE[]			= { L"Centre", L"Side" };
	wchar_t *LOOKUP_TYPE_OF_LIFT[]		= { L"Single Deck", L"Double Deck", L"Twin" };
	wchar_t *LOOKUP_CAR_ENTRANCES[]		= { L"Front", L"Rear", L"Both" };
	wchar_t *LOOKUP_CNTRWEIGHT_POS[]	= { L"Side", L"Rear" };
	wchar_t *LOOKUP_LIFT_STRUCTURE[]	= { L"Concrete", L"Steel" };
	int _lookup(const wchar_t *pVal, wchar_t *ppLookup[], int n)
	{
		for (int i = 0; i < n; i++) 
			if (wcscmp(pVal, ppLookup[i]) == 0) return i;
		return n;
	}
	void _helperParse(AVULONG &var, LPCWSTR pVal)	{ var = _wtoi(pVal); }
	void _helperParse(AVFLOAT &var, LPCWSTR pVal)	{ var = (AVFLOAT)_wtof(pVal); }
	void _helperParse(bool &var, LPCWSTR pVal)	{ var = (_lookup(pVal, LOOKUP_BOOL, 2) == 1); }
	#define DEFINE_HELPER_PARSE(TYPE) void _helperParse(CBuilding::TYPE &var, LPCWSTR pVal)	{ var = (CBuilding::TYPE)(_lookup(pVal, LOOKUP_##TYPE, sizeof(LOOKUP_TYPE_OF_LIFT) / sizeof(wchar_t*))); }
	DEFINE_HELPER_PARSE(SHAFT_ARRANGEMENT)
	DEFINE_HELPER_PARSE(LOBBY_ARRANGEMENT)
	DEFINE_HELPER_PARSE(DOOR_TYPE)
	DEFINE_HELPER_PARSE(TYPE_OF_LIFT)
	DEFINE_HELPER_PARSE(CAR_ENTRANCES)
	DEFINE_HELPER_PARSE(CNTRWEIGHT_POS)
	DEFINE_HELPER_PARSE(LIFT_STRUCTURE)


HRESULT CBuilding::LoadAsXML(LPCOLESTR pFileName)
{
    HRESULT h;
    CComPtr<IStream> pFileStream;
    CComPtr<IXmlReader> pReader;

	h = SHCreateStreamOnFile(pFileName, STGM_READ, &pFileStream); if (FAILED(h)) return h;
    h = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL); if (FAILED(h)) return h;
    h = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit); if (FAILED(h)) return h;
    h = pReader->SetInput(pFileStream); if (FAILED(h)) return h;

	AVULONG iLift = 0, iStorey = 0;//StoreysBelowGround;
	
	XmlNodeType nodeType;
	LPCWSTR pLocalName;
	while ((h = pReader->Read(&nodeType)) == S_OK)
	{
		switch (nodeType)
		{
		case XmlNodeType_Element:
			h = pReader->GetLocalName(&pLocalName, NULL);
			if (wcscmp(pLocalName, L"LobbyLayoutData") == 0) 
				XmlParse(pReader);
			else if (wcscmp(pLocalName, L"LiftLayoutData") == 0)
			{
				if (iLift < NoOfLifts)
				{
					pLifts[iLift].LiftID = iLift;
					pLifts[iLift++].XmlParse(pReader);
				}
			}
			else if (wcscmp(pLocalName, L"FloorData") == 0) 
			{
				if (iStorey < NoOfStoreys)
				{
					pStoreys[iStorey].StoreyID = iStorey;
					pStoreys[iStorey++].XmlParse(pReader);
				}
			}
			break;
		case XmlNodeType_Text:
			break;
		case XmlNodeType_EndElement:
			break;
		}
	}

	// Finalise all...
	if (!NoOfLifts || !NoOfStoreys || !pLifts || !pStoreys || !NoOfLiftsInLine)
		return E_FAIL;

	// set positions along the first line of the lifts...
	AVFLOAT posShaft = 0;
	for (AVULONG i = 0; i < NoOfLiftsInLine; i++)
	{
		pLifts[i].LeftBeam = pLifts[i].RightBeam = pLifts[i].RearBeam = pLifts[i].IntDivBeam;
		if (i > 0) pLifts[i].LeftBeam = pLifts[i-1].RightBeam = max(pLifts[i].LeftBeam, pLifts[i-1].RightBeam);

		pLifts[i].LiftLine  = 0;
		pLifts[i].ShaftPos  = posShaft;
		posShaft += pLifts[i].LeftBeam + pLifts[i].ShaftWidth;
	}
	LobbyWidth = posShaft + pLifts[NoOfLiftsInLine-1].RightBeam;

	// set positions along the first line of the lifts... (maybe none)
	posShaft = 0;
	for (AVULONG i = NoOfLiftsInLine; i < NoOfLifts; i++)
	{
		pLifts[i].LeftBeam = pLifts[i].RightBeam = pLifts[i].RearBeam = pLifts[i].IntDivBeam;
		if (i > NoOfLiftsInLine) pLifts[i].LeftBeam = pLifts[i-1].RightBeam = max(pLifts[i].LeftBeam, pLifts[i-1].RightBeam);

		pLifts[i].LiftLine  = 1;
		pLifts[i].ShaftPos  = posShaft;
		posShaft += pLifts[i].LeftBeam + pLifts[i].ShaftWidth;
	}
	LobbyWidth = max(LobbyWidth, posShaft + pLifts[NoOfLifts-1].RightBeam);

	if (LobbyDepth == 0) LobbyDepth = LobbyWidth * 2 / 3;
	else if (LobbyDepth < 5.0f) LobbyDepth = LobbyWidth * LobbyDepth;

	AVFLOAT Level = 0;
	for (AVULONG i = 0; i < NoOfStoreys; i++)
	{
		pStoreys[i].StoreyLevel = Level;
		Level += pStoreys[i].HeightValue;
	}

	return S_OK;
}

void CBuilding::XmlParse(CComPtr<IXmlReader> pReader)
{
	HRESULT h;
	XmlNodeType nodeType;
	LPCWSTR pLocalName, pValue;
	while ((h = pReader->Read(&nodeType)) == S_OK)
	{
		switch (nodeType)
		{
		case XmlNodeType_Element:
			h = pReader->GetLocalName(&pLocalName, NULL);
			break;
		case XmlNodeType_Text:
			h = pReader->GetValue(&pValue, NULL);
			if (wcscmp(pLocalName, L"NoOfLifts") == 0) _helperParse(NoOfLifts, pValue);
			else if (wcscmp(pLocalName, L"PosLiftBookM") == 0) _helperParse(PosLiftBookM, pValue);
			else if (wcscmp(pLocalName, L"NoOfBook") == 0) _helperParse(NoOfBook, pValue);
			else if (wcscmp(pLocalName, L"LobbyDepth") == 0) _helperParse(LobbyDepth, pValue);
			else if (wcscmp(pLocalName, L"FrontWallThickness") == 0) _helperParse(FrontWallThickness, pValue);
			else if (wcscmp(pLocalName, L"LobbyCeilingSlabHeight") == 0) _helperParse(LobbyCeilingSlabHeight, pValue);
			else if (wcscmp(pLocalName, L"FloorsAboveGround") == 0) _helperParse(StoreysAboveGround, pValue);
			else if (wcscmp(pLocalName, L"FloorsBelowGround") == 0) _helperParse(StoreysBelowGround, pValue);
			else if (wcscmp(pLocalName, L"HomeFloor") == 0) _helperParse(HomeStorey, pValue);
			else if (wcscmp(pLocalName, L"LiftShaftArrang") == 0) _helperParse(LiftShaftArrang, pValue);
			else if (wcscmp(pLocalName, L"LobbyArrangement") == 0) _helperParse(LobbyArrangement, pValue);
			else TRACE(L"Unrecognised XML tag: \"%ls\" with the value of: \"%ls\".\n", pLocalName, pValue);
			break;
		case XmlNodeType_EndElement:
			h = pReader->GetLocalName(&pLocalName, NULL);
			if (wcscmp(pLocalName, L"LobbyLayoutData") == 0)
			{
				// finalise LOBBY data
				NoOfStoreys = StoreysBelowGround + StoreysAboveGround;
				NoOfLiftLines = (LiftShaftArrang == SHAFT_OPPOSITE) ? 2 : 1;
				NoOfLiftsInLine = (LiftShaftArrang == SHAFT_OPPOSITE) ? (NoOfLifts+1) / 2 : NoOfLifts;
				if (pLifts) delete [] pLifts;
				pLifts = new LIFT[NoOfLifts];
				if (pStoreys) delete [] pStoreys;
				pStoreys = new STOREY[NoOfStoreys];
				return;
			}
			break;
		}
	}
}

void CBuilding::SetScaledDims(AVFLOAT fScale)
{
	this->fScale = fScale;

	m_box = BOX(-LobbyWidth*fScale/2, -LobbyDepth*fScale/2, LobbyWidth*fScale, LobbyDepth*fScale);
	m_box.SetThickness(FrontWallThickness*fScale);
	m_ceiling = LobbyCeilingSlabHeight*fScale;
	for (AVULONG i = 0; i < NoOfLifts; i++) pLifts[i].SetScaledDims(m_box, fScale);
	for (AVULONG i = 0; i < NoOfStoreys; i++) pStoreys[i].SetScaledDims(fScale);
}

void CBuilding::LIFT::XmlParse(CComPtr<IXmlReader> pReader)
{
	HRESULT h;
	XmlNodeType nodeType;
	LPCWSTR pLocalName, pValue;
	while ((h = pReader->Read(&nodeType)) == S_OK)
	{
		switch (nodeType)
		{
		case XmlNodeType_Element:
			h = pReader->GetLocalName(&pLocalName, NULL);
			break;
		case XmlNodeType_Text:
			h = pReader->GetValue(&pValue, NULL);
			if (wcscmp(pLocalName, L"Capacity") == 0) _helperParse(Capacity, pValue);
			else if (wcscmp(pLocalName, L"Speed") == 0) _helperParse(Speed, pValue);
			else if (wcscmp(pLocalName, L"DoorType") == 0) _helperParse(DoorType, pValue);
			else if (wcscmp(pLocalName, L"UnOc") == 0) _helperParse(UnOc, pValue);
			else if (wcscmp(pLocalName, L"LiftDoorHeight") == 0) _helperParse(LiftDoorHeight, pValue);
			else if (wcscmp(pLocalName, L"LiftDoorWidth") == 0) _helperParse(LiftDoorWidth, pValue);
			else if (wcscmp(pLocalName, L"CarHeight") == 0) _helperParse(CarHeight, pValue);
			else if (wcscmp(pLocalName, L"TypeOfLift") == 0) _helperParse(TypeOfLift, pValue);
			else if (wcscmp(pLocalName, L"MachRoomSlab") == 0) _helperParse(MachRoomSlab, pValue);
			else if (wcscmp(pLocalName, L"LiftBeamHeight") == 0) _helperParse(LiftBeamHeight, pValue);
			else if (wcscmp(pLocalName, L"NoOfCarEntrances") == 0) _helperParse(NoOfCarEntrances, pValue);
			else if (wcscmp(pLocalName, L"CounterWeightPosition") == 0) _helperParse(CounterWeightPosition, pValue);
			else if (wcscmp(pLocalName, L"Structure") == 0) _helperParse(Structure, pValue);
			else if (wcscmp(pLocalName, L"CarWidth") == 0) _helperParse(CarWidth, pValue);
			else if (wcscmp(pLocalName, L"CarDepth") == 0) _helperParse(CarDepth, pValue);
			else if (wcscmp(pLocalName, L"ShaftWidth") == 0) _helperParse(ShaftWidth, pValue);
			else if (wcscmp(pLocalName, L"ShaftDepth") == 0) _helperParse(ShaftDepth, pValue);
			else if (wcscmp(pLocalName, L"IntDivBeam") == 0) _helperParse(IntDivBeam, pValue);
			else if (wcscmp(pLocalName, L"PitDepth") == 0) _helperParse(PitDepth, pValue);
			else if (wcscmp(pLocalName, L"OverallHeight") == 0) _helperParse(OverallHeight, pValue);
			else if (wcscmp(pLocalName, L"HeadRoom") == 0) _helperParse(HeadRoom, pValue);
			else if (wcscmp(pLocalName, L"MachRoomHeight") == 0) _helperParse(MachRoomHeight, pValue);
			else TRACE(L"Unrecognised XML tag: \"%ls\" with the value of: \"%ls\".\n", pLocalName, pValue);
			break;
		case XmlNodeType_EndElement:
			h = pReader->GetLocalName(&pLocalName, NULL);
			if (wcscmp(pLocalName, L"LiftLayoutData") == 0)
				return;
			break;
		}
	}
}

void CBuilding::LIFT::SetScaledDims(BOX &lobbyBox, AVFLOAT fScale)
{
	// imposed parameters
	AVFLOAT wallThickness = LeftBeam*fScale;					// lift wall thickness
	AVFLOAT doorThickness = lobbyBox.FrontThickness() / 4;		// lift (external) door thickness
	AVFLOAT gap = 1.0f;											// gap between lift & floor

	if (LiftLine == 0)
	{
		m_box = BOX(lobbyBox.Left() + (ShaftPos + LeftBeam)*fScale, lobbyBox.FrontExt(), ShaftWidth*fScale, -ShaftDepth*fScale);
		m_box.SetThickness(LeftBeam*fScale, RightBeam*fScale, 0, -RearBeam*fScale);

		m_boxDoor = BOX(m_box.Left() + (m_box.Width() - LiftDoorWidth*fScale) / 2, m_box.Front(),               0, LiftDoorWidth*fScale, doorThickness,      LiftDoorHeight*fScale);

		m_boxLift = BOX(m_box.Left() + (m_box.Width() - CarWidth     *fScale) / 2, m_box.Front()-wallThickness-gap, 0, CarWidth     *fScale, -CarDepth * fScale, CarHeight     *fScale);
		m_boxLift.SetThickness(wallThickness, wallThickness, -wallThickness, -wallThickness, wallThickness, wallThickness);
	}
	else
	{
		m_box = BOX(lobbyBox.Left() + (ShaftPos + LeftBeam)*fScale, lobbyBox.RearExt(), ShaftWidth*fScale, ShaftDepth*fScale);
		m_box.SetThickness(LeftBeam*fScale, RightBeam*fScale, 0, RearBeam*fScale);

		m_boxDoor = BOX(m_box.Left() + (m_box.Width() - LiftDoorWidth*fScale) / 2, m_box.Front(),               0, LiftDoorWidth*fScale, doorThickness,     LiftDoorHeight*fScale);

		m_boxLift = BOX(m_box.Left() + (m_box.Width() - CarWidth     *fScale) / 2, m_box.Front()+wallThickness+gap, 0, CarWidth     *fScale, CarDepth * fScale, CarHeight     *fScale);
		m_boxLift.SetThickness(wallThickness, wallThickness, wallThickness, wallThickness, wallThickness, wallThickness);
	}
}

void CBuilding::STOREY::XmlParse(CComPtr<IXmlReader> pReader)
{
	HRESULT h;
	XmlNodeType nodeType;
	LPCWSTR pLocalName, pValue;
	while ((h = pReader->Read(&nodeType)) == S_OK)
	{
		switch (nodeType)
		{
		case XmlNodeType_Element:
			h = pReader->GetLocalName(&pLocalName, NULL);
			break;
		case XmlNodeType_Text:
			h = pReader->GetValue(&pValue, NULL);
			if (wcscmp(pLocalName, L"HeightValue") == 0) _helperParse(HeightValue, pValue);
			else if (wcscmp(pLocalName, L"ExpressZone") == 0) _helperParse(ExpressZone, pValue);
			else TRACE(L"Unrecognised XML tag: \"%ls\" with the value of: \"%ls\".\n", pLocalName, pValue);
			break;
		case XmlNodeType_EndElement:
			h = pReader->GetLocalName(&pLocalName, NULL);
			if (wcscmp(pLocalName, L"FloorData") == 0)
				return;
			break;
		}
	}
}

void CBuilding::STOREY::SetScaledDims(AVFLOAT fScale)
{ 
	SH = HeightValue*fScale; SL = StoreyLevel*fScale;
}

