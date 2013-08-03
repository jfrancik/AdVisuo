Dim shell, systempath
set shell = WScript.CreateObject( "WScript.Shell" )
systempath = shell.ExpandEnvironmentStrings("%SystemRoot%")
shell.Run Chr(34) & systempath & "\system32\msiexec.exe" & Chr(34) & "  /x {D7B206A5-7CE8-49A9-8F3D-E7EABDAFE56A}"
WScript.Quit
