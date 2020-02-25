# Microsoft Developer Studio Project File - Name="dba_odbc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=odbc - Win32 Release Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dba_odbc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dba_odbc.mak" CFG="odbc - Win32 Release Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "odbc - Win32 Debug Dll" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "odbc - Win32 Debug Static" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "odbc - Win32 Release Dll" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "odbc - Win32 Release Static" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "odbc - Win32 Debug Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\build\vc_debug_shared"
# PROP BASE Intermediate_Dir ".\build\vc_debug_shared\odbc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\build\vc_debug_shared"
# PROP Intermediate_Dir ".\build\vc_debug_shared\odbc"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /Od /Gm /I "$(DEVEL)\include" /I "." /Zi /Fd.\build\vc_debug_shared\dbaodbc.pdb /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /D "_DEBUG" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /Od /Gm /I "$(DEVEL)\include" /I "." /Zi /Fd.\build\vc_debug_shared\dbaodbc.pdb /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /D "_DEBUG" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i "$(DEVEL)\include" /i "." /d _DEBUG
# ADD RSC /l 0x409 /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i "$(DEVEL)\include" /i "." /d _DEBUG
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 .\build\vc_debug_shared\dbad_dll.lib odbc32.lib /nologo /dll /machine:i386 /out:".\build\vc_debug_shared\dbaodbc.dll" /libpath:"$(DEVEL)\lib" /debug /pdb:".\build\vc_debug_shared\dbaodbc.pdb"
# ADD LINK32 .\build\vc_debug_shared\dbad_dll.lib odbc32.lib /nologo /dll /machine:i386 /out:".\build\vc_debug_shared\dbaodbc.dll" /libpath:"$(DEVEL)\lib" /debug /pdb:".\build\vc_debug_shared\dbaodbc.pdb"

!ELSEIF  "$(CFG)" == "odbc - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\build\vc_debug_static"
# PROP BASE Intermediate_Dir ".\build\vc_debug_static\odbc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\build\vc_debug_static"
# PROP Intermediate_Dir ".\build\vc_debug_static\odbc"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /Od /Gm /I "$(DEVEL)\include" /I "." /Zi /Fd.\build\vc_debug_static\dbaodbc-static.pdb /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /D "_DEBUG" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /Od /Gm /I "$(DEVEL)\include" /I "." /Zi /Fd.\build\vc_debug_static\dbaodbc-static.pdb /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /D "_DEBUG" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i "$(DEVEL)\include" /i "." /d _DEBUG
# ADD RSC /l 0x409 /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i "$(DEVEL)\include" /i "." /d _DEBUG
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 .\build\vc_debug_static\dbad.lib odbc32.lib /nologo /dll /machine:i386 /out:".\build\vc_debug_static\dbaodbc-static.dll" /libpath:"$(DEVEL)\lib" /debug /pdb:".\build\vc_debug_static\dbaodbc-static.pdb"
# ADD LINK32 .\build\vc_debug_static\dbad.lib odbc32.lib /nologo /dll /machine:i386 /out:".\build\vc_debug_static\dbaodbc-static.dll" /libpath:"$(DEVEL)\lib" /debug /pdb:".\build\vc_debug_static\dbaodbc-static.pdb"

!ELSEIF  "$(CFG)" == "odbc - Win32 Release Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\build\vc_release_shared"
# PROP BASE Intermediate_Dir ".\build\vc_release_shared\odbc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\vc_release_shared"
# PROP Intermediate_Dir ".\build\vc_release_shared\odbc"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /O2 /I "$(DEVEL)\include" /I "." /Fd.\build\vc_release_shared\dbaodbc.pdb /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /O2 /I "$(DEVEL)\include" /I "." /Fd.\build\vc_release_shared\dbaodbc.pdb /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i "$(DEVEL)\include" /i .
# ADD RSC /l 0x409 /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i "$(DEVEL)\include" /i .
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 .\build\vc_release_shared\dba_dll.lib odbc32.lib /nologo /dll /machine:i386 /out:".\build\vc_release_shared\dbaodbc.dll" /libpath:"$(DEVEL)\lib" /pdb:".\build\vc_release_shared\dbaodbc.pdb"
# ADD LINK32 .\build\vc_release_shared\dba_dll.lib odbc32.lib /nologo /dll /machine:i386 /out:".\build\vc_release_shared\dbaodbc.dll" /libpath:"$(DEVEL)\lib" /pdb:".\build\vc_release_shared\dbaodbc.pdb"

!ELSEIF  "$(CFG)" == "odbc - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\build\vc_release_static"
# PROP BASE Intermediate_Dir ".\build\vc_release_static\odbc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\vc_release_static"
# PROP Intermediate_Dir ".\build\vc_release_static\odbc"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /O2 /I "$(DEVEL)\include" /I "." /Fd.\build\vc_release_static\dbaodbc-static.pdb /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /O2 /I "$(DEVEL)\include" /I "." /Fd.\build\vc_release_static\dbaodbc-static.pdb /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i "$(DEVEL)\include" /i .
# ADD RSC /l 0x409 /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i "$(DEVEL)\include" /i .
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 .\build\vc_release_static\dba.lib odbc32.lib /nologo /dll /machine:i386 /out:".\build\vc_release_static\dbaodbc-static.dll" /libpath:"$(DEVEL)\lib" /pdb:".\build\vc_release_static\dbaodbc-static.pdb"
# ADD LINK32 .\build\vc_release_static\dba.lib odbc32.lib /nologo /dll /machine:i386 /out:".\build\vc_release_static\dbaodbc-static.dll" /libpath:"$(DEVEL)\lib" /pdb:".\build\vc_release_static\dbaodbc-static.pdb"

!ENDIF

# Begin Target

# Name "odbc - Win32 Debug Dll"
# Name "odbc - Win32 Debug Static"
# Name "odbc - Win32 Release Dll"
# Name "odbc - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dba\odbc.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dba\odbc.h
# End Source File
# End Group
# End Target
# End Project

