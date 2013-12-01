/****** Script for SelectTopNRows command from SSMS  ******/
SELECT 
       s.SimulationId
      ,f.Name AS Folder
      ,p.ProjectName AS Project
      ,s.Name AS Simulation
      ,p.ProjectId
      ,s.Phase
  FROM [Adsimulo_Console].[dbo].[Simulations] s, [Adsimulo_Console].[dbo].[Projects] p, [Adsimulo_Console].[dbo].[ProjectFolders] f
  WHERE p.ProjectFolderId = f.ProjectFolderId AND s.ProjectId = p.ProjectId AND s.SimulationId IN 
		(SELECT SimulationId FROM [Adsimulo_Visualisation].[dbo].[AVProjects])
