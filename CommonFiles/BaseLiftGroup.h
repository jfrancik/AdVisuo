// BaseGroup.h - AdVisuo Common Source File

#pragma once

#include "Box.h"
#include "DBTools.h"
#include <functional>

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
	enum SHAFT_ARRANG		{ SHAFT_INLINE = 1, SHAFT_OPPOSITE, SHAFT_UNKNOWN = -1 };
	enum LOBBY_ARRANG		{ LOBBY_THROUGH = 1, LOBBY_OPENPLAN, LOBBY_DEADEND_LEFT, LOBBY_DEADEND_RIGHT, LOBBY_UNKNOWN = -1 };
	enum DOOR_TYPE			{ DOOR_CENTRE_CAR = 1, DOOR_CENTRE_SHAFT = 2, DOOR_SIDE_LEFT = 3, DOOR_SIDE_RIGHT = 4, DOOR_UNKNOWN = -1 };
//	enum TYPE_OF_LIFT		{ LIFT_CONVENTIONAL = 1, LIFT_MRL, LIFT_UNKNOWN };
//	enum TYPE_OF_DECK		{ DECK_SINGLE = 1, DECK_DOUBLE, DECK_TWIN, DECK_UNKNOWN = -1 };
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
		XBOX m_box;								// lobby floor plan (the same as lift group's, but includes storey height - from floor to ceiling
		
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

		XBOX &GetBox()							{ return m_box; }
		bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }
		bool Within(AVVECTOR &pos)				{ return pos.z >= GetLevel() && pos.z < GetLevel() + GetHeight(); }

		bool IsStoreyServed()					{ for each (SHAFT *pShaft in GetLiftGroup()->GetShafts()) if (pShaft->IsStoreyServed(GetId())) return true; return false; }
		bool IsStoreyServed(AVULONG nShaft)		{ return GetLiftGroup()->GetShaft(nShaft)->IsStoreyServed(GetId()); }

		bool IsMain()							{ return GetLiftGroup()->IsStoreyMain(GetId()); }

		// Operations:
		void ConsoleCreate(AVULONG nId, AVFLOAT fLevel);
		void Create();
		void Scale(AVFLOAT fScale);
		void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z);
	};

	class MR : public STOREY
	{
	public:
		MR(CLiftGroup *pLiftGroup) : STOREY(pLiftGroup, 9999)		{ }
		virtual ~MR()												{ }
		void ConsoleCreate();
		void Create();

		AVFLOAT GetExt();
		XBOX GetBoxMain();
		XBOX GetBoxExt();
	};

	class PIT : public STOREY
	{
	public:
		PIT(CLiftGroup *pLiftGroup) : STOREY(pLiftGroup, 9998)		{ }
		virtual ~PIT()												{ }
		void ConsoleCreate();
		void Create();
	};

	// Shaft Layout data
	class SHAFT : public dbtools::CCollection
	{
		// Identification and Relations
		AVULONG m_nId;							// Shaft Id
		CLiftGroup *m_pLiftGroup;				// lift group

		// Directly from the Console DB
		AVLONG m_nOpeningTime;					// door opening time
		AVLONG m_nClosingTime;					// door closing time
		AVLONG m_nLoadingTime;
		AVLONG m_nUnloadingTime;
		AVLONG m_nDwellTime;					// dwell time - not main floor
		AVLONG m_nMainFloorDwellTime;			// dwell time - main floor
		AVLONG m_nPreOpeningTime;				// preopening time [ms]
		AVLONG m_nMotorStartDelayTime;			// motor delay time [ms]
		AVULONG m_nReopenings;					// max limit for reopenings

		AVFLOAT m_fCapacity;					// capacity & kinematic parameters
		AVFLOAT m_fSpeed;
		AVFLOAT m_fAcceleration;
		AVFLOAT m_fJerk;

		AVULONG m_nLiftType;					// 1 = conventional, 2 = MRL
		AVULONG m_nDeckType;					// 
		AVULONG m_nDoorType;					// door type (1 = centre car; 2 = centre shaft; 3 = side left; 4 = side right)
		AVULONG m_nDoorPanels;					// door panel number
		AVULONG m_nCounterWeightPosition;		// counterweight position (1 = rear, 2 = left, 3 - right)

		// Not currently in DB: FOR FUTURE EXTENSION - MULTI-CAR SHAFTS)
		AVULONG m_nLiftCount;					// usually 1, may be more for multiple lifts options

		// Special Value: modelled in CLiftGroupSrv::LoadFromConsole
		std::wstring m_strStoreysServed;		// storeys served; "0" for not served, "1" for served

		// Derived Values
		AVULONG m_nShaftLine;					// 0 for SHAFT_INLINE and 0 or 1 for SHAFT_OPPOSITE
		AVULONG m_nLiftBegin;					// index of the first lift, usually m_nId but may be different for multiple lifts options

		// Dimensions and similar
		XBOX m_boxShaft;						// shaft box (including ext walls thickness)
		AVFLOAT m_fWallLtStart, m_fWallRtStart;	// Y-coordinate where left & right walls start, 0 if no wall
		AVFLOAT m_fBeamLtHeight, m_fBeamRtHeight;// Height of the Left/Right Int Beam
		BOX m_boxDoor[2];						// shaft external door (thickness = 0)
		XBOX m_boxCar;							// lift car (including car walls thickness)
		BOX m_boxCarDoor[2];					// car internal door (thickness = 0)
		BOX m_boxCwt;							// counterweight
		XBOX m_boxCombinationBracket;			// combination bracket
		BOX m_boxGovernor;						// governors
		BOX m_boxLadder;						// pit ladder
		BOX m_boxPanelCtrl;						// control panel
		BOX m_boxPanelDrv;						// drive panel
		BOX m_boxPanelIso;						// isolator panel (may be 0-size)

		AVFLOAT m_fShaftOrientation;			// shaft orientation (0 for line 0, M_PI for line 1)
		AVULONG m_nMachineType;					// machine type (1 - 4)
		AVFLOAT m_fMachineOrientation;			// machine orientation
		AVULONG m_nPanelCtrlType, m_nPanelDrvType, m_nPanelIsoType;		// control/drive/isolator panel type
		AVFLOAT m_fPanelIsoOrientation;			// isolator panel orientation
		AVFLOAT m_fRailWidth;					// guiding rail width
		AVFLOAT m_fRailLength;					// guiding rail length
		AVULONG m_nBufferNum;					// number of car/cwt buffers
		AVULONG m_nBufferDiameter;				// diameter of the buffers
		AVULONG m_nBufferHeight;				// height of the buffers
		AVFLOAT m_fLightingXPos;				// position of the lighting

	public:

		SHAFT(CLiftGroup *pLiftGroup, AVULONG nId) : m_pLiftGroup(pLiftGroup), m_nId(nId), 
			m_nShaftLine(0), m_nLiftType(1), m_nDeckType(1), m_nLiftCount(1), m_fWallLtStart(0), m_fWallRtStart(0), m_fBeamLtHeight(0), m_fBeamRtHeight(0)	{ }
		virtual ~SHAFT()						{ }

		// Attributes:
		AVULONG GetId()							{ return m_nId; }
		CLiftGroup *GetLiftGroup()				{ return m_pLiftGroup; }
		CProject *GetProject()					{ return GetLiftGroup()->GetProject(); }
		AVULONG GetShaftLine()					{ return m_nShaftLine; }
		AVULONG GetIndexInLine()				{ return GetId() - GetLiftGroup()->GetShaftBegin(GetShaftLine()); }
		std::wstring GetName()					{ wchar_t buf[256]; _snwprintf_s(buf, 256, L"Lift %c", GetId() + 'A'); return buf; }

		AVLONG GetNativeId()					{ return ME[L"LiftId"]; }

		AVULONG GetDeckCount()					{ return m_nDeckType == 2 ? 2 : 1; }

		AVLONG GetOpeningTime()					{ return m_nOpeningTime; }
		AVLONG GetClosingTime()					{ return m_nClosingTime; }
		AVLONG GetLoadingTime()					{ return m_nLoadingTime; }
		AVLONG GetUnloadingTime()				{ return m_nUnloadingTime; }
		AVLONG GetDwellTime(AVULONG nFloor)		{ return GetLiftGroup()->IsStoreyMain(nFloor) ? m_nMainFloorDwellTime : m_nDwellTime; }
		AVLONG GetPreOpeningTime()				{ return m_nPreOpeningTime; }
		AVLONG GetMotorStartDelayTime()			{ return m_nMotorStartDelayTime; }
		AVULONG GetReopenings()					{ return m_nReopenings; }
		AVFLOAT GetCapacity()					{ return m_fCapacity; }
		AVFLOAT GetSpeed()						{ return m_fSpeed; }
		AVFLOAT GetAcceleration()				{ return m_fAcceleration; }
		AVFLOAT GetJerk()						{ return m_fJerk; }

		bool IsStoreyServed(AVULONG nStorey)	{ return (nStorey < m_strStoreysServed.length() && m_strStoreysServed[nStorey] == '1') ? true : false; }
		AVULONG GetHighestStoreyServed()		{ return m_strStoreysServed.find_last_of('1'); }
		AVULONG GetLowestStoreyServed()			{ return m_strStoreysServed.find('1'); }

		AVULONG GetLiftBegin()					{ return m_nLiftBegin; }
		AVULONG GetLiftCount()					{ return m_nLiftCount; }
		AVULONG GetLiftEnd()					{ return m_nLiftBegin + m_nLiftCount; }
		void SetLiftRange(AVULONG i, AVULONG n)	{ m_nLiftBegin = i; m_nLiftCount = n; } 

		XBOX &GetBox()							{ return m_boxShaft; }
		BOX &GetBoxDoor(AVULONG i = 0)			{ return m_boxDoor[i]; }
		XBOX &GetBoxCar()						{ return m_boxCar; }
		BOX &GetBoxCarDoor(AVULONG i = 0)		{ return m_boxCarDoor[i]; }
		BOX &GetBoxCwt()						{ return m_boxCwt; }
		XBOX &GetCombinationBracketBox()		{ return m_boxCombinationBracket; }
		XBOX &GetCombBox()						{ return m_boxCombinationBracket; }
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
		BOX &GetBoxPanelCtrl()					{ return m_boxPanelCtrl; }
		BOX &GetBoxPanelDrv()					{ return m_boxPanelDrv; }
		BOX &GetBoxPanelIso()					{ return m_boxPanelIso; }
		AVFLOAT GetPanelCtrlOrientation()		{ return GetShaftOrientation() + M_PI; }	// no separate orientation for ctrl panels - mirrors that of the shaft
		AVFLOAT GetPanelDrvOrientation()		{ return GetShaftOrientation() + M_PI; }	// no separate orientation for ctrl panels - mirrors that of the shaft
		AVFLOAT GetPanelIsoOrientation()		{ return m_fPanelIsoOrientation; }
		void SetPanelIsoOrientation(AVFLOAT f)	{ m_fPanelIsoOrientation = f; }


		AVFLOAT GetRailWidth()					{ return m_fRailWidth; }
		AVFLOAT GetRailLength()					{ return m_fRailLength; }
		AVULONG GetBufferNum()					{ return m_nBufferNum; }
		AVULONG GetBufferDiameter()				{ return m_nBufferDiameter; }
		AVULONG GetBufferHeight()				{ return m_nBufferHeight; }
		AVFLOAT GetLightingXPos()				{ return m_fLightingXPos; }
		AVULONG GetDoorType()					{ return m_nDoorType; }					// 1 = centre car; 2 = centre shaft; 3 = side left; 4 = side right
		AVULONG GetDoorPanelsCount()			{ return m_nDoorPanels; }				// number of panels per door (between 1 and 6)
		AVULONG GetDoorPanelsPerDoor()			{ return m_nDoorType <= 2 ? m_nDoorPanels / 2 : m_nDoorPanels; }	// number of panels per door (1, 2, or 3)
		AVULONG GetCounterWeightPosition()		{ return m_nCounterWeightPosition; }	// (1 = rear, 2 = left, 3 - right)

		AVVECTOR GetLiftPos(AVULONG nStorey)	{ return GetBoxCar() + Vector(0, 0, GetLiftGroup()->GetStorey(nStorey)->GetLevel()); }

		// raw data functions
//		AVFLOAT GetRawWidth()					{ return (*this)[L"ShaftWidth"]; }
//		AVFLOAT GetRawBeamWidth()				{ return (*this)[L"DividingBeamWidth"]; }

		// checks if point within the shaft, beam or door box
		bool InBox(AVVECTOR &pt)							{ return GetBox().InBoxExt(pt) || GetBoxDoor().InBox(pt); }
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
			XBOX box(posLift.x, posLift.y, posLift.z, GetShaft()->GetBoxCar().Width(), GetShaft()->GetBoxCar().Depth(), GetShaft()->GetBoxCar().Height());
			return box.InWidth(pos.x) && box.InDepth(pos.y) && box.InHeight(pos.z);
		}

	};

private:

	AVULONG m_nId;						// Lift Group ID
	AVULONG m_nProjectId;				// Project ID
	AVULONG m_nIndex;					// index in multi-group structures

	CProject *m_pProject;				// The Project Object
	std::wstring m_strName;				// Lift Group name

	AVFLOAT m_fScale;					// scale factor (applied to all dimensions)

	AVULONG m_pnShaftCount[2];			// counter of lifts per line

	// lobby arrangement
	SHAFT_ARRANG m_LiftShaftArrang;		// Lift shaft arrangements
	LOBBY_ARRANG m_LobbyArrangement;	// Lobby arrangement

	// the lobby box
	XBOX m_box;							// scaled lobby floor plan (size of the lobby & its walls, zero height)
	
	// machine room layout
	XBOX m_boxMR;						// scaled floor plan and level for the machine room
	AVFLOAT m_fMRLevel;					// machine room level
	AVFLOAT m_fMRDoorOffset;			// machine room door offset value
	AVFLOAT m_fLiftingBeamHeight;		// lifting beam height
	AVFLOAT m_fLiftingBeamWidth;		// lifting beam height
	AVFLOAT m_fMRIndentFront;			// position of indentation for extended/indented MR (to provide space for excessive panels)
	AVFLOAT m_fMRIndentRear;			// position of indentation for extended/indented MR (to provide space for excessive panels)

	// group panel orientation (MR)
	BOX m_boxPanelGrp;					// group panel (box & orientation)
	AVFLOAT m_fPanelGrpOrientation;

	// pit level layout
	XBOX m_boxPit;						// scaled floor plan and level for the pit level
	AVFLOAT m_fPitLevel;

	// main floors (a.k.a. lobbies)
	std::wstring m_strMainStoreys;		// catalogue of storeys: "1" for the main storey (a.k.a. lobby), "0" for any other


protected:
	std::vector<STOREY*> m_storeys;
	std::vector<SHAFT*>  m_shafts;
	std::vector<LIFT*>   m_lifts;
	std::vector<CSim*>	 m_sims;

	CSim *m_pCurSim;					// currently active Sim (used by AdVisuo client)
	
	MR *m_pMR;
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

	// scale
	AVFLOAT GetScale()						{ return m_fScale; }

	// the box
	XBOX &GetBox()							{ return m_box; }
	bool InBox(AVVECTOR &pt)				{ return m_box.InBoxExt(pt); }

	// other info
	bool IsMRL()							{ for (AVULONG i = 0; i < GetShaftCount(); i++) if (!GetShaft(i)->IsMRL()) return false; return true; }
	XBOX GetTotalAreaBox();

	// collection generic
	std::vector<STOREY*> &GetStoreys()		{ return m_storeys; }
	std::vector<SHAFT*> &GetShafts()		{ return m_shafts; }
	std::vector<LIFT*> &GetLifts()			{ return m_lifts; }
	std::vector<CSim*> &GetSims()			{ return m_sims; }

	// Storeys
	void CreateStoreys(AVULONG nStoreyCount, AVULONG nBasementStoreyCount = 0);
	STOREY *AddStorey();
	void DeleteStoreys();
	AVULONG GetStoreyCount()				{ return m_storeys.size(); }
	AVULONG GetBasementStoreyCount()		{ return 0; }
	STOREY *GetStorey(AVULONG i)			{ return i < GetStoreyCount() ? m_storeys[i] : NULL; }

	// Extras: Machine Room & Pit
	void AddExtras();
	void DeleteExtras();

	// MR
	MR *GetMR()								{ return m_pMR; }

	XBOX &GetBoxMR()						{ return m_boxMR; }
	bool InBoxMR(AVVECTOR &pt)				{ return m_boxMR.InBoxExt(pt); }
	BOX &GetBoxPanelGrp()					{ return m_boxPanelGrp; }
	AVFLOAT GetPanelGrpOrientation()		{ return m_fPanelGrpOrientation; }

	AVFLOAT GetMRLevel()					{ return m_fMRLevel; }
	AVFLOAT GetMRSlabThickness()			{ return GetBoxMR().LowerThickness(); }
	AVFLOAT GetLiftingBeamHeight()			{ return m_fLiftingBeamHeight; }
	AVFLOAT GetLiftingBeamWidth()			{ return m_fLiftingBeamWidth; }
	AVFLOAT GetMRDoorOffset()				{ return m_fMRDoorOffset; }
	AVFLOAT GetMRDoorWidth()				{ return 1200 * GetScale(); }
	AVFLOAT GetMRDoorHeight()				{ return 2100 * GetScale(); }
	AVFLOAT GetMRIndentFront()				{ return m_fMRIndentFront; }
	AVFLOAT GetMRIndentRear()				{ return m_fMRIndentRear; }

	AVSIZE CalcPanelFootstep(AVULONG iFrom = 0, AVULONG iTo = 9999);
	AVSIZE CalcPanelFootstepIso(AVULONG iFrom = 0, AVULONG iTo = 9999);
	AVSIZE CalcPanelFootstepGrp()			{ return AVSIZE(GetBoxPanelGrp().Width(), abs(GetBoxPanelGrp().Depth())); }
	AVSIZE CalcPanelFootstepLn(AVULONG iLn)	{ return CalcPanelFootstep(GetShaftBegin(iLn), GetShaftEnd(iLn)); }

	// Pit
	PIT *GetPit()							{ return m_pPit; }

	XBOX &GetBoxPit()						{ return m_boxPit; }
	bool InBoxPit(AVVECTOR &pt)				{ return m_boxPit.InBoxExt(pt); }

	AVFLOAT GetPitLevel()					{ return m_fPitLevel; }
	AVFLOAT GetPitHeight()					{ return -m_fPitLevel; }
	bool IsPitLadder()						{ return GetPitHeight() > 600 * GetScale() && GetPitHeight() < 2500 * GetScale(); }
	bool IsPitDoor()						{ return GetPitHeight() >= 2500 * GetScale(); }
	AVULONG GetPitLadderRungs()				{ if (IsPitLadder()) return (GetPitHeight() / GetScale() + 1000) / 300; else return 0; }
	AVFLOAT GetPitLadderHeight()			{ if (IsPitLadder()) return (GetPitLadderRungs() + 1) * 300 * GetScale(); else return 0; }
	AVFLOAT GetPitLadderLowerBracket()		{ if (IsPitLadder()) return 500 * GetScale(); else return 0; }
	AVFLOAT GetPitLadderUpperBracket()		{ if (IsPitLadder()) return (GetPitLadderHeight() - 600) * GetScale(); else return 0; }

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
	AVULONG GetHighestStoreyServed();
	AVULONG GetLowestStoreyServed();
	AVULONG GetFloorUp(AVULONG nStorey);
	AVULONG GetFloorDown(AVULONG nStorey);
	AVULONG GetValidFloor(AVULONG nStorey);

	// Main Floors (a.k.a. lobbies)
	bool IsStoreyMain(AVULONG nStorey)		{ return m_strMainStoreys[nStorey] == '1' ? true : false; }
	AVULONG GetLowestMainStorey()			{ return m_strMainStoreys.find('1'); }
	AVULONG GetLobby()						{ return m_strMainStoreys.find('1'); }

	// The Sim
	CSim *AddSim();
	void DeleteSims();
	CSim *GetSim(AVULONG i)					{ return i < GetSimCount() ? m_sims[i] : NULL; }
	AVULONG GetSimCount()					{ return m_sims.size(); }

	CSim *GetCurSim()						{ return m_pCurSim; }
	void SetCurSim(CSim *pSim = NULL)		{ m_pCurSim = pSim; }

	// Various
	SHAFT_ARRANG GetLiftShaftArrang()		{ return m_LiftShaftArrang; }
	LOBBY_ARRANG GetLobbyArrangement()		{ return m_LobbyArrangement; }

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
	virtual MR *CreateMR() = 0;
	virtual PIT *CreatePit() = 0;

	virtual CSim *CreateSim() = 0;
};

