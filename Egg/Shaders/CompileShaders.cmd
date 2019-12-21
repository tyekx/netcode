@echo off
rem Copyright (c) Microsoft Corporation. All rights reserved.
rem Licensed under the MIT License.

setlocal
set error=0

set FXCOPTS=/nologo /WX /Ges /Zi /Zpc /Qstrip_reflect /Qstrip_debug

if %1.==xbox. goto continuexbox
if %1.==dxil. goto continuedxil
if %1.==. goto continuepc
echo usage: CompileShaders [xbox]
exit /b

:continuexbox
set XBOXPREFIX=XboxOne
set XBOXOPTS=/D__XBOX_DISABLE_SHADER_NAME_EMPLACEMENT
if NOT %2.==noprecompile. goto skipnoprecompile
set XBOXOPTS=%XBOXOPTS% /D__XBOX_DISABLE_PRECOMPILE=1
:skipnoprecompile

set XBOXFXC="%XboxOneXDKLatest%\xdk\FXC\amd64\FXC.exe"
if exist %XBOXFXC% goto continue
set XBOXFXC="%XboxOneXDKLatest%xdk\FXC\amd64\FXC.exe"
if exist %XBOXFXC% goto continue
set XBOXFXC="%XboxOneXDKBuild%xdk\FXC\amd64\FXC.exe"
if exist %XBOXFXC% goto continue
set XBOXFXC="%DurangoXDK%xdk\FXC\amd64\FXC.exe"
if not exist %XBOXFXC% goto needxdk
goto continue

:continuedxil
set PCDXC="%WindowsSdkBinPath%%WindowsSDKVersion%\x86\dxc.exe"
if exist %PCDXC% goto continue
goto needdxil

:continuepc
set PCOPTS=/force_rootsig_ver rootsig_1_0

set PCFXC="%WindowsSdkBinPath%%WindowsSDKVersion%\x86\fxc.exe"
if exist %PCFXC% goto continue
set PCFXC="%WindowsSdkDir%bin\%WindowsSDKVersion%\x86\fxc.exe"
if exist %PCFXC% goto continue

set PCFXC=fxc.exe

:continue
@if not exist Compiled mkdir Compiled

call :CompileShader%1 SpriteFont vs SpriteVertexShader
call :CompileShader%1 SpriteFont ps SpritePixelShader

:skipxboxonly

echo.

if %error% == 0 (
    echo Shaders compiled ok
) else (
    echo There were shader compilation errors!
)

endlocal
exit /b

:CompileShader
set fxc=%PCFXC% %1.hlsl %FXCOPTS% /T%2_5_1 %PCOPTS% /E%3 /FhCompiled\%1_%3.inc /FdCompiled\%1_%3.pdb /Vn%1_%3
echo.
echo %fxc%
%fxc% || set error=1
exit /b

:CompileComputeShader
set fxc=%PCFXC% %1.hlsl %FXCOPTS% /Tcs_5_1 %PCOPTS% /E%2 /FhCompiled\%1_%2.inc /FdCompiled\%1_%2.pdb /Vn%1_%2
echo.
echo %fxc%
%fxc% || set error=1
exit /b

:CompileShaderdxil
set dxc=%PCDXC% %1.hlsl %FXCOPTS% /T%2_6_0 /E%3 /FhCompiled\%1_%3.inc /FdCompiled\%1_%3.pdb /Vn%1_%3
echo.
echo %dxc%
%dxc% || set error=1
exit /b

:CompileComputeShaderdxil
set dxc=%PCDXC% %1.hlsl %FXCOPTS% /Tcs_6_0 /E%2 /FhCompiled\%1_%2.inc /FdCompiled\%1_%2.pdb /Vn%1_%2
echo.
echo %dxc%
%dxc% || set error=1
exit /b

:needxdk
echo ERROR: CompileShaders xbox requires the Microsoft Xbox One XDK
echo        (try re-running from the XDK Command Prompt)
exit /b

:needdxil
echo ERROR: CompileShaders dxil requires the Microsoft Windows 10 SDK (16299 or later)
exit /b
