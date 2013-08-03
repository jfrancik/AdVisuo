<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.1" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="html" version="5.0" encoding="utf-16" indent="yes" />

<xsl:template match="/">
	<xsl:apply-templates/>
</xsl:template>

<xsl:template match="AdVisuo-Saved-Project">

  <xsl:text disable-output-escaping='yes'>&lt;!DOCTYPE html></xsl:text>
  <html xmlns="http://www.w3.org/1999/xhtml">

    <head>
      <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
      <title>AdVisuo: Select Project</title>

      <link type="text/css" rel="stylesheet">
        <xsl:attribute name="href">
          res://<xsl:value-of select="AdVisuo-Path"/>/CSS/#341
        </xsl:attribute>
      </link>
      
    </head>

    <script type="text/javascript" language="ecmascript">
    <xsl:text disable-output-escaping="yes">
<![CDATA[
    // Forbid user's mouse selecting for the content (allow text selection in EditBox only)
    function onSelect1()
    {
        if (window.event.srcElement.tagName != "INPUT")
        {
            window.event.returnValue = false;
            window.event.cancelBubble = true;
        }
    }

    function onLoad()
    {
        // Install Context Menu
        document.onselectstart = onSelect1;

        // initialise project items
        for (i = 0; i < document.all.length; i++)
            if (document.all(i).className == "SelectPrjItem")
                document.all(i).onclick = function () { selectProject(this); }

        // initialise project folders
        for (i = 0; i < document.all.length; i++)
            if (document.all(i).className == "SelectFolder")
                document.all(i).onclick = function () { selectFolder(this); }

        // select the first folder
        for (i = 0; i < document.all.length; i++)
            if (document.all(i).className == "SelectFolder")
            {
                selectFolder(document.all(i));
                break;
            }

        // convert all dates
        for (i = 0; i < document.all.length; i++)
            if (document.all(i).className == "date")
            {
      				var date = new Date(document.all(i).innerText);
			      	document.all(i).innerText = ("0" + date.getDate()).slice(-2) + "/" + ("0" + date.getMonth()).slice(-2) + "/" + date.getFullYear() + " " + ("0" + date.getHours()).slice(-2) + ":" + ("0" + date.getMinutes()).slice(-2);
			      }
    }

    function selectFolder(a)
    {
        var folderName = a.innerText;

        // un-select all other items
        var Objs = document.all;
        for (i = 0; i < Objs.length; i++)
            if (Objs(i).className == "SelectFolder selected")
                Objs(i).className = "SelectFolder";

        // select the clicked item
        a.className = "SelectFolder selected";

        // display the folder name in the folder selector
        document.getElementById("idFolderSelector").innerText = folderName + " ▼";

        // filter projects...
        var Objs = document.all;
        var sel = null;
        var num = 0;
        for (i = 0; i < Objs.length; i++)
            if (Objs(i).className == "SelectPrjItem selected" || Objs(i).className == "SelectPrjItem")
            {
                var tabRow = Objs(i);

                // check project's folder - set display depending on its value
                var cells = tabRow.getElementsByTagName("TD");
                for (j = 0; j < cells.length; j++)
                    if (cells[j].className == "folder")
                        if (cells[j].innerHTML == folderName)
                        {
                            num++;
                            tabRow.style.display = "block";
                            if (sel == null || tabRow.className == "SelectPrjItem selected")
                                sel = tabRow;
                        }
                        else
                            tabRow.style.display = "none";
                    }

        if (num == 0)
            document.getElementById("idProjectDetails").style.display = "none";
        else
            document.getElementById("idProjectDetails").style.display = "block";

        // select the first project
        if (sel != null)
            selectProject(sel);
    }

    function selectProject(tabRow)
    {
        // un-select all other rows
        var Objs = document.all;
        for (i = 0; i < Objs.length; i++)
            if (Objs(i).style.display == "block" && Objs(i).className == "SelectPrjItem selected")
                Objs(i).className = "SelectPrjItem";

        // select the clicked row
        tabRow.className = "SelectPrjItem selected";

        // remember project id
        var cells = tabRow.getElementsByTagName("TD");
        for (i = 0; i < cells.length; i++)
        {
            if (cells[i].className == "idProject")
                document.getElementById("idProjectId").value = cells[i].innerHTML;
            if (cells[i].className == "idSimulation")
                document.getElementById("idSimulationId").value = cells[i].innerHTML;
        }
    }
]]>
    </xsl:text>
    </script>
    
    

<body class="select" onload="onLoad();">

  <div class="select-panel">

    <table style="padding-top:15px;width:85%">
      <tr>
        <td style="width:220px;">
          <div id="idSelectFolder">
            <ul>
              <li>
                <a id="idFolderSelector"></a>
                <ul>
                  <xsl:for-each select="AVFolder">
                    <li>
                      <a class="SelectFolder">
                        <xsl:value-of select="."/>
                      </a>
                    </li>  
                  </xsl:for-each>
                </ul>
              </li>
            </ul>
          </div>
        </td>
            
        <td>
          <input id="MainContentNested_txtSearchBasic" class="TxtSearchBasic" type="search" />
          <input type="submit" name="ctl00$MainContentNested$btnBasicSearch" value="Search" id="MainContentNested_btnBasicSearch" class="RoundedButton SearchButton" />
        </td>
      </tr>
    </table>

    <div id="idProjectDetails" class="SelectDetailsHeader">
      <span style="font-size:2em;">Project details</span>
      <input type="submit" value="Open" name="OpenButton" id="idOpenProject" style="position:absolute; right: 30px; top:0;" class="RoundedButton" />
      <hr />
    </div>

    <!-- list of projects -->
    <div style="position:absolute;width:850px;top:50px;left:15px;padding-top:25px;">
      <table class="SelectPrjListPanel" style="width:50%;">
        <thead>
          <tr>
            <th scope="col" style="width:110px;">
              <a class="selected" href="#">Created Date ▼</a>
            </th>
            <th scope="col">
              <a href="#">Project Name</a>
            </th>
            <th scope="col" style="width:90px;">
              <a href="#">Project No.</a>
            </th>
          </tr>
        </thead>
        <tbody>
          
          <xsl:for-each select="DataSet/NewDataSet/AVProject">
            <tr class="SelectPrjItem">
              
              <td>
                <span class="date"><xsl:value-of select="CreatedDate"/></span>
              </td>
              <td>
                <xsl:value-of select="ProjectName"/>
              </td>
              <td>
                <xsl:if test="ProjectNo != 0">
                  <xsl:value-of select="ProjectNo" />
                </xsl:if>
                
                <div class="SelectDetailsPanel">
                  <p>
                    <span class="label">Project Name</span>
                    <xsl:value-of select="ProjectName"/>
                  </p>
                  <p>
                    <span class="label">Project Id</span>
                    <xsl:value-of select="ProjectId"/>
                  </p>
                  <p>
                    <span class="label">Simulation Id</span>
                    <xsl:value-of select="SimulationId"/>
                  </p>
                  <xsl:if test="ProjectNo != 0">
                    <p>
                      <span class="label">Project No.</span>
                      <xsl:value-of select="ProjectNo" />
                    </p>
                  </xsl:if>
                  <xsl:if test="LiftDesigner != 0">
                    <p>
                      <span class="label">Lift designer</span>
                      <xsl:value-of select="LiftDesigner" />
                    </p>
                  </xsl:if>
                  <xsl:if test="CheckedBy != 0">
                    <p>
                      <span class="label">Checked By</span>
                      <xsl:value-of select="CheckedBy" />
                    </p>
                  </xsl:if>
                  <xsl:if test="ClientCompanyName != 0">
                    <p>
                      <span class="label">Client Name</span>
                      <xsl:value-of select="ClientCompanyName" />
                    </p>
                  </xsl:if>
                  <xsl:if test="BuildingName != 0">
                    <p>
                      <span class="label">Building Name</span>
                      <xsl:value-of select="BuildingName" />
                    </p>
                  </xsl:if>
                  <xsl:if test="City != 0">
                    <p>
                      <span class="label">City</span>
                      <xsl:value-of select="City" />
                    </p>
                  </xsl:if>
                  <xsl:if test="StateCounty != 0">
                    <p>
                      <span class="label">State/County</span>
                      <xsl:value-of select="StateCounty" />
                    </p>
                  </xsl:if>
                  <xsl:if test="Country != 0">
                    <p>
                      <span class="label">Country</span>
                      <xsl:value-of select="Country" />
                    </p>
                  </xsl:if>
                  <xsl:if test="LBRegionDistrict != 0">
                    <p>
                      <span class="label">LB Region/District</span>
                      <xsl:value-of select="LBRegionDistrict" />
                    </p>
                  </xsl:if>
                  <xsl:if test="PostalZipCode != 0">
                    <p>
                      <span class="label">Postal/Zip Code</span>
                      <xsl:value-of select="PostalZipCode" />
                    </p>
                  </xsl:if>
                  <hr />
                  <xsl:if test="CreatedBy != 0">
                    <p>
                      <span class="label">Created by</span>
                      <xsl:value-of select="CreatedBy" />
                    </p>
                  </xsl:if>
                  <xsl:if test="CreatedDate != 0">
                    <p>
                      <span class="label">Created date</span>
                      <span class="date">
                        <xsl:value-of select="CreatedDate" />
                      </span>
                    </p>
                  </xsl:if>
                  <xsl:if test="LastModifiedBy != 0">
                    <p>
                      <span class="label">Modified by</span>
                      <xsl:value-of select="LastModifiedBy" />
                    </p>
                  </xsl:if>
                  <xsl:if test="LastModifiedDate != 0">
                    <p>
                      <span class="label">Modified date</span>
                      <span class="date">
                        <xsl:value-of select="LastModifiedDate" />
                      </span>
                    </p>
                  </xsl:if>
                </div>
              </td>
              <td class="idProject">
                <xsl:value-of select="ProjectId"/>
              </td>
              <td class="idSimulation">
                <xsl:value-of select="SimulationId"/>
              </td>
              <td class="folder">
                <xsl:value-of select="ProjectFolderName"/>
              </td>
                
            </tr>
          </xsl:for-each>
          
        </tbody>
      </table>
    </div>  <!-- list of projects -->

    <input type="text" id="idProjectId" />
    <input type="text" id="idSimulationId" />

  </div>
</body>

  </html>  
</xsl:template>

</xsl:stylesheet>
