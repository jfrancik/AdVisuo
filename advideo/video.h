// Video.h Main File Header

#ifdef ADVIDEO_EXPORTS

	// DLL internals
	class CTempLoader;
	class CEngine;
	class CProjectVis;
	class CCamera;

	#define ADVIDEO_API __declspec(dllexport)

	//bool LoadStructure(CTempLoader &Loader);
	//bool LoadData(CTempLoader &Loader, CEngine &Engine, LONG nTimeMax);
	
	bool Load(dbtools::CDataBase db, CProjectVis *pProject, AVULONG nProjectId, std::wstring &diagStr);

	
	bool Build(CEngine &Engine, CProjectVis &Prj);
	CCamera *BuildCamera(CEngine &Engine, CProjectVis &Prj, int nLiftGroup, int nCamera, int nLift, int nFloor);
	CSize GetStandardSize(int nSize);
	AVULONG GetStandardBitrate(int nSize, int nFramerate);


#else
	#define ADVIDEO_API __declspec(dllimport)
#endif

// The Official Exports
ADVIDEO_API LONG AVVideo(ULONG idVideo, ULONG idSimulation, ULONG nLiftGroup, ULONG nCamera, ULONG nLift, ULONG nFloor, ULONG nSize, ULONG nTimeFrom, ULONG nTimeTo, LPCTSTR pFileName);
