# Microsoft Developer Studio Project File - Name="inheritance" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=inheritance - Win32 Release Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "inheritance.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "inheritance.mak" CFG="inheritance - Win32 Release Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "inheritance - Win32 Debug Dll" (based on "Win32 (x86) Console Application")
!MESSAGE "inheritance - Win32 Debug Static" (based on "Win32 (x86) Console Application")
!MESSAGE "inheritance - Win32 Release Dll" (based on "Win32 (x86) Console Application")
!MESSAGE "inheritance - Win32 Release Static" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "inheritance - Win32 Debug Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\build\vc_debug_shared"
# PROP BASE Intermediate_Dir ".\build\vc_debug_shared\inheritance"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\build\vc_debug_shared"
# PROP Intermediate_Dir ".\build\vc_debug_shared\inheritance"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /I ".\..\.." /Od /Gm /Zi /Fd.\build\vc_debug_shared\inheritance.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_CONSOLE" /D "TEST_ODBC" /D "TEST_CSV" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /I ".\..\.." /Od /Gm /Zi /Fd.\build\vc_debug_shared\inheritance.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_CONSOLE" /D "TEST_ODBC" /D "TEST_CSV" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD BASE RSC /l 0x409 /d "_CONSOLE" /i ".\..\.." /d "TEST_ODBC" /d "TEST_CSV" /d "_DEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /i $(DEVEL)\include
# ADD RSC /l 0x409 /d "_CONSOLE" /i ".\..\.." /d "TEST_ODBC" /d "TEST_CSV" /d "_DEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /i $(DEVEL)\include
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dbad.lib unicows.lib "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comctl32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" winmm.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib /nologo /machine:i386 /out:".\build\vc_debug_shared\inheritance.exe" /subsystem:console /libpath:"..\..\.\build\vc_debug_shared" /debug /pdb:".\build\vc_debug_shared\inheritance.pdb" /libpath:"$(DEVEL)\lib" $(NOINHERIT) /NODEFAULTLIB:MSVCRT
# ADD LINK32 dbad.lib unicows.lib "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comctl32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" winmm.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib /nologo /machine:i386 /out:".\build\vc_debug_shared\inheritance.exe" /subsystem:console /libpath:"..\..\.\build\vc_debug_shared" /debug /pdb:".\build\vc_debug_shared\inheritance.pdb" /libpath:"$(DEVEL)\lib" $(NOINHERIT) /NODEFAULTLIB:MSVCRT

!ELSEIF  "$(CFG)" == "inheritance - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\build\vc_debug_static"
# PROP BASE Intermediate_Dir ".\build\vc_debug_static\inheritance"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\build\vc_debug_static"
# PROP Intermediate_Dir ".\build\vc_debug_static\inheritance"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /I ".\..\.." /Od /Gm /Zi /Fd.\build\vc_debug_static\inheritance.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_CONSOLE" /D "TEST_ODBC" /D "TEST_CSV" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /I ".\..\.." /Od /Gm /Zi /Fd.\build\vc_debug_static\inheritance.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_CONSOLE" /D "TEST_ODBC" /D "TEST_CSV" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD BASE RSC /l 0x409 /d "_CONSOLE" /i ".\..\.." /d "TEST_ODBC" /d "TEST_CSV" /d "_DEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /i $(DEVEL)\include
# ADD RSC /l 0x409 /d "_CONSOLE" /i ".\..\.." /d "TEST_ODBC" /d "TEST_CSV" /d "_DEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /i $(DEVEL)\include
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dbad.lib unicows.lib "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comctl32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" winmm.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib /nologo /machine:i386 /out:".\build\vc_debug_static\inheritance.exe" /subsystem:console /libpath:"..\..\.\build\vc_debug_static" /debug /pdb:".\build\vc_debug_static\inheritance.pdb" /libpath:"$(DEVEL)\lib" $(NOINHERIT) /NODEFAULTLIB:MSVCRT
# ADD LINK32 dbad.lib unicows.lib "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comctl32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" winmm.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib /nologo /machine:i386 /out:".\build\vc_debug_static\inheritance.exe" /subsystem:console /libpath:"..\..\.\build\vc_debug_static" /debug /pdb:".\build\vc_debug_static\inheritance.pdb" /libpath:"$(DEVEL)\lib" $(NOINHERIT) /NODEFAULTLIB:MSVCRT

!ELSEIF  "$(CFG)" == "inheritance - Win32 Release Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\build\vc_release_shared"
# PROP BASE Intermediate_Dir ".\build\vc_release_shared\inheritance"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\vc_release_shared"
# PROP Intermediate_Dir ".\build\vc_release_shared\inheritance"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /I ".\..\.." /O2 /Fd.\build\vc_release_shared\inheritance.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_CONSOLE" /D "TEST_ODBC" /D "TEST_CSV" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /I ".\..\.." /O2 /Fd.\build\vc_release_shared\inheritance.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_CONSOLE" /D "TEST_ODBC" /D "TEST_CSV" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD BASE RSC /l 0x409 /d "_CONSOLE" /i ".\..\.." /d "TEST_ODBC" /d "TEST_CSV" /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /i $(DEVEL)\include
# ADD RSC /l 0x409 /d "_CONSOLE" /i ".\..\.." /d "TEST_ODBC" /d "TEST_CSV" /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /i $(DEVEL)\include
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dba.lib unicows.lib "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comctl32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" winmm.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib /nologo /machine:i386 /out:".\build\vc_release_shared\inheritance.exe" /subsystem:console /libpath:"..\..\.\build\vc_release_shared" /pdb:".\build\vc_release_shared\inheritance.pdb" /libpath:"$(DEVEL)\lib" $(NOINHERIT)
# ADD LINK32 dba.lib unicows.lib "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comctl32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" winmm.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib /nologo /machine:i386 /out:".\build\vc_release_shared\inheritance.exe" /subsystem:console /libpath:"..\..\.\build\vc_release_shared" /pdb:".\build\vc_release_shared\inheritance.pdb" /libpath:"$(DEVEL)\lib" $(NOINHERIT)

!ELSEIF  "$(CFG)" == "inheritance - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\build\vc_release_static"
# PROP BASE Intermediate_Dir ".\build\vc_release_static\inheritance"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\vc_release_static"
# PROP Intermediate_Dir ".\build\vc_release_static\inheritance"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /I ".\..\.." /O2 /Fd.\build\vc_release_static\inheritance.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_CONSOLE" /D "TEST_ODBC" /D "TEST_CSV" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /I ".\..\.." /O2 /Fd.\build\vc_release_static\inheritance.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_CONSOLE" /D "TEST_ODBC" /D "TEST_CSV" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD BASE RSC /l 0x409 /d "_CONSOLE" /i ".\..\.." /d "TEST_ODBC" /d "TEST_CSV" /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /i $(DEVEL)\include
# ADD RSC /l 0x409 /d "_CONSOLE" /i ".\..\.." /d "TEST_ODBC" /d "TEST_CSV" /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /i $(DEVEL)\include
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dba.lib unicows.lib "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comctl32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" winmm.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib /nologo /machine:i386 /out:".\build\vc_release_static\inheritance.exe" /subsystem:console /libpath:"..\..\.\build\vc_release_static" /pdb:".\build\vc_release_static\inheritance.pdb" /libpath:"$(DEVEL)\lib" $(NOINHERIT)
# ADD LINK32 dba.lib unicows.lib "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comctl32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" winmm.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib /nologo /machine:i386 /out:".\build\vc_release_static\inheritance.exe" /subsystem:console /libpath:"..\..\.\build\vc_release_static" /pdb:".\build\vc_release_static\inheritance.pdb" /libpath:"$(DEVEL)\lib" $(NOINHERIT)

!ENDIF

# Begin Target

# Name "inheritance - Win32 Debug Dll"
# Name "inheritance - Win32 Debug Static"
# Name "inheritance - Win32 Release Dll"
# Name "inheritance - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\inheritance.cpp
# End Source File
# End Group
# End Target
# End Project

