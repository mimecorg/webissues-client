@echo off

if "%1" == "" goto usage

set VERSION=1.1.2
set BUILDVERSION=1.1.2.5260

set NSISDIR=C:\Program Files (x86)\NSIS\Unicode

if not %1 == x86 goto elsex64

set BUILDDIR=D:\Release\webissues-x86

set QTDIR=D:\Qt\4.8-x86
set OPENSSLDIR=D:\OpenSSL\x86
set VCRTDIR=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT

set ARCHITECTURE=win_x86
set SUFFIX=

goto endopt
:elsex64

if not %1 == x64 goto usage

set BUILDDIR=D:\Release\webissues-x64

set QTDIR=D:\Qt\4.8-x64
set OPENSSLDIR=D:\OpenSSL\x64
set VCRTDIR=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT

set ARCHITECTURE=win_x64
set SUFFIX= (64-bit)

goto endopt
:usage

echo usage: build-bat x86^|x64

goto :eof
:endopt

call "%QTDIR%\bin\qtvars.bat" vsvars

if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"

echo.
echo Exporting...
echo.

svn export --quiet --force ..\.. "%BUILDDIR%"

pushd "%BUILDDIR%"

call configure.bat

nmake release

if errorlevel 1 goto cleanup

call ..\sign.bat release\webissues.exe

echo.
echo Creating uninstaller...
echo.

"%NSISDIR%\makensis.exe" /DINNER "/DQTDIR=%QTDIR%" "/DOPENSSLDIR=%OPENSSLDIR%" "/DVCRTDIR=%VCRTDIR%" "/DVERSION=%VERSION%" "/DBUILDVERSION=%BUILDVERSION%" "/DARCHITECTURE=%ARCHITECTURE%" "/DSUFFIX=%SUFFIX%" /V2 packages\win32\webissues.nsi

if errorlevel 1 goto cleanup

"%TEMP%\innerinst.exe"

call ..\sign.bat "%TEMP%\uninstall.exe"

echo.
echo Creating installer...
echo.

"%NSISDIR%\makensis.exe" "/DQTDIR=%QTDIR%" "/DOPENSSLDIR=%OPENSSLDIR%" "/DVCRTDIR=%VCRTDIR%" "/DVERSION=%VERSION%" "/DBUILDVERSION=%BUILDVERSION%" "/DARCHITECTURE=%ARCHITECTURE%" "/DSUFFIX=%SUFFIX%" /V2 packages\win32\webissues.nsi

if errorlevel 1 goto cleanup

call ..\sign.bat "packages\win32\webissues-%VERSION%-%ARCHITECTURE%.exe"

if not %1 == x86 goto cleanup

echo.
echo Creating portable launcher...
echo.

"%NSISDIR%\makensis.exe" "/DVERSION=%VERSION%" "/DBUILDVERSION=%BUILDVERSION%" /V2 packages\win32\launcher.nsi

if errorlevel 1 goto cleanup

call ..\sign.bat "packages\win32\portable\WebIssuesPortable.exe"

echo.
echo Creating portable installer...
echo.

"%NSISDIR%\makensis.exe" "/DQTDIR=%QTDIR%" "/DOPENSSLDIR=%OPENSSLDIR%" "/DVCRTDIR=%VCRTDIR%" "/DVERSION=%VERSION%" "/DBUILDVERSION=%BUILDVERSION%" /V2 packages\win32\portable.nsi

if errorlevel 1 goto cleanup

call ..\sign.bat "packages\win32\webissues-portable-%VERSION%.paf.exe"

:cleanup

del /q /f "%TEMP%\innerinst.exe"
del /q /f "%TEMP%\uninstall.exe"

popd
