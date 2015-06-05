// Video.h Main File Header

#ifdef ADVIDEO_EXPORTS

	// DLL internals
	class CAdVisuoLoader;
	class CEngine;
	class CProjectVis;
	class CCamera;

	#define ADVIDEO_API __declspec(dllexport)

	bool LoadStructure(CAdVisuoLoader &Loader);
	bool LoadData(CAdVisuoLoader &Loader, CEngine &Engine, LONG nTimeMax);
	bool Build(CEngine &Engine, CProjectVis &Prj);
	CCamera *BuildCamera(CEngine &Engine, CProjectVis &Prj, int nLiftGroup, int nCamera, int nLift, int nFloor);
	CSize GetStandardSize(int nSize);

#else
	#define ADVIDEO_API __declspec(dllimport)
#endif

// The Official Exports
ADVIDEO_API ULONG AVVideo(ULONG idVideo, ULONG idSimulation, ULONG nLiftGroup, ULONG nCamera, ULONG nLift, ULONG nFloor, ULONG nSize, ULONG nTimeFrom, ULONG nTimeTo, LPCTSTR pFileName);
