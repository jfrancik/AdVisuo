// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "VisProject.h"
#include "VisLftGroup.h"
#include "VisSim.h"

#include <fwrender.h>	// to start the renderer

#include "Block.h"

#pragma warning (disable:4996)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElemVis

CElemVis::CElemVis(CProject *pProject, CLftGroup *pLftGroup, CElem *pParent, AVULONG nElemId, AVSTRING name, AVLONG i, AVVECTOR vec) 
	: CElem(pProject, (CLftGroup*)pLftGroup, pParent, nElemId, name, i, vec)
{ 
	m_pBone = NULL;

	// create name
	if (GetParent() && GetParent()->GetName().size())
		SetName(GetParent()->GetName() + L" - " + GetName());

	if (!GetLftGroup()) 
		return;

	switch (nElemId)
	{
	case ELEM_PROJECT:
	case ELEM_SITE:
		break;

	case ELEM_BONE:
	case ELEM_DECK:
		((CElemVis*)pParent)->GetBone()->CreateChild((AVSTRING)GetName().c_str(), &m_pBone);
		Move(vec);
		break;

	default:
		GetLftGroup()->GetScene()->NewObject((AVSTRING)GetName().c_str(), (ISceneObject**)&m_pBone);
		Move(vec);
		break;
	}
}

CElemVis::~CElemVis()
{
	if (m_pBone) m_pBone->Release(); 
}

ISceneObject *CElemVis::GetObject()
{
	ISceneObject *pObj = NULL;
	if (m_pBone) m_pBone->QueryInterface(&pObj);
	if (pObj)
	{
		pObj->Release();	// weak pointer!
		return pObj;
	}

	if (GetParent())
		return GetParent()->GetObject();
	else
		return NULL;
}

void CElemVis::BuildWall(AVULONG nWallId, AVSTRING strName, AVLONG nIndex, BOX box, AVVECTOR vecRot, AVULONG nDoorNum, FLOAT *pDoorData)
{
	OLECHAR _name[257];
	_snwprintf_s(_name, 256, strName, nIndex);

	IKineNode *pNewBone = NULL;
	GetBone()->CreateChild(_name, &pNewBone);

	CBlock block;
	block.Open(GetObject(), pNewBone, box.Width(), box.Height(), box.Depth(), box.LeftFrontLower(), vecRot.z, vecRot.x, vecRot.y);
	block.BuildFrontSection();
	
	for (AVULONG i = 0; i < nDoorNum * 3; i += 3)
	{
		block.BuildWallTo(pDoorData[i]);
		block.BuildDoor(pDoorData[i + 1], pDoorData[i + 2]);
	}
	
	block.BuildWall();
	block.BuildRearSection();

	block.SetMaterial(GetLftGroup()->GetMaterial(nWallId, LOWORD(nIndex)));
	
	pNewBone->Release();

	block.Close();
}


	BOX __helper(AVVECTOR base, AVFLOAT w, AVFLOAT d, AVFLOAT h)
	{
		return BOX(base + Vector(-w/2, -d/2, 0), w, d, h);
	}

void CElemVis::BuildModel(AVULONG nModelId, AVSTRING strName, AVLONG nIndex, BOX box, AVFLOAT fRot, AVULONG nParam, AVFLOAT fParam1, AVFLOAT fParam2)
{
	return;
//	BuildWall(WALL_BEAM, strName, nIndex, box, Vector(0, 0, fRot));
	
	if (!GetBone()) return;
	AVFLOAT fScale = GetLftGroup()->GetScale();
	float f;
	AVVECTOR centre = box.CentreLower();

	switch (nModelId)
	{
	case MODEL_MACHINE:
		box.SetDepth(-box.Depth());
		box.Grow(0.9f, 0.9f, 0.9f);
		//BuildWall(WALL_BEAM, strName, nIndex, box);
		{
		OLECHAR _name[257];
		_snwprintf_s(_name, 256, L"Machine %d", nIndex);

		IKineNode *pNewBone = NULL;
		GetBone()->CreateChild(_name, &pNewBone);




		LPOLESTR pLabel;
		pNewBone->GetLabel(&pLabel);
		Load((LPOLESTR)(LPCOLESTR)(_stdPathModels + "bunny.obj"), pLabel, 700, 100);
		
		ITransform *pT = NULL;
		pNewBone->CreateCompatibleTransform(&pT);
		pT->FromRotationX(M_PI/2);
		AVVECTOR vec = box.CentreLower();
		pT->MulTranslationVector((FWVECTOR*)(&vec));
		pNewBone->PutLocalTransform(pT);
		pT->Release();
		pNewBone->Release();

	IMaterial *pMaterial = NULL;
	ITexture *pTexture = NULL;
	GetLftGroup()->GetRenderer()->CreateTexture(&pTexture);
	pTexture->LoadFromFile((LPOLESTR)(LPCOLESTR)(_stdPathModels + L"dafoldil.jpg"));
	pTexture->SetUVTile(1, 1);
	GetLftGroup()->GetRenderer()->FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	pMaterial->SetTexture(0, pTexture);
	IKineChild *pChild = NULL;
	m_pBone->GetChild(L"main", &pChild);
	IMesh *pMesh = NULL;
	pChild->QueryInterface(&pMesh);
	pMesh->SetMaterial(pMaterial);
	pMesh->Release();
	pChild->Release();
	pMaterial->Release();
	pTexture->Release();
		}


		break;
	case MODEL_OVERSPEED:
		f = (GetLftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		box.SetDepth(-box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		box.Move(0, f*60, 0);
		centre = box.CentreLower();
		BuildWall(WALL_BEAM, strName, nIndex, __helper(centre, 7, f*30, 30));
		break;
	//case MODEL_CONTROL:
	//	i = GetLftGroup()->GetShaft(nIndex)->GetShaftLine();	// which shaft line we are
	//	centre.x += p->Width() * (nIndex - GetLftGroup()->GetShaftBegin(i) - (AVFLOAT)(GetLftGroup()->GetShaftCount(i) - 1) / 2.0f);
	//	if (i == 0)
	//		centre.y -= p->Depth()/2;
	//	else
	//		centre.y += p->Depth()/2;
	//	p->build(this, nModelId, strName, nIndex, centre, fRot);
	//	break;
	//case MODEL_ISOLATOR:
	//	p->build(this, nModelId, strName, nIndex, centre, fRot);
	//	break;
	case MODEL_CWT:
		f = (GetLftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		box.SetDepth(-box.Depth());
		box.Move(0, -f*2, 0);
		BuildWall(WALL_BEAM, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_RAIL:
		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_BUFFER_CAR:
//		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_BUFFER_CWT:
//		BuildWall(WALL_BEAM, strName, nIndex, box);
		break;
	case MODEL_PULLEY:
		f = (GetLftGroup()->GetShaft(nIndex)->GetShaftLine() == 0) ? -1 : 1;
		box.SetDepth(-box.Depth() / 2);		// adjustment to avoid collision with the guide rail
		box.Move(0, f*60, 0);
		centre = box.CentreLower();
		BuildWall(WALL_BEAM, strName, nIndex, __helper(centre, 6, f*20, 30));
		break;
	//case MODEL_LADDER:
	//	box.SetDepth(box.Depth() / 2);		// adjustment to avoid collision with the guide rail
	//	centre = box.CentreLower();
	//	p->build(this, nModelId, strName, nIndex, centre, 0.8, 0.8, 1, fRot);
	//	break;
	case MODEL_JAMB:
	case MODEL_JAMB_CAR:
		//box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_HEADING:
	case MODEL_HEADING_CAR:
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	case MODEL_SILL:
	case MODEL_SILL_CAR:
		box.SetDepth(-box.Depth());	// unclear why needed
		BuildWall(WALL_OPENING, strName, nIndex, box, Vector(0, 0, fRot));
		break;
	}
}

void CElemVis::Move(AVVECTOR vec)
{
	ITransform *pT = NULL;
	GetBone()->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&vec));
	GetBone()->TransformLocal(pT);
	pT->Release();
}

IMesh *CElemVis::AddMesh(AVSTRING strName)
{
	IMesh *pMesh = NULL;
	GetObject()->NewMesh(strName, &pMesh);
	pMesh->Open(NULL, NULL);
	pMesh->SupportNormal(0);
	return pMesh;
}

void CElemVis::Load(AVSTRING strFilename, AVSTRING strBone, AVFLOAT fScale, AVFLOAT fTexScale)
{
	IMesh *pVMesh = NULL;
	IMesh *pFMesh = NULL;
	wstring name;
	
	wifstream verfile(strFilename);
	wstring line;
	AVULONG nVertexBase = 1;
	AVULONG nVertex = 0;
	AVULONG nFace = 0;
	while (getline(verfile, line))
	{
		auto i = line.find(L" ", 0);
		if (i == wstring::npos) continue;

		if (line.substr(0, i) == L"g")
		{
			line.replace(0, i, L"");
			wistringstream iss(line);
			if (!(iss >> name))
				break;
		}
		else
		if (line.substr(0, i) == L"v")
		{
			line.replace(0, i, L"");
			wistringstream iss(line);
			FWFLOAT a, b, c;
			if (!(iss >> a >> b >> c))
				break;

			if (pFMesh)
			{
				pFMesh->SupportBlendWeight(0.01f, 0);
				for (AVULONG i = 0; i < nVertex; i++)
					pFMesh->AddBlendWeight(i, 1.0f, strBone);
				pFMesh->Close();
				pFMesh = NULL;
				nVertexBase += nVertex;
				nVertex = 0;
			}
			if (pVMesh == NULL)
				pVMesh = AddMesh(L"temporary");

			pVMesh->SetVertexXYZ(nVertex, a * fScale, b * fScale, c * fScale);
			double l = sqrt(a * a + b * b + c * c);
			pVMesh->AddNormal(&nVertex, a/l, b/l, c/l);
			pVMesh->SetVertexTextureUV(nVertex, 0, sqrt(a*a+c*c) * fTexScale, b * fTexScale);
			nVertex++;
		}
		else
		if (line.substr(0, i) == L"f")
		{
			line[0] = ' ';
			wistringstream iss(line);
			FWFLOAT a, b, c;
			if (!(iss >> a >> b >> c))
				break;

			if (pFMesh == NULL)
			{
				if (pVMesh == NULL) continue;
				pFMesh = pVMesh;
				pVMesh = NULL;
				IKineChild *pChild = NULL;
				pFMesh->QueryInterface(&pChild);
				pChild->PutLabel((AVSTRING)name.c_str());
				pChild->Release();
			}

			a -= nVertexBase;
			b -= nVertexBase;
			c -= nVertexBase;
			pFMesh->SetFace(nFace, a, b, c);
			nFace++;
		}
	}
	
	if (pFMesh)
	{
		pFMesh->SupportBlendWeight(0.01f, 0);
		for (AVULONG i = 0; i < nVertex; i++)
			pFMesh->AddBlendWeight(i, 1.0f, strBone);
		pFMesh->Close();
	}
}

void CElemVis::PushState()
{
	if (!GetObject()) return;
	GetObject()->PushState();
}

void CElemVis::PopState()
{
	if (!GetObject()) return;
	GetObject()->PopState(); 
	GetObject()->Invalidate(); 
	GetObject()->PushState();
}

void CElemVis::Invalidate()
{
	if (!GetObject()) return;
	GetObject()->Invalidate(); 
}

void CElemVis::Render(IRenderer *pRenderer)
{
	if (!GetObject()) return;
	GetObject()->Render((IRndrGeneric*)pRenderer);
}

//////////////////////////////////////////////////////////////////////////////////
// CProjectVis Implementation

CLftGroup *CProjectVis::CreateLftGroup(AVULONG iIndex)
{ 
	return new CLftGroupVis(this, iIndex); 
}

//////////////////////////////////////////////////////////////////////////////////
// FW Specific

void CProjectVis::SetScene(IScene *pScene, IMaterial *pMaterial, IKineChild *pBiped)	
{ 
	for each (CLftGroupVis *pGroup in m_groups)
		pGroup->GetSim()->SetScene(pScene, pMaterial, pBiped); 
}

void CProjectVis::SetRenderer(IRenderer *pRenderer)	
{ 
	for each (CLftGroupVis *pGroup in m_groups)
		pGroup->SetRenderer(pRenderer); 
}

void CProjectVis::StoreConfig()
{ 
	for each (CLftGroupVis *pGroup in m_groups)
		pGroup->StoreConfig();
}

//////////////////////////////////////////////////////////////////////////////////
// Error Messages

std::wstring _prj_error::ErrorMessage()
{
	switch (_error)
	{
		case E_PRJ_NOT_FOUND:	return L"project not found";
		case E_PRJ_NO_BUILDING:	return L"corrupt or missing building structure";
		case E_PRJ_PASSENGERS:	return L"no hall calls found";
		case E_PRJ_LIFTS:		return L"inconsistent building structure: too many or too few lifts";
		case E_PRJ_FLOORS:		return L"inconsistent building structure: too many or too few floors";
		case E_PRJ_LIFT_DECKS:	return L"inconsistent building structure: wrong number of lift decks";
		case E_PRJ_FILE_STRUCT:	return L"data appear in wrong sequence within the simulation file";
		case E_PRJ_INTERNAL:	return L"internal error";
		default:				return L"unidentified error";
	}
}

//////////////////////////////////////////////////////////////////////////////////
// Load from XML

void CProjectVis::Load(xmltools::CXmlReader reader)
{
	AVULONG iShaft = 0, iStorey = 0;

	while (reader.read())
	{
		if (reader.getName() == L"AVProject")
		{
			reader >> ME;
			ResolveMe();
		}
		else
		if (reader.getName() == L"AVLiftGroup")
		{
			CLftGroupVis *pGroup = AddLftGroup();
			reader >> *pGroup ;
			pGroup ->ResolveMe();
		}
		else
		if (reader.getName() == L"AVFloor")
		{
			AVULONG nLftGroupId = reader[L"LiftGroupId"];
			CLftGroupVis *pGroup = FindLftGroup(nLftGroupId);
			if (!pGroup || pGroup->GetSim()) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			CLftGroup::STOREY *pStorey = pGroup->AddStorey();
			reader >> *pStorey;
		}
		else
		if (reader.getName() == L"AVShaft")
		{
			AVULONG nLftGroupId = reader[L"LiftGroupId"];
			CLftGroupVis *pGroup = FindLftGroup(nLftGroupId);
			if (!pGroup || pGroup->GetSim()) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			CLftGroup::SHAFT *pShaft = pGroup->AddShaft();
			reader >> *pShaft;
		}
		else
		if (reader.getName() == L"AVSim")
		{
			AVULONG nLftGroupId = reader[L"LiftGroupId"];
			CLftGroupVis *pGroup = FindLftGroup(nLftGroupId);
			if (!pGroup || pGroup->GetSim()) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);

			pGroup->AddExtras();
			CSimVis *pSim = pGroup->AddSim();
			pGroup->ResolveMe();
			pGroup->Create();
			pGroup->Scale(0.04f);

			reader >> *pSim;
			pSim->ResolveMe();

			for (AVULONG i = 0; i < pGroup->GetLiftCount(); i++)
				pSim->AddLift(pSim->CreateLift(i));
		}
		else
		if (reader.getName() == L"AVJourney")
		{
			AVULONG nSimId = reader[L"SimID"];
			CSimVis *pSim = FindSim(nSimId);
			if (!pSim || !pSim->GetLftGroup()) 
				throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			CLftGroupVis *pGroup = pSim->GetLftGroup();

			JOURNEY journey;
			AVULONG nLiftID = reader[L"LiftID"];
			journey.m_shaftFrom = reader[L"ShaftFrom"];
			journey.m_shaftTo = reader[L"ShaftTo"];
			journey.m_floorFrom = reader[L"FloorFrom"];
			journey.m_floorTo = reader[L"FloorTo"];
			journey.m_timeGo = reader[L"TimeGo"];
			journey.m_timeDest = reader[L"TimeDest"];
			journey.ParseDoorCycles(reader[L"DC"]);
				  
			if (nLiftID >= pGroup->GetLiftCount() || nLiftID >= LIFT_MAXNUM || journey.m_shaftFrom >= pGroup->GetShaftCount() || journey.m_shaftTo >= pGroup->GetShaftCount()) 
				throw _prj_error(_prj_error::E_PRJ_LIFTS);
			if (nLiftID >= pSim->GetLiftCount()) 
				throw _prj_error(_prj_error::E_PRJ_LIFTS);

			pSim->GetLift(nLiftID)->AddJourney(journey);
		}
		else
		if (reader.getName() == L"AVPassenger")
		{
			AVULONG nSimId = reader[L"SimID"];
			CSimVis *pSim = FindSim(nSimId);
			CLftGroupVis *pGroup = pSim->GetLftGroup();

			CPassengerVis *pPassenger = (CPassengerVis*)pSim->CreatePassenger(0);
			reader >> *pPassenger;
			pPassenger->ResolveMe();
			pSim->AddPassenger(pPassenger);
		}
	}
}

void CProjectVis::LoadIndex(xmltools::CXmlReader reader, vector<CProjectVis*> &prjs)
{
	while (reader.read())
		if (reader.getName() == L"AVProject")
		{
			CProjectVis *pPrj = new CProjectVis();
			reader >> *pPrj;
			pPrj->ResolveMe();
			prjs.push_back(pPrj);
		}
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Store as XML

void CProjectVis::Store(xmltools::CXmlWriter writer)
{
	writer.write(L"AVProject", *this);
	for (AVULONG i = 0; i < GetLiftGroupsCount(); i++)
	{
		writer.write(L"AVLiftGroup", *GetLftGroup(i));
	
		for (ULONG i = 0; i < GetLftGroup(i)->GetShaftCount(); i++)
			writer.write(L"AVShaft", *GetLftGroup(i)->GetShaft(i));

		for (ULONG i = 0; i < GetLftGroup(i)->GetStoreyCount(); i++)
			writer.write(L"AVFloor", *GetLftGroup(i)->GetStorey(i));

		for (ULONG i = 0; i < GetLftGroup(i)->GetSim()->GetLiftCount(); i++)
			for (ULONG j = 0; j < GetLftGroup(i)->GetSim()->GetLift(i)->GetJourneyCount(); j++)
			{
				JOURNEY *pJ = GetLftGroup(i)->GetSim()->GetLift(i)->GetJourney(j);
				writer[L"LiftID"] = GetLftGroup(i)->GetSim()->GetLift(i)->GetId();
				writer[L"ShaftFrom"] = pJ->m_shaftFrom;
				writer[L"ShaftTo"] = pJ->m_shaftTo;
				writer[L"FloorFrom"] = pJ->m_floorFrom;
				writer[L"FloorTo"] = pJ->m_floorTo;
				writer[L"TimeGo"] = pJ->m_timeGo;
				writer[L"TimeDest"] = pJ->m_timeDest;
				writer[L"DC"] = pJ->StringifyDoorCycles();
				writer.write(L"AVJourney");
			}

		for (ULONG i = 0; i < GetLftGroup(i)->GetSim()->GetPassengerCount(); i++)
			writer.write(L"AVPassenger", *GetLftGroup(i)->GetSim()->GetPassenger(i));
	}
}
