Dim shell, systempath
set shell = WScript.CreateObject( "WScript.Shell" )
systempath = shell.ExpandEnvironmentStrings("%SystemRoot%")
shell.Run Chr(34) & systempath & "\system32\msiexec.exe" & Chr(34) & "  /x {B176910A-E097-41CA-9615-6BA7DFAE979F}"
WScript.Quit
