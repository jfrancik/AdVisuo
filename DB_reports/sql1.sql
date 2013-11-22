SELECT l.LiftLogId
		,l.LiftId
      ,l.Time
      ,l.CurrentFloor
      ,l.DestinationFloor
      ,s.Name AS LiftState
  FROM [Adsimulo_Reports].[dbo].[LiftLogs] l, [Adsimulo_Reports].[dbo].[LiftStates] s
  WHERE l.LiftStateId = s.LiftStateId
  AND l.TrafficScenarioId = 745 AND l.LiftId = 4034 AND l.Iteration = 0
  AND l.Time BETWEEN 3400 AND 3420
  ORDER BY l.Time
