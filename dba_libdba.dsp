# Microsoft Developer Studio Project File - Name="dba_libdba" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libdba - Win32 Release Static
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dba_libdba.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dba_libdba.mak" CFG="libdba - Win32 Release Static"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libdba - Win32 Debug Dll" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libdba - Win32 Debug Static" (based on "Win32 (x86) Static Library")
!MESSAGE "libdba - Win32 Release Dll" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libdba - Win32 Release Static" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libdba - Win32 Debug Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\build\vc_debug_shared"
# PROP BASE Intermediate_Dir ".\build\vc_debug_shared\libdba"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\build\vc_debug_shared"
# PROP Intermediate_Dir ".\build\vc_debug_shared\libdba"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /I "." /I "dba" /Od /Gm /Zi /Fd.\build\vc_debug_shared\dbad_dll.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /I "." /I "dba" /Od /Gm /Zi /Fd.\build\vc_debug_shared\dbad_dll.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "." /i "dba" /d "_DEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i $(DEVEL)\include
# ADD RSC /l 0x409 /i "." /i "dba" /d "_DEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i $(DEVEL)\include
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /machine:i386 /out:".\build\vc_debug_shared\dbad_dll.dll" /implib:".\build\vc_debug_shared\dbad_dll.lib" /debug /pdb:".\build\vc_debug_shared\dbad_dll.pdb" /libpath:"$(DEVEL)\lib"
# ADD LINK32 /nologo /dll /machine:i386 /out:".\build\vc_debug_shared\dbad_dll.dll" /implib:".\build\vc_debug_shared\dbad_dll.lib" /debug /pdb:".\build\vc_debug_shared\dbad_dll.pdb" /libpath:"$(DEVEL)\lib"

!ELSEIF  "$(CFG)" == "libdba - Win32 Debug Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\build\vc_debug_static"
# PROP BASE Intermediate_Dir ".\build\vc_debug_static\libdba"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\build\vc_debug_static"
# PROP Intermediate_Dir ".\build\vc_debug_static\libdba"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MDd /W1 /GR /EHsc /I "." /I "dba" /Od /Gm /Zi /Fd.\build\vc_debug_static\dbad.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_LIB" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD CPP /nologo /FD /MDd /W1 /GR /EHsc /I "." /I "dba" /Od /Gm /Zi /Fd.\build\vc_debug_static\dbad.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_LIB" /D "_DEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\build\vc_debug_static\dbad.lib"
# ADD LIB32 /nologo /out:".\build\vc_debug_static\dbad.lib"

!ELSEIF  "$(CFG)" == "libdba - Win32 Release Dll"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\build\vc_release_shared"
# PROP BASE Intermediate_Dir ".\build\vc_release_shared\libdba"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\vc_release_shared"
# PROP Intermediate_Dir ".\build\vc_release_shared\libdba"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /I "." /I "dba" /O2 /Fd.\build\vc_release_shared\dba_dll.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /I "." /I "dba" /O2 /Fd.\build\vc_release_shared\dba_dll.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /c
# ADD BASE MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_USRDLL" /D "DLL_EXPORTS" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /D "DLL_EXPORTS" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "." /i "dba" /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i $(DEVEL)\include
# ADD RSC /l 0x409 /i "." /i "dba" /d "NDEBUG" /d APPVERSION="1.4.2" /d "_CRT_SECURE_NO_DEPRECATE" /d "_CRT_NONSTDC_NO_DEPRECATE" /d "DLL_EXPORTS" /i $(DEVEL)\include
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /dll /machine:i386 /out:".\build\vc_release_shared\dba_dll.dll" /implib:".\build\vc_release_shared\dba_dll.lib" /pdb:".\build\vc_release_shared\dba_dll.pdb" /libpath:"$(DEVEL)\lib"
# ADD LINK32 /nologo /dll /machine:i386 /out:".\build\vc_release_shared\dba_dll.dll" /implib:".\build\vc_release_shared\dba_dll.lib" /pdb:".\build\vc_release_shared\dba_dll.pdb" /libpath:"$(DEVEL)\lib"

!ELSEIF  "$(CFG)" == "libdba - Win32 Release Static"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\build\vc_release_static"
# PROP BASE Intermediate_Dir ".\build\vc_release_static\libdba"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\build\vc_release_static"
# PROP Intermediate_Dir ".\build\vc_release_static\libdba"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /FD /MD /W1 /GR /EHsc /I "." /I "dba" /O2 /Fd.\build\vc_release_static\dba.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_LIB" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD CPP /nologo /FD /MD /W1 /GR /EHsc /I "." /I "dba" /O2 /Fd.\build\vc_release_static\dba.pdb /I "$(DEVEL)\include" /D "WIN32" /D "_LIB" /D "NDEBUG" /D APPVERSION=\"1.4.2\" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\build\vc_release_static\dba.lib"
# ADD LIB32 /nologo /out:".\build\vc_release_static\dba.lib"

!ENDIF

# Begin Target

# Name "libdba - Win32 Debug Dll"
# Name "libdba - Win32 Debug Static"
# Name "libdba - Win32 Release Dll"
# Name "libdba - Win32 Release Static"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dba\archive.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\bindedvar.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\bool_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\connectstring.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\connectstringparser.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\conversion.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\convspec.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\database.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\datetime_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\dbplugin.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\dbupdate.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\dbupdatescriptparser.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\defs.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\double_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\exception.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\fileutils.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\filtermapper.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\genericfetcher.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\idlocker.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\int_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\istream.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\localechanger.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\memarchive.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\membertree.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\ostream.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\sharedsqlarchive.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\sql.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\sqlarchive.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\sqlistream.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\sqlostream.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\sqlutils.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\storeable.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\storeablefilter.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\stream.cpp
# End Source File
# Begin Source File

SOURCE=.\dba\string_filter.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dba\archive.h
# End Source File
# Begin Source File

SOURCE=.\dba\archiveexception.h
# End Source File
# Begin Source File

SOURCE=.\dba\bindedvar.h
# End Source File
# Begin Source File

SOURCE=.\dba\bool_filter.h
# End Source File
# Begin Source File

SOURCE=.\dba\collectionfilter.h
# End Source File
# Begin Source File

SOURCE=.\dba\connectstring.h
# End Source File
# Begin Source File

SOURCE=.\dba\connectstringparser.h
# End Source File
# Begin Source File

SOURCE=.\dba\conversion.h
# End Source File
# Begin Source File

SOURCE=.\dba\convspec.h
# End Source File
# Begin Source File

SOURCE=.\dba\csv.h
# End Source File
# Begin Source File

SOURCE=.\dba\database.h
# End Source File
# Begin Source File

SOURCE=.\dba\datetime_filter.h
# End Source File
# Begin Source File

SOURCE=.\dba\dba.h
# End Source File
# Begin Source File

SOURCE=.\dba\dbplugin.h
# End Source File
# Begin Source File

SOURCE=.\dba\dbupdate.h
# End Source File
# Begin Source File

SOURCE=.\dba\dbupdatescriptparser.h
# End Source File
# Begin Source File

SOURCE=.\dba\defs.h
# End Source File
# Begin Source File

SOURCE=.\dba\double_filter.h
# End Source File
# Begin Source File

SOURCE=.\dba\exception.h
# End Source File
# Begin Source File

SOURCE=.\dba\fileutils.h
# End Source File
# Begin Source File

SOURCE=.\dba\filtermapper.h
# End Source File
# Begin Source File

SOURCE=.\dba\genericfetcher.h
# End Source File
# Begin Source File

SOURCE=.\dba\idlocker.h
# End Source File
# Begin Source File

SOURCE=.\dba\int_filter.h
# End Source File
# Begin Source File

SOURCE=.\dba\istream.h
# End Source File
# Begin Source File

SOURCE=.\dba\localechanger.h
# End Source File
# Begin Source File

SOURCE=.\dba\memarchive.h
# End Source File
# Begin Source File

SOURCE=.\dba\membertree.h
# End Source File
# Begin Source File

SOURCE=.\dba\ostream.h
# End Source File
# Begin Source File

SOURCE=.\dba\plugininfo.h
# End Source File
# Begin Source File

SOURCE=.\dba\shared_ptr.h
# End Source File
# Begin Source File

SOURCE=.\dba\sharedsqlarchive.h
# End Source File
# Begin Source File

SOURCE=.\dba\single.h
# End Source File
# Begin Source File

SOURCE=.\dba\sql.h
# End Source File
# Begin Source File

SOURCE=.\dba\sqlarchive.h
# End Source File
# Begin Source File

SOURCE=.\dba\sqlidfetcher.h
# End Source File
# Begin Source File

SOURCE=.\dba\sqlistream.h
# End Source File
# Begin Source File

SOURCE=.\dba\sqlostream.h
# End Source File
# Begin Source File

SOURCE=.\dba\sqlutils.h
# End Source File
# Begin Source File

SOURCE=.\dba\stddeque.h
# End Source File
# Begin Source File

SOURCE=.\dba\stdfilters.h
# End Source File
# Begin Source File

SOURCE=.\dba\stdlist.h
# End Source File
# Begin Source File

SOURCE=.\dba\stdmultiset.h
# End Source File
# Begin Source File

SOURCE=.\dba\stdset.h
# End Source File
# Begin Source File

SOURCE=.\dba\stdvector.h
# End Source File
# Begin Source File

SOURCE=.\dba\stlutils.h
# End Source File
# Begin Source File

SOURCE=.\dba\storeable.h
# End Source File
# Begin Source File

SOURCE=.\dba\storeablefilter.h
# End Source File
# Begin Source File

SOURCE=.\dba\storeablelist.h
# End Source File
# Begin Source File

SOURCE=.\dba\stream.h
# End Source File
# Begin Source File

SOURCE=.\dba\string_filter.h
# End Source File
# Begin Source File

SOURCE=.\dba\xmlarchive.h
# End Source File
# Begin Source File

SOURCE=.\dba\xmlerrorhandler.h
# End Source File
# Begin Source File

SOURCE=.\dba\xmlexception.h
# End Source File
# Begin Source File

SOURCE=.\dba\xmlistream.h
# End Source File
# Begin Source File

SOURCE=.\dba\xmlostream.h
# End Source File
# End Group
# End Target
# End Project

