// Project.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "VisProject.h"
#include "VisBuilding.h"
#include "VisSim.h"

#include "Block.h"

#pragma warning (disable:4996)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBoneVis

CBoneVis::CBoneVis(IKineNode *pNode) : m_pNode(pNode)
{ 
	m_pNode->AddRef(); 
}

CBoneVis::~CBoneVis()
{ 
	if (m_pNode) m_pNode->Release(); 
}

void CBoneVis::PushState()					{ if (m_pNode) m_pNode->PushState(); }
void CBoneVis::PopState()					{ if (m_pNode) m_pNode->PopState(); }
void CBoneVis::Invalidate()					{ if (m_pNode) m_pNode->Invalidate(); }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CElemVis

CElemVis::~CElemVis()
{
	if (m_pObj) m_pObj->Release();
	m_pObj = NULL;
	if (m_pBone) delete m_pBone;
	m_pBone = NULL;
}

std::wstring CElemVis::onCreateName(AVULONG nElemId, std::wstring name,  AVLONG i)
{
	if (nElemId == ELEM_PROJECT || nElemId == ELEM_SITE)
		return L"";

	static OLECHAR _name[257];
	_snwprintf_s(_name, 256, (AVSTRING)name.c_str(), LOWORD(i), HIWORD(i));

	if (GetParent() && GetParent()->GetName().size())
		return GetParent()->GetName() + L" - " + _name;
	else
		return _name;
}

void CElemVis::onCreate(AVULONG nElemId, AVVECTOR &vec)
{
	if (!GetBuilding()) return;
	if (nElemId == ELEM_PROJECT || nElemId == ELEM_SITE)
		return;

	IKineNode *pNode = NULL;
	GetBuilding()->GetScene()->NewObject((AVSTRING)GetName().c_str(), &m_pObj);
	m_pObj->CreateChild((AVSTRING)GetName().c_str(), &pNode);
	m_pBone = new CBoneVis(pNode);
	pNode->Release();

	ITransform *pT = NULL;
	GetNode()->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&vec));
	GetNode()->PutLocalTransform(pT);
	pT->Release();
}

void CElemVis::onMove(AVVECTOR &vec)
{
	ITransform *pT = NULL;
	m_pObj->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&vec));
	m_pObj->TransformLocal(pT);
	pT->Release();
}

CBone *CElemVis::onAddBone(AVULONG nBoneId, AVSTRING name, AVVECTOR &vec)
{
	IKineNode *pNode = NULL;
	GetNode()->CreateChild(name, &pNode);
	CBoneVis *pBone = new CBoneVis(pNode);
	pNode->Release();

	ITransform *pT = NULL;
	pBone->GetNode()->CreateCompatibleTransform(&pT);
	pT->FromTranslationVector((FWVECTOR*)(&vec));
	pBone->GetNode()->PutLocalTransform(pT);
	pT->Release();

	return pBone;
}

void CElemVis::onAddWall(CBone *pBone, AVULONG nWallId, AVSTRING strName, AVLONG nIndex, AVVECTOR vecPos, AVFLOAT l, AVFLOAT h, AVFLOAT d, AVVECTOR vecRot,
					AVULONG nDoorNum, FLOAT *pDoorData, CBone **ppNewBone)
{
	//TRACE(L"Building wall: pos = (%f, %f, %f), l = %f, h = %f, d = %f\n", vecPos.x/0.04f, vecPos.y/0.04f, vecPos.z/0.04f, l/0.04f, h/0.04f, d/0.04f);

	CBlock block;
	block.Open(m_pObj, GetNode(pBone), strName, l, h, d, vecPos, vecRot.z, vecRot.x, vecRot.y);
	block.BuildFrontSection();
	
	for (AVULONG i = 0; i < nDoorNum * 3; i += 3)
	{
		block.BuildWallTo(pDoorData[i]);
		block.BuildDoor(pDoorData[i + 1], pDoorData[i + 2]);
	}
	
	block.BuildWall();
	block.BuildRearSection();

	block.SetMaterial(GetBuilding()->GetMaterial(nWallId, LOWORD(nIndex)));
	if (ppNewBone)
	{
		IKineNode *pNewNode = block.GetBone();
		*ppNewBone = new CBoneVis(pNewNode);
		pNewNode->Release();
	}
	block.Close();
}

IMesh *CElemVis::AddMesh(AVSTRING strName)
{
	IMesh *pMesh = NULL;
	m_pObj->NewMesh(strName, &pMesh);
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
	if (!m_pObj) return;
	m_pObj->PushState();
}

void CElemVis::PopState()
{
	if (!m_pObj) return;
	m_pObj->PopState(); 
	m_pObj->Invalidate(); 
	m_pObj->PushState();
}

void CElemVis::Render(IRenderer *pRenderer)
{
	if (!m_pObj) return;
	m_pObj->Render((IRndrGeneric*)pRenderer);
}

//////////////////////////////////////////////////////////////////////////////////
// CProjectVis Implementation

CBuilding *CProjectVis::CreateBuilding(AVULONG iIndex)
{ 
	return new CBuildingVis(this, iIndex); 
}

CSim *CProjectVis::CreateSim(CBuilding *pBuilding, AVULONG iIndex)
{ 
	m_phases.push_back(PHASE_NONE); 
	return new CSimVis(pBuilding, iIndex); 
}

//////////////////////////////////////////////////////////////////////////////////
// FW Specific

void CProjectVis::SetScene(IScene *pScene, IMaterial *pMaterial, IKineChild *pBiped)	
{ 
	for each (CSimVis *pSim in m_sims)
		pSim->SetScene(pScene, pMaterial, pBiped); 
}

void CProjectVis::SetRenderer(IRenderer *pRenderer)	
{ 
	for each (CSimVis *pSim in m_sims)
		if (pSim->GetBuilding())
			pSim->GetBuilding()->SetRenderer(pRenderer); 
}

void CProjectVis::StoreConfig()
{ 
	for each (CSimVis *pSim in m_sims)
		if (pSim->GetBuilding())
			pSim->GetBuilding()->StoreConfig();
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

void CProjectVis::Load(xmltools::CXmlReader reader, AVLONG nLiftGroup)
{
	CSimVis *pSim = nLiftGroup >= 0 ? GetSim(nLiftGroup) : NULL;
	CBuildingVis *pBuilding = nLiftGroup >= 0 ? GetBuilding(nLiftGroup) : NULL;
	AVULONG iShaft = 0, iStorey = 0;

	while (reader.read())
	{
		if (reader.getName() == L"AVProject")
		{
			if (nLiftGroup >= 0) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			reader >> ME;
			ResolveMe();
			ResolveLiftGroups();
			nLiftGroup = 0;
			m_phases[nLiftGroup] = PHASE_PRJ;
		}
		else
		if (reader.getName() == L"AVSim")
		{
			if (nLiftGroup < 0) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			if (m_phases[nLiftGroup] != PHASE_PRJ && m_phases[nLiftGroup] != PHASE_SIM) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			if (m_phases[nLiftGroup] == PHASE_SIM)
				nLiftGroup++;
			pSim = GetSim(nLiftGroup);
			pBuilding = GetBuilding(nLiftGroup);
			
			m_phases[nLiftGroup] = PHASE_SIM;

			reader >> *pSim;
			pSim->ResolveMe();
		}
		else
		if (reader.getName() == L"AVBuilding")
		{
			if (nLiftGroup < 0) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			if (m_phases[nLiftGroup] != PHASE_SIM) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_BLD;

			reader >> *pBuilding;
			pBuilding->ResolveMe();
		}
		else
		if (reader.getName() == L"AVShaft")
		{
			if (nLiftGroup < 0) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			if (m_phases[nLiftGroup] != PHASE_BLD && m_phases[nLiftGroup] != PHASE_STRUCT) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_STRUCT;
			if (iShaft >= pBuilding->GetShaftCount()) throw _prj_error(_prj_error::E_PRJ_LIFTS);
			
			reader >> *pBuilding->GetShaft(iShaft++);
		}
		else
		if (reader.getName() == L"AVFloor")
		{
			if (nLiftGroup < 0) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			if (m_phases[nLiftGroup] != PHASE_BLD && m_phases[nLiftGroup] != PHASE_STRUCT) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_STRUCT;
			if (iStorey >= pBuilding->GetStoreyCount()) throw _prj_error(_prj_error::E_PRJ_FLOORS);
			
			reader >> *pBuilding->GetStorey(iStorey++);
		}
		else
		if (reader.getName() == L"AVJourney")
		{
			if (nLiftGroup < 0) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			if (pBuilding->GetLiftCount() == 0)
			{
				pBuilding->ResolveMore();
				for (AVULONG i = 0; i < pBuilding->GetLiftCount(); i++)
					pSim->AddLift(pSim->CreateLift(i));
			}

			if (m_phases[nLiftGroup] != PHASE_STRUCT && m_phases[nLiftGroup] != PHASE_SIMDATA) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_SIMDATA;

			JOURNEY journey;
			AVULONG nLiftID = reader[L"LiftID"];
			journey.m_shaftFrom = reader[L"ShaftFrom"];
			journey.m_shaftTo = reader[L"ShaftTo"];
			journey.m_floorFrom = reader[L"FloorFrom"];
			journey.m_floorTo = reader[L"FloorTo"];
			journey.m_timeGo = reader[L"TimeGo"];
			journey.m_timeDest = reader[L"TimeDest"];
			journey.ParseDoorCycles(reader[L"DC"]);
				  
			if (nLiftID >= pBuilding->GetLiftCount() || nLiftID >= LIFT_MAXNUM || journey.m_shaftFrom >= pBuilding->GetShaftCount() || journey.m_shaftTo >= pBuilding->GetShaftCount()) 
				throw _prj_error(_prj_error::E_PRJ_LIFTS);

			pSim->GetLift(nLiftID)->AddJourney(journey);
		}
		else
		if (reader.getName() == L"AVPassenger")
		{
			if (nLiftGroup < 0) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			if (m_phases[nLiftGroup] != PHASE_STRUCT && m_phases[nLiftGroup] != PHASE_SIMDATA) throw _prj_error(_prj_error::E_PRJ_FILE_STRUCT);
			m_phases[nLiftGroup] = PHASE_SIMDATA;

			CPassengerVis *pPassenger = (CPassengerVis*)pSim->CreatePassenger(0);
			reader >> *pPassenger;
			pPassenger->ResolveMe();
			pSim->AddPassenger(pPassenger);
		}
	}

	// Init lifts when known
	if (m_phases[nLiftGroup] >= PHASE_STRUCT && pBuilding->GetLiftCount() == 0)
	{
		pBuilding->ResolveMore();
		for (AVULONG i = 0; i < pBuilding->GetLiftCount(); i++)
			pSim->AddLift(pSim->CreateLift(i));
	}

	// Some tests
	if (GetId() == 0)
		throw _prj_error(_prj_error::E_PRJ_NOT_FOUND);
	if (m_phases[nLiftGroup] == PHASE_STRUCT && iShaft != pBuilding->GetShaftCount())
		throw _prj_error(_prj_error::E_PRJ_LIFTS);
	if (m_phases[nLiftGroup] == PHASE_STRUCT && iStorey != pBuilding->GetStoreyCount())
		throw _prj_error(_prj_error::E_PRJ_FLOORS);

	if (m_phases[nLiftGroup] == PHASE_STRUCT) 
		m_phases[nLiftGroup] = PHASE_SIMDATA;

	if (m_phases[nLiftGroup] == PHASE_SIMDATA && !pBuilding->IsValid()) 
	{
		pBuilding->Create();
		pBuilding->Scale(0.04f);
	}
	if (m_phases[nLiftGroup] >= PHASE_STRUCT && !pBuilding->IsValid())
		throw _prj_error(_prj_error::E_PRJ_NO_BUILDING);
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
	writer.write(L"AVBuilding", *GetBuilding());
	
	for (ULONG i = 0; i < GetBuilding()->GetShaftCount(); i++)
		writer.write(L"AVShaft", *GetBuilding()->GetShaft(i));

	for (ULONG i = 0; i < GetBuilding()->GetStoreyCount(); i++)
		writer.write(L"AVFloor", *GetBuilding()->GetStorey(i));

	for (ULONG i = 0; i < GetSim()->GetLiftCount(); i++)
		for (ULONG j = 0; j < GetSim()->GetLift(i)->GetJourneyCount(); j++)
		{
			JOURNEY *pJ = GetSim()->GetLift(i)->GetJourney(j);
			writer[L"LiftID"] = GetSim()->GetLift(i)->GetId();
			writer[L"ShaftFrom"] = pJ->m_shaftFrom;
			writer[L"ShaftTo"] = pJ->m_shaftTo;
			writer[L"FloorFrom"] = pJ->m_floorFrom;
			writer[L"FloorTo"] = pJ->m_floorTo;
			writer[L"TimeGo"] = pJ->m_timeGo;
			writer[L"TimeDest"] = pJ->m_timeDest;
			writer[L"DC"] = pJ->StringifyDoorCycles();
			writer.write(L"AVJourney");
		}

	for (ULONG i = 0; i < GetSim()->GetPassengerCount(); i++)
		writer.write(L"AVPassenger", *GetSim()->GetPassenger(i));
}
