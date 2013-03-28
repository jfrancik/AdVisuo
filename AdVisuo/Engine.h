// Sprite.h

#pragma once

#include "repos.h"

#include <freewill.h>	// obligatory
#include <fwrender.h>	// to start the renderer
#include <fwaction.h>	// actions

extern bool g_bFullScreen;
extern bool g_bReenter;

interface IAction;
interface IBody;
interface IKineNode;
interface IMaterial;

interface ILostDeviceObserver
{
	virtual void OnLostDevice() = 0;
	virtual void OnResetDevice() = 0;
};

enum
{
	MAT_BACKGROUND, MAT_LOBBY_0, MAT_LOBBY_1, MAT_LOBBY_2, MAT_LOBBY_3, MAT_LOBBY_4, MAT_LOBBY_5, MAT_LOBBY_6, MAT_LOBBY_7, 
	MAT_FLOOR, MAT_CEILING, MAT_DOOR, MAT_LIFT_DOOR, MAT_OPENING, MAT_LIFT, MAT_LIFT_FLOOR, MAT_LIFT_CEILING, MAT_SHAFT, MAT_BEAM,
	MAT_NUM,
	MAT_PLATE_FLOOR = MAT_NUM, MAT_PLATE_LIFT
};

typedef int (*CB_HANDLE)(struct ACTION_EVENT *pEvent, IAction *pAction, FWULONG nParam, void *pParam);
typedef IAction *ANIM_HANDLE;

class CEngine : protected CRepos<IBody>
{
	// Main Scene
	IFWDevice *m_pFWDevice;					// FreeWill Device
	IRenderer *m_pRenderer;					// The Renderer
	IScene *m_pScene;						// The Scene

	// action tick
	IAction *m_pActionTick;					// The Clock Tick Action...

	// auxiliary action tick & ref time
	IAction *m_pAuxActionTick;				// The Clock Tick Action for Camera Animation...
	AVULONG m_nAuxTimeRef;					// The Clock Value for m_pAuxActionTick

	// bipeds spawning machine
	IKineChild *m_pBiped;					// biped (template)
	IMaterial *m_pMaterial;					// material (for produced bipeds)
	BYTE *m_pBipedBuf;						// Data buffer for Store/RetrieveState functions
	AVULONG m_nBipedBufCount;

	// lights
	ISceneLightDir *m_pLight1;				// light 1
	ISceneLightDir *m_pLight2;				// light 2

	// Frame Rate calculation
	static const DWORD c_fpsNUM = 120;		// Frame per Second rate calculation
	DWORD *m_pfps;
	int m_nfps;

	// Animation Constants
	static const AVULONG c_nStepLen = 15;
	static const AVULONG c_nStepDuration = 150;
	static const AVULONG c_nTurnDuration = 300;

	// Materials
	struct MATERIAL
	{
		AVSTRING m_pLabel;
		bool m_bSolid;
		AVCOLOR m_color;
		AVSTRING m_pFName;
		AVFLOAT m_fUTile, m_fVTile;
		IMaterial *m_pMaterial;
		
		MATERIAL() { memset(this, 0, sizeof(*this)); }
		~MATERIAL() { if (m_pLabel) free(m_pLabel); if (m_pFName) free(m_pFName); }
	} m_materials[MAT_NUM];
	std::vector<IMaterial*> m_matFloorPlates;
	std::vector<IMaterial*> m_matLiftPlates;

public:
	CEngine();
	virtual ~CEngine();

private:
	IRenderer *GetRenderer()					{ return m_pRenderer; }
	IScene *GetScene()							{ return m_pScene; }
	friend class CAdVisuoView;
	friend class CElemVis;
public:

	// Engine Creation
	bool Create(HWND hWnd, ILostDeviceObserver *pLDO = NULL);

	// Status
	bool IsReady()								{ return m_pScene != NULL; }
	bool IsRunning()							{ return (m_pActionTick->AnySubscriptionsLeft() == TRUE); }
	CSize GetViewSize()							{ CSize size; m_pRenderer->GetViewSize((FWULONG*)&size.cx, (FWULONG*)&size.cy); return size; }
	FWULONG GeViewtWidth()						{ FWULONG x, y; m_pRenderer->GetViewSize(&x, &y); return x; }
	FWULONG GetViewHeight()						{ FWULONG x, y; m_pRenderer->GetViewSize(&x, &y); return y; }
	CString GetDiagnosticMessage();

	// Material manager
	AVULONG GetMatCount()						{ return sizeof(m_materials) / sizeof(MATERIAL); }
	void SetSolidMat(AVULONG i, AVCOLOR color, AVSTRING pLabel = NULL);
	void SetSolidMat(AVULONG i, FWULONG r, FWULONG g, FWULONG b, FWULONG a = 256, AVSTRING pLabel = NULL);
	void SetTexturedMat(AVULONG i, AVSTRING pFName, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT alpha = 1.0f, AVSTRING pLabel = NULL);

	void CreateMat(AVULONG i);
	void ReleaseMat(AVULONG i);
	void CreateAllMats()						{ for (AVULONG i = 0; i < GetMatCount(); i++) CreateMat(i); }
	void ReleaseAllMats()						{ for (AVULONG i = 0; i < GetMatCount(); i++) ReleaseMat(i); }

	void CreateFloorPlateMats(AVULONG nStoreys, AVULONG nBasementStoreys);
	void CreateLiftPlateMats(AVULONG nShafts);
	void ReleaseFloorPlateMats()				{ for each (IMaterial *pMaterial in m_matFloorPlates) pMaterial->Release(); }
	void ReleaseLiftPlateMats()					{ for each (IMaterial *pMaterial in m_matLiftPlates) pMaterial->Release(); }

	AVSTRING GetMatLabel(AVULONG i)				{ return m_materials[i].m_pLabel; }
	bool GetMatSolid(AVULONG i)					{ return m_materials[i].m_bSolid; }
	AVCOLOR GetMatColor(AVULONG i)				{ return m_materials[i].m_color; }
	AVFLOAT GetMatAlpha(AVULONG i)				{ return m_materials[i].m_color.a; }
	AVSTRING GetMatFName(AVULONG i)				{ return m_materials[i].m_pFName; }
	AVFLOAT GetMatUTile(AVULONG i)				{ return m_materials[i].m_fUTile; }
	AVFLOAT GetMatVTile(AVULONG i)				{ return m_materials[i].m_fVTile; }

	IMaterial *GetMat(AVULONG nWallId, AVLONG i = 0);
	
	void InitMats(AVULONG nStoreys, AVULONG nBasementStoreys, AVULONG nShafts);

	// Rendering cycle
	void BeginFrame();
	void EndFrame();

	// Proceed the simulation to the given time stamp
	void Proceed(FWLONG nMSec)					{ m_pActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, 0); }
	
	// Auxiliary Player
	void AuxPlay(IAction **pAuxAction, AVULONG nClockValue = 0x7FFFFFFF);
	void ProceedAux(FWLONG nMSec);
	
	// Tools
	bool Play()									{ if (!IsReady()) return false; m_pRenderer->Play(); return true; }
	bool Play(FWLONG nMSec)						{ if (!IsReady()) return false; m_pRenderer->Play(); m_pRenderer->PutPlayTime(nMSec); return true; }
	bool Pause()								{ if (!IsReady()) return false; m_pRenderer->Pause(); return true; }
	bool Stop()									{ if (!IsReady()) return false; m_pRenderer->Stop(); m_pActionTick->UnSubscribeAll(); return true; }
	bool IsPlaying()							{ return IsReady() && (m_pRenderer->IsPlaying() == S_OK); }
	bool IsPaused()								{ return IsReady() && (m_pRenderer->IsPaused() == S_OK); }
	FWLONG GetPlayTime()						{ FWLONG nTime; if (!IsPlaying()) return 0; m_pRenderer->GetPlayTime(&nTime); return nTime; }
	FWULONG GetFPS()							{ return m_pfps[m_nfps] ? 1000 * (c_fpsNUM-1) / (m_pfps[(m_nfps+c_fpsNUM-1)%c_fpsNUM] - m_pfps[m_nfps]) : 0; }
	FWFLOAT GetAccel()							{ FWFLOAT accel; m_pRenderer->GetAccel(&accel); return accel; }
	void PutAccel(FWFLOAT accel)				{ m_pRenderer->PutAccel(accel); }
	void SlowDown(FWFLOAT f = 2)				{ PutAccel(GetAccel() / f); }
	void SpeedUp(FWFLOAT f = 2)					{ PutAccel(GetAccel() * f); }
	void ResetAccel()							{ m_pRenderer->PutAccel(1); }

	// Device Reset and off screen modes
	void ResetDevice(HWND hWnd = NULL)			{ m_pRenderer->ResetDeviceEx(hWnd, 0, 0); }
	void InitOffScreen(CSize size, LPCTSTR pAviFile, FWULONG nFPS);
	void InitOffScreen(CSize size, LPCTSTR pImgFile, FW_RENDER_BITMAP fmt);
	void SetTargetToScreen()					{ m_pRenderer->SetTargetToScreen(); }
	void SetTargetOffScreen()					{ m_pRenderer->SetTargetOffScreen(); }
	void DoneOffScreen();

	// Various functions
	void RenderLights();
	void PutCamera(ISceneCamera *p)				{ m_pScene->PutCamera(p); }

	// Repository Utilities
	IBody *SpawnBiped();
	void KillBiped(IBody*);

	// Animators
	ANIM_HANDLE StartAnimation(LONG nTime);
	ANIM_HANDLE SetAnimationCB(ANIM_HANDLE aHandle, CB_HANDLE fn, AVULONG nParam, void *pParam);
	ANIM_HANDLE DoNothing(ANIM_HANDLE aHandle);
	ANIM_HANDLE Move(ANIM_HANDLE aHandle, IBody *pBody, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
	ANIM_HANDLE Move(ANIM_HANDLE aHandle, IKineNode *pBone, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
	ANIM_HANDLE MoveTo(ANIM_HANDLE aHandle, IBody *pBody, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
	ANIM_HANDLE MoveTo(ANIM_HANDLE aHandle, IKineNode *pBone, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
	ANIM_HANDLE Wait(ANIM_HANDLE aHandle, IBody *pBody, LONG nTimeUntil, std::wstring wstrStyle = L"");
	ANIM_HANDLE Walk(ANIM_HANDLE aHandle, IBody *pBody, AVFLOAT x, AVFLOAT y, std::wstring wstrStyle = L"");
	ANIM_HANDLE Turn(ANIM_HANDLE aHandle, IBody *pBody, std::wstring wstrStyle = L"");
	ANIM_HANDLE SetParabolicEnvelopeT(ANIM_HANDLE aHandle, AVFLOAT fEaseIn, AVFLOAT fEaseOut);


protected:
	// CRepos<IBody> implementation
	virtual IBody *create();
	virtual void destroy(IBody*);
};

