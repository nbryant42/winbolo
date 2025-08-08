# Microsoft Developer Studio Project File - Name="StdAutoPilot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=StdAutoPilot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Ryan.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Ryan.mak" CFG="StdAutoPilot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StdAutoPilot - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "StdAutoPilot - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Projects/boloBrains/StdAutoPilot", KYDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StdAutoPilot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STDAUTOPILOT_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /machine:I386 /out:"Release/Ryan2.brn"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=install.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "StdAutoPilot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STDAUTOPILOT_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /debug /machine:I386 /out:"Debug/Ryan.brn"

!ENDIF 

# Begin Target

# Name "StdAutoPilot - Win32 Release"
# Name "StdAutoPilot - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\actionqueue.c
# End Source File
# Begin Source File

SOURCE=.\attackpill.c
# End Source File
# Begin Source File

SOURCE=.\attacktank.c
# End Source File
# Begin Source File

SOURCE=.\baseguard.c
# End Source File
# Begin Source File

SOURCE=.\BRAIN.RC
# End Source File
# Begin Source File

SOURCE=.\getman.c
# End Source File
# Begin Source File

SOURCE=.\gettrees.c
# End Source File
# Begin Source File

SOURCE=.\heap.c
# End Source File
# Begin Source File

SOURCE=.\issort.c
# End Source File
# Begin Source File

SOURCE=.\laypill.c
# End Source File
# Begin Source File

SOURCE=.\pickup.c
# End Source File
# Begin Source File

SOURCE=.\pointtank.c
# End Source File
# Begin Source File

SOURCE=.\qksort.c
# End Source File
# Begin Source File

SOURCE=.\refuel.c
# End Source File
# Begin Source File

SOURCE=.\repair.c
# End Source File
# Begin Source File

SOURCE=.\Ryan.c
# End Source File
# Begin Source File

SOURCE=.\scout.c
# End Source File
# Begin Source File

SOURCE=.\setgui.c
# End Source File
# Begin Source File

SOURCE=.\settings.c
# End Source File
# Begin Source File

SOURCE=.\travelto.c
# End Source File
# Begin Source File

SOURCE=.\update.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\actionqueue.h
# End Source File
# Begin Source File

SOURCE=.\attackpill.h
# End Source File
# Begin Source File

SOURCE=.\attacktank.h
# End Source File
# Begin Source File

SOURCE=.\baseguard.h
# End Source File
# Begin Source File

SOURCE=.\Brain.h
# End Source File
# Begin Source File

SOURCE=.\brain_win.h
# End Source File
# Begin Source File

SOURCE=.\getman.h
# End Source File
# Begin Source File

SOURCE=.\gettrees.h
# End Source File
# Begin Source File

SOURCE=.\heap.h
# End Source File
# Begin Source File

SOURCE=.\laypill.h
# End Source File
# Begin Source File

SOURCE=.\pickup.h
# End Source File
# Begin Source File

SOURCE=.\pointtank.h
# End Source File
# Begin Source File

SOURCE=.\pqueue.h
# End Source File
# Begin Source File

SOURCE=.\refuel.h
# End Source File
# Begin Source File

SOURCE=.\repair.h
# End Source File
# Begin Source File

SOURCE=.\Ryan.h
# End Source File
# Begin Source File

SOURCE=.\scout.h
# End Source File
# Begin Source File

SOURCE=.\setgui.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\sort.h
# End Source File
# Begin Source File

SOURCE=.\travelto.h
# End Source File
# Begin Source File

SOURCE=.\update.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\mvc-020s.bmp"
# End Source File
# Begin Source File

SOURCE=".\mvc-030s.bmp"
# End Source File
# Begin Source File

SOURCE=".\mvc-032s.bmp"
# End Source File
# End Group
# End Target
# End Project
