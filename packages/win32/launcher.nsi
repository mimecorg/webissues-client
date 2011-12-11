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

!define VERSION "1.0-rc1"
!define BUILDVERSION "1.0.0.4363"

!define SRCDIR "..\.."

!include "MUI2.nsh"
!include "FileFunc.nsh"

!include "languages\launcher_en.nsh"

SetCompressor lzma
SetCompressorDictSize 1
OutFile "portable\WebIssuesPortable.exe"

RequestExecutionLevel user

Name "$(NAME)"
BrandingText "$(BRANDING_NAME)"
InstallButtonText "$(RUN)"

!define MUI_ICON "${SRCDIR}\src\webissues.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "images\header.bmp"
!define MUI_HEADERIMAGE_RIGHT

Page custom ShowLauncherOptions LeaveLauncherOptions "" 

ShowInstDetails nevershow
!insertmacro MUI_PAGE_INSTFILES

AutoCloseWindow true

!insertmacro MUI_LANGUAGE "English"

VIProductVersion "${BUILDVERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "WebIssues Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "WebIssues Portable Client"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) 2007-2011 WebIssues Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "WebIssuesPortable.exe"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "WebIssues Portable Client"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"

Var StoreSettings
Var StoreCache
Var RadioFull
Var RadioNoCache
Var RadioReadOnly
Var TempDir

Function .onInit

    ${GetOptions} "$CMDLINE" "/ShowOptions" $1
    ${If} ${Errors}
        SetSilent silent
    ${EndIf}

FunctionEnd

Function ShowLauncherOptions

    nsDialogs::Create /NOUNLOAD 1018
    Pop $0

    !insertmacro MUI_HEADER_TEXT_PAGE "$(DATASTORE_TITLE)" "$(DATASTORE_SUBTITLE)"

    ${NSD_CreateLabel} 0 0 100% 20u "$(LABEL_SELECTSTORE)"
    Pop $0

    ${NSD_CreateRadioButton} 0 20u 100% 12u "$(RADIO_FULL)"
    Pop $RadioFull
    ${NSD_CreateLabel} 12u 33u 100% 20u "$(LABEL_FULL)"
    Pop $0

    ${NSD_CreateRadioButton} 0 55u 100% 12u "$(RADIO_NOCACHE)"
    Pop $RadioNoCache
    ${NSD_CreateLabel} 12u 68u 100% 20u "$(LABEL_NOCACHE)"
    Pop $0

    ${NSD_CreateRadioButton} 0 90u 100% 12u "$(RADIO_READONLY)"
    Pop $RadioReadOnly
    ${NSD_CreateLabel} 12u 103u 100% 20u "$(LABEL_READONLY)"
    Pop $0

    ${NSD_Check} $RadioFull

    nsDialogs::Show

FunctionEnd

Function LeaveLauncherOptions

    ${NSD_GetState} $RadioFull $0
    ${NSD_GetState} $RadioNoCache $1

    ${If} $0 == ${BST_CHECKED}
        StrCpy $StoreSettings "true"
        StrCpy $StoreCache "true"
    ${ElseIf} $1 == ${BST_CHECKED}
        StrCpy $StoreSettings "true"
        StrCpy $StoreCache "false"
    ${Else}
        StrCpy $StoreSettings "false"
        StrCpy $StoreCache "false"
    ${EndIf}

FunctionEnd

Function SetFileAttributesDirectoryNormal

    Exch $1
    Push $2
    Push $3

    FindFirst $2 $3 "$1\*.*"

    ${While} $3 != ""
        ${If} $3 != "."
        ${AndIf} $3 != ".."
            ${If} ${FileExists} "$1\$3\*.*"
                Push "$1\$3"
                Call SetFileAttributesDirectoryNormal
            ${Else}
                SetFileAttributes "$1\$3" normal
            ${EndIf}
        ${EndIf}
        FindNext $2 $3
    ${EndWhile}

    FindClose $2

    Pop $3
    Pop $2
    Pop $1

FunctionEnd

Function CheckDirectoryWritable

    Exch $1
    Push $2

    ClearErrors
    CreateDirectory "$1"
    FileOpen $2 "$1\writetest.tmp" w
    ${IfNot} ${Errors}
        FileClose $2
        Delete "$1\writetest.tmp"
        ClearErrors
    ${Else}
        SetErrors
    ${EndIf}

    Pop $2
    Pop $1

FunctionEnd

Section

    ${IfNot} ${Silent}
        CreateDirectory "$EXEDIR\Data\settings"
        WriteINIStr "$EXEDIR\Data\settings\portable.ini" "DataStore" "Settings" $StoreSettings
        WriteINIStr "$EXEDIR\Data\settings\portable.ini" "DataStore" "Cache" $StoreCache
    ${Else}
        ${IfNot} ${FileExists} "$EXEDIR\App\WebIssues\bin\Webissues.exe"
            MessageBox MB_OK|MB_ICONEXCLAMATION "$(MESSAGE_NOTFOUND)"
    		Abort
        ${EndIf}

        ${If} ${FileExists} "$EXEDIR\Data\settings\portable.ini"
            ReadINIStr $StoreSettings "$EXEDIR\Data\settings\portable.ini" "DataStore" "Settings"
            ReadINIStr $StoreCache "$EXEDIR\Data\settings\portable.ini" "DataStore" "Cache"

            ${If} $StoreSettings == "true"
            ${OrIf} $StoreCache == "true"
                Push "$EXEDIR\Data"
                Call CheckDirectoryWritable
                ${If} ${Errors}
                    ${If} ${Cmd} `MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(MESSAGE_READONLY)" IDNO`
                        Abort
                    ${EndIf}
                    StrCpy $StoreSettings "false"
                    StrCpy $StoreCache "false"
                ${EndIf}
            ${EndIf}
        ${Else}
            Push "$EXEDIR\Data"
            Call CheckDirectoryWritable
            ${If} ${Errors}
                StrCpy $StoreSettings "false"
                StrCpy $StoreCache "false"
            ${Else}
                ${GetExeName} $0
                ExecWait '"$0" /ShowOptions'
                ${IfNot} ${FileExists} "$EXEDIR\Data\settings\portable.ini"
                    Abort
                ${EndIf}
                ReadINIStr $StoreSettings "$EXEDIR\Data\settings\portable.ini" "DataStore" "Settings"
                ReadINIStr $StoreCache "$EXEDIR\Data\settings\portable.ini" "DataStore" "Cache"
            ${EndIf}
        ${EndIf}

        ${Do}
            System::Call 'ole32::CoCreateGuid(g .s)'
            Pop $0
            StrCpy $1 $0 6 1
            StrCpy $TempDir "$TEMP\WI$1"
        ${LoopWhile} ${FileExists} "$TempDir\*.*"

        CreateDirectory "$TempDir"

        CreateDirectory "$TempDir\data"
        CopyFiles /SILENT "$EXEDIR\Data\profiles\*.*" "$TempDir\data"

        CreateDirectory "$TempDir\cache"
        ${If} $StoreCache == "true"
            CopyFiles /SILENT "$EXEDIR\Data\cache\*.*" "$TempDir\cache"
        ${EndIf}

        CreateDirectory "$TempDir\temp"

        Push "$TempDir"
        Call SetFileAttributesDirectoryNormal

        ExecWait '"$EXEDIR\App\WebIssues\bin\Webissues.exe"  -data "$TempDir\data" -cache "$TempDir\cache" -temp "$TempDir\temp"'

        ${If} $StoreSettings == "true"
            CreateDirectory "$EXEDIR\Data\profiles"
            CopyFiles /SILENT "$TempDir\data\*.*" "$EXEDIR\Data\profiles"
        ${EndIf}

        ${If} $StoreCache == "true"
            CreateDirectory "$EXEDIR\Data\cache"
            CopyFiles /SILENT "$TempDir\cache\*.*" "$EXEDIR\Data\cache"
        ${EndIf}

        RMDir /r "$TempDir"
    ${EndIf}

SectionEnd
