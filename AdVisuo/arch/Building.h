#pragma once

#include "Box.h"
#include <xmllite.h>

class CBuilding
{
// Public Attributes
public:
	enum SHAFT_ARRANGEMENT	{ SHAFT_INLINE, SHAFT_OPPOSITE, SHAFT_UNKNOWN };
	enum LOBBY_ARRANGEMENT	{ LOBBY_THROUGH, LOBBY_OPENPLAN, LOBBY_DEADEND_LEFT, LOBBY_DEADEND_RIGHT, LOBBY_UNKNOWN };
	enum DOOR_TYPE			{ DOOR_CENTRE, DOOR_SIDE, DOOR_UNKNOWN };
	enum TYPE_OF_LIFT		{ LIFT_SINGLE_DECK, LIFT_DOUBLE_DECK, LIFT_TWIN, LIFT_UNKNOWN };
	enum CAR_ENTRANCES		{ CAR_FRONT, CAR_REAR, CAR_BOTH, CAR_UNKNOWN };
	enum CNTRWEIGHT_POS		{ CNTRWEIGHT_SIDE, CNTRWEIGHT_REAR, CNTRWEIGHT_UNKNOWN };
	enum LIFT_STRUCTURE		{ STRUCT_CONCRETE, STRUCT_UNKNOWN };

	// Lobby Layout Data (global) - XML Data
	AVULONG NoOfLifts;					// No. of lifts
	AVFLOAT PosLiftBookM;				// Position of lift booking, meters from lobby
	AVULONG NoOfBook;					// No. of Lift booking points
	AVFLOAT LobbyDepth;					// Lobby Depth. mm
	AVFLOAT FrontWallThickness;			// Front wall thickness. mm
	AVFLOAT LobbyCeilingSlabHeight;		// Ceiling slab height
	AVULONG StoreysAboveGround;			// Number of Storeys above MT1
	AVULONG StoreysBelowGround;			// Number of Storeys below MT1
	AVULONG HomeStorey;					// Name of the Home Storey
	SHAFT_ARRANGEMENT LiftShaftArrang;	// Lift shaft arrangements
	LOBBY_ARRANGEMENT LobbyArrangement;	// Lobby arrangement

	// Derived (non-XML) Data (global)
	AVULONG NoOfStoreys;				// Number Of Storeys
	AVFLOAT LobbyWidth;					// Total Lobby Width (calculated from no and width of shafts)
	AVULONG NoOfLiftLines;				// 1 for SHAFT_INLINE, 2 for SHAFT_OPPOSITE
	AVULONG NoOfLiftsInLine;			// Number of Lifts in Line

	// Scaled Dimensions
	AVFLOAT fScale;						// the scale factor
	BOX m_box;							// scaled lobby floor plan (size of the lobby & its walls, zero height)
	AVFLOAT m_ceiling;					// scaled ceiling

 	// Lift Layout data
	struct LIFT
	{
		// XML Data
		AVULONG LiftID;						// Lift Id
		AVULONG Capacity;					// Lift capacity (kg)
		AVFLOAT Speed;						// Lift speed. (Ms2)
		DOOR_TYPE DoorType;					// Lift door type
		bool UnOc;							// Un – occupied lift behaviour (park at last storey)
		AVFLOAT LiftDoorHeight;				// Lift door height. (mm)
		AVFLOAT LiftDoorWidth;				// Lift door width. (mm)
		AVFLOAT CarHeight;					// Car height mm
		TYPE_OF_LIFT TypeOfLift;			// Type of lift
		AVFLOAT MachRoomSlab;				// Machine room slab.  (mm)
		AVFLOAT LiftBeamHeight;				// Lifting beam height. (mm)
		CAR_ENTRANCES NoOfCarEntrances;		// Number (Type) of car entrances
		CNTRWEIGHT_POS CounterWeightPosition;// Counter weight position
		LIFT_STRUCTURE Structure;			// Structure (???)
		AVFLOAT CarWidth;					// Car width. (mm)	
		AVFLOAT CarDepth;					// Car depth. (mm)	
		AVFLOAT ShaftWidth;					// Shaft width. (mm)	
		AVFLOAT ShaftDepth;					// Shaft depth. (mm)	
		AVFLOAT IntDivBeam;					// Intermediate divided beam. (mm)
		AVFLOAT PitDepth;					// Lift pit depth . (mm)	
		AVFLOAT OverallHeight;				// OverallHeight. (mm)	Overall 
		AVFLOAT HeadRoom;					// Lift headroom. (mm)	
		AVFLOAT MachRoomHeight;				// Lift machine room height . (mm)

		// Derived (non XML) data
		AVULONG LiftLine;						// 0 for SHAFT_INLINE arrangement and 0 or 1 for SHAFT_OPPOSITE
		AVFLOAT ShaftPos;						// left side of the Lift shaft, internal
		AVFLOAT LeftBeam, RightBeam, RearBeam;	// detailed Beam width information (derived from IntDivBeam)

		// Scaled Dimensions
		BOX m_box;							// scaled shaft plan
		BOX m_boxDoor;						// scaled shaft door plan
		BOX m_boxLift;						// scaled lift plan

		void XmlParse(CComPtr<IXmlReader> pReader);
		void SetScaledDims(BOX &lobbyBox, AVFLOAT scale);

		LIFT();
	} *pLifts;

	// Storey Data
	struct STOREY
	{
		// XML Data
		AVULONG StoreyID;					// Storey Id
		AVFLOAT HeightValue;				// Height
		bool ExpressZone;					// Express Zone?

		// Derived (non XML) data
		AVFLOAT StoreyLevel;				// the level above the lowest storey

		// Scaled Dimensions
		AVFLOAT SH, SL;						// Storey Height (Floor to Floor), Level (from Ground)
		
		void XmlParse(CComPtr<IXmlReader> pReader);
		void SetScaledDims(AVFLOAT scale);

		STOREY();
	} *pStoreys;

	CBuilding(void);
	~CBuilding(void);

	HRESULT LoadAsXML(LPCOLESTR pFileName);

private:
	void XmlParse(CComPtr<IXmlReader> pReader);
public:
	void SetScaledDims(AVFLOAT fScale);
};
