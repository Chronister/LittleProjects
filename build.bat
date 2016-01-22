@echo off
setlocal

IF NOT EXIST build mkdir build
pushd build

:: 64 bit
call "%VC%\vcvarsall.bat" amd64
set SUBSYSTEM=5.2
IF NOT EXIST win64 mkdir win64
pushd win64

:: :: 32 bit
:: call "%VC%\vcvarsall.bat" amd64_x86
:: set SUBSYSTEM=5.1
:: IF NOT EXIST win32 mkdir win32
:: pushd win32

if "%~1" NEQ "" (
  if /i "%~1"=="headerify"   goto headerify
  goto eof
)
goto headerify

:headerify
    echo Compiling Headerify...
	cl /nologo /MTd /Gm- /GR- /EHa- /Od /Oi /WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /wd4706 /wd4996 /wd4127 /FC /Z7 /Fm ../../code/headerify.c /link /incremental:no /opt:ref /subsystem:console,%SUBSYSTEM% /OUT:header.exe
    echo.
goto :eof


:eof
popd
popd
