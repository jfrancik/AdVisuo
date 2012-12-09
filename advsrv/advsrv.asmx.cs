using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Services;
using System.Data;
using System.Data.OleDb;
using Microsoft.Win32;

namespace advsrv
{
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
        //static string strConn = "Provider=SQLOLEDB;Data Source=KATEPC\\SQLEXPRESS;Initial Catalog=Adsimulo_Visualisation;Integrated Security=SSPI;";

        private string GetConnStr()
        {
            RegistryKey ourKey = Registry.LocalMachine.OpenSubKey("Software\\LerchBates\\AdVisuo\\ServerModule");
            string str = ourKey.GetValue("VisualisationConnectionString").ToString();
            return str;
        }

        [WebMethod(Description = "Returns the Connection String.")]
        public string AVConnStr()
        {
            return GetConnStr();
        }
        
        [WebMethod(Description = "Returns system version.")]
        public int AVVersion()
        {
            return 110;
        }

        [WebMethod(Description = "Returns project index.")]
        public DataSet AVIndex()
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVProjects", conn);
            myDataAdapter.Fill(myDataSet, "AVProject");
            return myDataSet;
        }

        [WebMethod(Description = "Returns project information")]
        public DataSet AVProject(int nSimulationId)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVProjects WHERE SimulationId=" + nSimulationId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVProject");
            return myDataSet;
        }

        [WebMethod(Description = "Returns lift group structure")]
        public DataSet AVLiftGroups(int nProjectId)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVLiftGroups WHERE ProjectID=" + nProjectId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVLiftGroup");
            return myDataSet;
        }
        
        [WebMethod(Description = "Returns floor structure for a lift group")]
        public DataSet AVFloors(int nLiftGroupId)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVFloors WHERE LiftGroupId=" + nLiftGroupId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVFloor");
            return myDataSet;
        }

        [WebMethod(Description = "Returns shaft structure for a lift group")]
        public DataSet AVShafts(int nLiftGroupId)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVShafts WHERE LiftGroupId=" + nLiftGroupId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVShaft");
            return myDataSet;
        }

        [WebMethod(Description = "Returns SIM package information")]
        public DataSet AVSim(int nLiftGroupId)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVSims WHERE LiftGroupId=" + nLiftGroupId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVSim");
            return myDataSet;
        }

        [WebMethod(Description = "Returns simulation data")]
        public DataSet[] AVSimData(int nSimId, int timeFrom, int timeTo)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet[] myDataSets = new DataSet[2];
            OleDbDataAdapter myDataAdapter;

            if (timeFrom == 0)
            {
                // the journeys
                myDataSets[0] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVJourneys WHERE SimID=" + nSimId + " AND TimeGo < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[0], "AVJourney");

                // the passengers
                myDataSets[1] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVPassengers WHERE SimID=" + nSimId + " AND TimeBorn < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[1], "AVPassenger");
            }
            else
            {
                // the journeys
                myDataSets[0] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVJourneys WHERE SimID=" + nSimId + " AND TimeGo >= " + timeFrom + "  AND TimeGo < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[0], "AVJourney");

                // the passengers
                myDataSets[1] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVPassengers WHERE SimID=" + nSimId + " AND TimeBorn >= " + timeFrom + " AND TimeBorn < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[1], "AVPassenger");
            }

            return myDataSets;
        }

        [WebMethod(Description = "Returns simulation data for all sims/groups within a project")]
        public DataSet[] AVPrjData(int nProjectId, int timeFrom, int timeTo)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet[] myDataSets = new DataSet[2];
            OleDbDataAdapter myDataAdapter;

            string prjins = "SELECT s.ID FROM AVSims s, AVLiftGroups g WHERE s.LiftGroupId = g.ID AND g.ProjectId = " + nProjectId;

            if (timeFrom == 0)
            {
                // the journeys
                myDataSets[0] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVJourneys WHERE SimID IN (" + prjins + ") AND TimeGo < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[0], "AVJourney");

                // the passengers
                myDataSets[1] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVPassengers WHERE SimID IN (" + prjins + ") AND TimeBorn < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[1], "AVPassenger");
            }
            else
            {
                // the journeys
                myDataSets[0] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVJourneys WHERE SimID IN (" + prjins + ") AND TimeGo >= " + timeFrom + "  AND TimeGo < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[0], "AVJourney");

                // the passengers
                myDataSets[1] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVPassengers WHERE SimID IN (" + prjins + ") AND TimeBorn >= " + timeFrom + " AND TimeBorn < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[1], "AVPassenger");
            }

            return myDataSets;
        }

    }
}