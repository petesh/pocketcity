# Microsoft Developer Studio Project File - Name="pcity" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=pcity - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pcity.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pcity.mak" CFG="pcity - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pcity - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "pcity - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "pcity - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f pcity.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "pcity.exe"
# PROP BASE Bsc_Name "pcity.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "makeallreal.bat"
# PROP Rebuild_Opt ""
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "pcity - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f pcity.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "pcity.exe"
# PROP BASE Bsc_Name "pcity.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "makeall.bat"
# PROP Rebuild_Opt ""
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "pcity - Win32 Release"
# Name "pcity - Win32 Debug"

!IF  "$(CFG)" == "pcity - Win32 Release"

!ELSEIF  "$(CFG)" == "pcity - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\source\build.c
# End Source File
# Begin Source File

SOURCE=.\source\drawing.c
# End Source File
# Begin Source File

SOURCE=.\source\globals.c
# End Source File
# Begin Source File

SOURCE=.\source\handler.c
# End Source File
# Begin Source File

SOURCE=.\source\simulation.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\source\build.h
# End Source File
# Begin Source File

SOURCE=.\source\drawing.h
# End Source File
# Begin Source File

SOURCE=.\source\globals.h
# End Source File
# Begin Source File

SOURCE=.\source\handler.h
# End Source File
# Begin Source File

SOURCE=.\source\simulation.h
# End Source File
# Begin Source File

SOURCE=.\source\ui.h
# End Source File
# Begin Source File

SOURCE=.\source\zakdef.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Group "Palm Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\palm\simcity.rcp
# End Source File
# End Group
# Begin Group "Nokia Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\nokia\graphic\commercial.goh
# End Source File
# Begin Source File

SOURCE=.\nokia\graphic.goh
# End Source File
# Begin Source File

SOURCE=.\nokia\graphic\icons.goh
# End Source File
# Begin Source File

SOURCE=.\nokia\graphic\industrial.goh
# End Source File
# Begin Source File

SOURCE=.\nokia\graphic\misc.goh
# End Source File
# Begin Source File

SOURCE=.\nokia\graphic\power.goh
# End Source File
# Begin Source File

SOURCE=.\nokia\graphic\residential.goh
# End Source File
# Begin Source File

SOURCE=.\nokia\graphic\roads.goh
# End Source File
# End Group
# End Group
# Begin Group "UI"

# PROP Default_Filter ""
# Begin Group "Palm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\palm\Makefile
# End Source File
# Begin Source File

SOURCE=.\palm\simcity.c
# End Source File
# Begin Source File

SOURCE=.\palm\simcity.h
# End Source File
# End Group
# Begin Group "Nokia"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Makefile
# End Source File
# Begin Source File

SOURCE=.\nokia\nokia.goc
# End Source File
# Begin Source File

SOURCE=.\pcity.gp
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\version.txt
# End Source File
# End Target
# End Project
