/**************************************************************************
* This file is part of the WebIssues Desktop Client program
* Copyright (C) 2006 Michał Męciński
* Copyright (C) 2007-2011 WebIssues Team
*
* Inspired by code written by John T. Haller of PortableApps.com
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

!define VERSION "1.0-alpha4"
!define BUILDVERSION "1.0.0.4091"

!define SRCDIR "..\.."
!define BUILDDIR "bin"

!define QTDIR "D:\Qt4-msvc\win32-static"

!include "MUI2.nsh"
!include "FileFunc.nsh"

!include "languages\portable_en.nsh"

SetCompressor /SOLID lzma
SetCompressorDictSize 32
OutFile "webissues-portable-${VERSION}.paf.exe"

RequestExecutionLevel user

Name "$(NAME)"

!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-blue.ico"

!define MUI_WELCOMEFINISHPAGE_BITMAP "images\wizard.bmp"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "images\header.bmp"
!define MUI_HEADERIMAGE_RIGHT

!define MUI_WELCOMEPAGE_TITLE "$(TITLE)"
!define MUI_WELCOMEPAGE_TEXT "$(WELCOME_TEXT)"
!insertmacro MUI_PAGE_WELCOME

!define MUI_LICENSEPAGE_CHECKBOX
!insertmacro MUI_PAGE_LICENSE "${SRCDIR}\COPYING"

InstallDir "\WebIssuesPortable"
!define MUI_DIRECTORYPAGE_VERIFYONLEAVE
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE LeaveDirectory
!insertmacro MUI_PAGE_DIRECTORY

ShowInstDetails nevershow
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "$(TITLE)"
!define MUI_FINISHPAGE_TEXT "$(FINISH_TEXT)"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

VIProductVersion "${BUILDVERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "WebIssues Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "WebIssues Portable Client Setup"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) 2007-2011 WebIssues Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "webissues-portable-${VERSION}.paf.exe"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "WebIssues Portable Client"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"

Function .onInit

    ${GetOptions} "$CMDLINE" "/DESTINATION=" $0
    ${IfNot} ${Errors}
        StrCpy $INSTDIR "$0WebIssuesPortable"
    ${Else}
        ${GetOptions} "$CMDLINE" "-o" $0
        ${IfNot} ${Errors}
            StrCpy $INSTDIR "$0WebIssuesPortable"
        ${Else}
            ${GetDrives} "HDD+FDD" GetDrivesCallBack
        ${EndIf}
    ${EndIf}

FunctionEnd

Function GetDrivesCallBack

    ${IfNot} $9 == "A:\"
    ${AndIfNot} $9 == "B:\"
    ${AndIf} ${FileExists} "$9PortableApps"
        StrCpy $INSTDIR "$9PortableApps\WebIssuesPortable"
    ${EndIf}

    Push $0

FunctionEnd

Function LeaveDirectory

    GetInstDirError $0

    ${If} $0 == 1
        MessageBox MB_OK|MB_ICONEXCLAMATION "$(MESSAGE_INVALIDDIR)"
        Abort
    ${ElseIf} $0 == 2
        ${If} ${FileExists} "$INSTDIR"
            SectionGetSize 0 $1
            ${GetRoot} "$INSTDIR" $2
            ${DriveSpace} "$2\" "/D=F /S=K" $3
            ${GetSize} "$INSTDIR" "/S=0K /G=0" $4 $5 $6
            IntOp $3 $3 + $4
            ${GetSize} "$INSTDIR\App" "/S=0K /G=1" $4 $5 $6
            IntOp $3 $3 + $4
            ${If} $1 >= $3
                MessageBox MB_OK|MB_ICONEXCLAMATION "$(MESSAGE_NOFREESPACE)"
                Abort
            ${EndIf}
        ${Else}
            MessageBox MB_OK|MB_ICONEXCLAMATION "$(MESSAGE_NOFREESPACE)"
            Abort
        ${EndIf}
    ${EndIf}

FunctionEnd

Section

    SetOutPath "$INSTDIR"

    File "portable\WebIssuesPortable.exe"
    File "portable\help.html"

    SetOutPath "$INSTDIR\App\WebIssues"

    File "${SRCDIR}\ChangeLog"
    File "${SRCDIR}\COPYING"
    File "${SRCDIR}\README"

    SetOutPath "$INSTDIR\App\WebIssues\bin"

    Delete "$INSTDIR\App\WebIssues\bin\*.*"

    File "${BUILDDIR}\webissues.exe"

    Delete "$INSTDIR\App\WebIssues\doc\*.*"

    SetOutPath "$INSTDIR\App\WebIssues\translations"

    Delete "$INSTDIR\App\WebIssues\translations\*.*"

    File "${SRCDIR}\translations\locale.ini"

    File "${SRCDIR}\translations\webissues_fr.qm"
    File "${SRCDIR}\translations\webissues_pl.qm"
    File "${SRCDIR}\translations\webissues_pt_BR.qm"

    File "${QTDIR}\translations\qt_fr.qm"
    File "${QTDIR}\translations\qt_pl.qm"
    File "${QTDIR}\translations\qt_pt.qm"

    SetOutPath "$INSTDIR\App\AppInfo"

    File "portable\appinfo.ini"
    File /oname=appicon.ico "${SRCDIR}\src\webissues.ico"

    CreateDirectory "$INSTDIR\Data"
    CreateDirectory "$INSTDIR\Data\settings"
    CreateDirectory "$INSTDIR\Data\profiles"
    CreateDirectory "$INSTDIR\Data\cache"

SectionEnd
