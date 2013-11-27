Dim shell, systempath
set shell = WScript.CreateObject( "WScript.Shell" )
systempath = shell.ExpandEnvironmentStrings("%SystemRoot%")
shell.Run Chr(34) & systempath & "\system32\msiexec.exe" & Chr(34) & "  /x {7A3F3830-6FEF-46E2-B7F5-A3C2C13E2C4E}"
WScript.Quit
