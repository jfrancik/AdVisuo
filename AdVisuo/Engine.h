// Sprite.h

#pragma once

#include "repos.h"

namespace fw
{
	interface IFWDevice;
	interface IRenderer;
	interface IScene;
	interface IKineChild;
	interface ISceneLightDir;
};

interface IDirect3DDevice9;

extern bool g_bFullScreen;
extern bool g_bReenter;

enum
{
	MAT_BACKGROUND, MAT_LOBBY_0, MAT_LOBBY_1, MAT_LOBBY_2, MAT_LOBBY_3, MAT_LOBBY_4, MAT_LOBBY_5, MAT_LOBBY_6, MAT_LOBBY_7, 
	MAT_FLOOR, MAT_CEILING, MAT_DOOR, MAT_LIFT_DOOR, MAT_OPENING, MAT_LIFT, MAT_LIFT_FLOOR, MAT_LIFT_CEILING, MAT_SHAFT, MAT_BEAM,
	MAT_NUM,
	MAT_PLATE_FLOOR = MAT_NUM, MAT_PLATE_LIFT
};

enum BMP_FORMAT { FORMAT_BMP, FORMAT_JPG, FORMAT_TGA, FORMAT_PNG };

class CEngine : protected CRepos<fw::IBody>
{
	// Main Scene
	fw::IFWDevice *m_pFWDevice;				// FreeWill Device
	fw::IRenderer *m_pRenderer;				// The Renderer
	fw::IScene *m_pScene;					// The Scene

	// action tick
	fw::IAction *m_pActionTick;				// The Clock Tick Action...

	// auxiliary action tick & ref time
	fw::IAction *m_pAuxActionTick;			// The Clock Tick Action for Camera Animation...
	AVULONG m_nAuxTimeRef;					// The Clock Value for m_pAuxActionTick

	// bipeds spawning machine
	fw::IKineChild *m_pBiped;				// biped (template)
	fw::IMaterial *m_pMaterial;				// material (for produced bipeds)
	BYTE *m_pBipedBuf;						// Data buffer for Store/RetrieveState functions
	AVULONG m_nBipedBufCount;

	// lights
	fw::ISceneLightDir *m_pLight1;			// light 1
	fw::ISceneLightDir *m_pLight2;			// light 2

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
		fw::IMaterial *m_pMaterial;
		
		MATERIAL() { memset(this, 0, sizeof(*this)); }
		~MATERIAL() { if (m_pLabel) free(m_pLabel); if (m_pFName) free(m_pFName); }
	} m_materials[MAT_NUM];
	std::vector<fw::IMaterial*> m_matFloorPlates;
	std::vector<fw::IMaterial*> m_matLiftPlates;

public:
	CEngine();
	virtual ~CEngine();

	// Engine Creation
	bool Create(HWND hWnd, ILostDeviceObserver *pLDO = NULL);

	// DirectX bridge
	IDirect3DDevice9 *GetDXDevice();

	// Status
	bool IsReady();
	bool IsRunning();
	CSize GetViewSize();
	CSize GetBackBufferSize();
	AVULONG GeViewtWidth();
	AVULONG GetViewHeight();
	CString GetDiagnosticMessage();

	// Material manager
	AVULONG GetMatCount()						{ return sizeof(m_materials) / sizeof(MATERIAL); }
	void SetSolidMat(AVULONG i, AVCOLOR color, AVSTRING pLabel = NULL);
	void SetSolidMat(AVULONG i, AVULONG r, AVULONG g, AVULONG b, AVULONG a = 256, AVSTRING pLabel = NULL);
	void SetTexturedMat(AVULONG i, AVSTRING pFName, AVFLOAT fUTile, AVFLOAT fVTile, AVFLOAT alpha = 1.0f, AVSTRING pLabel = NULL);

	void CreateMat(AVULONG i);
	void ReleaseMat(AVULONG i);
	void CreateAllMats()						{ for (AVULONG i = 0; i < GetMatCount(); i++) CreateMat(i); }
	void ReleaseAllMats()						{ for (AVULONG i = 0; i < GetMatCount(); i++) ReleaseMat(i); }

	void CreateFloorPlateMats(AVULONG nStoreys, AVULONG nBasementStoreys);
	void CreateLiftPlateMats(AVULONG nShafts);
	void ReleaseFloorPlateMats();
	void ReleaseLiftPlateMats();

	AVSTRING GetMatLabel(AVULONG i)				{ return m_materials[i].m_pLabel; }
	bool GetMatSolid(AVULONG i)					{ return m_materials[i].m_bSolid; }
	AVCOLOR GetMatColor(AVULONG i)				{ return m_materials[i].m_color; }
	AVFLOAT GetMatAlpha(AVULONG i)				{ return m_materials[i].m_color.a; }
	AVSTRING GetMatFName(AVULONG i)				{ return m_materials[i].m_pFName; }
	AVFLOAT GetMatUTile(AVULONG i)				{ return m_materials[i].m_fUTile; }
	AVFLOAT GetMatVTile(AVULONG i)				{ return m_materials[i].m_fVTile; }

	HMATERIAL GetMat(AVULONG nWallId, AVLONG i = 0);
	
	void InitMats(AVULONG nStoreys, AVULONG nBasementStoreys, AVULONG nShafts);

	// Rendering cycle
	void BeginFrame();
	void EndFrame();

	// Proceed the simulation to the given time stamp
	void Proceed(AVLONG nMSec);
	
	// Auxiliary Player
	void AuxPlay(HACTION *pAuxAction, AVULONG nClockValue = 0x7FFFFFFF);
	void ProceedAux(AVLONG nMSec);
	
	// Animation Control Tools
	bool Play();
	bool Play(AVLONG nMSec);
	bool Pause();
	bool Stop();
	bool IsPlaying();
	bool IsPaused();
	AVLONG GetPlayTime();
	AVULONG GetFPS();
	AVFLOAT GetAccel();
	void PutAccel(AVFLOAT accel);
	void SlowDown(AVFLOAT f = 2);
	void SpeedUp(AVFLOAT f = 2);
	void ResetAccel();

	// Device Reset and off screen modes
	void ResetDevice(HWND hWnd = NULL);
	void StartTargetToImage(CSize size, LPCTSTR pImgFile, enum BMP_FORMAT);
	void StartTargetToImage(CSize size, LPCTSTR pImgFile);	// chooses format automatically - after the file ext
	void StartTargetToVideo(CSize size, LPCTSTR pAviFile, AVULONG nFPS);
	void SetTargetToScreen();
	void SetTargetOffScreen();
	void DoneTargetOffScreen();

	// Viewport preparation
	void PrepareViewport(AVFLOAT fX, AVFLOAT fY, AVFLOAT fWidth, AVFLOAT fHeight, AVLONG nX, AVLONG nY, AVLONG nWidth, AVLONG nHeight, bool bClear = true);
	void PrepareViewport(AVFLOAT fX, AVFLOAT fY, AVFLOAT fWidth, AVFLOAT fHeight, AVLONG nX, AVLONG nY, AVLONG nWidth, AVLONG nHeight, AVCOLOR backColor, bool bClear = true);

	// Various functions
	void RenderLights();
	void Render(HCAMERA p);
	void Render(HOBJECT p);
	void RenderPassenger(HBODY pBody, AVULONG nColourMode, AVULONG nPhase, AVLONG timeSpawn, AVLONG timeLoad, AVLONG spanWait);
	
	void PutCamera(HCAMERA p);
	void Embark(HBODY pBody, HBONE pNode, bool bSwitchCoord = true);	// embarks the body as the pNode node

	HBONE CreateChild(HBONE pParent, AVSTRING strLabel);
	HOBJECT CreateObject(AVSTRING strLabel);
	HCAMERA CreateCamera(HBONE pParent, AVSTRING strLabel);
	void DeleteChild(HBONE pParent, HBONE hBone);

	static AVVECTOR GetBonePos(HBONE bone);
	static AVVECTOR GetBonePosLocal(HBONE bone, HBONE ref);

	static void ResetBone(HBONE bone);
	static void MoveBone(HBONE bone, AVFLOAT x, AVFLOAT y, AVFLOAT z);
	static void MoveBoneTo(HBONE bone, AVFLOAT x, AVFLOAT y, AVFLOAT z);
	static void RotateBoneX(HBONE bone, AVFLOAT f);
	static void RotateBoneY(HBONE bone, AVFLOAT f);
	static void RotateBoneZ(HBONE bone, AVFLOAT f);
	static void PushState(HBONE bone);
	static void PopState(HBONE bone);
	static void Invalidate(HBONE bone);

	static void SetCameraPerspective(HCAMERA hCamera, AVFLOAT fFOV, AVFLOAT fClipNear, AVFLOAT fClipFar, AVFLOAT fAspect);




	// Biped Repository Utilities
	HBODY SpawnBiped();
	void KillBiped(HBODY);

	// Animators
	HACTION StartAnimation(LONG nTime);
	HACTION SetAnimationListener(HACTION aHandle, IAnimationListener *pListener, AVULONG nParam = 0);
	HACTION DoNothing(HACTION aHandle);
	HACTION Move(HACTION aHandle, HBODY pBody, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
	HACTION Move(HACTION aHandle, HBONE pBone, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
	HACTION MoveTo(HACTION aHandle, HBODY pBody, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
	HACTION MoveTo(HACTION aHandle, HBONE pBone, AVULONG nDuration, AVFLOAT x, AVFLOAT y, AVFLOAT z, std::wstring wstrStyle = L"");
	HACTION Wait(HACTION aHandle, HBODY pBody, LONG nTimeUntil, std::wstring wstrStyle = L"");
	HACTION Walk(HACTION aHandle, HBODY pBody, AVFLOAT x, AVFLOAT y, std::wstring wstrStyle = L"");
	HACTION Turn(HACTION aHandle, HBODY pBody, std::wstring wstrStyle = L"");
	HACTION SetEnvelope(HACTION aHandle, AVFLOAT fEaseInMsec, AVFLOAT fEaseOutMsec);

	// Camera Animators
	void StartCameraAnimation(AVULONG nClockValue = 0x7FFFFFFF);
	HACTION MoveCameraTo(HBONE pNode, AVVECTOR v, AVULONG nTime, AVULONG nDuration, HBONE pRef);
	HACTION PanCamera(HBONE pNode, AVFLOAT alpha, AVULONG nTime, AVULONG nDuration);
	HACTION TiltCamera(HBONE pNode, AVFLOAT alpha, AVULONG nTime, AVULONG nDuration);
	HACTION ZoomCamera(HCAMERA pCamera, AVFLOAT fFOV, AVFLOAT fClipNear, AVFLOAT fClipFar, AVULONG nTime, AVULONG nDuration);
	HACTION SetCameraEnvelope(HACTION aHandle, AVFLOAT fEaseInMsec, AVFLOAT fEaseOutMsec);


protected:
	// CRepos<IBody> implementation
	virtual HBODY create();
	virtual void destroy(HBODY);
};

