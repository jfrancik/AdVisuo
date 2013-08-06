Dim shell, systempath
set shell = WScript.CreateObject( "WScript.Shell" )
systempath = shell.ExpandEnvironmentStrings("%SystemRoot%")
shell.Run Chr(34) & systempath & "\system32\msiexec.exe" & Chr(34) & "  /x {8439C207-4488-4130-8AE3-B59C74097D87}"
WScript.Quit
