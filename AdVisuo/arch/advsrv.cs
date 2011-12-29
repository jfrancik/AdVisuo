using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Services;
using System.Data;
using System.Data.OleDb;

//namespace advsrv
//{
	/// <summary>
	/// Summary description for Service1
	/// </summary>
	[WebService(Namespace = "http://tempuri.org/")]
	[WebServiceBinding(ConformsTo = WsiProfiles.BasicProfile1_1)]
	[System.ComponentModel.ToolboxItem(false)]
	// To allow this Web Service to be called from script, using ASP.NET AJAX, uncomment the following line. 
	// [System.Web.Script.Services.ScriptService]
	public class AdVServer : System.Web.Services.WebService
	{

        static string strConn = "Provider=SQLOLEDB;Data Source=LBDEMO\\SQLEXPRESS;Initial Catalog=Adsimulo_Visualisation;Integrated Security=SSPI;";

		string[,] cmdString = {
			{"AVProjects",			"SELECT * FROM AVProjects WHERE ID=", " ORDER BY ID"},
			{"AVBuildings",			"SELECT b.* FROM AVBuildings b, AVProjects p WHERE b.ID=p.BuildingID AND p.ID=", " ORDER BY ID"},
			{"AVShafts",			"SELECT l.* FROM AVShafts l, AVProjects p WHERE l.BuildingID=p.BuildingID AND p.ID=", " ORDER BY ShaftID"},
			{"AVFloors",			"SELECT f.* FROM AVFloors f, AVProjects p WHERE f.BuildingID=p.BuildingID AND p.ID=", " ORDER BY FloorID"},
			{"AVJourneys",			"SELECT * FROM AVJourneys WHERE ProjectID=", " ORDER BY ID"},
			{"AVJourneyDoorCycles",	"SELECT d.* FROM AVJourneyDoorCycles d, AVJourneys j WHERE d.JourneyID=j.ID AND j.ProjectID=", " ORDER BY d.ID"},
			{"AVPassengers",		"SELECT * FROM AVPassengers WHERE ProjectID=", " ORDER BY ID"},
			{"AVPassengerWayPoints","SELECT w.* FROM AVPassengerWayPoints w, AVPassengers p WHERE w.PassengerID=p.ID AND p.ProjectID=", " ORDER BY w.ID"},
		};
        string[,] cmdString2 = {
			{"AVProjects",			"SELECT * FROM AVProjects WHERE ID=", " ORDER BY ID"},
			{"AVBuildings",			"SELECT b.* FROM AVBuildings b, AVProjects p WHERE b.ID=p.BuildingID AND p.ID=", " ORDER BY ID"},
			{"AVShafts",			"SELECT l.* FROM AVShafts l, AVProjects p WHERE l.BuildingID=p.BuildingID AND p.ID=", " ORDER BY ShaftID"},
			{"AVFloors",			"SELECT f.* FROM AVFloors f, AVProjects p WHERE f.BuildingID=p.BuildingID AND p.ID=", " ORDER BY FloorID"},
			{"AVJourneys",			"SELECT * FROM AVJourneys WHERE TimeGo<50000 AND ProjectID=", " ORDER BY ID"},
			{"AVJourneyDoorCycles",	"SELECT d.* FROM AVJourneyDoorCycles d, AVJourneys j WHERE d.JourneyID=j.ID AND j.TimeGo<50000 AND j.ProjectID=", " ORDER BY d.ID"},
			{"AVPassengers",		"SELECT * FROM AVPassengers WHERE TimeBorn<50000 AND ProjectID=", " ORDER BY ID"},
			{"AVPassengerWayPoints","SELECT w.* FROM AVPassengerWayPoints w, AVPassengers p WHERE w.PassengerID=p.ID AND p.TimeBorn<50000 AND p.ProjectID=", " ORDER BY w.ID"},
		};

		[WebMethod(Description="Returns combined datasets for entire AdVisuo project.")]
		public DataSet[] GetAVProject(int id)
		{
			OleDbConnection conn = new OleDbConnection(strConn);
			DataSet[] myDataSets = new DataSet[cmdString.Length / 3];

			for (int i = 0; i < myDataSets.Length; i++)
			{
				myDataSets[i] = new DataSet();
				OleDbDataAdapter myDataAdapter = new OleDbDataAdapter(cmdString[i, 1] + id + cmdString[i, 2], conn);
				myDataAdapter.Fill(myDataSets[i], cmdString[i, 0]);
			}
			return myDataSets;
		}
        [WebMethod(Description = "Returns combined datasets for a time slice of AdVisuo project.")]
        public DataSet[] GetAVProject2(int id)
        {
            OleDbConnection conn = new OleDbConnection(strConn);
            DataSet[] myDataSets = new DataSet[cmdString2.Length / 3];

            for (int i = 0; i < myDataSets.Length; i++)
            {
                myDataSets[i] = new DataSet();
                OleDbDataAdapter myDataAdapter = new OleDbDataAdapter(cmdString2[i, 1] + id + cmdString2[i, 2], conn);
                myDataAdapter.Fill(myDataSets[i], cmdString2[i, 0]);
            }
            return myDataSets;
        }
    }
//}