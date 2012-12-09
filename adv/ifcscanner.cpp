// IfcScanner.cpp - a part of the AdVisuo Server Module

#include "StdAfx.h"
#include "IFCScanner.h"

void CIFCModelScanner::GetTypeValue(ITEM &item, std::function<void (int nType, VALUE value)> f)
{
	// firstly, check some special cases - based on IFC2X3_TC1.exp
	if (item.pParent && item.pParent->type == CIFCModelScanner::ITEM::INSTANCE && item.pParent->pstrAttrName)
	{
		if (strcmp(item.pParent->pstrAttrName, "Coordinates") == 0 || strcmp(item.pParent->pstrAttrName, "DirectionRatios") == 0)
		{
			item.nType = sdaiREAL;
			f(item.nType, &item.dvalue);
			return;
		}
		if (strcmp(item.pParent->pstrAttrName, "Trim1") == 0 || strcmp(item.pParent->pstrAttrName, "Trim2") == 0)
		{
			f(sdaiSTRING, &item.value);
			if (item.value)
			{
				item.nType = sdaiREAL;
				item.dvalue = (float)atof((const char*)item.value);
				item.nType = -1;
				return;
			}
		}
	}

	if (item.type == ITEM::INSTANCE && item.pstrAttrName)
	{
		if (strcmp(item.pstrAttrName, "MasterRepresentation") == 0 || strcmp(item.pstrAttrName, "TargetView") == 0
			|| strcmp(item.pstrAttrName, "ProfileType") == 0 || strcmp(item.pstrAttrName, "Transition") == 0)
		{
			item.nType = sdaiENUM; 
			f(sdaiSTRING, &item.value);

			char *p = (char*)item.value;
			int n = strlen(p);
			if (n >= 2 && p[0] == '.' && p[n-1] == '.')
			{
				p = _strdup(p + 1);
				p[n-2] = '\0';
				item.value = (VALUE)p;
			}
			return;
		}
		if (strcmp(item.pstrAttrName, "SenseAgreement") == 0)
		{
			item.nType = sdaiBOOLEAN; 
			f(sdaiSTRING, &item.value);
			if (strcmp((char*)item.value, ".T.") == 0) item.value = (VALUE)1; else item.value = (VALUE)0;
			return;
		}
	}

	item.value = NULL;
	item.dvalue = 0;
	item.nType = sdaiINSTANCE;		f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiAGGR;			f(item.nType, &item.value); if (item.value) return;
	
	item.nType = sdaiINTEGER;		f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiREAL;			f(item.nType, &item.dvalue); if (item.dvalue)	return;
	item.nType = sdaiSTRING;		f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiBOOLEAN;		f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiENUM;			f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiLOGICAL;		f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiUNICODE;		f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiEXPRESSSTRING;	f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiADB;			f(item.nType, &item.value); if (item.value) return;
	item.nType = sdaiBINARY;		f(item.nType, &item.value); if (item.value) return;

	item.nType = -1;
}

CIFCModelScanner::HAGGREG CIFCModelScanner::Scan(HAGGREG hAggreg, ITEM *pParent)
{
	ITEM item = { ITEM::AGGREG, pParent, pParent ? pParent->nLevel+1 : 0, hAggreg, hAggreg, 0 };
	item.nIndex = -1;
	if (m_callbackFn) m_callbackFn(&item);
	if (item.hNewAggreg)
	{
		int n = sdaiGetMemberCount(hAggreg);
		for (int i = 0; i < n; ++i) 
		{
			item.nIndex = i;

			GetTypeValue(item, [hAggreg, i](int nType, VALUE value) { engiGetAggrElement(hAggreg, i, nType, value); });

			if (item.nType == sdaiAGGR)
				item.value = (VALUE)Scan((int*)(item.value), &item);
			else if (item.nType == sdaiINSTANCE)
				item.value = (VALUE)Scan((int)(item.value), &item);

			if (m_callbackFn) m_callbackFn(&item);
		}
	}
	return item.hNewAggreg;
}

CIFCModelScanner::HINSTANCE CIFCModelScanner::Scan(HINSTANCE hInstance, ITEM *pParent)
{
	ITEM item = { ITEM::INSTANCE, pParent, pParent ? pParent->nLevel+1 : 0 };
	item.hInstance = item.hNewInstance = hInstance;
	item.pstrAttrName = NULL;
	if (m_callbackFn) m_callbackFn(&item);
	if (item.hNewInstance)
	{
		int iAttr = 0;
		xxxxGetAttrNameByIndex(hInstance, iAttr++, &item.pstrAttrName);
		while (item.pstrAttrName)
		{
			GetTypeValue(item, [hInstance, item](int nType, VALUE value) { sdaiGetAttrBN(hInstance, item.pstrAttrName, nType, value); });
		
			if (item.nType == sdaiINSTANCE)
				item.value = (VALUE)Scan((int)(item.value), &item);
			else if (item.nType == sdaiAGGR)
				item.value = (VALUE)Scan((int*)(item.value), &item);

			if (m_callbackFn) m_callbackFn(&item);

			xxxxGetAttrNameByIndex(hInstance, iAttr++, &item.pstrAttrName);
		}
	}
	return item.hNewInstance;
}

void CIFCModelScanner::Null()
{
	SetCallback([] (ITEM *pItem) {});
}

void CIFCModelScanner::Dump()
{
	SetCallback([] (ITEM *pItem)
				{
					switch (pItem->type)
					{
					case CIFCModelScanner::ITEM::AGGREG:
						if (pItem->nIndex < 0)
						{
							// aggregate
							if (pItem->pParent == NULL)
								printf ("Root = Aggregate of %d item(s):\n", sdaiGetMemberCount(pItem->hAggreg));
							else
							{
								for (int i = 0; i < pItem->pParent->nLevel; i++) printf("  ");
								if (pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE)
									printf("- %s = Aggregate of %d item(s):\n", pItem->pParent->pstrAttrName, sdaiGetMemberCount(pItem->hAggreg));
								else
									printf("%d. Aggregate of %d item(s):\n", pItem->pParent->nIndex, sdaiGetMemberCount(pItem->hAggreg));
							}
						}
						else
						{
							// aggregate element
							if (pItem->nType == sdaiAGGR || pItem->nType == sdaiINSTANCE)
								return;

							if (pItem->nType < 0 && pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE)
								if (strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 || strcmp(pItem->pParent->pstrAttrName, "DirectionRatios") == 0)
									pItem->nType = sdaiREAL;
					
							for (int i = 0; i < pItem->nLevel; i++) printf("  ");
							switch (pItem->nType)
							{
							case sdaiADB: 				printf("%d. (?) (ADB)\n", pItem->nIndex); break;
							case sdaiBINARY: 			printf("%d. (?) (binary)\n", pItem->nIndex); break;
							case sdaiBOOLEAN: 			printf("%d. %d (bool)\n", pItem->nIndex, pItem->value); break;
							case sdaiENUM: 				printf("%d. %d (enum)\n", pItem->nIndex, pItem->value); break;
							case sdaiINTEGER: 			printf("%d. %d (integer)\n", pItem->nIndex, pItem->value); break;
							case sdaiLOGICAL: 			printf("%d. %d (logical)\n", pItem->nIndex, pItem->value); break;
							case sdaiREAL: 				printf("%d. %lf (real)\n", pItem->nIndex, pItem->dvalue); break;
							case sdaiSTRING: 			printf("%d. %s (string)\n", pItem->nIndex, pItem->value); break;
							case sdaiUNICODE: 			printf("%d. %ls (unicode)\n", pItem->nIndex, pItem->value); break;
							case sdaiEXPRESSSTRING: 	printf("%d. %s (express)\n", pItem->nIndex, pItem->value); break;
							default:					printf("%d. (?) (unknown)\n", pItem->nIndex); break;
							}
						}
						break;
					case CIFCModelScanner::ITEM::INSTANCE:
						if (pItem->pstrAttrName == NULL)
						{
							// instance
							if (pItem->pParent == NULL)
								printf ("Root = Instance of %s\n", engiGetInstanceClassInfo(pItem->hInstance));
							else
							{
								for (int i = 0; i < pItem->pParent->nLevel; i++) printf("  ");
								if (pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE)
									printf("- %s = Instance of %s\n", pItem->pParent->pstrAttrName, engiGetInstanceClassInfo(pItem->hInstance));
								else
									printf("%d. Instance of %s\n", pItem->pParent->nIndex, engiGetInstanceClassInfo(pItem->hInstance));
							}
						}
						else
						{
							// instance attribute
							if (pItem->nType == sdaiAGGR || pItem->nType == sdaiINSTANCE)
								return;

							for (int i = 0; i < pItem->nLevel; i++) printf("  ");
							switch (pItem->nType)
							{
								case sdaiADB:			printf("- %s = %s (ADB)\n", pItem->pstrAttrName, "?ADB"); break;
								case sdaiBINARY:		printf("- %s = %s (binary)\n", pItem->pstrAttrName, "?BIN"); break;
								case sdaiBOOLEAN:		printf("- %s = %d (bool)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiENUM:			printf("- %s = %d (enum)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiINTEGER:		printf("- %s = %d (integer)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiLOGICAL:		printf("- %s = %d (logical)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiREAL:			printf("- %s = %lf (real)\n", pItem->pstrAttrName, pItem->dvalue); break;
								case sdaiSTRING:		printf("- %s = %s (string)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiUNICODE:		printf("- %s = %ls (unicode)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiEXPRESSSTRING:	printf("- %s = %s (express)\n", pItem->pstrAttrName, pItem->value); break;
								default:				printf("- %s = %s (unknown)\n", pItem->pstrAttrName, "(NULL)"); break;
							}
						}
						break;
					}
				});
}

void CIFCModelScanner::DumpAsPoints()
{
	SetCallback([] (ITEM *pItem)
				{
					switch (pItem->type)
					{
					case CIFCModelScanner::ITEM::AGGREG:
						if (pItem->nIndex >= 0)
						{
							// aggregate element
							if (pItem->nType == sdaiAGGR || pItem->nType == sdaiINSTANCE)
								return;

							if (pItem->nType < 0 && pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE)
								if (strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 || strcmp(pItem->pParent->pstrAttrName, "DirectionRatios") == 0)
									pItem->nType = sdaiREAL;
					
							for (int i = 0; i < pItem->nLevel; i++) printf("  ");
							if (pItem->nType== sdaiREAL) printf("%lf\n", pItem->dvalue); 
						}
						break;
					case CIFCModelScanner::ITEM::INSTANCE:
						if (pItem->pstrAttrName == NULL)
						{
							// instance
							if (pItem->pParent == NULL)
								printf ("Root = Instance of %s\n", engiGetInstanceClassInfo(pItem->hInstance));
							else
							{
								for (int i = 0; i < pItem->pParent->nLevel; i++) printf("  ");
								if (pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE)
									printf("- %s = Instance of %s\n", pItem->pParent->pstrAttrName, engiGetInstanceClassInfo(pItem->hInstance));
								else
									printf("%d. Instance of %s\n", pItem->pParent->nIndex, engiGetInstanceClassInfo(pItem->hInstance));
							}
						}
						else
						{
							// instance attribute
							if (pItem->nType == sdaiAGGR || pItem->nType == sdaiINSTANCE)
								return;

							for (int i = 0; i < pItem->nLevel; i++) printf("  ");
							switch (pItem->nType)
							{
								case sdaiADB:			printf("- %s = %s (ADB)\n", pItem->pstrAttrName, "?ADB"); break;
								case sdaiBINARY:		printf("- %s = %s (binary)\n", pItem->pstrAttrName, "?BIN"); break;
								case sdaiBOOLEAN:		printf("- %s = %d (bool)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiENUM:			printf("- %s = %d (enum)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiINTEGER:		printf("- %s = %d (integer)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiLOGICAL:		printf("- %s = %d (logical)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiREAL:			printf("- %s = %lf (real)\n", pItem->pstrAttrName, pItem->dvalue); break;
								case sdaiSTRING:		printf("- %s = %s (string)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiUNICODE:		printf("- %s = %ls (unicode)\n", pItem->pstrAttrName, pItem->value); break;
								case sdaiEXPRESSSTRING:	printf("- %s = %s (express)\n", pItem->pstrAttrName, pItem->value); break;
								default:				printf("- %s = %s (unknown)\n", pItem->pstrAttrName, "(NULL)"); break;
							}
						}
						break;
					}
				});
}

void CIFCModelScanner::DumpAsCpp()
{
	SetCallback([] (ITEM *pItem)
				{
					switch (pItem->type)
					{
					case CIFCModelScanner::ITEM::AGGREG:
						if (pItem->nIndex < 0)
						{
							// aggregate
							printf("hAggreg%d = sdaiCreateAggrBN(hInstance%d, \"%s\");\n", pItem->nLevel, pItem->nLevel-1, pItem->pParent->pstrAttrName);
						}
						else
						{
							// aggregate element
							if (pItem->nType == sdaiREAL)
								printf("dVal = %lf; sdaiAppend((int)hAggreg%d, %d, (void*)(&dVal));\n", pItem->dvalue, pItem->nLevel, pItem->nType);
							else if (pItem->nType == sdaiINSTANCE)
								printf("sdaiAppend((int)hAggreg%d, %d, (void*)hInstance%d);\n", pItem->nLevel, pItem->nType, pItem->nLevel+1);
							else
								printf("sdaiAppend((int)hAggreg%d, %d, pItem->value);\n", pItem->nLevel, pItem->nType, pItem->value);
						}
						break;
					case CIFCModelScanner::ITEM::INSTANCE:
						if (pItem->pstrAttrName == NULL)
						{
							// instance
							printf("hInstance%d = sdaiCreateInstanceBN(nTargetModel, \"%s\");\n", pItem->nLevel, engiGetInstanceClassInfo(pItem->hInstance));
						}
						else
						{
							// instance attribute
							if (pItem->nType >= 0)
							{
								switch (pItem->nType)
								{
								case sdaiBOOLEAN:	printf("bVal = %d; sdaiPutAttrBN(hInstance%d, \"%s\", %d, &bVal); \n", pItem->value, pItem->nLevel, pItem->pstrAttrName, pItem->nType); break;
								case sdaiINTEGER:	printf("nVal = %d;  sdaiPutAttrBN(hInstance%d, \"%s\", %d, &nVal); \n", pItem->value, pItem->nLevel, pItem->pstrAttrName, pItem->nType); break;
//								case sdaiENUM:		printf("sdaiPutAttrBN(hInstance, \"%s\", %d, pItem->value); \n", pItem->pstrAttrName, pItem->nType); break;
								case sdaiREAL:		printf("dVal = %lf; sdaiPutAttrBN(hInstance%d, \"%s\", %d, &dVal); \n", pItem->dvalue, pItem->nLevel, pItem->pstrAttrName, pItem->nType); break;
								case sdaiAGGR:		printf("sdaiPutAttrBN(hInstance%d, \"%s\", %d, (void*)hAggreg%d); \n", pItem->nLevel, pItem->pstrAttrName, pItem->nType, pItem->nLevel+1); break;
								case sdaiINSTANCE:	printf("sdaiPutAttrBN(hInstance%d, \"%s\", %d, (void*)hInstance%d); \n", pItem->nLevel, pItem->pstrAttrName, pItem->nType, pItem->nLevel+1); break;
								case sdaiSTRING:	printf("sdaiPutAttrBN(hInstance%d, \"%s\", %d, \"%s\"); \n", pItem->nLevel, pItem->pstrAttrName, pItem->nType, pItem->value); break;
								default:			printf("sdaiPutAttrBN(hInstance%d, \"%s\", %d, \"%s\"); \n", pItem->nLevel, pItem->pstrAttrName, pItem->nType, pItem->value); break;
								}
							}
						}
						break;
					}
				});
}

void CIFCModelScanner::Clone(HINSTANCE nTargetModel, CB_FUNC cbUserFilter)
{
	SetCallback([nTargetModel, cbUserFilter] (ITEM *pItem)
				{
					if (cbUserFilter) cbUserFilter(pItem);
					switch (pItem->type)
					{
					case CIFCModelScanner::ITEM::AGGREG:
						if (pItem->nIndex < 0)
						{
							// aggregate
							ASSERT(pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE); // we don't know how to create an aggregate in an aggregate
							pItem->hNewAggreg = sdaiCreateAggrBN(pItem->pParent->hNewInstance, pItem->pParent->pstrAttrName);
						}
						else
						{
							// aggregate element
							sdaiAppend((int)pItem->hNewAggreg, pItem->nType, pItem->nType == sdaiREAL ? (VALUE)(&pItem->dvalue) : pItem->value);
						}
						break;
					case CIFCModelScanner::ITEM::INSTANCE:
						if (pItem->pstrAttrName == NULL)
						{
							// instance
							char *p = engiGetInstanceClassInfo(pItem->hInstance);
							pItem->hNewInstance = p ? sdaiCreateInstanceBN(nTargetModel, p) : NULL;
							// printf("sdaiCreateInstanceBN(%d, \"%s\") -> %d;\n", nMyModel, engiGetInstanceClassInfo(pItem->hInstance, pItem->hNewInstance));
						}
						else
						{
							// instance attribute
							if (pItem->nType >= 0)
							{
								//switch (pItem->nType)
								//{
								//case sdaiAGGR:		printf("sdaiPutAttrBN(%d, \"%s\", sdaiAGGR, %d);\n", pItem->hNewInstance, pItem->pstrAttrName, pItem->value); break;
								//case sdaiINSTANCE:	printf("sdaiPutAttrBN(%d, \"%s\", sdaiINSTANCE, %d);\n", pItem->hNewInstance, pItem->pstrAttrName, pItem->value); break;
								//case sdaiINTEGER:	printf("sdaiPutAttrBN(%d, \"%s\", sdaiINTEGER, %d);\n", pItem->hNewInstance, pItem->pstrAttrName, pItem->value); break;
								//case sdaiREAL:		printf("sdaiPutAttrBN(%d, \"%s\", sdaiREAL, %lf);\n", pItem->hNewInstance, pItem->pstrAttrName, pItem->dvalue); break;
								//case sdaiSTRING:	printf("sdaiPutAttrBN(%d, \"%s\", sdaiSTRING, \"%s\");\n", pItem->hNewInstance, pItem->pstrAttrName, pItem->value); break;
								//default:			printf("sdaiPutAttrBN(%d, \"%s\", UNKNOWN, [???]);\n", pItem->hNewInstance, pItem->pstrAttrName); break;
								//}

								switch (pItem->nType)
								{
								case sdaiBOOLEAN:	sdaiPutAttrBN(pItem->hNewInstance, pItem->pstrAttrName, pItem->nType, &pItem->value); break;
								case sdaiINTEGER:	sdaiPutAttrBN(pItem->hNewInstance, pItem->pstrAttrName, pItem->nType, &pItem->value); break;
								case sdaiENUM:		
									sdaiPutAttrBN(pItem->hNewInstance, pItem->pstrAttrName, pItem->nType, pItem->value); break;
								case sdaiREAL:		sdaiPutAttrBN(pItem->hNewInstance, pItem->pstrAttrName, pItem->nType, &pItem->dvalue); break;
								default:			if (pItem->value) sdaiPutAttrBN(pItem->hNewInstance, pItem->pstrAttrName, pItem->nType, pItem->value); break;
								}
							}
						}
						break;
					}
				});
}

void CIFCModelScanner::GetBB(BB &bb)
{
	bb.x0 = bb.y0 = bb.z0 = FLT_MAX;
	bb.x1 = bb.y1 = bb.z1 = FLT_MIN;
	SetCallback([&bb] (ITEM *pItem)
				{
					if (pItem->type == CIFCModelScanner::ITEM::AGGREG && pItem->nIndex >= 0 && pItem->nType == sdaiREAL 
						&& pItem->pParent && pItem->pParent->type == CIFCModelScanner::ITEM::INSTANCE && strcmp(pItem->pParent->pstrAttrName, "Coordinates") == 0 && sdaiGetMemberCount(pItem->hAggreg) == 3)
					{
						switch (pItem->nIndex)
						{
						case 0: if (pItem->dvalue < bb.x0) bb.x0 = pItem->dvalue; if (pItem->dvalue > bb.x1) bb.x1 = pItem->dvalue; break;
						case 1: if (pItem->dvalue < bb.y0) bb.y0 = pItem->dvalue; if (pItem->dvalue > bb.y1) bb.y1 = pItem->dvalue; break;
						case 2: if (pItem->dvalue < bb.z0) bb.z0 = pItem->dvalue; if (pItem->dvalue > bb.z1) bb.z1 = pItem->dvalue; break;
						}
					}
				});
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCRevitElem

CIFCRevitElem::CIFCRevitElem(CIFCRoot *pParent, transformationMatrixStruct *pMatrix) : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Default Revit Model";
	strDescription = "The project default Revit model";
	setRelNameAndDescription("RevitContainer", "RevitContainer");
}
	
int CIFCRevitElem::build(HINSTANCE hSourceInstance, CIFCModelScanner::CB_FUNC cbUserFilter)
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), "IFCBUILDINGELEMENTPROXY");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);

	pParent->appendRelContainedInSpatialStructure(ifcInstance);

	hCloneInstance = CIFCModelScanner::Clone(hSourceInstance, getModel(), cbUserFilter);

	sdaiPutAttrBN(getInstance(), "Representation", sdaiINSTANCE, (void*)hCloneInstance);

	return ifcInstance;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
// CIFCPointsElem

CIFCPointsElem::CIFCPointsElem(CIFCRoot *pParent, transformationMatrixStruct *pMatrix) : CIFCElement(pParent, pMatrix)
{
	reset();
	strName = "Default Revit Model";
	strDescription = "The project default Revit model";
	setRelNameAndDescription("RevitContainer", "RevitContainer");
}
	
int CIFCPointsElem::build(AVULONG nFaceSets, AVULONG *pnFacesPerSet, double *pData, AVULONG *pnPointsPerFace)
{
	ifcInstance = sdaiCreateInstanceBN(getModel(), "IFCBUILDINGELEMENTPROXY");

	sdaiPutAttrBN(ifcInstance, "GlobalId", sdaiSTRING, (void*) CreateCompressedGuidString());
	sdaiPutAttrBN(ifcInstance, "OwnerHistory", sdaiINSTANCE, (void*) getProject()->getOwnerHistoryInstance());
	sdaiPutAttrBN(ifcInstance, "Name", sdaiSTRING, strName);
	sdaiPutAttrBN(ifcInstance, "Description", sdaiSTRING, strDescription);

	ifcPlacementInstance = buildLocalPlacementInstance(pMatrix, pParent->getPlacementInstance());
	sdaiPutAttrBN(ifcInstance, "ObjectPlacement", sdaiINSTANCE, (void*) ifcPlacementInstance);

	pParent->appendRelContainedInSpatialStructure(ifcInstance);

	hCloneInstance = _build(nFaceSets, pnFacesPerSet, pData, pnPointsPerFace);

	sdaiPutAttrBN(getInstance(), "Representation", sdaiINSTANCE, (void*)hCloneInstance);

	return ifcInstance;
}

// PRIVATE FUNCTIONS

void CIFCPointsElem::_buildCartesianPoint(int *hAggregPolygon, double dX, double dY, double dZ)
{
	int hInstCP = sdaiCreateInstanceBN(model, "IfcCartesianPoint");
	int *fAggrCoord = sdaiCreateAggrBN(hInstCP, "Coordinates");
	sdaiAppend((int)fAggrCoord, 9, (void*)(&dX));
	sdaiAppend((int)fAggrCoord, 9, (void*)(&dY));
	sdaiAppend((int)fAggrCoord, 9, (void*)(&dZ));
	sdaiPutAttrBN(hInstCP, "Coordinates", 2, (void*)fAggrCoord); 
	sdaiAppend((int)hAggregPolygon, 6, (void*)hInstCP);
}

double *CIFCPointsElem::_buildFace(int *hAggregCfsFaces, double *pData, AVULONG nData)
{
	int hInstFace = sdaiCreateInstanceBN(model, "IfcFace");
	int *hAggrBounds = sdaiCreateAggrBN(hInstFace, "Bounds");
	int hInstOuterBound = sdaiCreateInstanceBN(model, "IfcFaceOuterBound");
	int hInstPolyLoop = sdaiCreateInstanceBN(model, "IfcPolyLoop");
	int *hAggregPolygon = sdaiCreateAggrBN(hInstPolyLoop, "Polygon");
	
	for (AVULONG i = 0; i < nData; i++, pData += 3)
		_buildCartesianPoint(hAggregPolygon, pData[0], pData[1], pData[2]);
	
	// Finishing
	sdaiPutAttrBN(hInstPolyLoop, "Polygon", 2, (void*)hAggregPolygon); 
	sdaiPutAttrBN(hInstOuterBound, "Bound", 6, (void*)hInstPolyLoop); 
	sdaiPutAttrBN(hInstOuterBound, "Orientation", 10, ".T."); 
	sdaiAppend((int)hAggrBounds, 6, (void*)hInstOuterBound);
	sdaiPutAttrBN(hInstFace, "Bounds", 2, (void*)hAggrBounds); 
	sdaiAppend((int)hAggregCfsFaces, 6, (void*)hInstFace);

	return pData;
}

double *CIFCPointsElem::_buildConnectedFaceSet(int *hAggregFbsmFaces, double *pData, AVULONG nData, AVULONG *pnPointsPerFace)
{
	int hInstConFaceSets = sdaiCreateInstanceBN(model, "IfcConnectedFaceSet");
	int *hAggregCfsFaces = sdaiCreateAggrBN(hInstConFaceSets, "CfsFaces");

	for (AVULONG i = 0; i < nData; i++)
		pData = _buildFace(hAggregCfsFaces, pData, pnPointsPerFace ? *pnPointsPerFace++ : 4);

	sdaiPutAttrBN(hInstConFaceSets, "CfsFaces", 2, (void*)hAggregCfsFaces); 
	sdaiAppend((int)hAggregFbsmFaces, 6, (void*)hInstConFaceSets);

	return pData;
}

int CIFCPointsElem::_build(AVULONG nFaceSets, AVULONG *pnFacesPerSet, double *pData, AVULONG *pnPointsPerFace)
{
	int nVal;
	double dVal;
	int *hAggreg1, *hAggreg3, *hAggreg6, *hAggreg7, *hAggreg8, *hAggreg9, *hAggreg10;
	int hInstance0, hInstance2, hInstance3, hInstance4, hInstance5, hInstance6, hInstance7, hInstance8, hInstance9;

	hInstance0 = sdaiCreateInstanceBN(model, "IfcProductDefinitionShape");
	if (!hInstance0) return NULL;
	hAggreg1 = sdaiCreateAggrBN(hInstance0, "Representations");
	hInstance2 = sdaiCreateInstanceBN(model, "IfcShapeRepresentation");
	hInstance3 = sdaiCreateInstanceBN(model, "IfcGeometricRepresentationContext");
	sdaiPutAttrBN(hInstance3, "ContextType", 10, "Model"); 
	nVal = 3;  sdaiPutAttrBN(hInstance3, "CoordinateSpaceDimension", 7, &nVal); 
	dVal = 0.000001; sdaiPutAttrBN(hInstance3, "Precision", 9, &dVal); 
	hInstance4 = sdaiCreateInstanceBN(model, "IfcAxis2Placement3D");
	hInstance5 = sdaiCreateInstanceBN(model, "IfcCartesianPoint");
	hAggreg6 = sdaiCreateAggrBN(hInstance5, "Coordinates");
	dVal = 0.000000; sdaiAppend((int)hAggreg6, 9, (void*)(&dVal));
	dVal = 0.000000; sdaiAppend((int)hAggreg6, 9, (void*)(&dVal));
	dVal = 0.000000; sdaiAppend((int)hAggreg6, 9, (void*)(&dVal));
	sdaiPutAttrBN(hInstance5, "Coordinates", 2, (void*)hAggreg6); 
	sdaiPutAttrBN(hInstance4, "Location", 6, (void*)hInstance5); 
	sdaiPutAttrBN(hInstance3, "WorldCoordinateSystem", 6, (void*)hInstance4); 
	sdaiPutAttrBN(hInstance2, "ContextOfItems", 6, (void*)hInstance3); 
	sdaiPutAttrBN(hInstance2, "RepresentationIdentifier", 10, "Body"); 
	sdaiPutAttrBN(hInstance2, "RepresentationType", 10, "MappedRepresentation"); 
	hAggreg3 = sdaiCreateAggrBN(hInstance2, "Items");
	hInstance4 = sdaiCreateInstanceBN(model, "IfcMappedItem");
	hInstance5 = sdaiCreateInstanceBN(model, "IfcRepresentationMap");
	hInstance6 = sdaiCreateInstanceBN(model, "IfcAxis2Placement3D");
	hInstance7 = sdaiCreateInstanceBN(model, "IfcCartesianPoint");
	hAggreg8 = sdaiCreateAggrBN(hInstance7, "Coordinates");
	dVal = 0.000000; sdaiAppend((int)hAggreg8, 9, (void*)(&dVal));
	dVal = 0.000000; sdaiAppend((int)hAggreg8, 9, (void*)(&dVal));
	dVal = 0.000000; sdaiAppend((int)hAggreg8, 9, (void*)(&dVal));
	sdaiPutAttrBN(hInstance7, "Coordinates", 2, (void*)hAggreg8); 
	sdaiPutAttrBN(hInstance6, "Location", 6, (void*)hInstance7); 
	sdaiPutAttrBN(hInstance5, "MappingOrigin", 6, (void*)hInstance6); 
	hInstance6 = sdaiCreateInstanceBN(model, "IfcShapeRepresentation");
	hInstance7 = sdaiCreateInstanceBN(model, "IfcGeometricRepresentationContext");
	sdaiPutAttrBN(hInstance7, "ContextType", 10, "Model"); 
	nVal = 3;  sdaiPutAttrBN(hInstance7, "CoordinateSpaceDimension", 7, &nVal); 
	dVal = 0.000001; sdaiPutAttrBN(hInstance7, "Precision", 9, &dVal); 
	hInstance8 = sdaiCreateInstanceBN(model, "IfcAxis2Placement3D");
	hInstance9 = sdaiCreateInstanceBN(model, "IfcCartesianPoint");
	hAggreg10 = sdaiCreateAggrBN(hInstance9, "Coordinates");
	dVal = 0.000000; sdaiAppend((int)hAggreg10, 9, (void*)(&dVal));
	dVal = 0.000000; sdaiAppend((int)hAggreg10, 9, (void*)(&dVal));
	dVal = 0.000000; sdaiAppend((int)hAggreg10, 9, (void*)(&dVal));
	sdaiPutAttrBN(hInstance9, "Coordinates", 2, (void*)hAggreg10); 
	sdaiPutAttrBN(hInstance8, "Location", 6, (void*)hInstance9); 
	sdaiPutAttrBN(hInstance7, "WorldCoordinateSystem", 6, (void*)hInstance8); 
	sdaiPutAttrBN(hInstance6, "ContextOfItems", 6, (void*)hInstance7); 
	sdaiPutAttrBN(hInstance6, "RepresentationIdentifier", 10, "Body"); 
	sdaiPutAttrBN(hInstance6, "RepresentationType", 10, "SurfaceModel"); 
	hAggreg7 = sdaiCreateAggrBN(hInstance6, "Items");
	hInstance8 = sdaiCreateInstanceBN(model, "IfcFaceBasedSurfaceModel");
	hAggreg9 = sdaiCreateAggrBN(hInstance8, "FbsmFaces");

	// CONNECTED FACE SETS
	for (AVULONG i = 0; i < nFaceSets; i++)
	{
		AVULONG nFacesPerSet = *pnFacesPerSet++;
		pData = _buildConnectedFaceSet(hAggreg9, pData, nFacesPerSet, pnPointsPerFace);
		if (pnPointsPerFace) pnPointsPerFace += nFacesPerSet;
	}

	// END OF THE FILE SEQ
	sdaiPutAttrBN(hInstance8, "FbsmFaces", 2, (void*)hAggreg9); 
	sdaiAppend((int)hAggreg7, 6, (void*)hInstance8);
	sdaiPutAttrBN(hInstance6, "Items", 2, (void*)hAggreg7); 
	sdaiPutAttrBN(hInstance5, "MappedRepresentation", 6, (void*)hInstance6); 
	sdaiPutAttrBN(hInstance4, "MappingSource", 6, (void*)hInstance5); 
	hInstance5 = sdaiCreateInstanceBN(model, "IfcCartesianTransformationOperator3D");
	hInstance6 = sdaiCreateInstanceBN(model, "IfcCartesianPoint");
	hAggreg7 = sdaiCreateAggrBN(hInstance6, "Coordinates");
	dVal = 0.000000; sdaiAppend((int)hAggreg7, 9, (void*)(&dVal));
	dVal = 0.000000; sdaiAppend((int)hAggreg7, 9, (void*)(&dVal));
	dVal = 0.000000; sdaiAppend((int)hAggreg7, 9, (void*)(&dVal));
	sdaiPutAttrBN(hInstance6, "Coordinates", 2, (void*)hAggreg7); 
	sdaiPutAttrBN(hInstance5, "LocalOrigin", 6, (void*)hInstance6); 
	nVal = 1;  sdaiPutAttrBN(hInstance5, "Scale", 7, &nVal); 
	sdaiPutAttrBN(hInstance4, "MappingTarget", 6, (void*)hInstance5); 
	sdaiAppend((int)hAggreg3, 6, (void*)hInstance4);
	sdaiPutAttrBN(hInstance2, "Items", 2, (void*)hAggreg3); 
	sdaiAppend((int)hAggreg1, 6, (void*)hInstance2);
	sdaiPutAttrBN(hInstance0, "Representations", 2, (void*)hAggreg1); 

	return hInstance0;
}
