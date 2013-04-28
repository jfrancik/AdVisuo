// BaseGroup.h - AdVisuo Common Source File

#pragma once

#include "Box.h"
#include "DBTools.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLiftGroup

class CProject;
class CSim;

#define SIDE_LEFT	0
#define SIDE_RIGHT	1
#define SIDE_REAR	2

class CLiftGroup : public dbtools::CCollection
{
// enum & struct definitions
public:
	enum SHAFT_ARRANGEMENT	{ SHAFT_INLINE = 1, SHAFT_OPPOSITE, SHAFT_UNKNOWN = -1 };
	enum LOBBY_ARRANGEMENT	{ LOBBY_THROUGH = 1, LOBBY_OPENPLAN, LOBBY_DEADEND_LEFT, LOBBY_DEADEND_RIGHT, LOBBY_UNKNOWN = -1 };
	enum DOOR_TYPE			{ DOOR_CENTRE = 1, DOOR_LSIDE, DOOR_RSIDE, DOOR_CENTRE_2, DOOR_LSIDE_2, DOOR_RSIDE_2, DOOR_CENTRE_3, DOOR_LSIDE_3, DOOR_RSIDE_3, DOOR_UNKNOWN = -1 };
	enum TYPE_OF_LIFT		{ LIFT_CONVENTIONAL = 1, LIFT_MRL, LIFT_UNKNOWN };
	enum TYPE_OF_DECK		{ DECK_SINGLE = 1, DECK_DOUBLE, DECK_TWIN, DECK_UNKNOWN = -1 };
	enum CAR_ENTRANCES		{ CAR_FRONT = 1, CAR_REAR = 999, CAR_BOTH = 2, CAR_UNKNOWN = -1 };
	enum CNTRWEIGHT_POS		{ CNTRWEIGHT_REAR = 1, CNTRWEIGHT_LSIDE, CNTRWEIGHT_RSIDE, CNTRWEIGHT_UNKNOWN = -1 };
	enum LIFT_STRUCTURE		{ STRUCT_STEEL = 1, STRUCT_CONCRETE = 2, STRUCT_UNKNOWN = -1 };

	// Storey Data
	class STOREY : public dbtools::CCollection
	{
	protected:
		AVULONG m_nId;							// Storey Id
		CLiftGroup *m_pLiftGroup;				// lift group
		std::wstring m_strName;					// storey name

		AVFLOAT m_fLevel;						// lobby floor level
		BOX m_box;								// lobby floor plan (the same as lift group's, but includes storey height - from floor to ceiling
		
	public:
		STOREY(CLiftGroup *pLiftGroup, AVULONG nId) : m_pLiftGroup(pLiftGroup), m_nId(nId), m_fLevel(0)	{ }
		virtual ~STOREY()						{ }

		// Attributes
		AVULONG GetId()							{ return m_nId; }
		CLiftGroup *GetLiftGroup()				{ return m_pLiftGroup; }
		CProject *GetProject()					{ return GetLiftGroup()->GetProject(); }
		std::wstring GetName()					{ return m_strName; }

		AVFLOAT GetLevel()						{ return m_fLevel; }
		AVFLOAT GetHeight()						{ return GetBox().HeightExt(); }
		AVFLOAT GetCeilingHeight()				{ return GetBox().Height(); }
		AVFLOAT GetCeilingLevel()				{ return m_fLevel + GetBox().Height(); }
		AVFLOAT GetRoofLevel()					{ return m_fLevel + GetBox().HeightExt(); }

		void SetName(std::wstring strName)		{ m_strName = strName; }
		void SetLevel(AVFLOAT fLevel)			{ m_fLevel = fLevel; }

		AVVECTOR GetLiftPos(AVULONG nShaft)		{ return GetLiftGroup()->GetShaft(nShaft)->GetBoxCar() + Vector(0, 0, GetLevel()); }

		BOX &GetBox()							{ return m_box; }
		bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }
		bool Within(AVVECTOR &pos)				{ return pos.z >= GetLevel() && pos.z < GetLevel() + GetHeight(); }

		bool IsStoreyServed()					{ for (AVULONG i = 0; i < GetLiftGroup()->GetShaftCount(); i++) if (GetLiftGroup()->GetShaft(i)->IsStoreyServed(GetId())) return true; return false; }
		bool IsStoreyServed(AVULONG nShaft)		{ return GetLiftGroup()->GetShaft(nShaft)->IsStoreyServed(GetId()); }

		// Operations:
		void ConsoleCreate(AVULONG nId, AVFLOAT fLevel);
		void Create();
		void Scale(AVFLOAT fScale);
		void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);
	};

	class MACHINEROOM : public STOREY
	{
	public:
		MACHINEROOM(CLiftGroup *pLiftGroup) : STOREY(pLiftGroup, 9999)	{ }
		virtual ~MACHINEROOM()										{ }
		void ConsoleCreate();
		void Create();
	};

	class PIT : public STOREY
	{
	public:
		PIT(CLiftGroup *pLiftGroup) : STOREY(pLiftGroup, 9998)		{ }
		virtual ~PIT()											{ }
		void ConsoleCreate();
		void Create();
	};

	// Shaft Layout data
	class SHAFT : public dbtools::CCollection
	{
		AVULONG m_nId;							// Shaft Id
		CLiftGroup *m_pLiftGroup;				// lift group
		AVULONG m_nShaftLine;					// 0 for SHAFT_INLINE and 0 or 1 for SHAFT_OPPOSITE

		TYPE_OF_LIFT m_type;					// type of lift (conventional/MRL)
		TYPE_OF_DECK m_deck;					// type of deck (single/double/twin)

		AVULONG m_nOpeningTime;					// door opening time
		AVULONG m_nClosingTime;					// door closing time

		std::wstring m_strStoreysServed;		// storeys served; "0" for not served, "1" for served

		AVULONG m_nLiftCount;					// usually 1, may be more for multiple lifts options
		AVULONG m_nLiftBegin;					// index of the first lift, usually m_nId but may be different for multiple lifts options

		// Dimensions and similar
		BOX m_boxShaft;							// shaft box (including ext walls thickness)
		AVFLOAT m_fWallLtStart, m_fWallRtStart;	// Y-coordinate where left & right walls start, 0 if no wall
		AVFLOAT m_fBeamLtHeight, m_fBeamRtHeight;// Height of the Left/Right Int Beam
		BOX m_boxDoor[2];						// shaft external door (thickness = 0)
		BOX m_boxCar;							// lift car (including car walls thickness)
		BOX m_boxCarDoor[2];					// car internal door (thickness = 0)
		BOX m_boxCwt;							// counterweight
		BOX m_boxGovernor;						// governors
		BOX m_boxLadder;						// pit ladder

		AVULONG m_nLiftType;					// 1 = conventional, 2 = MRL
		AVFLOAT m_fShaftOrientation;			// machine orientation (0 for line 0, M_PI for line 1)
		AVULONG m_nMachineType;					// machine type (1 - 4)
		AVULONG m_nPanelCtrlType, m_nPanelDrvType, m_nPanelIsoType;		// control/drive/isolator panel type
		AVFLOAT m_fPanelCtrlWidth, m_fPanelDrvWidth, m_fPanelIsoWidth;	// control/drive/isolator panel width
		AVFLOAT m_fMachineOrientation;			// machine orientation
		AVFLOAT m_fRailWidth;					// guiding rail width
		AVFLOAT m_fRailLength;					// guiding rail length
		AVULONG m_nBufferNum;					// number of car/cwt buffers
		AVULONG m_nBufferDiameter;				// diameter of the buffers
		AVULONG m_nBufferHeight;				// height of the buffers
		AVFLOAT m_fLightingXPos;				// position of the lighting
		AVULONG m_nDoorType;					// door type - see GetDoorType() and GetDoorPanelsCount()

	public:

		SHAFT(CLiftGroup *pLiftGroup, AVULONG nId) : m_pLiftGroup(pLiftGroup), m_nId(nId), 
			m_nShaftLine(0), m_type(LIFT_CONVENTIONAL), m_deck(DECK_SINGLE), m_nLiftCount(1), m_fWallLtStart(0), m_fWallRtStart(0), m_fBeamLtHeight(0), m_fBeamRtHeight(0)	{ }
		virtual ~SHAFT()						{ }

		// Attributes:
		AVULONG GetId()							{ return m_nId; }
		CLiftGroup *GetLiftGroup()				{ return m_pLiftGroup; }
		CProject *GetProject()					{ return GetLiftGroup()->GetProject(); }
		AVULONG GetShaftLine()					{ return m_nShaftLine; }
		AVULONG GetIndexInLine()				{ return GetId() - GetLiftGroup()->GetShaftBegin(GetShaftLine()); }
		std::wstring GetName()					{ wchar_t buf[256]; _snwprintf_s(buf, 256, L"Lift %c", GetId() + 'A'); return buf; }

		AVLONG GetNativeId()					{ return ME[L"LiftId"]; }

		TYPE_OF_LIFT GetType()					{ return m_type; }
		TYPE_OF_DECK GetDeck()					{ return m_deck; }
		AVULONG GetDeckCount()					{ return m_deck == DECK_DOUBLE ? 2 : 1; }

		AVULONG GetOpeningTime()				{ return m_nOpeningTime; }
		AVULONG GetClosingTime()				{ return m_nClosingTime; }

		bool IsStoreyServed(AVULONG nStorey)	{ return m_strStoreysServed[nStorey] == '1' ? true : false; }
		AVULONG GetHighestStoreyServed()		{ return m_strStoreysServed.find_last_of('1'); }
		AVULONG GetLowestStoreyServed()			{ return m_strStoreysServed.find('1'); }

		AVULONG GetLiftBegin()					{ return m_nLiftBegin; }
		AVULONG GetLiftCount()					{ return m_nLiftCount; }
		AVULONG GetLiftEnd()					{ return m_nLiftBegin + m_nLiftCount; }
		void SetLiftRange(AVULONG i, AVULONG n)	{ m_nLiftBegin = i; m_nLiftCount = n; } 

		enum SHAFT_BOX { BOX_SHAFT, BOX_DOOR, BOX_CAR, BOX_CARDOOR, BOX_MOUNTING, BOX_CW, BOX_GOVERNOR, BOX_LADDER };
		BOX &GetBox(enum SHAFT_BOX n = BOX_SHAFT, AVULONG i = 0);
		BOX &GetBoxDoor(AVULONG i = 0)			{ return m_boxDoor[i]; }
		BOX &GetBoxCar()						{ return m_boxCar; }
		BOX &GetBoxCarDoor(AVULONG i = 0)		{ return m_boxCarDoor[i]; }
		BOX &GetBoxCwt()						{ return m_boxCwt; }
		BOX &GetBoxGovernor()					{ return m_boxGovernor; }
		BOX &GetBoxLadder()						{ return m_boxLadder; }
		AVFLOAT GetWallLtStart()				{ return m_fWallLtStart; }
		AVFLOAT GetWallRtStart()				{ return m_fWallRtStart; }
		AVFLOAT GetBeamLtHeight()				{ return m_fBeamLtHeight; }
		AVFLOAT GetBeamRtHeight()				{ return m_fBeamRtHeight; }

		bool IsMRL()							{ return (m_nLiftType == 2); }
		AVFLOAT GetShaftOrientation()			{ return m_fShaftOrientation; }
		AVULONG GetMachineType()				{ return m_nMachineType; }
		AVFLOAT GetMachineOrientation()			{ return m_fMachineOrientation; }
		AVULONG GetPanelCtrlType()				{ return m_nPanelCtrlType; }
		AVULONG GetPanelDrvType()				{ return m_nPanelDrvType; }
		AVULONG GetPanelIsoType()				{ return m_nPanelIsoType; }
		AVFLOAT GetPanelCtrlWidth()				{ return m_fPanelCtrlWidth; }
		AVFLOAT GetPanelDrvWidth()				{ return m_fPanelDrvWidth; }
		AVFLOAT GetPanelTwoWidth()				{ return m_fPanelCtrlWidth + m_fPanelDrvWidth; }
		AVFLOAT GetPanelIsoWidth()				{ return m_fPanelIsoWidth; }
		AVFLOAT GetRailWidth()					{ return m_fRailWidth; }
		AVFLOAT GetRailLength()					{ return m_fRailLength; }
		AVULONG GetBufferNum()					{ return m_nBufferNum; }
		AVULONG GetBufferDiameter()				{ return m_nBufferDiameter; }
		AVULONG GetBufferHeight()				{ return m_nBufferHeight; }
		AVFLOAT GetLightingXPos()				{ return m_fLightingXPos; }
		AVULONG GetDoorType()					{ return (m_nDoorType - 1) % 3; }		// 0 = center; 1 = left; 2 = right
		AVULONG GetDoorPanelsCount()			{ return (m_nDoorType - 1) / 3 + 1; }	// 1, 2, or 3 (no zero!)

		AVVECTOR GetLiftPos(AVULONG nStorey)	{ return GetBoxCar() + Vector(0, 0, GetLiftGroup()->GetStorey(nStorey)->GetLevel()); }

		// raw data functions
		AVFLOAT GetRawWidth()					{ return (*this)[L"ShaftWidth"]; }
		AVFLOAT GetRawBeamWidth()				{ return (*this)[L"DividingBeamWidth"]; }

		// checks if point within the shaft, beam or door box
		bool InBox(AVVECTOR &pt)							{ return GetBox().InBoxExt(pt) || GetBox(BOX_DOOR).InBoxExt(pt); }
		// checks if x coordinate within the shaft or beam width
		bool InWidth(AVFLOAT x)								{ return GetBox().InWidthExt(x); }

		// Operations:
		void ConsoleCreate(AVULONG nId, AVULONG nLine, AVFLOAT fShaftPosX, AVFLOAT fShaftPosY, AVFLOAT fFrontWall, AVFLOAT fRearWall);
		void ConsoleCreateBeam(AVULONG side, SHAFT *pNeighbour);
		void ConsoleCreateSideWall(AVULONG side, AVFLOAT fThickness, AVFLOAT fOffset = 0);
		void ConsoleCreateAmend();
		void Create();
		void Scale(AVFLOAT fScale);
		void Reflect();
		void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);
	};

	// Lift Car data - added for multi-car lifts, of little use otherwise.
	// By default, all parameters of a lift are accessible through its SHAFT (use GetShaft())
	class LIFT : public dbtools::CCollection
	{
		AVULONG m_nId;							// Lift Id
		AVULONG m_nShaftId;						// Shaft Id - will be different if many lifts per shaft
		CLiftGroup *m_pLiftGroup;				// lift group

	public:
		LIFT(CLiftGroup *pLiftGroup, AVULONG nId) : m_pLiftGroup(pLiftGroup), m_nId(nId), m_nShaftId(0)	{ }
		virtual ~LIFT()							{ }

		AVULONG GetId()							{ return m_nId; }
		AVULONG GetShaftId()					{ return m_nShaftId; }
		void SetShaftId(AVULONG nShaftId)		{ m_nShaftId = nShaftId; }
		SHAFT *GetShaft()						{ return m_pLiftGroup->GetShaft(GetShaftId() >= 0 ? GetShaftId() : 0); }
		CLiftGroup *GetLiftGroup()				{ return m_pLiftGroup; }
		CProject *GetProject()					{ return GetLiftGroup()->GetProject(); }

		std::wstring GetName()					{ wchar_t buf[256]; _snwprintf_s(buf, 256, L"Lift %c", GetId() + 'A'); return buf; }

		// checks if pos is within the lift car bounds, given the lift position posLift
		bool Within(AVVECTOR pos, AVVECTOR posLift)			
		{
			BOX box(posLift.x, posLift.y, posLift.z, GetShaft()->GetBoxCar().Width(), GetShaft()->GetBoxCar().Depth(), GetShaft()->GetBoxCar().Height());
			return box.InWidth(pos.x) && box.InDepth(pos.y) && box.InHeight(pos.z);
		}

	};

private:

	AVULONG m_nId;						// Lift Group ID
	AVULONG m_nProjectId;				// Project ID
	AVULONG m_nIndex;					// index in multi-group structures

	CProject *m_pProject;				// The Project Object
	std::wstring m_strName;				// Lift Group name
	CSim *m_pSim;						// Sim object

	AVFLOAT m_fScale;					// scale factor (applied to all dimensions)

	AVULONG m_nBasementStoreyCount;		// basement storey count
	AVULONG m_pnShaftCount[2];			// counter of lifts per line

	SHAFT_ARRANGEMENT m_LiftShaftArrang;// Lift shaft arrangements
	LOBBY_ARRANGEMENT m_LobbyArrangement;// Lobby arrangement

	BOX m_box;							// scaled lobby floor plan (size of the lobby & its walls, zero height)
	
	BOX m_boxMachineRoom;				// scaled floor plan and level for the machine room
	AVFLOAT m_fMachineRoomLevel;
	AVFLOAT m_fLiftingBeamHeight;		// lifting beam height
	AVFLOAT m_fLiftingBeamWidth;		// lifting beam height

	BOX m_boxPit;						// scaled floor plan and level for the pit level
	AVFLOAT m_fPitLevel;

protected:
	std::vector<STOREY*> m_storeys;
	std::vector<SHAFT*>  m_shafts;
	std::vector<LIFT*>   m_lifts;
	
	MACHINEROOM *m_pMachineRoom;
	PIT *m_pPit;

public:
	CLiftGroup(CProject *pProject, AVULONG nIndex);
	virtual ~CLiftGroup();

	CProject *GetProject()					{ return m_pProject; }
	void SetProject(CProject *pProject)		{ m_pProject = pProject; }

	std::wstring GetName()					{ return m_strName; }

	AVULONG GetId()							{ return m_nId; }
	void SetId(AVULONG nId)					{ m_nId = nId; }

	AVULONG GetProjectId()					{ return m_nProjectId; }
	void SetProjectId(AVULONG nId)			{ m_nProjectId = nId; }

	AVULONG GetIndex()						{ return m_nIndex; }
	void SetIndex(AVULONG n)				{ m_nIndex = n; }

	BOX &GetBox()							{ return m_box; }
	bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }

	bool IsMRL()							{ for (AVULONG i = 0; i < GetShaftCount(); i++) if (!GetShaft(i)->IsMRL()) return false; return true; }

	BOX &GetBoxMachineRoom()				{ return m_boxMachineRoom; }
	bool InBoxMachineRoom(AVVECTOR &pt)		{ return m_boxMachineRoom.InBoxExt(pt); }
	BOX &GetBoxPit()						{ return m_boxPit; }
	bool InBoxPit(AVVECTOR &pt)				{ return m_boxPit.InBoxExt(pt); }

	BOX GetTotalAreaBox();

	AVFLOAT GetScale()						{ return m_fScale; }


	// Storeys
	void CreateStoreys(AVULONG nStoreyCount, AVULONG nBasementStoreyCount = 0);
	STOREY *AddStorey();
	void DeleteStoreys();
	AVULONG GetStoreyCount()				{ return m_storeys.size(); }
	AVULONG GetBasementStoreyCount()		{ return m_nBasementStoreyCount; }
	STOREY *GetStorey(AVULONG i)			{ return i < GetStoreyCount() ? m_storeys[i] : NULL; }
	STOREY *GetGroundStorey(AVULONG i = 0)	{ return GetStorey(i + GetBasementStoreyCount()); }

	// Extras: Machine Room & Pit
	void AddExtras();
	void DeleteExtras();

	MACHINEROOM *GetMachineRoom()			{ return m_pMachineRoom; }
	AVFLOAT GetMachineRoomLevel()			{ return m_fMachineRoomLevel; }
	AVFLOAT GetMachineRoomSlabThickness()	{ return GetBoxMachineRoom().LowerThickness(); }
	AVFLOAT GetLiftingBeamHeight()			{ return m_fLiftingBeamHeight; }
	AVFLOAT GetLiftingBeamWidth()			{ return m_fLiftingBeamWidth; }
	PIT *GetPit()							{ return m_pPit; }
	AVFLOAT GetPitLevel()					{ return m_fPitLevel; }
	AVFLOAT GetPitHeight()					{ return -m_fPitLevel; }
	bool IsPitLadder()						{ return GetPitHeight() > 600 * GetScale() && GetPitHeight() < 2500 * GetScale(); }
	bool IsPitDoor()						{ return GetPitHeight() >= 2500 * GetScale(); }
	AVULONG GetPitLadderRungs()				{ if (IsPitLadder()) return (GetPitHeight() / GetScale() + 1000) / 300; else return 0; }
	AVFLOAT GetPitLadderHeight()			{ if (IsPitLadder()) return (GetPitLadderRungs() + 1) * 300 * GetScale(); else return 0; }
	AVFLOAT GetPitLadderLowerBracket()		{ if (IsPitLadder()) return 500 * GetScale(); else return 0; }
	AVFLOAT GetPitLadderUpperBracket()		{ if (IsPitLadder()) return (GetPitLadderHeight() - 600) * GetScale(); else return 0; }
	AVFLOAT GetPanelGrpWidth()				{ return 700.0f; }

	// Shafts
	SHAFT *AddShaft();
	void DeleteShafts();
	SHAFT *GetShaft(AVULONG i)				{ return i < GetShaftCount() ? m_shafts[i] : NULL; }
	SHAFT *GetShaft(AVULONG nLine, AVULONG i)	{ return GetShaft(GetShaftBegin(nLine) + i); }

	AVULONG GetShaftCount()					{ return m_shafts.size(); }
	AVULONG GetShaftBegin(AVULONG nLine)	{ return nLine == 0 ? 0 : m_pnShaftCount[0]; }
	AVULONG GetShaftCount(AVULONG nLine)	{ return m_pnShaftCount[nLine]; }
	AVULONG GetShaftEnd(AVULONG nLine)		{ return GetShaftBegin(nLine) + GetShaftCount(nLine); }
	AVULONG GetShaftLinesCount()			{ return m_pnShaftCount[1] == 0 ? 1 : 2; }
	AVULONG GetShaftLineFor(AVULONG iShaft)		{ return iShaft < GetShaftCount(0) ? 0 : 1; }
	AVULONG GetShaftIndexInLine(AVULONG iShaft)	{ return iShaft - GetShaftBegin(GetShaftLineFor(iShaft)); }

	AVULONG GetLiftCount(AVULONG nShaft)	{ return GetShaft(nShaft)->GetLiftCount(); }

	AVVECTOR GetCarPos(AVULONG nShaft, AVULONG nStorey)	{ return GetShaft(nShaft)->GetLiftPos(nStorey); }

	// Lifts
	LIFT *AddLift();
	void DeleteLifts();
	LIFT *GetLift(AVULONG i)				{ return i < GetLiftCount() ? m_lifts[i] : NULL; }

	AVULONG GetLiftCount()					{ return m_lifts.size(); }

	// Storeys Served
	bool IsStoreyServed(AVULONG nStorey)	{ return GetStorey(nStorey)->IsStoreyServed(); }
	bool IsStoreyServed(AVULONG nStorey, AVULONG nShaft)	{ return GetShaft(nShaft)->IsStoreyServed(nStorey); }
	AVULONG GetHighestStoreyServed()		{ AVLONG N = 0; for (AVULONG i = 0; i < GetShaftCount(); i++) { AVLONG n = GetShaft(i)->GetHighestStoreyServed(); if (n > N) N = n; } return N; }
	AVULONG GetLowestStoreyServed()			{ AVLONG N = 32767; for (AVULONG i = 0; i < GetShaftCount(); i++) { AVLONG n = GetShaft(i)->GetLowestStoreyServed(); if (n < N) N = n; } return N; }

	// The Sim
	CSim *AddSim();
	void DeleteSim();
	CSim *GetSim()							{ return m_pSim; }



	// Various
	SHAFT_ARRANGEMENT GetLiftShaftArrang()	{ return m_LiftShaftArrang; }
	LOBBY_ARRANGEMENT GetLobbyArrangement()	{ return m_LobbyArrangement; }
	


	// Status
	bool IsValid()							{ return GetStoreyCount() && GetShaftCount() && GetShaftCount(0); }

	// Calculations!
	virtual void ResolveMe();				// initialises id and basic variables
	
	virtual void ConsoleCreate();
	virtual void Create();

	void Scale(AVFLOAT fScale);
	virtual void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);

protected:
	virtual STOREY *CreateStorey(AVULONG nId) = 0;
	virtual SHAFT *CreateShaft(AVULONG nId) = 0;
	virtual LIFT *CreateLift(AVULONG nId) = 0;
	virtual MACHINEROOM *CreateMachineRoom() = 0;
	virtual PIT *CreatePit() = 0;

	virtual CSim *CreateSim() = 0;
};
