@echo off

@REM check if we already have the tools in the environment
if exist "%VCINSTALLDIR%" (
	exit
)

@REM Check for Visual Studio
if exist "%VS140COMNTOOLS%" (
	set VSPATH="%VS140COMNTOOLS%"
	goto set_env
)
if exist "%VS120COMNTOOLS%" (
	set VSPATH="%VS120COMNTOOLS%"
	goto set_env
)
if exist "%VS100COMNTOOLS%" (
	set VSPATH="%VS100COMNTOOLS%"
	goto set_env
)
if exist "%VS90COMNTOOLS%" (
	set VSPATH="%VS90COMNTOOLS%"
	goto set_env
)
if exist "%VS80COMNTOOLS%" (
	set VSPATH="%VS80COMNTOOLS%"
	goto set_env
)

echo You need Microsoft Visual Studio 8, 9, 10, 12 or 14 installed
pause
exit

@ setup the environment
:set_env
@%comspec% /k "%VSPATH:~0,-15%VC\vcvarsall.bat"" x86