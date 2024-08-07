:: SPDX-FileCopyrightText: Stone Tickle <lattis@mochiro.moe>
:: SPDX-License-Identifier: GPL-3.0-only

@echo off
setlocal
cd /D "%~dp0"

if "%~1" == "" goto :usage

set dir=%1
if not exist "%dir%" mkdir "%dir%"

call cl /nologo /Zi /std:c11 /Iinclude src/amalgam.c /link /out:"%dir%/muon-bootstrap.exe"
goto :eof

:usage
echo usage: %0 build_dir
