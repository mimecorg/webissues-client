/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2015 WebIssues Team
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

!define SRCDIR "..\.."
!define BUILDDIR "..\..\release"

!define UNINST_KEY "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WebIssues Client 1.1"

!include "MUI2.nsh"

!include "languages\webissues_en.nsh"

!ifdef INNER
    SetCompress off

    OutFile "$%TEMP%\innerinst.exe"
!else
    !verbose 4

    SetCompressor /SOLID lzma
    SetCompressorDictSize 32

    OutFile "webissues-${VERSION}-${ARCHITECTURE}.exe"
!endif

!define MULTIUSER_EXECUTIONLEVEL "Highest"
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "UninstallString"
!define MULTIUSER_INSTALLMODE_INSTDIR "WebIssues Client\1.1"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstallLocation"
!if ${ARCHITECTURE} == "win_x64"
  !define MULTIUSER_USE_PROGRAMFILES64
!endif
!ifndef INNER
    !define MULTIUSER_NOUNINSTALL
!endif
!include "include\multiuser64.nsh"

Name "$(NAME)"

!define MUI_ICON "${SRCDIR}\src\webissues.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-blue.ico"

!define MUI_WELCOMEFINISHPAGE_BITMAP "images\wizard.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "images\wizard.bmp"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "images\header.bmp"
!define MUI_HEADERIMAGE_RIGHT

!define MUI_WELCOMEPAGE_TITLE "$(TITLE)"
!define MUI_WELCOMEPAGE_TEXT "$(WELCOME_TEXT)"
!insertmacro MUI_PAGE_WELCOME

!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "${SRCDIR}\COPYING"

!insertmacro MULTIUSER_PAGE_INSTALLMODE

!insertmacro MUI_PAGE_DIRECTORY

ShowInstDetails nevershow
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "$(TITLE)"
!insertmacro MUI_PAGE_FINISH
  
!ifdef INNER  
    !define MUI_WELCOMEPAGE_TITLE "$(TITLE)"
    !insertmacro MUI_UNPAGE_WELCOME

    !insertmacro MUI_UNPAGE_CONFIRM

    ShowUninstDetails nevershow
    !insertmacro MUI_UNPAGE_INSTFILES

    !define MUI_FINISHPAGE_TITLE "$(TITLE)"
    !insertmacro MUI_UNPAGE_FINISH
!endif

!insertmacro MUI_LANGUAGE "English"

VIProductVersion "${BUILDVERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "WebIssues Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "WebIssues Desktop Client Setup"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) 2007-2015 WebIssues Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "webissues-${VERSION}-${ARCHITECTURE}.exe"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "WebIssues Desktop Client"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"

Function .onInit

!if ${ARCHITECTURE} == "win_x64"
    SetRegView 64
!endif

!ifdef INNER
    WriteUninstaller "$%TEMP%\uninstall.exe"
    Quit
!endif

    !insertmacro MULTIUSER_INIT

FunctionEnd

Section

    SetOutPath "$INSTDIR"

    File "${SRCDIR}\ChangeLog"
    File "${SRCDIR}\COPYING"
    File "${SRCDIR}\README"

    SetOutPath "$INSTDIR\bin"

    Delete "$INSTDIR\bin\*.*"

    File "${BUILDDIR}\webissues.exe"

    File "qt.conf"

    File "${QTDIR}\bin\Qt5Core.dll"
    File "${QTDIR}\bin\Qt5Gui.dll"
    File "${QTDIR}\bin\Qt5Network.dll"
    File "${QTDIR}\bin\Qt5OpenGL.dll"
    File "${QTDIR}\bin\Qt5PrintSupport.dll"
    File "${QTDIR}\bin\Qt5Qml.dll"
    File "${QTDIR}\bin\Qt5Quick.dll"
    File "${QTDIR}\bin\Qt5Sql.dll"
    File "${QTDIR}\bin\Qt5WebKit.dll"
    File "${QTDIR}\bin\Qt5WebKitWidgets.dll"
    File "${QTDIR}\bin\Qt5Widgets.dll"
    File "${QTDIR}\bin\Qt5Xml.dll"

    File "${QTDIR}\bin\icudt55.dll"
    File "${QTDIR}\bin\icuin55.dll"
    File "${QTDIR}\bin\icuuc55.dll"

    File "${OPENSSLDIR}\bin\libeay32.dll"
    File "${OPENSSLDIR}\bin\ssleay32.dll"

    File "${VCRTDIR}\msvcp100.dll"
    File "${VCRTDIR}\msvcr100.dll"

    SetOutPath "$INSTDIR\plugins\platforms"

    File "${QTDIR}\plugins\platforms\qwindows.dll"

    SetOutPath "$INSTDIR\plugins\printsupport"

    File "${QTDIR}\plugins\printsupport\windowsprintersupport.dll"

    SetOutPath "$INSTDIR\doc"

    Delete "$INSTDIR\doc\*.*"

    File /r /x .svn "${SRCDIR}\doc\*.*"

    SetOutPath "$INSTDIR\translations"

    Delete "$INSTDIR\translations\*.*"

    File "${SRCDIR}\translations\locale.ini"

    File "${SRCDIR}\translations\webissues_cs.qm"
    File "${SRCDIR}\translations\webissues_de.qm"
    File "${SRCDIR}\translations\webissues_es.qm"
    File "${SRCDIR}\translations\webissues_fr.qm"
    File "${SRCDIR}\translations\webissues_it.qm"
    File "${SRCDIR}\translations\webissues_nl.qm"
    File "${SRCDIR}\translations\webissues_pl.qm"
    File "${SRCDIR}\translations\webissues_pt_BR.qm"
    File "${SRCDIR}\translations\webissues_ru.qm"
    File "${SRCDIR}\translations\webissues_zh_CN.qm"

    File "${QTDIR}\translations\qt_cs.qm"
    File "${QTDIR}\translations\qt_de.qm"
    File "${QTDIR}\translations\qt_es.qm"
    File "${QTDIR}\translations\qt_fr.qm"
    File "${QTDIR}\translations\qt_it.qm"
    ;File "${QTDIR}\translations\qt_nl.qm"
    File "${QTDIR}\translations\qt_pl.qm"
    File "${QTDIR}\translations\qt_pt.qm"
    File "${QTDIR}\translations\qt_ru.qm"
    File "${QTDIR}\translations\qt_zh_CN.qm"

    SetOutPath "$INSTDIR\bin"

    CreateShortCut "$SMPROGRAMS\WebIssues Desktop Client 1.1.lnk" "$INSTDIR\bin\webissues.exe"
    CreateShortCut "$DESKTOP\WebIssues Desktop Client 1.1.lnk" "$INSTDIR\bin\webissues.exe"

    WriteRegStr SHCTX "${UNINST_KEY}" "DisplayIcon" '"$INSTDIR\bin\webissues.exe"'
    WriteRegStr SHCTX "${UNINST_KEY}" "DisplayName" "WebIssues Desktop Client ${VERSION}${SUFFIX}"
    WriteRegStr SHCTX "${UNINST_KEY}" "DisplayVersion" "${VERSION}"
    WriteRegStr SHCTX "${UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe" /$MultiUser.InstallMode'
    WriteRegStr SHCTX "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr SHCTX "${UNINST_KEY}" "Publisher" "WebIssues Team"
    WriteRegStr SHCTX "${UNINST_KEY}" "HelpLink" "http://webissues.mimec.org"
    WriteRegStr SHCTX "${UNINST_KEY}" "URLInfoAbout" "http://webissues.mimec.org"
    WriteRegStr SHCTX "${UNINST_KEY}" "URLUpdateInfo" "http://webissues.mimec.org/downloads"
    WriteRegDWORD SHCTX "${UNINST_KEY}" "NoModify" 1
    WriteRegDWORD SHCTX "${UNINST_KEY}" "NoRepair" 1

!ifndef INNER
    SetOutPath "$INSTDIR"
    File "$%TEMP%\uninstall.exe"
!endif

SectionEnd

!ifdef INNER

Function un.onInit

!if ${ARCHITECTURE} == "win_x64"
    SetRegView 64
!endif

    !insertmacro MULTIUSER_UNINIT

FunctionEnd

Section "Uninstall"

    DeleteRegKey SHCTX "${UNINST_KEY}"

    Delete "$SMPROGRAMS\WebIssues Desktop Client 1.1.lnk"
    Delete "$DESKTOP\WebIssues Desktop Client 1.1.lnk"

    Delete "$INSTDIR\ChangeLog"
    Delete "$INSTDIR\COPYING"
    Delete "$INSTDIR\README"
    RMDir /r "$INSTDIR\bin"
    RMDir /r "$INSTDIR\plugins"
    RMDir /r "$INSTDIR\doc"
    RMDir /r "$INSTDIR\translations"
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"

SectionEnd

!endif
