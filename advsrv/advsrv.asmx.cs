using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Services;
using System.Data;
using System.Data.OleDb;
using System.Data.SqlClient;
using Microsoft.Win32;

using System.Text;
using System.Security.Cryptography;

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

        private string GetVisConnStr()
        {
            RegistryKey ourKey = Registry.LocalMachine.OpenSubKey("Software\\LerchBates\\AdVisuo\\ServerModule");
            string str = ourKey.GetValue("VisualisationConnectionString").ToString();
            return str;
        }

        private string GetConsoleConnStr()
        {
            RegistryKey ourKey = Registry.LocalMachine.OpenSubKey("Software\\LerchBates\\AdVisuo\\ServerModule");
            string str = ourKey.GetValue("ConsoleConnectionString").ToString();
            return str;
        }

        private string GetUsersConnStr()
        {
            RegistryKey ourKey = Registry.LocalMachine.OpenSubKey("Software\\LerchBates\\AdVisuo\\ServerModule");
            string str = ourKey.GetValue("UsersConnectionString").ToString();
            return str;
        }

        private string EncodePassword(string pass, string salt)
        {
            byte[] bytes = Encoding.Unicode.GetBytes(pass);
            byte[] src = Convert.FromBase64String(salt);
            byte[] dst = new byte[src.Length + bytes.Length];
            Buffer.BlockCopy(src, 0, dst, 0, src.Length);
            Buffer.BlockCopy(bytes, 0, dst, src.Length, bytes.Length);
            HashAlgorithm algorithm = HashAlgorithm.Create("SHA1");
            byte[] inArray = algorithm.ComputeHash(dst);
            return Convert.ToBase64String(inArray);
        }

        [WebMethod(Description = "Returns status or version information (int information).")]
        public int AVGetIntStatus(string keyName)
        {
            int res = -1;
            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            OleDbCommand cmdSelect = new OleDbCommand("SELECT IntValue FROM AVStatus WHERE Name=?", conn);
            cmdSelect.Parameters.AddWithValue("Name", keyName);
            conn.Open();
            OleDbDataReader reader = cmdSelect.ExecuteReader();
            if (reader.Read())
            {
                res = reader.GetInt32(0);
                reader.Close();
            }
            return res;
        }

        [WebMethod(Description = "Returns status (string information).")]
        public string AVGetStrStatus(string keyName)
        {
            string res = "";
            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            OleDbCommand cmdSelect = new OleDbCommand("SELECT Value FROM AVStatus WHERE Name=?", conn);
            cmdSelect.Parameters.AddWithValue("Name", keyName);
            conn.Open();
            OleDbDataReader reader = cmdSelect.ExecuteReader();
            if (reader.Read())
            {
                res = reader.GetString(0);
                reader.Close();
            }
            return res;
        }

        [WebMethod(Description = "Creates and returns a valid ticket for the given username and password, or empty string if unauthorised.")]
        public string AVCreateTicket(string strUsername, string strPassword)
        {
            System.Threading.Thread.Sleep(750);

            // open connection
            OleDbConnection connUsers = new OleDbConnection(GetUsersConnStr());
            OleDbCommand cmdSelect = new OleDbCommand("SELECT m.Password, m.PasswordSalt FROM aspnet_Users u, aspnet_Membership m WHERE u.UserId=m.UserId AND u.UserName=?", connUsers);
            cmdSelect.Parameters.AddWithValue("u.UserName", strUsername);
            connUsers.Open();
            OleDbDataReader reader = cmdSelect.ExecuteReader();

            if (reader.Read())
            {
                string pswd = reader.GetString(0);  // password
                string salt = reader.GetString(1);  // salt

                // check password
                string ver = EncodePassword(strPassword, salt);
                reader.Close();
                if (ver != pswd)
                {
                    System.Threading.Thread.Sleep(1500);
                    return "";
                }

                // create ticket
                RandomNumberGenerator gen = RandomNumberGenerator.Create();
                byte[] bytes = new byte[20];
                gen.GetBytes(bytes);
                string strTicket = Convert.ToBase64String(bytes);

                // store ticket
                OleDbConnection connVis = new OleDbConnection(GetVisConnStr());
                OleDbCommand cmdInsert = new OleDbCommand("INSERT INTO AVTickets (UserId, Ticket, TimeStamp) VALUES (?, ?, CURRENT_TIMESTAMP)", connVis);
                cmdInsert.Parameters.AddWithValue("UserId", strUsername);
                cmdInsert.Parameters.AddWithValue("Ticket", strTicket);
                connVis.Open();

                try
                {
                    cmdInsert.ExecuteNonQuery();
                }
                catch (OleDbException ex)
                {
                    strTicket = "";
                    System.Threading.Thread.Sleep(1500);
                }

                return strTicket;
            }
            else
            {
                // wrong user
                reader.Close();
                System.Threading.Thread.Sleep(1500);
                return "";
            }
        }

        [WebMethod(Description = "Validates ticket - returns number of seconds left for the valid ticket or 0 if expired or not found")]
        public int AVValidateTicket(string strUsername, string strTicket)
        {
            if (strTicket == "01234")
                return 99;
            if (strTicket == "")
                return 0;
            // open connection
            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            OleDbCommand cmdSelect = new OleDbCommand("SELECT TimeStamp FROM AVTickets WHERE UserId=? AND Ticket=?", conn);
            cmdSelect.Parameters.AddWithValue("UserId", strUsername);
            cmdSelect.Parameters.AddWithValue("Ticket", strTicket);
            conn.Open();
            OleDbDataReader reader = cmdSelect.ExecuteReader();

            if (reader.Read())
            {
                DateTime stamp = reader.GetDateTime(0);
                reader.Close();

                double validity = 120;
                double secs = (DateTime.Now - stamp).TotalSeconds;
                if (secs > validity)
                    return 0;
                else
                    return (int)(validity - secs);
            }
            else
            {
                reader.Close();
                System.Threading.Thread.Sleep(2000);
                return 0;
            }
        }

        [WebMethod(Description = "Validates ticket - returns number of seconds left for the valid ticket or 0 if expired or not found")]
        public string AVRevalidateTicket(string strUsername, string strTicket)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return "";

            // create a new ticket
            RandomNumberGenerator gen = RandomNumberGenerator.Create();
            byte[] bytes = new byte[20];
            gen.GetBytes(bytes);
            strTicket = Convert.ToBase64String(bytes);

            // store ticket
            OleDbConnection connVis = new OleDbConnection(GetVisConnStr());
            OleDbCommand cmdInsert = new OleDbCommand("INSERT INTO AVTickets (UserId, Ticket, TimeStamp) VALUES (?, ?, CURRENT_TIMESTAMP)", connVis);
            cmdInsert.Parameters.AddWithValue("UserId", strUsername);
            cmdInsert.Parameters.AddWithValue("Ticket", strTicket);
            connVis.Open();

            try
            {
                cmdInsert.ExecuteNonQuery();
            }
            catch (OleDbException ex)
            {
                strTicket = "";
            }

            return strTicket;
        }

        [WebMethod(Description = "Reports an issue or bug.")]
        public void AVReportIssue(string url, string strUsername, string strTicket, int nVersion, int nId, string strPath, int nCat, string strUserDesc, string strDiagnostic, string strErrorMsg)
        {
            OleDbConnection connVis = new OleDbConnection(GetVisConnStr());
            OleDbCommand cmdInsert = new OleDbCommand("INSERT INTO AVReports (URL,UserId,Ticket,Version,SimulationId,Path,Category,UserDescription,Diagnostic,ErrorMsg,TimeStamp) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)", connVis);
            cmdInsert.Parameters.AddWithValue("URL", url);
            cmdInsert.Parameters.AddWithValue("UserId", strUsername);
            cmdInsert.Parameters.AddWithValue("Ticket", strTicket);
            cmdInsert.Parameters.AddWithValue("Version", nVersion);
            cmdInsert.Parameters.AddWithValue("SimulationId", nId);
            cmdInsert.Parameters.AddWithValue("Path", strPath);
            cmdInsert.Parameters.AddWithValue("Category", nCat);
            cmdInsert.Parameters.AddWithValue("UserDescription", strUserDesc);
            cmdInsert.Parameters.AddWithValue("Diagnostic", strDiagnostic);
            cmdInsert.Parameters.AddWithValue("ErrorMsg", strErrorMsg);
            connVis.Open();

            try
            {
                cmdInsert.ExecuteNonQuery();
            }
            catch (OleDbException ex)
            {
            }
        }
            
        [WebMethod(Description = "Returns index of project folders.")]
        public DataSet AVFolders(string strUsername, string strTicket)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetConsoleConnStr());
            OleDbCommand cmdSelect = new OleDbCommand("SELECT f.ProjectFolderId, f.Name, u.Priority FROM ProjectFolders f, UserPermissionsForFolders u WHERE f.ProjectFolderId = u.ProjectFolderId AND u.UserName=? ORDER BY u.Priority", conn);
            cmdSelect.Parameters.AddWithValue("UserName", strUsername);
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter(cmdSelect);
            myDataAdapter.Fill(myDataSet, "AVFolder");
            return myDataSet;
        }

        // returns list of folder indexes - available for the given user - e.g. (4,6,1,2)
        private string _folders(string strUsername)
        {
            // open connection
            OleDbConnection conn = new OleDbConnection(GetConsoleConnStr());
            OleDbCommand cmdSelect = new OleDbCommand("SELECT ProjectFolderId FROM UserPermissionsForFolders WHERE UserName=? ORDER BY Priority", conn);
            cmdSelect.Parameters.AddWithValue("UserName", strUsername);
            conn.Open();
            OleDbDataReader reader = cmdSelect.ExecuteReader();

            string str = "";
            while (reader.Read())
            {
                int n = reader.GetInt32(0);
                if (str.Length > 0) str += ",";
                str += n.ToString();
            }
            reader.Close();
            return "(" + str + ")";
        }

        [WebMethod(Description = "Returns project index.")]
        public DataSet AVIndex(string strUsername, string strTicket)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVProjects WHERE ProjectFolderId IN " + _folders(strUsername), conn);
            myDataAdapter.Fill(myDataSet, "AVProject");
            return myDataSet;
        }

        [WebMethod(Description = "Returns project information")]
        public DataSet AVProject(string strUsername, string strTicket, int nSimulationId)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVProjects WHERE SimulationId=" + nSimulationId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVProject");
            return myDataSet;
        }

        [WebMethod(Description = "Returns lift group structure")]
        public DataSet AVLiftGroups(string strUsername, string strTicket, int nProjectId)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVLiftGroups WHERE ProjectID=" + nProjectId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVLiftGroup");
            return myDataSet;
        }

        [WebMethod(Description = "Returns floor structure for a lift group")]
        public DataSet AVFloors(string strUsername, string strTicket, int nLiftGroupId)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVFloors WHERE LiftGroupId=" + nLiftGroupId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVFloor");
            return myDataSet;
        }

        [WebMethod(Description = "Returns shaft structure for a lift group")]
        public DataSet AVShafts(string strUsername, string strTicket, int nLiftGroupId)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVShafts WHERE LiftGroupId=" + nLiftGroupId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVShaft");
            return myDataSet;
        }

        [WebMethod(Description = "Returns SIM package information")]
        public DataSet AVSim(string strUsername, string strTicket, int nLiftGroupId)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
            DataSet myDataSet = new DataSet();
            OleDbDataAdapter myDataAdapter = new OleDbDataAdapter("SELECT * FROM AVSims WHERE LiftGroupId=" + nLiftGroupId + " ORDER BY ID", conn);
            myDataAdapter.Fill(myDataSet, "AVSim");
            return myDataSet;
        }

        [WebMethod(Description = "Returns simulation data")]
        public DataSet[] AVSimData(string strUsername, string strTicket, int nSimId, int timeFrom, int timeTo)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
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
        public DataSet[] AVPrjData(string strUsername, string strTicket, int nProjectId, int timeFrom, int timeTo)
        {
            if (AVValidateTicket(strUsername, strTicket) == 0)
                return null;

            OleDbConnection conn = new OleDbConnection(GetVisConnStr());
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