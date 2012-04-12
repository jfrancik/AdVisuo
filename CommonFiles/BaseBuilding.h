// BaseBuilding.h - AdVisuo Common Source File

#pragma once

#include "Box.h"
#include "DBTools.h"
#include <string>

class CBuildingBase : public dbtools::CCollection
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
		AVULONG m_nId;							// Storey Id
		CBuildingBase *m_pBuilding;				// main building
		std::wstring m_strName;					// storey name

		AVFLOAT m_fLevel;						// lobby floor level
		BOX m_box;								// lobby floor plan (the same as building, but includes storey height - from floor to ceiling
		
	public:
		STOREY(CBuildingBase *pBuilding, AVULONG nId) : m_pBuilding(pBuilding), m_nId(nId), m_fLevel(0)	{ }
		virtual ~STOREY()						{ }

		// Attributes
		AVULONG GetId()							{ return m_nId; }
		CBuildingBase *GetBuilding()			{ return m_pBuilding; }
		std::wstring GetName()					{ return m_strName; }

		AVFLOAT GetLevel()						{ return m_fLevel; }
		AVFLOAT GetHeight()						{ return GetBox().HeightExt(); }
		AVFLOAT GetCeilingHeight()				{ return GetBox().Height(); }
		AVFLOAT GetCeilingLevel()				{ return m_fLevel + GetBox().Height(); }
		AVFLOAT GetRoofLevel()					{ return m_fLevel + GetBox().HeightExt(); }

		AVVECTOR GetLiftPos(AVULONG nShaft)		{ return GetBuilding()->GetShaft(nShaft)->GetBoxCar() + Vector(0, 0, GetLevel()); }

		BOX &GetBox()							{ return m_box; }
		bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }
		bool Within(AVVECTOR &pos)				{ return pos.z >= GetLevel() && pos.z < GetLevel() + GetHeight(); }

		// Operations:
		void ConsoleCreate(AVULONG nId, AVFLOAT fLevel);
		void Create();
		void Scale(AVFLOAT fScale)	{ Scale(fScale, fScale, fScale); }
		void Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z);
		void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);
	};

	// Shaft Layout data
	class SHAFT : public dbtools::CCollection
	{
		AVULONG m_nId;							// Shaft Id
		CBuildingBase *m_pBuilding;				// main building
		AVULONG m_nShaftLine;					// 0 for SHAFT_INLINE and 0 or 1 for SHAFT_OPPOSITE

		TYPE_OF_LIFT m_type;					// type of lift (conventional/MRL)
		TYPE_OF_DECK m_deck;					// type of deck (single/double/twin)

		AVULONG m_nLiftCount;					// usually 1, may be more for multiple lifts options
		AVULONG m_nLiftBegin;					// index of the first lift, usually m_nId but may be different for multiple lifts options

		// Dimensions
		BOX m_boxShaft;							// shaft box (including ext walls thickness)
		AVFLOAT m_fWallLtStart, m_fWallRtStart;	// offset at which left & right walls start, 0 if all the length
		BOX m_boxBeam;							// left-side int beam (thickness = 0)
		BOX m_boxDoor[2];						// shaft external door (thickness = 0)
		BOX m_boxCar;							// lift car (including car walls thickness)
		BOX m_boxCarDoor[2];					// car internal door (thickness = 0)

	public:

		SHAFT(CBuildingBase *pBuilding, AVULONG nId) : m_pBuilding(pBuilding), m_nId(nId), 
			m_nShaftLine(0), m_type(LIFT_CONVENTIONAL), m_deck(DECK_SINGLE), m_nLiftCount(1), m_fWallLtStart(0), m_fWallRtStart(0)	{ }
		virtual ~SHAFT()						{ }

		// Attributes:
		AVULONG GetId()							{ return m_nId; }
		CBuildingBase *GetBuilding()			{ return m_pBuilding; }
		AVULONG GetShaftLine()					{ return m_nShaftLine; }
		std::wstring GetName()					{ wchar_t buf[256]; _snwprintf_s(buf, 256, L"Lift %c", GetId() + 'A'); return buf; }

		TYPE_OF_LIFT GetType()					{ return m_type; }
		TYPE_OF_DECK GetDeck()					{ return m_deck; }
		AVULONG GetDeckCount()					{ return m_deck == DECK_DOUBLE ? 2 : 1; }

		AVULONG GetLiftBegin()					{ return m_nLiftBegin; }
		AVULONG GetLiftCount()					{ return m_nLiftCount; }
		AVULONG GetLiftEnd()					{ return m_nLiftBegin + m_nLiftCount; }
		void SetLiftRange(AVULONG i, AVULONG n)	{ m_nLiftBegin = i; m_nLiftCount = n; } 

		enum SHAFT_BOX { BOX_SHAFT, BOX_BEAM, BOX_DOOR, BOX_CAR, BOX_CARDOOR };
		BOX &GetBox(enum SHAFT_BOX n = BOX_SHAFT, AVULONG i = 0);
		BOX &GetBoxBeam()						{ return m_boxBeam; }
		BOX &GetBoxDoor(AVULONG i = 0)			{ return m_boxDoor[i]; }
		BOX &GetBoxCar()						{ return m_boxCar; }
		BOX &GetBoxCarDoor(AVULONG i = 0)		{ return m_boxCarDoor[i]; }
		BOX GetLeftWallBox();
		BOX GetRightWallBox();

		AVVECTOR GetLiftPos(AVULONG nStorey)	{ return GetBoxCar() + Vector(0, 0, GetBuilding()->GetStorey(nStorey)->GetLevel()); }

		// raw data functions
		AVFLOAT GetRawWidth()					{ return (*this)[L"ShaftWidth"]; }
		AVFLOAT GetRawBeamWidth()				{ return (*this)[L"DividingBeamWidth"]; }

		// checks if point within the shaft, beam or door box
		bool InBox(AVVECTOR &pt)							{ return GetBox().InBoxExt(pt) || GetBox(BOX_DOOR).InBoxExt(pt) || GetBox(BOX_BEAM).InBoxExt(pt); }
		// checks if x coordinate within the shaft or beam width
		bool InWidth(AVFLOAT x)								{ return GetBox().InWidthExt(x) || GetBox(BOX_DOOR).InWidthExt(x) || GetBox(BOX_BEAM).InWidthExt(x); }
		// checks if z coordiante within the height of the lift car, given the car position above 0
		bool Within(AVFLOAT z, AVFLOAT nLiftZPos = 0)		{ return z >= nLiftZPos && z < nLiftZPos + m_boxCar.Height(); }


		// Operations:
		void ConsoleCreate(AVULONG nId, AVULONG nLine, AVFLOAT fShaftPosX, AVFLOAT fShaftPosY, AVFLOAT fFrontWall, AVFLOAT fRearWall);
		void ConsoleCreateLeftBeam(AVFLOAT fDepth, SHAFT *pPrev);
		void ConsoleCreateRightBeam(AVFLOAT fDepth, SHAFT *pPrev);
		void ConsoleCreateLeftWall(AVFLOAT fThickness, AVFLOAT fStart = 0);
		void ConsoleCreateRightWall(AVFLOAT fThickness, AVFLOAT fStart = 0);
		void ConsoleCreateAmend();
		void Create();
		void Scale(AVFLOAT fScale)	{ Scale(fScale, fScale, fScale); }
		void Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z);
		void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);
	};

	// Lift Car data - added for multi-car lifts, of little use otherwise.
	// By default, all parameters of a lift are accessible through its SHAFT (use GetShaft())
	class LIFT : public dbtools::CCollection
	{
		AVULONG m_nId;							// Lift Id
		AVULONG m_nShaftId;						// Shaft Id - will be different if many lifts per shaft
		CBuildingBase *m_pBuilding;				// main building

	public:
		LIFT(CBuildingBase *pBuilding, AVULONG nId) : m_pBuilding(pBuilding), m_nId(nId), m_nShaftId(0)	{ }
		virtual ~LIFT()							{ }

		AVULONG GetId()							{ return m_nId; }
		AVULONG GetShaftId()					{ return m_nShaftId; }
		void SetShaftId(AVULONG nShaftId)		{ m_nShaftId = nShaftId; }
		SHAFT *GetShaft()						{ return m_pBuilding->GetShaft(GetShaftId() >= 0 ? GetShaftId() : 0); }
		CBuildingBase *GetBuilding()			{ return m_pBuilding; }
	};

private:

	AVULONG m_nId;						// Building ID

	AVULONG m_nStoreyCount;				// Counters (floors/shafts/lifts)
	AVULONG m_nShaftCount;
	AVULONG m_nLiftCount;

	AVULONG m_nBasementStoreyCount;
	AVULONG m_pnShaftCount[2];			// counter of lifts per line

	SHAFT_ARRANGEMENT m_LiftShaftArrang;// Lift shaft arrangements
	LOBBY_ARRANGEMENT m_LobbyArrangement;// Lobby arrangement

	BOX m_box;							// scaled lobby floor plan (size of the lobby & its walls, zero height)

	STOREY **m_ppStoreys;
	SHAFT **m_ppShafts;
	LIFT **m_ppLifts;

public:
	CBuildingBase(void);
	~CBuildingBase(void);

	AVULONG GetId()							{ return m_nId; }
	void SetId(AVULONG nId)					{ m_nId = nId; }

	BOX &GetBox()							{ return m_box; }
	bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }

	// Storeys
	void CreateStoreys(AVULONG nStoreyCount, AVULONG nBasementStoreyCount = 0);
	void DeleteStoreys();
	AVULONG GetStoreyCount()				{ return m_nStoreyCount; }
	AVULONG GetBasementStoreyCount()		{ return m_nBasementStoreyCount; }
	STOREY *GetStorey(AVULONG i)			{ return i < GetStoreyCount() ? m_ppStoreys[i] : NULL; }
	STOREY *GetGroundStorey(AVULONG i = 0)	{ return GetStorey(i + GetBasementStoreyCount()); }

	// Shafts
	void CreateShafts(AVULONG nShaftCount);
	void DeleteShafts();
	SHAFT *GetShaft(AVULONG i)				{ return i < GetShaftCount() ? m_ppShafts[i] : NULL; }

	AVULONG GetShaftCount()					{ return m_nShaftCount; }
	AVULONG GetShaftBegin(AVULONG nLine)	{ return nLine == 0 ? 0 : m_pnShaftCount[0]; }
	AVULONG GetShaftCount(AVULONG nLine)	{ return m_pnShaftCount[nLine]; }
	AVULONG GetShaftEnd(AVULONG nLine)		{ return GetShaftBegin(nLine) + GetShaftCount(nLine); }
	AVULONG GetShaftLinesCount()			{ return m_pnShaftCount[1] == 0 ? 1 : 2; }

	AVULONG GetLiftCount(AVULONG nShaft)	{ return GetShaft(nShaft)->GetLiftCount(); }

	AVVECTOR GetCarPos(AVULONG nShaft, AVULONG nStorey)	{ return GetShaft(nShaft)->GetLiftPos(nStorey); }

	// Lifts
	void CreateLifts(AVULONG nLiftCount);
	void DeleteLifts();
	LIFT *GetLift(AVULONG i)				{ return i < GetLiftCount() ? m_ppLifts[i] : NULL; }

	AVULONG GetLiftCount()					{ return m_nLiftCount; }

	// Various
	SHAFT_ARRANGEMENT GetLiftShaftArrang()	{ return m_LiftShaftArrang; }
	LOBBY_ARRANGEMENT GetLobbyArrangement()	{ return m_LobbyArrangement; }
	


	// Status
	bool IsValid()							{ return m_nShaftCount && GetStoreyCount() && m_ppShafts && m_ppStoreys && GetShaftCount(0); }

	// Calculations!
	void Init(AVLONG nId = -1);				// initialises is and basic structure of storeys & shafts - uses DB info only, may be called before actual creation
											// ATTENTION: Lifts structure should be created separately, after all lifts are loaded from the DB - see InitLifts below
	
	void ConsoleCreate();
	void Create();

	void InitLifts();						// initialises basic structure of lifts - uses SHAFT DB info, call after Shafts are loaded but may be called before actual creation

	void Scale(AVFLOAT fScale)	{ Scale(fScale, fScale, fScale); }
	void Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z);
	void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);

protected:
	virtual STOREY *CreateStorey(AVULONG nId) = 0;
	virtual SHAFT *CreateShaft(AVULONG nId) = 0;
	virtual LIFT *CreateLift(AVULONG nId) = 0;
};

class CBuildingBaseEx : public CBuildingBase
{
protected:
	virtual STOREY *CreateStorey(AVULONG nId)	{ return new STOREY(this, nId); }
	virtual SHAFT *CreateShaft(AVULONG nId)		{ return new SHAFT(this, nId); }
	virtual LIFT *CreateLift(AVULONG nId)		{ return new LIFT(this, nId); }
};

