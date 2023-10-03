@echo off
pushd %~dp0\..\
call Application\Premake\premake5.exe vs2022
popd
PAUSE
