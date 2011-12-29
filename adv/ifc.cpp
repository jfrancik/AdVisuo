// ifc.cpp - a part the CBuilding class, the AdVisuo Server Module

#include "StdAfx.h"
#include "Building.h"
#include "ifc/baseIfcObject.h"
#include "ifcscanner.h"

#pragma warning (disable:4996)

HRESULT CBuilding::SaveAsIFC(LPCOLESTR pFileName, bool bBrep, bool bPresentation)
{
	//////////////////////////////////////////////////////////////////
	// Prepare for creation of REVIT elements
	CRevitFile revit1("Machine30T_UPSTAND.ifc");
	int hMachine = revit1.GetInstance(0);
	CRevitFile revit2("21April2011AllComponents.ifc");
	int hLift = revit2.GetInstance(0);
	int hBuffer = revit2.GetInstance(1);
	int hDoorCar = revit2.GetInstance(4);
	int hDoorLanding = revit2.GetInstance(5);

	// Bounding boxes
	CIFCModelScanner::BB bbMachine;
	CIFCModelScanner::GetBB(hMachine, bbMachine);
	CIFCModelScanner::BB bbLift;
	CIFCModelScanner::GetBB(hLift, bbLift);
	CIFCModelScanner::BB bbBuffer;
	CIFCModelScanner::GetBB(hBuffer, bbBuffer);


	//freopen( "c:\\users\\jarek\\desktop\\output.txt", "w", stdout );
	//CIFCModelScanner::Dump(hMachine);
	//freopen( "CON", "w", stdout );
	
	//////////////////////////////////////////////////////////////////
	// create the project, building & site

	transformationMatrixStruct    matrix;
	identityMatrix(&matrix);

	//CIFCProject prj(_stdPathModels + "IFC2X3_TC1.exp", "MILLI");
	CIFCProject prj("IFC2X3_TC1.exp", "MILLI");
	if (!prj.build()) return Logf(ERROR_IFC_PRJ, L"project");

	CIFCSite site(&prj, &matrix);
	if (!site.build()) return Logf(ERROR_IFC_PRJ, L"site");

	CIFCBuilding building(&site, &matrix);
	if (!building.build()) return Logf(ERROR_IFC_PRJ, L"building");

	char buf[257];

	for (AVULONG i = 0; i < GetStoreyCount(); i++)
	{
		STOREY *pStorey = GetStorey(i);

		identityMatrix(&matrix);
		matrix._43 = pStorey->StoreyLevel;
		CIFCStorey storey(&building, &matrix);
		_snprintf(buf, 256, "Storey %d", pStorey->StoreyID);
		storey.setStoreyInfo(buf, buf);
		if (!storey.build()) return Logf(ERROR_IFC_PRJ, L"storey");

		// Floors & Ceilings
		identityMatrix(&matrix);
		if (i == 0)
		{
			CIFCSlab floor(&storey, &matrix, LobbyWidth, 2, LobbyDepth + 2*FrontWallThickness, bBrep, bPresentation);
			floor.setSlabInfo("Floor", "Floor");
			if (!floor.build()) return Logf(ERROR_IFC_PRJ, L"slab");
		}
		matrix._43 = pStorey->HeightValue - LobbyCeilingSlabHeight;
		CIFCSlab ceiling(&storey, &matrix, LobbyWidth, LobbyCeilingSlabHeight, LobbyDepth + 2*FrontWallThickness, bBrep, bPresentation);
		ceiling.setSlabInfo("Ceiling", "Ceiling");
		if (!ceiling.build()) return Logf(ERROR_IFC_PRJ, L"slab");

		// Front Wall
		identityMatrix(&matrix);
		CIFCWall wallfront(&storey, &matrix, LobbyWidth, pStorey->HeightValue - LobbyCeilingSlabHeight, FrontWallThickness, bBrep, bPresentation);
		wallfront.setWallInfo("Lobby Front Wall", "Lobby Front Wall");
		if (!wallfront.build()) return Logf(ERROR_IFC_PRJ, L"wall");
		for (AVULONG j = 0; j < GetShaftCount(0); j++)
		{
			SHAFT *pLift = GetShaft(j);
			identityMatrix(&matrix);
			matrix._41 = pLift->ShaftPos + (pLift->ShaftWidth - pLift->LiftDoorWidth) / 2;
			CIFCOpening door(&wallfront, &matrix, pLift->LiftDoorWidth, pLift->LiftDoorHeight, FrontWallThickness, bBrep, bPresentation);
			_snprintf(buf, 256, "Door Opening for Lift Shaft %d", pLift->ShaftID);
			door.setOpeningInfo(buf, buf);
			if (!door.build()) return Logf(ERROR_IFC_PRJ, L"opening");
		}

		// Rear Wall
		if (LobbyArrangement != LOBBY_OPENPLAN)
		{
			identityMatrix(&matrix);
			matrix._42 = LobbyDepth + FrontWallThickness;
			CIFCWall wallrear(&storey, &matrix, LobbyWidth, pStorey->HeightValue - LobbyCeilingSlabHeight, FrontWallThickness, bBrep, bPresentation);
			wallrear.setWallInfo("Lobby Rear Wall", "Lobby Rear Wall");
			if (!wallrear.build()) return Logf(ERROR_IFC_PRJ, L"wall");
			if (GetShaftLinesCount() > 1)
				for (AVULONG j = GetShaftCount(0); j < NoOfShafts; j++)
				{
					SHAFT *pLift = GetShaft(j);
					identityMatrix(&matrix);
					matrix._41 = pLift->ShaftPos + (pLift->ShaftWidth - pLift->LiftDoorWidth) / 2;
					CIFCOpening door(&wallrear, &matrix, pLift->LiftDoorWidth, pLift->LiftDoorHeight, FrontWallThickness, bBrep, bPresentation);
					_snprintf(buf, 256, "Door Opening for Lift Shaft %d", pLift->ShaftID);
					door.setOpeningInfo(buf, buf);
					if (!door.build()) return Logf(ERROR_IFC_PRJ, L"opening");
				}
		}

		// Side Walls
		if (LobbyArrangement == LOBBY_DEADEND_LEFT)
		{
			// Left Wall
			identityMatrix(&matrix);
			matrix._11 = 0; matrix._12 = 1;
			matrix._21 = 1; matrix._22 = 0;
			matrix._41 = SideWallThickness;
			matrix._42 = FrontWallThickness;
			CIFCWall wallleft(&storey, &matrix, LobbyDepth/*+FrontWallThickness*/, pStorey->HeightValue - LobbyCeilingSlabHeight, SideWallThickness, bBrep, bPresentation);
			wallleft.setWallInfo("Lobby Left Side Wall", "Lobby Left Side Wall");
			if (!wallleft.build()) return Logf(ERROR_IFC_PRJ, L"wall");
		}

		if (LobbyArrangement == LOBBY_DEADEND_RIGHT)
		{
			// Right Wall
			matrix._11 = 0; matrix._12 = 1;
			matrix._21 = 1; matrix._22 = 0;
			matrix._41 = LobbyWidth;
			matrix._42 = FrontWallThickness;
			CIFCWall wallright(&storey, &matrix, LobbyDepth/*+FrontWallThickness*/, pStorey->HeightValue - LobbyCeilingSlabHeight, SideWallThickness, bBrep, bPresentation);
			wallright.setWallInfo("Lobby Right Side Wall", "Lobby Right Side Wall");
			if (!wallright.build()) return Logf(ERROR_IFC_PRJ, L"wall");
		}

		// The Shaft
		for (AVULONG j = 0; j < NoOfShafts; j++)
		{
			SHAFT *pLift = GetShaft(j);
			SHAFT *pPrevLift = (j != 0 && j != GetShaftCount(0)) ? GetShaft(j-1) : NULL;
			AVFLOAT fBeamLen;
			switch (pLift->ShaftLine)
			{
			case 0:
				{
					// Left Beam
					identityMatrix(&matrix);
					matrix._11 = 0; matrix._12 = -1;
					matrix._21 = 1; matrix._22 = 0;
					matrix._41 = pLift->ShaftPos;
					fBeamLen = max(pLift->ShaftDepth + ShaftWallThickness, (pPrevLift ? pPrevLift->ShaftDepth + ShaftWallThickness: 0));
					CIFCWall beam(&storey, &matrix, fBeamLen, pStorey->HeightValue, IntDivBeamWidth, bBrep, bPresentation);
					beam.setWallInfo("Shaft Left Division Beam", "Shaft Left Division Beam");
					if (!beam.build()) return Logf(ERROR_IFC_PRJ, L"wall");

					bool bLastInRow = (j == GetShaftCount(0)-1);

					// Right Beam
					if (bLastInRow)
					{
						matrix._41 = pLift->ShaftPos + pLift->ShaftWidth;
						CIFCWall beam(&storey, &matrix, pLift->ShaftDepth + ShaftWallThickness, pStorey->HeightValue, IntDivBeamWidth, bBrep, bPresentation);
						beam.setWallInfo("Shaft Right Division Beam", "Shaft Right Division Beam");
						if (!beam.build()) return Logf(ERROR_IFC_PRJ, L"wall");
					}

					// Rear Beam
					identityMatrix(&matrix);
					matrix._41 = pLift->ShaftPos;
					matrix._42 = -pLift->ShaftDepth - ShaftWallThickness;
					CIFCWall rear(&storey, &matrix, pLift->ShaftWidth, pStorey->HeightValue, ShaftWallThickness, bBrep, bPresentation);
					rear.setWallInfo("Shaft Rear Beam", "Shaft Rear Beam");
					if (!rear.build()) return Logf(ERROR_IFC_PRJ, L"wall");

					// The Lift Car
					if (i == StoreysBelowGround && hLift)
					{
						identityMatrix(&matrix);

						matrix._11 = -1; matrix._12 = 0;
						matrix._21 = 0; matrix._22 = -1;

						matrix._41 = pLift->ShaftPos - bbLift.x0 + (pLift->ShaftWidth - bbLift.x1 + bbLift.x0) / 2; 
						matrix._42 = 120;

						if (hLift)
						{
							CIFCRevitElem lift(&storey, &matrix);
							lift.setRevitInfo("lift", "Sample Proxy");
							int h = lift.build(hLift);
							if (!h) return Logf(ERROR_IFC_PRJ, L"revit");
						}

						// Car Door
						if (hDoorCar)
						{
							CIFCRevitElem DoorCar(&storey, &matrix);
							DoorCar.setRevitInfo("car doors", "car doors");
							int h = DoorCar.build(hDoorCar);
							if (!h) return Logf(ERROR_IFC_PRJ, L"revit");
						}

						if (hDoorLanding)
						{
							// Landing Door
							CIFCRevitElem DoorLanding(&storey, &matrix);
							DoorLanding.setRevitInfo("landing doors", "landing doors");
							int h = DoorLanding.build(hDoorLanding);
							if (!h) return Logf(ERROR_IFC_PRJ, L"revit");
						}
					}

					break;
				}
			case 1:
				{
					// Left Beam
					identityMatrix(&matrix);
					matrix._11 = 0; matrix._12 = 1;
					matrix._21 = 1; matrix._22 = 0;
					matrix._41 = pLift->ShaftPos;
					matrix._42 = LobbyDepth + 2 * FrontWallThickness;
					fBeamLen = max(pLift->ShaftDepth + ShaftWallThickness, (pPrevLift ? pPrevLift->ShaftDepth + ShaftWallThickness : 0));
					CIFCWall beam(&storey, &matrix, fBeamLen, pStorey->HeightValue, IntDivBeamWidth, bBrep, bPresentation);
					beam.setWallInfo("Shaft Left Division Beam (Rear Side)", "Shaft Left Division Beam (Rear Side)");
					if (!beam.build()) return Logf(ERROR_IFC_PRJ, L"wall");

					bool bLastInRow = (j == GetShaftCount(0));

					// Right Beam
					if (bLastInRow)
					{
						matrix._41 = pLift->ShaftPos + pLift->ShaftWidth + IntDivBeamWidth ;
						CIFCWall beam(&storey, &matrix, pLift->ShaftDepth + ShaftWallThickness, pStorey->HeightValue, IntDivBeamWidth, bBrep, bPresentation);
						beam.setWallInfo("Shaft Right Division Beam (Rear Side)", "Shaft Right Division Beam (Rear Side)");
						if (!beam.build()) return Logf(ERROR_IFC_PRJ, L"wall");
					}

					// Rear Beam
					identityMatrix(&matrix);
					matrix._41 = pLift->ShaftPos;
					matrix._42 = LobbyDepth + 2 * FrontWallThickness + pLift->ShaftDepth;
					CIFCWall rear(&storey, &matrix, pLift->ShaftWidth, pStorey->HeightValue, ShaftWallThickness, bBrep, bPresentation);
					rear.setWallInfo("Shaft Rear Beam (Rear Side)", "Shaft Rear Beam (Rear Side)");
					if (!rear.build()) return Logf(ERROR_IFC_PRJ, L"wall");

					// The Lift Car
					if (i == StoreysBelowGround && hLift)
					{
						identityMatrix(&matrix);

						matrix._11 = 1; matrix._12 = 0;
						matrix._21 = 0; matrix._22 = 1;

						matrix._41 = pLift->ShaftPos - bbLift.x0 + (pLift->ShaftWidth - bbLift.x1 + bbLift.x0) / 2; 
						matrix._42 = LobbyDepth + 2 * FrontWallThickness - 120;

						if (hLift)
						{
							CIFCRevitElem lift(&storey, &matrix);
							lift.setRevitInfo("lift", "Sample Proxy");
							int h = lift.build(hLift);
							if (!h) return Logf(ERROR_IFC_PRJ, L"revit");
						}

						if (hDoorCar)
						{
							// Car Door
							CIFCRevitElem DoorCar(&storey, &matrix);
							DoorCar.setRevitInfo("car doors", "car doors");
							int h = DoorCar.build(hDoorCar);
							if (!h) return Logf(ERROR_IFC_PRJ, L"revit");
						}

						if (hDoorLanding)
						{
							// Landing Door
							CIFCRevitElem DoorLanding(&storey, &matrix);
							DoorLanding.setRevitInfo("landing doors", "landing doors");
							int h = DoorLanding.build(hDoorLanding);
							if (!h) return Logf(ERROR_IFC_PRJ, L"revit");
						}
					}
					break;
				}
			}
		}
	}

	// Pit
	identityMatrix(&matrix);
	matrix._43 = GetStorey(0)->StoreyLevel;
	CIFCStorey storeyPit(&building, &matrix);
	storeyPit.setStoreyInfo("Lift Pit Level", "Lift Pit Level");
	if (!storeyPit.build()) return Logf(ERROR_IFC_PRJ, L"storey (the pit)");

	for (AVULONG j = 0; j < NoOfShafts; j++)
	{
		SHAFT *pLift = GetShaft(j);
		SHAFT *pPrevLift = (j != 0 && j != GetShaftCount(0)) ? GetShaft(j-1) : NULL;
		AVFLOAT fBeamLen;
		switch (pLift->ShaftLine)
		{
		case 0:
			{
				// Left Beam
				identityMatrix(&matrix);
				matrix._11 = 0; matrix._12 = -1;
				matrix._21 = 1; matrix._22 = 0;
				matrix._41 = pLift->ShaftPos; matrix._43 = -pLift->PitDepth; 
				fBeamLen = max(pLift->ShaftDepth + ShaftWallThickness, (pPrevLift ? pPrevLift->ShaftDepth + ShaftWallThickness: 0));
				CIFCWall beam(&storeyPit, &matrix, fBeamLen, pLift->PitDepth, IntDivBeamWidth, bBrep, bPresentation);
				beam.setWallInfo("Lift Pit Division Beam", "Lift Pit Division Beam");
				if (!beam.build()) return Logf(ERROR_IFC_PRJ, L"wall");

				bool bLastInRow = (j == GetShaftCount(0)-1);

				// Right Beam
				if (bLastInRow)
				{
					matrix._41 = pLift->ShaftPos + pLift->ShaftWidth; matrix._43 = -pLift->PitDepth;
					CIFCWall beam(&storeyPit, &matrix, pLift->ShaftDepth + ShaftWallThickness, pLift->PitDepth, IntDivBeamWidth, bBrep, bPresentation);
					beam.setWallInfo("Lift Pit Division Beam", "Lift Pit Division Beam");
					if (!beam.build()) return Logf(ERROR_IFC_PRJ, L"wall");
				}

				// Front Beam
				identityMatrix(&matrix);
				matrix._41 = pLift->ShaftPos;
				matrix._42 = 0;//-pLift->ShaftDepth - ShaftWallThickness;
				matrix._43 = -pLift->PitDepth;
				CIFCWall front(&storeyPit, &matrix, pLift->ShaftWidth + (bLastInRow ? IntDivBeamWidth : 0), pLift->PitDepth, ShaftWallThickness, bBrep, bPresentation);
				front.setWallInfo("Lift Pit Front Beam", "Lift Pit Front Beam");
				if (!front.build()) return Logf(ERROR_IFC_PRJ, L"wall");

				// Rear Beam
				identityMatrix(&matrix);
				matrix._41 = pLift->ShaftPos;
				matrix._42 = -pLift->ShaftDepth - ShaftWallThickness;
				matrix._43 = -pLift->PitDepth;
				CIFCWall rear(&storeyPit, &matrix, pLift->ShaftWidth, pLift->PitDepth, ShaftWallThickness, bBrep, bPresentation);
				rear.setWallInfo("Lift Pit Rear Beam", "Lift Pit Rear Beam");
				if (!rear.build()) return Logf(ERROR_IFC_PRJ, L"wall");

				// Buffer
				if (hBuffer)
				{
					identityMatrix(&matrix);
					matrix._41 = pLift->ShaftPos + (pLift->ShaftWidth) / 2; 
					matrix._42 = 0 - pLift->ShaftDepth / 2 - bbBuffer.y1;
					matrix._43 = -pLift->PitDepth;
					matrix._43 -= bbBuffer.z0;

					CIFCRevitElem Buffer(&storeyPit, &matrix);
					Buffer.setRevitInfo("buffer", "buffer");
					int h = Buffer.build(hBuffer);
					if (!h) return Logf(ERROR_IFC_PRJ, L"revit");
				}

				break;
			}
		case 1:
			{
				// Left Beam
				identityMatrix(&matrix);
				matrix._11 = 0; matrix._12 = 1;
				matrix._21 = 1; matrix._22 = 0;
				matrix._41 = pLift->ShaftPos;
				matrix._42 = LobbyDepth + 2 * FrontWallThickness;
				matrix._43 = -pLift->PitDepth; 
				fBeamLen = max(pLift->ShaftDepth + ShaftWallThickness, (pPrevLift ? pPrevLift->ShaftDepth + ShaftWallThickness : 0));
				CIFCWall beam(&storeyPit, &matrix, fBeamLen, pLift->PitDepth, IntDivBeamWidth, bBrep, bPresentation);
				beam.setWallInfo("Lift Pit Division Beam (Rear Side)", "Lift Pit Division Beam (Rear Side)");
				if (!beam.build()) return Logf(ERROR_IFC_PRJ, L"wall");

				bool bLastInRow = (j == GetShaftCount(0));

				// Right Beam
				if (bLastInRow)
				{
					matrix._41 = pLift->ShaftPos + pLift->ShaftWidth + IntDivBeamWidth ;
					matrix._43 = -pLift->PitDepth;
					CIFCWall beam(&storeyPit, &matrix, pLift->ShaftDepth + ShaftWallThickness, pLift->PitDepth, IntDivBeamWidth, bBrep, bPresentation);
					beam.setWallInfo("Lift Pit Division Beam (Rear Side)", "Lift Pit Division Beam (Rear Side)");
					if (!beam.build()) return Logf(ERROR_IFC_PRJ, L"wall");
				}

				// Front Beam
				identityMatrix(&matrix);
				matrix._41 = pLift->ShaftPos;
				matrix._42 = LobbyDepth + 2 * FrontWallThickness - ShaftWallThickness;
				matrix._43 = -pLift->PitDepth;
				CIFCWall front(&storeyPit, &matrix, pLift->ShaftWidth + IntDivBeamWidth + (bLastInRow ? ShaftWallThickness : 0), pLift->PitDepth, ShaftWallThickness, bBrep, bPresentation);
				front.setWallInfo("Lift Pit Front Beam (Rear Side)", "Lift Pit Front Beam (Rear Side)");
				if (!front.build()) return Logf(ERROR_IFC_PRJ, L"wall");

				// Rear Beam
				identityMatrix(&matrix);
				matrix._41 = pLift->ShaftPos;
				matrix._42 = LobbyDepth + 2 * FrontWallThickness + pLift->ShaftDepth;
				matrix._43 = -pLift->PitDepth;
				CIFCWall rear(&storeyPit, &matrix, pLift->ShaftWidth, pLift->PitDepth, ShaftWallThickness, bBrep, bPresentation);
				rear.setWallInfo("Lift Pit Rear Beam (Rear Side)", "Lift Pit Rear Beam (Rear Side)");
				if (!rear.build()) return Logf(ERROR_IFC_PRJ, L"wall");

				// Buffer
				if (hBuffer)
				{
					identityMatrix(&matrix);
					matrix._41 = pLift->ShaftPos + (pLift->ShaftWidth) / 2; 
					matrix._42 = LobbyDepth + 2 * FrontWallThickness + pLift->ShaftDepth / 2 - bbBuffer.y1;
					matrix._43 = -pLift->PitDepth;
					matrix._43 -= bbBuffer.z0;

					CIFCRevitElem Buffer(&storeyPit, &matrix);
					Buffer.setRevitInfo("buffer", "buffer");
					int h = Buffer.build(hBuffer);
					if (!h) return Logf(ERROR_IFC_PRJ, L"revit");
				}

				break;
			}
		}
	}

	// *** Machine Room
	identityMatrix(&matrix);
	matrix._43 = GetStorey(GetStoreyCount()-1)->StoreyLevel + GetStorey(GetStoreyCount()-1)->HeightValue;;
	CIFCStorey storeyMRoom(&building, &matrix);
	storeyMRoom.setStoreyInfo("Machine Room Level", "Machine Room Level");
	if (!storeyMRoom.build()) return Logf(ERROR_IFC_PRJ, L"storey (the machine room)");

	// find extreme values (for each of lines of lifts separately)
	AVFLOAT MachRoomHeight[2] = { 0, 0 };
	AVFLOAT ShaftDepth[2] = { 0, 0 };
	//AVFLOAT MachRoomSlab[2] = { 0, 0 };
	for (AVULONG j = 0; j < NoOfShafts; j++)
	{
		SHAFT *pLift = GetShaft(j);
		AVULONG nLine = pLift->ShaftLine;
		MachRoomHeight[nLine] = max(MachRoomHeight[nLine], pLift->MachRoomHeight);
		ShaftDepth[nLine] = max(ShaftDepth[nLine], pLift->ShaftDepth + ShaftWallThickness);
		//MachRoomSlab[nLine] = max(MachRoomSlab[nLine], pLift->MachRoomSlab);
	}
	AVFLOAT MaxMachRoomHeight = max(MachRoomHeight[0], MachRoomHeight[1]);
	//AVFLOAT MaxMachRoomSlab = max(MachRoomSlab[0], MachRoomSlab[1]);

	// Machine Room Slab
	identityMatrix(&matrix);
	matrix._42 = -ShaftDepth[0];
	CIFCSlab slabMRoom(&storeyMRoom, &matrix, LobbyWidth, MachRoomSlab, LobbyDepth+2*FrontWallThickness+ShaftDepth[0]+ShaftDepth[1], bBrep, bPresentation);
	slabMRoom.setSlabInfo("Machine Room Slab", "Machine Room Slab");
	if (!slabMRoom.build()) return Logf(ERROR_IFC_PRJ, L"slab");

	// Machine Room Front Wall
	identityMatrix(&matrix);
	matrix._42 = -ShaftDepth[0];
	matrix._43 = MachRoomSlab;
	CIFCWall wallfront(&storeyMRoom, &matrix, LobbyWidth, MaxMachRoomHeight, FrontWallThickness, bBrep, bPresentation);
	wallfront.setWallInfo("Machine Room Front Wall", "Machine Room Front Wall");
	if (!wallfront.build()) return Logf(ERROR_IFC_PRJ, L"wall");

	// Left Wall
	identityMatrix(&matrix);
	matrix._11 = 0; matrix._12 = 1;
	matrix._21 = 1; matrix._22 = 0;
	matrix._41 = FrontWallThickness;
	matrix._42 = FrontWallThickness-ShaftDepth[0];
	matrix._43 = MachRoomSlab;
	CIFCWall wallleft(&storeyMRoom, &matrix, LobbyDepth+FrontWallThickness+ShaftDepth[0]+ShaftDepth[1], MaxMachRoomHeight, FrontWallThickness, bBrep, bPresentation);
	wallleft.setWallInfo("Machine Room Front Left Side Wall", "Machine Room Front Left Side Wall");
	if (!wallleft.build()) return Logf(ERROR_IFC_PRJ, L"wall");

	// Right Wall
	matrix._11 = 0; matrix._12 = 1;
	matrix._21 = 1; matrix._22 = 0;
	matrix._41 = LobbyWidth;
	matrix._42 = FrontWallThickness-ShaftDepth[0];
	matrix._43 = MachRoomSlab;
	CIFCWall wallright(&storeyMRoom, &matrix, LobbyDepth+FrontWallThickness+ShaftDepth[0]+ShaftDepth[1], MaxMachRoomHeight, FrontWallThickness, bBrep, bPresentation);
	wallright.setWallInfo("Machine Room Front Right Side Wall", "Machine Room Front Right Side Wall");
	if (!wallright.build()) return Logf(ERROR_IFC_PRJ, L"wall");
		
	// Machine Room Rear Wall
	identityMatrix(&matrix);
	matrix._42 = LobbyDepth + FrontWallThickness + ShaftDepth[1];
	matrix._43 = MachRoomSlab;
	CIFCWall wallrear(&storeyMRoom, &matrix, LobbyWidth, MaxMachRoomHeight, FrontWallThickness, bBrep, bPresentation);
	wallrear.setWallInfo("Machine Room Rear Wall", "Machine Room Rear Wall");
	if (!wallrear.build()) return Logf(ERROR_IFC_PRJ, L"wall");
		
	// Machines!
	if (hMachine)
		for (AVULONG j = 0; j < NoOfShafts; j++)
		{
			SHAFT *pLift = GetShaft(j);
			switch (pLift->ShaftLine)
			{
			case 0:
				{
					double dScale = .25;

					identityMatrix(&matrix);
					matrix._11 = 0; matrix._12 = -1;
					matrix._21 = 1; matrix._22 = 0;
					matrix._41 = pLift->ShaftPos + (pLift->ShaftWidth - dScale * (bbMachine.y1 - bbMachine.y0)) / 2; 
					matrix._42 = 0 - (pLift->ShaftDepth - dScale * (bbMachine.x1 - bbMachine.x0)) / 2; 
					matrix._43 = MachRoomSlab;

					CIFCRevitElem machine(&storeyMRoom, &matrix);
					machine.setRevitInfo("Machine", "Lift Machine");
					int h = machine.build(hMachine, [dScale] (CIFCModelScanner::ITEM *pItem) 
											{
												if (pItem->type == CIFCModelScanner::ITEM::AGGREG && pItem->nIndex >= 0 && pItem->nType == sdaiREAL && pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE && strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 && sdaiGetMemberCount(pItem->hAggreg) == 3)
													pItem->dvalue *= dScale;
											}); 
					if (!h) return Logf(ERROR_IFC_PRJ, L"revit");

					break;
				}
			case 1:
				{
					double dScale = .25;

					identityMatrix(&matrix);
					matrix._11 = 0; matrix._12 = 1;
					matrix._21 = -1; matrix._22 = 0;
					matrix._41 = pLift->ShaftRPos - (pLift->ShaftWidth - dScale * (bbMachine.y1 - bbMachine.y0)) / 2; 
					matrix._42 = LobbyDepth + 2 * FrontWallThickness + (pLift->ShaftDepth - dScale * (bbMachine.x1 - bbMachine.x0)) / 2; 
					matrix._43 = MachRoomSlab;

					CIFCRevitElem machine(&storeyMRoom, &matrix);
					machine.setRevitInfo("Machine", "Lift Machine");
					int h = machine.build(hMachine, [dScale] (CIFCModelScanner::ITEM *pItem) 
											{
												if (pItem->type == CIFCModelScanner::ITEM::AGGREG && pItem->nIndex >= 0 && pItem->nType == sdaiREAL && pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE && strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 && sdaiGetMemberCount(pItem->hAggreg) == 3)
													pItem->dvalue *= dScale;
											}); 
					if (!h) return Logf(ERROR_IFC_PRJ, L"revit");

					break;
				}
			}
		}
				
	USES_CONVERSION;
	prj.save(OLE2A(pFileName));

	return S_OK;
}



