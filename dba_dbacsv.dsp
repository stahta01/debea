# Microsoft Developer Studio Project File - Name="dba_dbacsv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=dbacsv - Win32 Release Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dba_dbacsv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dba_dbacsv.mak" CFG="dbacsv - Win32 Release Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbacsv - Win32 Debug Dll" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dbacsv - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "dbacsv - Win32 Release Dll" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dbacsv - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dbacsv - Win32 Debug Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\build\vc_debug_shared"
# PROP BASE Intermediate_Dir ".\build\vc_debug_shared\dbacsv"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\build\vc_debug_shared"
# PROP Intermediate_Dir ".\build\vc_debug_shared\dbacsv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /I "." /Od /Gm /Zi /Fd.\build\vc_debug_shared\dbacsvd_dll.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /I "." /Od /Gm /Zi /Fd.\build\vc_debug_shared\dbacsvd_dll.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "." /d "_DEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i $(DEVEL)\include
# ADD RSC /l 0x409 /i "." /d "_DEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i $(DEVEL)\include
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 .\build\vc_debug_shared\dbad_dll.lib /nologo /dll /machine:i386 /out:".\build\vc_debug_shared\dbacsvd_dll.dll" /implib:".\build\vc_debug_shared\dbacsvd_dll.lib" /debug /pdb:".\build\vc_debug_shared\dbacsvd_dll.pdb" /libpath:"$(DEVEL)\lib"
# ADD LINK32 .\build\vc_debug_shared\dbad_dll.lib /nologo /dll /machine:i386 /out:".\build\vc_debug_shared\dbacsvd_dll.dll" /implib:".\build\vc_debug_shared\dbacsvd_dll.lib" /debug /pdb:".\build\vc_debug_shared\dbacsvd_dll.pdb" /libpath:"$(DEVEL)\lib"

!ELSEIF  "$(CFG)" == "dbacsv - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\build\vc_debug_static"
# PROP BASE Intermediate_Dir ".\build\vc_debug_static\dbacsv"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\build\vc_debug_static"
# PROP Intermediate_Dir ".\build\vc_debug_static\dbacsv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /I "." /Od /Gm /Zi /Fd.\build\vc_debug_static\dbacsvd.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_LIB" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /I "." /Od /Gm /Zi /Fd.\build\vc_debug_static\dbacsvd.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_LIB" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\build\vc_debug_static\dbacsvd.lib"
# ADD LIB32 /nologo /out:".\build\vc_debug_static\dbacsvd.lib"

!ELSEIF  "$(CFG)" == "dbacsv - Win32 Release Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\build\vc_release_shared"
# PROP BASE Intermediate_Dir ".\build\vc_release_shared\dbacsv"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\vc_release_shared"
# PROP Intermediate_Dir ".\build\vc_release_shared\dbacsv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /I "." /O2 /Fd.\build\vc_release_shared\dbacsv_dll.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /I "." /O2 /Fd.\build\vc_release_shared\dbacsv_dll.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "." /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i $(DEVEL)\include
# ADD RSC /l 0x409 /i "." /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i $(DEVEL)\include
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 .\build\vc_release_shared\dba_dll.lib /nologo /dll /machine:i386 /out:".\build\vc_release_shared\dbacsv_dll.dll" /implib:".\build\vc_release_shared\dbacsv_dll.lib" /pdb:".\build\vc_release_shared\dbacsv_dll.pdb" /libpath:"$(DEVEL)\lib"
# ADD LINK32 .\build\vc_release_shared\dba_dll.lib /nologo /dll /machine:i386 /out:".\build\vc_release_shared\dbacsv_dll.dll" /implib:".\build\vc_release_shared\dbacsv_dll.lib" /pdb:".\build\vc_release_shared\dbacsv_dll.pdb" /libpath:"$(DEVEL)\lib"

!ELSEIF  "$(CFG)" == "dbacsv - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\build\vc_release_static"
# PROP BASE Intermediate_Dir ".\build\vc_release_static\dbacsv"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\vc_release_static"
# PROP Intermediate_Dir ".\build\vc_release_static\dbacsv"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /I "." /O2 /Fd.\build\vc_release_static\dbacsv.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_LIB" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /I "." /O2 /Fd.\build\vc_release_static\dbacsv.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_LIB" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\build\vc_release_static\dbacsv.lib"
# ADD LIB32 /nologo /out:".\build\vc_release_static\dbacsv.lib"

!ENDIF

# Begin Target

# Name "dbacsv - Win32 Debug Dll"
# Name "dbacsv - Win32 Debug Static"
# Name "dbacsv - Win32 Release Dll"
# Name "dbacsv - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dba\csv.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dba\csv.h
# End Source File
# End Group
# End Target
# End Project

