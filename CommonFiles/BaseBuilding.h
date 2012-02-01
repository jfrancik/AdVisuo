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

	// Shaft Layout data
	class SHAFT : public dbtools::CCollection
	{
		AVULONG m_nId;							// Shaft Id
		CBuildingBase *m_pBuilding;				// main building
		AVULONG m_nShaftLine;					// 0 for SHAFT_INLINE and 0 or 1 for SHAFT_OPPOSITE

		AVULONG m_nLiftCount;					// number of lifts in a shaft (usually 1)
		TYPE_OF_LIFT m_type;					// type of lift (conventional/MRL)
		TYPE_OF_DECK m_deck;					// type of deck (single/double/twin)

		// Dimensions
		BOX m_boxShaft;							// shaft box (including ext walls thickness)
		AVFLOAT m_fWallLtStart, m_fWallRtStart;	// offset at which left & right walls start, 0 if all the length
		BOX m_boxBeam;							// left-side int beam (thickness = 0)
		BOX m_boxDoor[2];						// shaft external door (thickness = 0)
		BOX m_boxCar;							// lift car (including car walls thickness)
		BOX m_boxCarDoor[2];					// car internal door (thickness = 0)

	public:

		SHAFT();
		virtual ~SHAFT()						{ }

		// Attributes:
		AVULONG GetId()							{ return m_nId; }
		CBuildingBase *GetBuilding()			{ return m_pBuilding; }
		AVULONG GetShaftLine()					{ return m_nShaftLine; }
		AVFLOAT GetShaftDir()					{ return m_nShaftLine == 0 ? -1.0f : 1.0f; }	// -1 for line 0 lifts (far row), 1 for line 1 lifts (near raw)
		std::wstring GetName()					{ wchar_t buf[256]; _snwprintf_s(buf, 256, L"Lift %c", GetId() + 'A'); return buf; }

		AVULONG GetLiftCount()					{ return m_nLiftCount; }
		TYPE_OF_LIFT GetType()					{ return m_type; }
		TYPE_OF_DECK GetDeck()					{ return m_deck; }


		enum SHAFT_BOX { BOX_SHAFT, BOX_CAR, BOX_BEAM, BOX_DOOR };
		BOX &GetBox(enum SHAFT_BOX n = BOX_SHAFT);
		BOX &GetBoxBeam()						{ return m_boxBeam; }
		BOX &GetBoxDoor(AVULONG i = 0)			{ return m_boxDoor[i]; }
		BOX &GetBoxCar()						{ return m_boxCar; }
		BOX &GetBoxCarDoor(AVULONG i = 0)		{ return m_boxCarDoor[i]; }

		bool InBox(AVVECTOR &pt, enum SHAFT_BOX box = BOX_SHAFT){ return GetBox(box).InBoxExt(pt) || GetBox(BOX_DOOR).InBoxExt(pt); }
		bool Within(AVVECTOR &pos, AVFLOAT nLiftZPos = 0)		{ return pos.z >= nLiftZPos && pos.z < nLiftZPos + m_boxCar.Height(); }


		// Operations:
		void Xxxx(CBuildingBase *pBuilding, AVULONG nId, AVFLOAT fFrontWall, AVFLOAT fRearWall);
		void Xxxx(AVULONG nLine, AVFLOAT fShaftPosX, AVFLOAT fShaftPosY);
		void XxxxLeftBeam(AVFLOAT fDepth);
		void XxxxRightBeam(AVFLOAT fDepth);
		void XxxxLeftWall(AVFLOAT fThickness, AVFLOAT fStart = 0);
		void XxxxRightWall(AVFLOAT fThickness, AVFLOAT fStart = 0);
		void XxxxAmend();
		void Yyyy(CBuildingBase *pBuilding);
		void Scale(AVFLOAT fScale);
	};

	// Storey Data
	class STOREY : public dbtools::CCollection
	{
		AVULONG m_nId;							// Storey Id
		CBuildingBase *m_pBuilding;				// main building
		std::wstring m_strName;					// storey name

		AVFLOAT m_fLevel;						// lobby floor level
		BOX m_box;								// lobby floor plan (the same as building, but includes storey height - from floor to ceiling
		
	public:
		STOREY();
		virtual ~STOREY()						{ }

		// Attributes
		AVULONG GetId()							{ return m_nId; }
		CBuildingBase *GetBuilding()			{ return m_pBuilding; }
		std::wstring GetName()					{ return m_strName; }

		AVFLOAT GetLevel()						{ return m_fLevel; }
		AVFLOAT GetHeight()						{ return GetBox().HeightExt(); }
		AVFLOAT GetCeilingHeight()				{ return GetBox().Height(); }

		BOX &GetBox()							{ return m_box; }
		bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }
		bool Within(AVVECTOR &pos)				{ return pos.z >= GetLevel() && pos.z < GetLevel() + GetHeight(); }

		// Operations:
		void Xxxx(CBuildingBase *pBuilding, AVULONG nId, AVFLOAT fLevel);
		void Yyyy(CBuildingBase *pBuilding);
		void Scale(AVFLOAT fScale);
	};

private:

	AVULONG m_nId;						// Building ID

	AVULONG m_nShaftCount;				// Counters (shafts/floors)
	AVULONG m_nStoreyCount;
	AVULONG m_nBasementStoreyCount;
	AVULONG m_pnShaftCount[2];			// counter of lifts per line

	SHAFT_ARRANGEMENT m_LiftShaftArrang;// Lift shaft arrangements
	LOBBY_ARRANGEMENT m_LobbyArrangement;// Lobby arrangement

	BOX m_box;							// scaled lobby floor plan (size of the lobby & its walls, zero height)
	AVFLOAT m_fScale;					// the scale factor

	SHAFT **m_ppShafts;
	STOREY **m_ppStoreys;

public:
	CBuildingBase(void);
	~CBuildingBase(void);

	AVULONG GetId()							{ return m_nId; }
	void SetId(AVULONG nId)					{ m_nId = nId; }

	BOX &GetBox()							{ return m_box; }
	bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }

	// Shafts
	void CreateShafts(AVULONG nShaftCount);
	void DeleteShafts();
	SHAFT *GetShaft(AVULONG i)				{ return i < GetShaftCount() ? m_ppShafts[i] : NULL; }

	AVULONG GetShaftCount()					{ return m_nShaftCount; }
	AVULONG GetShaftCount(AVULONG nLine)	{ return m_pnShaftCount[nLine]; }
	AVULONG GetShaftLinesCount()			{ return m_pnShaftCount[1] == 0 ? 1 : 2; }

	AVULONG GetLiftCount()					{ AVULONG n = 0; for (AVULONG i = 0; i < GetShaftCount(); i++) n += GetShaft(i)->GetLiftCount(); return n; }

	// Storeys
	void CreateStoreys(AVULONG nStoreyCount, AVULONG nBasementStoreyCount = 0);
	void DeleteStoreys();
	AVULONG GetStoreyCount()				{ return m_nStoreyCount; }
	AVULONG GetBasementStoreyCount()		{ return m_nBasementStoreyCount; }
	STOREY *GetStorey(AVULONG i)			{ return i < GetStoreyCount() ? m_ppStoreys[i] : NULL; }

	// Various
	SHAFT_ARRANGEMENT GetLiftShaftArrang()	{ return m_LiftShaftArrang; }
	LOBBY_ARRANGEMENT GetLobbyArrangement()	{ return m_LobbyArrangement; }
	


	// Status
	bool IsValid()							{ return m_nShaftCount && GetStoreyCount() && m_ppShafts && m_ppStoreys && GetShaftCount(0); }

	// Calculations!
	void PreXxxx();
	void Xxxx();
	void Yyyy();
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

