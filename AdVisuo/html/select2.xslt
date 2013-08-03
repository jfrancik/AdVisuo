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
        var num = 0;
        for (i = 0; i < document.all.length; i++)
            if (document.all(i).className == "SelectPrjItem")
            {
                document.all(i).onclick = function () { selectProject(this); }
                document.all(i).style.display = "block";
                num++;
            }

        // select first project
        for (i = 0; i < document.all.length; i++)
            if (document.all(i).className == "SelectPrjItem")
            {
                selectProject(document.all(i));
                break;
            }

        // convert all dates
        for (i = 0; i < document.all.length; i++)
            if (document.all(i).className == "date")
            {
      				var date = new Date(document.all(i).innerText);
			      	document.all(i).innerText = ("0" + date.getDate()).slice(-2) + "/" + ("0" + date.getMonth()).slice(-2) + "/" + date.getFullYear() + " " + ("0" + date.getHours()).slice(-2) + ":" + ("0" + date.getMinutes()).slice(-2);
			      }

        // width correction (place for the scrollbar)
        if (num > 16)
          document.getElementById("idTablePrjList").style.width="51%";
        else
          document.getElementById("idTablePrjList").style.width="50%";
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

    function selectSimulationById(simulationId)
    {
        var Objs = document.all;
        for (i = 0; i < Objs.length; i++)
        {
            tabRow = Objs(i);
            if (tabRow.className == "SelectPrjItem" || tabRow.className == "SelectPrjItem selected")
            {
                var cells = tabRow.getElementsByTagName("TD");
                var id;
                for (j = 0; j < cells.length; j++)
                {
                    if (cells[j].className == "idSimulation")
                        id = cells[j].innerHTML;
                }
                if (id == simulationId)
                {
                    selectProject(tabRow);
                    return;
                }
            }
        }
    }
]]>
    </xsl:text>
    </script>
    
    

<body class="select" onload="onLoad();">

  <div style="font-size:2em;padding:1em 0 0 1em;">Project: <xsl:value-of select="DataSet/NewDataSet/AVProject/ProjectName"/></div>
  
  <input type="submit" value="Back" name="BackButton" id="idBack" style="position:absolute; right: 56px; top:35px;" class="RoundedButton" />
  

  <div class="select-panel">

    <div id="idProjectDetails" class="SelectDetailsHeader">
      <span style="font-size:2em;">Simulation details</span>
      <input type="submit" value="Open" name="OpenButton" id="idOpenSimulation" style="position:absolute; right: 30px; top:0;" class="RoundedButton" />
      <hr />
    </div>

    <!-- list of projects -->
    <div style="position:absolute;width:850px;top:50px;left:15px;padding-top:25px;">
      <table class="SelectPrjListPanel" style="width:50%;">
        <thead>
          <tr>
            <th scope="col" style="width:110px;">
              <a id="idSortBySimDate" href="#">
                <xsl:if test="SortMode=0">
                  <xsl:attribute name="class">selected</xsl:attribute>
                  <xsl:if test="SortAsc=0">Created Date &#9650;</xsl:if>
                  <xsl:if test="SortAsc=1">Created Date &#9660;</xsl:if>
                </xsl:if>
                <xsl:if test="SortMode!=0">
                  Created Date
                </xsl:if>

              </a>
            </th>
            <th scope="col" style="width:315px;">
              <a id="idSortBySimName" href="#">
                <xsl:if test="SortMode=1">
                  <xsl:attribute name="class">selected</xsl:attribute>
                  <xsl:if test="SortAsc=0">Simulation Name &#9650;</xsl:if>
                  <xsl:if test="SortAsc=1">Simulation Name &#9660;</xsl:if>
                </xsl:if>
                <xsl:if test="SortMode!=1">
                  Simulation Name
                </xsl:if>
              </a>
            </th>
          </tr>
        </thead>
      </table>

      <div style="height: 450px;overflow:auto;">
        <table class="SelectPrjListPanel" style="width:50%;" id="idTablePrjList">
          <tbody>

            <xsl:for-each select="DataSet/NewDataSet/AVProject">
              <tr class="SelectPrjItem">

                <td>
                  <span class="date">
                    <xsl:value-of select="SimCreatedDate"/>
                  </span>
                </td>
                <td>
                  <xsl:value-of select="SimName"/>

                  <div class="SelectDetailsPanel">
                    <p>
                      <span class="label">Simulation Name</span>
                      <xsl:value-of select="SimName"/>
                    </p>
                    <xsl:if test="SimComments != 0">
                      <p>
                        <span class="label">Comments</span>
                        <xsl:value-of select="SimComments"/>
                      </p>
                    </xsl:if>
                    <hr />
                    <xsl:if test="CreatedBy != 0">
                      <p>
                        <span class="label">Created by</span>
                        <xsl:value-of select="SimCreatedBy" />
                      </p>
                    </xsl:if>
                    <xsl:if test="CreatedDate != 0">
                      <p>
                        <span class="label">Created date</span>
                        <span class="date">
                          <xsl:value-of select="SimCreatedDate" />
                        </span>
                      </p>
                    </xsl:if>
                    <xsl:if test="LastModifiedBy != 0">
                      <p>
                        <span class="label">Modified by</span>
                        <xsl:value-of select="SimLastModifiedBy" />
                      </p>
                    </xsl:if>
                    <xsl:if test="LastModifiedDate != 0">
                      <p>
                        <span class="label">Modified date</span>
                        <span class="date">
                          <xsl:value-of select="SimLastModifiedDate" />
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
      </div>
    </div>  <!-- list of projects -->

    <input type="hidden" id="idProjectId" />
    <input type="hidden" id="idSimulationId" />

  </div>
</body>

  </html>  
</xsl:template>

</xsl:stylesheet>
