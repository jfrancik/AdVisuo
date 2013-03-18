// Sprite.h

#pragma once

#include "repos.h"

#include <D3d9.h>
#include <D3dx9core.h>

#include <freewill.h>	// obligatory
#include <fwrender.h>	// to start the renderer
#include <fwaction.h>	// actions

extern bool g_bFullScreen;
extern bool g_bReenter;

interface IAction;
interface IBody;
interface IKineNode;

interface ILostDeviceObserver
{
	virtual void OnLostDevice() = 0;
	virtual void OnResetDevice() = 0;
};

typedef int (*CB_HANDLE)(struct ACTION_EVENT *pEvent, IAction *pAction, FWULONG nParam, void *pParam);

class CEngine : protected CRepos<IBody>
{
	// Main Scene
	IFWDevice *m_pFWDevice;					// FreeWill Device
	IRenderer *m_pRenderer;					// The Renderer
	IScene *m_pScene;						// The Scene
	
	// spawning bipeds
	IKineChild *m_pBiped;					// biped (template)
	IMaterial *m_pMaterial;					// material (for produced bipeds)
	BYTE *m_pBipedBuf;						// Data buffer for Store/RetrieveState functions
	AVULONG m_nBipedBufCount;

	ISceneLightDir *m_pLight1;				// light 1
	ISceneLightDir *m_pLight2;				// light 2

	IAction *m_pActionTick;					// The Clock Tick Action...

	IAction *m_pAuxActionTick;				// The Clock Tick Action for Camera Animation...
	AVULONG m_nAuxTimeRef;					// The Clock Value for m_pAuxActionTick

	// Frame Rate calculation
	static DWORD c_fpsNUM;					// Frame per Second rate calculation
	DWORD *m_pfps;
	int m_nfps;

public:
	CEngine();
	virtual ~CEngine();

	bool Create(HWND hWnd, ILostDeviceObserver *pLDO = NULL);
	void ResetDevice(HWND hWnd = NULL)			{ m_pRenderer->ResetDeviceEx(hWnd, 0, 0); }
	void BeginFrame()							{ m_pRenderer->BeginFrame(); }
	void EndFrame()								{ m_pRenderer->EndFrame(); }
	void RenderLights();

	void InitOffScreen(CSize size, LPCTSTR pAviFile, FWULONG nFPS);
	void InitOffScreen(CSize size, LPCTSTR pImgFile, FW_RENDER_BITMAP fmt);
	void SetTargetToScreen()					{ m_pRenderer->SetTargetToScreen(); }
	void SetTargetOffScreen()					{ m_pRenderer->SetTargetOffScreen(); }
	void DoneOffScreen();

	// proceed the simulation to the given time stamp; returns false if simulation is finished
	void Proceed(FWLONG nMSec)					{ m_pActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, 0); }

	void AuxPlay(IAction **pAuxAction, AVULONG nClockValue = 0x7FFFFFFF);
	void ProceedAux(FWLONG nMSec);

	void OnTimer();

	bool IsReady()								{ return m_pScene != NULL; }
	bool IsRunning()							{ return (m_pActionTick->AnySubscriptionsLeft() == TRUE); }

	// repository
	IBody *SpawnBiped();
	void KillBiped(IBody*);


	friend class ANIMATOR;
	class ANIMATOR
	{
		CEngine *m_pEngine;
		IBody *m_pBody;
		IKineNode *m_pBone;
		IAction *m_pAction;
	public:
		ANIMATOR(CEngine *pEngine, LONG nTime);
		ANIMATOR(CEngine *pEngine, IBody *pBody, LONG nTime);
		ANIMATOR(CEngine *pEngine, IKineNode *pBone, LONG nTime);
		~ANIMATOR();
		void SetAt(LONG nTime);
		void SetCB(CB_HANDLE fn, AVULONG nParam, void *pParam);
		void Generic();
		void Move(AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
		void MoveTo(AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
		void Wait(LONG nTimeUntil, std::wstring wstrStyle = L"");
		void Walk(AVFLOAT x, AVFLOAT y, std::wstring wstrStyle = L"");
		void Turn(std::wstring wstrStyle = L"");
		void SetParabolicEnvelopeT(AVFLOAT fEaseIn, AVFLOAT fEaseOut);
	};





	
	IRenderer *GetRenderer()					{ return m_pRenderer; }
	IScene *GetScene()							{ return m_pScene; }



	CString GetDiagnosticMessage();




	bool Play()						{ if (!IsReady()) return false; m_pRenderer->Play(); return true; }
	bool Play(FWLONG nMSec)			{ if (!IsReady()) return false; m_pRenderer->Play(); m_pRenderer->PutPlayTime(nMSec); return true; }
	bool Pause()					{ if (!IsReady()) return false; m_pRenderer->Pause(); return true; }
	bool Stop()						{ if (!IsReady()) return false; m_pRenderer->Stop(); m_pActionTick->UnSubscribeAll(); return true; }
	bool IsPlaying()				{ return IsReady() && (m_pRenderer->IsPlaying() == S_OK); }
	bool IsPaused()					{ return IsReady() && (m_pRenderer->IsPaused() == S_OK); }
	FWLONG GetPlayTime()			{ FWLONG nTime; if (!IsPlaying()) return 0; m_pRenderer->GetPlayTime(&nTime); return nTime; }

	FWULONG GetFPS();


	FWFLOAT GetAccel()				{ FWFLOAT accel; m_pRenderer->GetAccel(&accel); return accel; }
	void PutAccel(FWFLOAT accel)	{ m_pRenderer->PutAccel(accel); }
	void SlowDown(FWFLOAT f = 2)	{ PutAccel(GetAccel() / f); }
	void SpeedUp(FWFLOAT f = 2)		{ PutAccel(GetAccel() * f); }
	void ResetAccel()				{ m_pRenderer->PutAccel(1); }

	CSize GetViewSize()				{ CSize size; m_pRenderer->GetViewSize((FWULONG*)&size.cx, (FWULONG*)&size.cy); return size; }
	FWULONG GeViewtWidth()			{ FWULONG x, y; m_pRenderer->GetViewSize(&x, &y); return x; }
	FWULONG GetViewHeight()			{ FWULONG x, y; m_pRenderer->GetViewSize(&x, &y); return y; }

protected:
	// CRepos<IBody> functions
	virtual IBody *create();
	virtual void destroy(IBody*);
};
