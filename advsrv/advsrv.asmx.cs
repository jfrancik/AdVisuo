﻿using System;
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

        string[,] cmdString = {
		    {"AVProject",	"SELECT * FROM AVProjects WHERE SimulationID=", " ORDER BY ID"},
		    {"AVBuilding",	"SELECT b.* FROM AVBuildings b, AVProjects p WHERE b.ProjectID=p.ID AND p.SimulationID=", " ORDER BY ID"},
		    {"AVShaft",		"SELECT s.* FROM AVShafts s, AVBuildings b, AVProjects p WHERE s.BuildingID=b.ID AND b.ProjectID=p.ID AND p.SimulationID=", " ORDER BY ShaftID"},
		    {"AVFloor",		"SELECT f.* FROM AVFloors f, AVBuildings b, AVProjects p WHERE f.BuildingID=b.ID AND b.ProjectID=p.ID AND p.SimulationID=", " ORDER BY FloorID"},
		    {"AVJourney",	"SELECT j.* FROM AVJourneys j, AVProjects p WHERE j.ProjectID = p.ID AND p.SimulationID=", " ORDER BY j.ID"},
		    {"AVPassenger",	"SELECT x.* FROM AVPassengers x, AVProjects p WHERE x.ProjectID = p.ID AND p.SimulationID=", " ORDER BY x.ID"},
	    };

        [WebMethod(Description = "Obsolete function - do not use!.")]
        public DataSet[] GetAVProject(int id)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet[] myDataSets = new DataSet[cmdString.Length / 3];

            for (int i = 0; i < myDataSets.Length; i++)
            {
                myDataSets[i] = new DataSet();
                OleDbDataAdapter myDataAdapter = new OleDbDataAdapter(cmdString[i, 1] + id + cmdString[i, 2], conn);
                myDataAdapter.Fill(myDataSets[i], cmdString[i, 0]);
            }
            return myDataSets;
        }

        [WebMethod(Description = "Returns all project index.")]
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
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVProjects WHERE SimulationID=" + nSimulationId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVProject");
            return myDataSet;
        }

        [WebMethod(Description = "Returns building structure")]
        public DataSet[] AVBuilding(int nProjectId)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet[] myDataSets = new DataSet[3];
            OleDbDataAdapter myDataAdapter;

            // the building
            myDataSets[0] = new DataSet();
            myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVBuildings WHERE ProjectID=" + nProjectId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSets[0], "AVBuilding");

            // the floors
            myDataSets[1] = new DataSet();
            myDataAdapter = new OleDbDataAdapter("SELECT f.* FROM AVFloors f, AVBuildings b WHERE f.BuildingID=b.ID AND b.ProjectID=" + nProjectId + " ORDER BY FloorID", conn);
            myDataAdapter.Fill(myDataSets[1], "AVFloor");

            // the shafts
            myDataSets[2] = new DataSet();
            myDataAdapter = new OleDbDataAdapter("SELECT s.* FROM AVShafts s, AVBuildings b WHERE s.BuildingID=b.ID AND b.ProjectID=" + nProjectId + " ORDER BY ShaftID", conn);
            myDataAdapter.Fill(myDataSets[2], "AVShaft");

            return myDataSets;
        }

        [WebMethod(Description = "Returns simulation data")]
        public DataSet[] AVSim(int nProjectId, int timeFrom, int timeTo)
        {
            OleDbConnection conn = new OleDbConnection(GetConnStr());
            DataSet[] myDataSets = new DataSet[2];
            OleDbDataAdapter myDataAdapter;

            if (timeFrom == 0)
            {
                // the journeys
                myDataSets[0] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVJourneys WHERE ProjectID=" + nProjectId + " AND TimeGo < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[0], "AVJourney");

                // the passengers
                myDataSets[1] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVPassengers WHERE ProjectID=" + nProjectId + " AND TimeBorn < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[1], "AVPassenger");
            }
            else
            {
                // the journeys
                myDataSets[0] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVJourneys WHERE ProjectID=" + nProjectId + " AND TimeGo >= " + timeFrom + "  AND TimeGo < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[0], "AVJourney");

                // the passengers
                myDataSets[1] = new DataSet();
                myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVPassengers WHERE ProjectID=" + nProjectId + " AND TimeBorn >= " + timeFrom + " AND TimeBorn < " + timeTo + " ORDER BY ID", conn);
                myDataAdapter.Fill(myDataSets[1], "AVPassenger");
            }

            return myDataSets;
        }

    }
}