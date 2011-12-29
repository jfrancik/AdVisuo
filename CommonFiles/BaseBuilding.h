// BaseBuilding.h - AdVisuo Common Source File

#pragma once

#include "Box.h"
#include "DBTools.h"
#include <string>

class CBuildingBase : public dbtools::CCollection
{
// enum & struct definitions
public:
	
	enum SHAFT_ARRANGEMENT	{ SHAFT_INLINE, SHAFT_OPPOSITE, SHAFT_UNKNOWN };
	enum LOBBY_ARRANGEMENT	{ LOBBY_THROUGH, LOBBY_OPENPLAN, LOBBY_DEADEND_LEFT, LOBBY_DEADEND_RIGHT, LOBBY_UNKNOWN };
	enum DOOR_TYPE			{ DOOR_CENTRE, DOOR_SIDE, DOOR_UNKNOWN };
	enum TYPE_OF_LIFT		{ LIFT_SINGLE_DECK, LIFT_DOUBLE_DECK, LIFT_MULTI_CAR, LIFT_UNKNOWN };
	enum CAR_ENTRANCES		{ CAR_FRONT, CAR_REAR, CAR_BOTH, CAR_UNKNOWN };
	enum CNTRWEIGHT_POS		{ CNTRWEIGHT_SIDE, CNTRWEIGHT_REAR, CNTRWEIGHT_UNKNOWN };
	enum LIFT_STRUCTURE		{ STRUCT_CONCRETE, STRUCT_STEEL, STRUCT_UNKNOWN = -1 };

	// Shaft Layout data
	struct SHAFT
	{
		CBuildingBase *m_pBuilding;				// main building

		// CONSOLE based data
		AVULONG ShaftID;						// Shaft Id
		AVULONG Capacity;						// Lift capacity (kg)
		AVFLOAT Speed;							// Lift speed. (m/s)
		AVFLOAT Acceleration;					// accelaration (m/s/s)
		AVFLOAT Jerk;							// jerk (m/s/s/s)
		DOOR_TYPE DoorType;						// Lift door type
		AVFLOAT LiftDoorHeight;					// Lift door height. (mm)
		AVFLOAT LiftDoorWidth;					// Lift door width. (mm)
		AVFLOAT CarHeight;						// Car height mm
		TYPE_OF_LIFT TypeOfLift;				// Type of lift
		AVULONG NumberOfLifts;					// Number of Lifts per Shaft (new to ver. 1.06)
		CAR_ENTRANCES NoOfCarEntrances;			// Number (Type) of car entrances
		CNTRWEIGHT_POS CounterWeightPosition;	// Counter weight position
		AVFLOAT CarWidth;						// Car width. (mm)	
		AVFLOAT CarDepth;						// Car depth. (mm)	
		AVFLOAT ShaftWidth;						// Shaft width. (mm)	
		AVFLOAT ShaftDepth;						// Shaft depth. (mm)	
		AVFLOAT PitDepth;						// Lift pit depth . (mm)	
		AVFLOAT OverallHeight;					// OverallHeight. (mm)
		AVFLOAT HeadRoom;						// Lift headroom. (mm)	
		AVFLOAT MachRoomHeight;					// Lift machine room height . (mm)
		AVFLOAT MachRoomExt;					// new proposal: extension of the machine room outside the outline of the shafts

		// New Console Based Data (plus: Acceleration and Jerk)
		AVULONG PreOperTime;					// pre-opening time (s)
		AVULONG OpeningTime;					// door opening time (s)
		AVULONG ClosingTime;					// door closing time (s)
		AVULONG MotorStartDelay;				// motor start delay (s)
		AVULONG LoadingTime;					// loading time (s)
		AVULONG UnloadingTime;					// unloading time (s)
		std::wstring FloorsServed;				// list of served floors

		// Derived (resolved) data - set by Resolve
		AVULONG ShaftIndex;
		AVULONG ShaftLine;						// 0 for SHAFT_INLINE and 0 or 1 for SHAFT_OPPOSITE
		AVFLOAT ShaftPos;						// left side of the Lift shaft, without the LeftBeam
		AVFLOAT ShaftRPos;						// right side of the Lift shaft, without the RightBeam

		// Scaled Dimensions - set by Scale
		BOX m_box;								// scaled shaft plan
		BOX m_boxDoor;							// scaled shaft door plan
		BOX m_boxCar;							// scaled lift car plan

		SHAFT();
		virtual ~SHAFT();

		CBuildingBase *GetBuilding()			{ return m_pBuilding; }
		AVFLOAT GetShaftWidth()					{ return ShaftWidth; }

		// Operations:
		void Resolve(CBuildingBase *pBuilding, AVULONG nIndex);
		virtual void Scale(AVFLOAT scale);
		bool InBox(AVVECTOR &pt)								{ return m_box.InBoxExt(pt) || m_boxDoor.InBoxExt(pt); }
		bool Within(AVVECTOR &pos, AVFLOAT nLiftZPos = 0)		{ return pos.z >= nLiftZPos && pos.z < nLiftZPos + m_boxCar.Height(); }
		std::wstring GetName()									{ wchar_t buf[256]; _snwprintf_s(buf, 256, L"Lift %d", ShaftID); return buf; }
	};

	// Storey Data
	struct STOREY
	{
		CBuildingBase *m_pBuilding;				// main building

		// CONSOLE based data
		AVULONG StoreyID;						// Storey Id
		AVFLOAT HeightValue;					// Height

		// New Console Based Data
		std::wstring Name;						// storey name
		AVFLOAT Area;							// area (m2)
		AVFLOAT PopDensity;						// population factors
		AVFLOAT Absentee;						// population factors
		AVFLOAT StairFactor;					// stair factor
		AVFLOAT Escalator;						// escalator factor

		// Derived (resolved) data - set by Resolve
		AVFLOAT StoreyLevel;					// the level above the lowest storey

		// Scaled Dimensions
		AVFLOAT SH, SL;							// Storey Height (Floor to Floor), Level (from Ground)
		BOX m_box;								// lobby floor plan (the same as building, but includes storey height - from floor to ceiling
		
		STOREY();
		virtual ~STOREY();

		CBuildingBase *GetBuilding()			{ return m_pBuilding; }

		// Operations:
		void Resolve(CBuildingBase *pBuilding, AVULONG nIndex);
		virtual void Scale(AVFLOAT scale);
		bool Within(AVVECTOR &pos)				{ return pos.z >= SL && pos.z < SL + SH; }
		std::wstring GetName()					{ return Name; }
	};

protected:

	// Lobby Layout Data - CONSOLE based
	std::wstring BuildingName;
	AVULONG nBuildingID;				// Building ID
	AVULONG NoOfShafts;					// No. of shafts
	AVULONG StoreysAboveGround;			// Number of Storeys above MT1
	AVULONG StoreysBelowGround;			// Number of Storeys below MT1
	SHAFT_ARRANGEMENT LiftShaftArrang;	// Lift shaft arrangements
	LOBBY_ARRANGEMENT LobbyArrangement;	// Lobby arrangement
	LIFT_STRUCTURE Structure;			// Structure (???)
	AVFLOAT LobbyCeilingSlabHeight;		// Ceiling slab height

	AVFLOAT PosLiftBookM;				// Position of lift booking, meters from lobby
	AVULONG NoOfBook;					// No. of lLift booking points
	AVFLOAT MachRoomSlab;				// Machine room slab.  (mm)
	AVFLOAT LiftBeamHeight;				// Lifting beam height. (mm)

	// size (raw data)
	AVFLOAT LobbyDepth;					// Lobby Depth. mm
	AVFLOAT FrontWallThickness;			// Front wall thickness. mm
	AVFLOAT SideWallThickness;			// Side wall thickness. mm (if applicable)
	AVFLOAT ShaftWallThickness;			// External shaft wall thickness. mm
	AVFLOAT IntDivBeamWidth;			// Intermediate divider beam width (mm)
	AVFLOAT IntDivBeamHeight;			// Intermediate divider beam width (mm)

	// Derived (resolved) data - set by Resolve
	AVULONG ShaftLinesCount;			// shaft lines; 1 or 2 depending on LiftShaftArrang
	AVULONG ShaftCount[2];				// counter of lifts per line
	AVFLOAT LineWidth[2];				// widths of lifts lines
	AVFLOAT LobbyWidth;					// Total Lobby Width
	BOX m_mbox;						// unscaled dimensions of everything

	// Scaled Dimensions
	AVFLOAT fScale;						// the scale factor
	BOX m_box;							// scaled lobby floor plan (size of the lobby & its walls, zero height)

private:
	SHAFT **ppShafts;
	STOREY **ppStoreys;

public:
	CBuildingBase(void);
	~CBuildingBase(void);

	AVULONG GetId()							{ return nBuildingID; }

	BOX *GetBox()							{ return &m_box; }
	bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }

	// Shafts
	void CreateShafts();
	void DeleteShafts();
	SHAFT *GetShaft(AVULONG i)				{ return i < GetShaftCount() ? ppShafts[i] : NULL; }

	AVULONG GetShaftCount()					{ return NoOfShafts; }
	AVULONG GetShaftCount(AVULONG nLine)	{ return ShaftCount[nLine]; }
	AVULONG GetShaftLinesCount()			{ return ShaftLinesCount; }

//	AVFLOAT GetShaftLineWidth(AVULONG nLine){ if (nLine == 0) return GetShaft(GetShaftsPerLineCount()-1)->ShaftRPos - GetShaft(0)->ShaftPos + 2 * ShaftWallThickness;
//											  else if (GetShaftLinesCount() == 1) return 0;
//											  else return GetShaft(GetShaftsPerLineCount())->ShaftRPos - GetShaft(GetShaftCount()-1)->ShaftPos + 2 * ShaftWallThickness; }
//	AVFLOAT GetMaxShaftLineWidth()			{ return max(GetShaftLineWidth(0), GetShaftLineWidth(1)); }

	AVULONG GetLiftCount()					{ AVULONG n = 0; for (AVULONG i = 0; i < GetShaftCount(); i++) n += GetShaft(i)->NumberOfLifts; return n; }

	// Storeys
	void CreateStoreys();
	void DeleteStoreys();
	AVULONG GetStoreyCount()				{ return StoreysBelowGround + StoreysAboveGround; }
	AVULONG GetBasementStoreyCount()		{ return StoreysBelowGround; }
	STOREY *GetStorey(AVULONG i)			{ return i < GetStoreyCount() ? ppStoreys[i] : NULL; }

	// Various
	SHAFT_ARRANGEMENT GetLiftShaftArrang()	{ return LiftShaftArrang; }
	LOBBY_ARRANGEMENT GetLobbyArrangement()	{ return LobbyArrangement; }
	AVFLOAT GetLobbyWidth()					{ return LobbyWidth; }
	


	// Status
	bool IsValid()							{ return NoOfShafts && GetStoreyCount() && ppShafts && ppStoreys && GetShaftCount(0); }

	// Calculations!
	void Resolve();
	void Scale(AVFLOAT fScale);

protected:
	virtual SHAFT *CreateShaft() = 0;
	virtual STOREY *CreateStorey() = 0;
};

class CBuildingBaseEx : public CBuildingBase
{
protected:
	virtual SHAFT *CreateShaft()			{ return new SHAFT; }
	virtual STOREY *CreateStorey()			{ return new STOREY; }
};

