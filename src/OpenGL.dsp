# Microsoft Developer Studio Project File - Name="OpenGL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=OpenGL - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "OpenGL.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "OpenGL.mak" CFG="OpenGL - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "OpenGL - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "OpenGL - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OpenGL - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x411 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows /machine:I386 /out:"Release/MDraw.exe"

!ELSEIF  "$(CFG)" == "OpenGL - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "MFC" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x411 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib glu32.lib glaux.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/MDraw.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "OpenGL - Win32 Release"
# Name "OpenGL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Asp.cxx
# End Source File
# Begin Source File

SOURCE=.\bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\cylspm.cxx
# End Source File
# Begin Source File

SOURCE=.\dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\draw.cpp
# End Source File
# Begin Source File

SOURCE=.\dspr.cxx
# End Source File
# Begin Source File

SOURCE=.\edit.cpp
# End Source File
# Begin Source File

SOURCE=.\esp.cxx
# End Source File
# Begin Source File

SOURCE=.\esptree.cxx
# End Source File
# Begin Source File

SOURCE=.\ffwarp.cxx
# End Source File
# Begin Source File

SOURCE=.\file.cxx
# End Source File
# Begin Source File

SOURCE=.\floater.cxx
# End Source File
# Begin Source File

SOURCE=.\glppd.cpp
# End Source File
# Begin Source File

SOURCE=.\gltile.cpp
# End Source File
# Begin Source File

SOURCE=.\hestack.cxx
# End Source File
# Begin Source File

SOURCE=.\hidden.cpp
# End Source File
# Begin Source File

SOURCE=.\intsec.cxx
# End Source File
# Begin Source File

SOURCE=.\linbcg.cxx
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Material.cpp
# End Source File
# Begin Source File

SOURCE=.\MaterialDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\matlib.cxx
# End Source File
# Begin Source File

SOURCE=.\mdw.cxx
# End Source File
# Begin Source File

SOURCE=.\MeshInfoDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\mytime.cxx
# End Source File
# Begin Source File

SOURCE=.\OpenGL.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenGL.rc
# End Source File
# Begin Source File

SOURCE=.\OpenGLDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenGLView.cpp
# End Source File
# Begin Source File

SOURCE=.\oppd.cxx
# End Source File
# Begin Source File

SOURCE=.\params.cxx
# End Source File
# Begin Source File

SOURCE=.\pick.cpp
# End Source File
# Begin Source File

SOURCE=.\ppd.cxx
# End Source File
# Begin Source File

SOURCE=.\ppdedge.cxx
# End Source File
# Begin Source File

SOURCE=.\ppdface.cxx
# End Source File
# Begin Source File

SOURCE=.\ppdlist.cxx
# End Source File
# Begin Source File

SOURCE=.\ppdloop.cxx
# End Source File
# Begin Source File

SOURCE=.\ppdnormal.cxx
# End Source File
# Begin Source File

SOURCE=.\ppdpart.cxx
# End Source File
# Begin Source File

SOURCE=.\ppdsolid.cxx
# End Source File
# Begin Source File

SOURCE=.\ppdvertex.cxx
# End Source File
# Begin Source File

SOURCE=.\ps.cxx
# End Source File
# Begin Source File

SOURCE=.\quadtree.cxx
# End Source File
# Begin Source File

SOURCE=.\QuitDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\remesh.cxx
# End Source File
# Begin Source File

SOURCE=.\screen.cxx
# End Source File
# Begin Source File

SOURCE=.\sellist.cxx
# End Source File
# Begin Source File

SOURCE=.\sgprique.cxx
# End Source File
# Begin Source File

SOURCE=.\sgraph.cxx
# End Source File
# Begin Source File

SOURCE=.\sigproc.cxx
# End Source File
# Begin Source File

SOURCE=.\smd.cpp
# End Source File
# Begin Source File

SOURCE=.\smf.cxx
# End Source File
# Begin Source File

SOURCE=.\SPDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\spedprique.cxx
# End Source File
# Begin Source File

SOURCE=.\spm.cxx
# End Source File
# Begin Source File

SOURCE=.\SRDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\texture.cxx
# End Source File
# Begin Source File

SOURCE=.\tfcremesh.cxx
# End Source File
# Begin Source File

SOURCE=.\tile.cxx
# End Source File
# Begin Source File

SOURCE=.\TileDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\umbrella.cxx
# End Source File
# Begin Source File

SOURCE=.\uvwsigproc.cxx
# End Source File
# Begin Source File

SOURCE=.\veclib.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Asp.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap.h
# End Source File
# Begin Source File

SOURCE=.\color.h
# End Source File
# Begin Source File

SOURCE=.\cylspm.h
# End Source File
# Begin Source File

SOURCE=.\dialog.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\dspr.h
# End Source File
# Begin Source File

SOURCE=.\edit.h
# End Source File
# Begin Source File

SOURCE=.\esp.h
# End Source File
# Begin Source File

SOURCE=.\esplist.h
# End Source File
# Begin Source File

SOURCE=.\esptree.h
# End Source File
# Begin Source File

SOURCE=.\ffwarp.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\floater.h
# End Source File
# Begin Source File

SOURCE=.\glppd.h
# End Source File
# Begin Source File

SOURCE=.\gltile.h
# End Source File
# Begin Source File

SOURCE=.\hestack.h
# End Source File
# Begin Source File

SOURCE=.\hidden.h
# End Source File
# Begin Source File

SOURCE=.\intsec.h
# End Source File
# Begin Source File

SOURCE=.\linbcg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Material.h
# End Source File
# Begin Source File

SOURCE=.\MaterialDialog.h
# End Source File
# Begin Source File

SOURCE=.\matlib.h
# End Source File
# Begin Source File

SOURCE=.\mdw.h
# End Source File
# Begin Source File

SOURCE=.\MeshInfoDialog.h
# End Source File
# Begin Source File

SOURCE=.\mtl.h
# End Source File
# Begin Source File

SOURCE=.\mytime.h
# End Source File
# Begin Source File

SOURCE=.\OpenGL.h
# End Source File
# Begin Source File

SOURCE=.\OpenGLDoc.h
# End Source File
# Begin Source File

SOURCE=.\OpenGLView.h
# End Source File
# Begin Source File

SOURCE=.\oppd.h
# End Source File
# Begin Source File

SOURCE=.\params.h
# End Source File
# Begin Source File

SOURCE=.\pick.h
# End Source File
# Begin Source File

SOURCE=.\ppd.h
# End Source File
# Begin Source File

SOURCE=.\ppdedge.h
# End Source File
# Begin Source File

SOURCE=.\ppdface.h
# End Source File
# Begin Source File

SOURCE=.\ppdlist.h
# End Source File
# Begin Source File

SOURCE=.\ppdloop.h
# End Source File
# Begin Source File

SOURCE=.\ppdnormal.h
# End Source File
# Begin Source File

SOURCE=.\ppdpart.h
# End Source File
# Begin Source File

SOURCE=.\ppdsolid.h
# End Source File
# Begin Source File

SOURCE=.\ppdvertex.h
# End Source File
# Begin Source File

SOURCE=.\ps.h
# End Source File
# Begin Source File

SOURCE=.\quadtree.h
# End Source File
# Begin Source File

SOURCE=.\QuitDialog.h
# End Source File
# Begin Source File

SOURCE=.\remesh.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\screen.h
# End Source File
# Begin Source File

SOURCE=.\sellist.h
# End Source File
# Begin Source File

SOURCE=.\sgprique.h
# End Source File
# Begin Source File

SOURCE=.\sgraph.h
# End Source File
# Begin Source File

SOURCE=.\sigproc.h
# End Source File
# Begin Source File

SOURCE=.\smd.h
# End Source File
# Begin Source File

SOURCE=.\smdsrc.h
# End Source File
# Begin Source File

SOURCE=.\smf.h
# End Source File
# Begin Source File

SOURCE=.\SPDialog.h
# End Source File
# Begin Source File

SOURCE=.\spedprique.h
# End Source File
# Begin Source File

SOURCE=.\spm.h
# End Source File
# Begin Source File

SOURCE=.\SRDialog.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\texture.h
# End Source File
# Begin Source File

SOURCE=.\tfcremesh.h
# End Source File
# Begin Source File

SOURCE=.\tile.h
# End Source File
# Begin Source File

SOURCE=.\TileDialog.h
# End Source File
# Begin Source File

SOURCE=.\umbrella.h
# End Source File
# Begin Source File

SOURCE=.\uvwsigproc.h
# End Source File
# Begin Source File

SOURCE=.\veclib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\OpenGL.ico
# End Source File
# Begin Source File

SOURCE=.\res\OpenGL.rc2
# End Source File
# Begin Source File

SOURCE=.\res\OpenGLDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
