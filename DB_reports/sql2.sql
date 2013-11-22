SELECT TOP 1000 [HallCallId]
      ,[ArrivalFloor]
      ,[DestinationFloor]
      ,[LiftId]
      ,[ArrivalTime]
      ,[StartLoading]
      ,[StartUnloading]
      ,[CarArrivalAtArrivalFloor]
      ,[CarArrivalAtDestinationFloor]
      ,[DepartureTimeFromArrivalFloor]
  FROM [Adsimulo_Reports].[dbo].[HallCalls]
  WHERE [TraffiicScenarioId] = 745 AND [LiftId] = 4034 AND [Iteration] = 0
  AND [StartLoading] BETWEEN  3200 AND 3440
  ORDER BY [StartLoading]
