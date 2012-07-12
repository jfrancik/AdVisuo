// adv.h - a part of the AdVisuo Server Module

#ifdef ADV_EXPORTS
#define ADV_API extern "C" __declspec(dllexport)
#else
#define ADV_API extern "C" __declspec(dllimport)
#endif

#ifndef AVULONG
#define AVULONG ULONG
#endif
#ifndef AVLONG
#define AVLONG LONG
#endif
#ifndef AVFLOAT
#define AVFLOAT FLOAT
#endif
#ifndef AVSTRING
#define AVSTRING LPOLESTR
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Recommended usage (pseudo-code):
//
// Call this once for the system installation:
//	AVSetConnStrings(pConsoleConnectionString, pReportsConnectionString, pVisualisationConnectionString)
//	AVSetScalingFactor(fScalingFactor)	// this is not necessary unless a non-standard scaling factor is used
//
//
// Call this each time the VISUALISE button is pressed:
//	if (AVTest(nSimId) == S_OK)
//		run the AdVisuo client software (remotely)
//	else
//		if SUCCEEDED(AVInit(nSimId, nPrjId))
//		{
//			run the AdVisuo client software (remotely)
//			if FAILED(AVProcess(nPrjId))
//				AVDelete(nSimId);
//		}
//
//
// Exit codes from functions (h):
// - S_OK												success
// - S_FALSE, S_FALSE+1									success
// - OLE_S_FIRST .. OLE_S_LAST							success, application-specific information code
// - 0x40000000L+OLE_S_FIRST .. 0x40000000L+OLE_S_LAST	warning, application-specific warning code
// - OLE_E_FIRST .. OLE_E_LAST							error, application-specific error code
// - any other error code >= 0x80000000L				error, system standard HRESULT error code
// Standard SUCCEEDED and FAILED macros may be used. Warnings are considered a success.
// All exit codes other than S_OK, S_FALSE, S_FALSE+1 are registered in the system Event Log.
// For all application-specific codes, check the value of (h - OLE_S_FIRST) against the codes in messages.h 


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Versioning

// Returns module version number
// Output: version number x 100000 (for 1.09.02 the value is 10902)
ADV_API AVULONG AVGetVersion();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup diagnostic output

// Set bRegisterEventLog to register diagnostic output in the system event log. Default setting: true
// Set bPrintOnScreen to display diagnostic output on screen. Default setting: false
// Set bBenchmark to display benchmark results. Default setting: false
ADV_API HRESULT AVSetupDiagnosticOutput(bool bRegisterEventLog = true, bool bPrintOnScreen = false, bool bBenchmark = false);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
// These functions store information to the system registry and should be called only once in the system lifetime,
// or again in unlikely case the parameters change.

// Configures the database connection strings for the Console and Visualisation databases
ADV_API HRESULT AVSetConnStrings(AVSTRING pConnConsole, AVSTRING pConnReports, AVSTRING pConnVisualisation);
ADV_API HRESULT AVSetConnStrings8(char *pConnConsole, char *pConnReports, char *pConnVisualisation);

// Configures the database connection strings for the Console and Visualisation databases
// Use "%s" sequence in the pConn string: it will be replaced by appropriate database names:
// Adsimulo_Console and Adsimulo_Visualisation
ADV_API HRESULT AVSetConnString(AVSTRING pConn);
ADV_API HRESULT AVSetConnString8(char *pConn);

// Scaling factor is used to convert metric units to the units used internally by AdVisuo client.
// The default value of 0.04f will be fine in most cases.
ADV_API HRESULT AVSetScalingFactor(AVFLOAT fScale = 0.04f);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
// The proper DLL interface

// Tests for presence of the simulation data in the visualisation database.
// Input:  - nSimulationId - simulation id as used in CONSOLE database
// Return values:
// S_OK			if the visualisation data found, processed, up-to-date and ready to start
// S_FALSE		if the visualisation data not found
// S_FALSE+1	if the visualisation data found but outdated
// Standard error code in case of errors - check with FAILED(...) macro
ADV_API HRESULT AVTest(AVULONG nSimulationId);

// Initialises processing for AdVisuo visualisation.
// Loads the buidling structure and general project parameters (including the SIM file path)
// from the CONSOLE database and initialises the VISUALISATION database.
// Deletes any simulation data previously stored with this nSimulationId in VISUALISATION database.
// This function may be called early in the processing queue. It will complete fast
// even for complex projects, and allows the AdVisuo client software to be executed
// immediately after the function returns. This function should be followed by AVProcess.
// Input:  - nSimulationId - simulation id as used in CONSOLE database
//         - the CONSOLE database only
// Output: - the VISUALISATION database
//         - nProjectID - project id
// Result: - standard exit code, errors if CONSOLE data missing or corrupt
ADV_API HRESULT AVInit(AVULONG nSimulationId, AVULONG &nProjectID);

// Processes the visualisation and creates new entries in the VISUALISATION database.
// This function MUST be preceded by AVInit call.
// Using the project and building parameters pre-processed by the AVInit,
// this function loads and processes binary simulation data from the SIM file.
// Execution may take time for complex projects, but the AdVisuo client may be run in parallel.
//
// Inputs: - nProjectID - project ID as returned by AVInit
//         - the VISUALISATION database - after AVInit call
//         - the SIM binary simulation file
// Output: - the VISUALISATION database
// Result: - standard exit code, errors if VISUALISATION data missing or corrupt, or SIM file missing or corrupt.

// Project parameters including building structure and SIM file path are read directly from the Console database.
// Simulation data are read from the pSimFilePath binary file.
// Function fails if data entries in the Console database are corrupt.
// Function also fails if SIM binary data are missing or inconsistent.
ADV_API HRESULT AVProcess(AVULONG nProjectId);

// Generates the IFC file for the given simulation ID
// Uses Advisuo_Console database to acquire data
// Stores IFC data in the location provided
ADV_API HRESULT AVIFC(AVULONG nSimulationId, AVSTRING strIFCPathName);
ADV_API HRESULT AVIFC8(AVULONG nSimulationId, char *pIFCPathName);

// Deletes the simulation data from the visualisation database.
// Returns immediately if data not found.
// Input:  - nSimulationId - simulation id as used in CONSOLE database
// Returns standard error codes in case of database/connection errors - check with FAILED(...) macro
ADV_API HRESULT AVDelete(AVULONG nSimulationId);

// Deletes all data from the visualisation database
// Returns standard error codes in case of database/connection errors - check with FAILED(...) macro
ADV_API HRESULT AVDeleteAll();

// Drops tables in the VISUALISATION database - may be used to re-initialize
// Returns standard error codes in case of database/connection errors - check with FAILED(...) macro
ADV_API HRESULT AVDropTables();


//// Parameters:
// pConnStr: connection string, eg. L"Provider=SQLOLEDB;Data Source=LBDEMO\\SQLEXPRESS;Initial Catalog=Adsimulo_Visualisation;Integrated Security=SSPI;"
// nProjectId: project id, to be used later to recall data
// pBuildingXml: building information XML pathname (this is the file generated by Peter)
// pBinarySim: binary simulation data pathname (this is the file generated by Krzysztof)
// fScale: a value used to scale the building information to the dimensions used within the 3D scene; should normally be left at the default value of 0.04f.
