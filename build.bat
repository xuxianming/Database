@echo off
REM build.bat

setlocal enabledelayedexpansion

set BUILD_DIR=build
set INSTALL_PREFIX=C:\Program Files\MyProject
set BUILD_TYPE=Release
set SKIP_INSTALL=0
set CLEAN_ONLY=0
set JOBS=%NUMBER_OF_PROCESSORS%
set GENERATOR=auto
set CMAKE_GENERATOR=

:parse_args
if "%1"=="" goto :end_parse
if "%1"=="-c" (
    set CLEAN_ONLY=1
    set SKIP_INSTALL=1
    shift
    goto :parse_args
)
if "%1"=="-h" goto :show_help
if "%1"=="-b" (
    set BUILD_DIR=%2
    shift
    shift
    goto :parse_args
)
if "%1"=="-p" (
    set INSTALL_PREFIX=%2
    shift
    shift
    goto :parse_args
)
if "%1"=="-t" (
    set BUILD_TYPE=%2
    shift
    shift
    goto :parse_args
)
if "%1"=="-j" (
    set JOBS=%2
    shift
    shift
    goto :parse_args
)
if "%1"=="-g" (
    set GENERATOR=%2
    shift
    shift
    goto :parse_args
)
echo Unknown option: %1
goto :show_help

:end_parse
goto :start_build

:show_help
echo Usage: %0 [OPTIONS]
echo.
echo Options:
echo   -c                Clean, build but do NOT install
echo   -h                Show this help
echo   -b ^<dir^>          Build directory
echo   -p ^<prefix^>       Install prefix
echo   -t ^<type^>         Build type: Debug/Release
echo   -j ^<jobs^>         Parallel jobs
echo   -g ^<generator^>    vs2022/vs2019/ninja/mingw/auto
exit /b 0

:start_build

if "%GENERATOR%"=="auto" (
    where ninja >nul 2>&1
    if not errorlevel 1 (
        set CMAKE_GENERATOR=-G "Ninja"
        goto :generator_detected
    )
    where cl.exe >nul 2>&1
    if not errorlevel 1 (
        set CMAKE_GENERATOR=-G "Visual Studio 17 2022" -A x64
        goto :generator_detected
    )
    where g++.exe >nul 2>&1
    if not errorlevel 1 (
        set CMAKE_GENERATOR=-G "MinGW Makefiles"
        goto :generator_detected
    )
    echo No suitable generator found!
    exit /b 1
) else (
    if "%GENERATOR%"=="vs2022" (
        set CMAKE_GENERATOR=-G "Visual Studio 17 2022" -A x64
    ) else if "%GENERATOR%"=="vs2019" (
        set CMAKE_GENERATOR=-G "Visual Studio 16 2019" -A x64
    ) else if "%GENERATOR%"=="ninja" (
        set CMAKE_GENERATOR=-G "Ninja"
    ) else if "%GENERATOR%"=="mingw" (
        set CMAKE_GENERATOR=-G "MinGW Makefiles"
    ) else (
        echo Unknown generator: %GENERATOR%
        exit /b 1
    )
)

:generator_detected

if "%CLEAN_ONLY%"=="1" (
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
    )
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo Configuring...
cmake .. %CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX="%INSTALL_PREFIX%"
if errorlevel 1 (
    cd ..
    exit /b 1
)

echo Building...
if "%GENERATOR%"=="mingw" (
    mingw32-make -j%JOBS%
) else if "%GENERATOR%"=="ninja" (
    ninja
) else (
    cmake --build . --parallel %JOBS% --config %BUILD_TYPE%
)
if errorlevel 1 (
    cd ..
    exit /b 1
)

if "%SKIP_INSTALL%"=="0" (
    echo Installing...
    echo %INSTALL_PREFIX% | findstr /i "Program Files" >nul
    if errorlevel 1 (
        if "%GENERATOR%"=="mingw" (
            mingw32-make install
        ) else if "%GENERATOR%"=="ninja" (
            ninja install
        ) else (
            cmake --install . --config %BUILD_TYPE%
        )
        if errorlevel 1 (
            cd ..
            exit /b 1
        )
    ) else (
        echo Skipping install to Program Files
    )
)

cd ..
echo Build completed successfully!
exit /b 0