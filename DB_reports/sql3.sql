SELECT TOP 1000 l.DeckLogId
      ,l.Time
      ,l.Deck
      ,s.Name AS DoorState
      ,l.NumberOfPassengers
  FROM [Adsimulo_Reports].[dbo].[DeckLogs] l, [Adsimulo_Reports].[dbo].[DoorStates] s
  WHERE l.DoorStateId = s.DoorStateId
  AND l.TrafficScenarioId = 745 AND l.LiftId = 4034 AND l.Iteration = 0
  AND l.Time BETWEEN 3400 AND 3420
  ORDER BY l.Time
