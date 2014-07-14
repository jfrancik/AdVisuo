Dim shell, systempath
set shell = WScript.CreateObject( "WScript.Shell" )
systempath = shell.ExpandEnvironmentStrings("%SystemRoot%")
shell.Run Chr(34) & systempath & "\system32\msiexec.exe" & Chr(34) & "  /x {1E30640F-CB8A-47CF-BBE2-4F07A9E930FE}"
WScript.Quit
