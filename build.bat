@echo off
REM Windows批处理构建脚本

setlocal EnableDelayedExpansion

REM 默认设置
set BUILD_TYPE=Release
set BUILD_DIR=build
set INSTALL_DIR=install
set GENERATOR=
set USE_CONFIGURE=0
set CLEAN_BUILD=0
set THREADS=4
set VERBOSE=0

REM 获取CPU核心数
for /f "tokens=*" %%i in ('wmic cpu get NumberOfCores /value ^| find "NumberOfCores"') do (
    for /f "tokens=2 delims==" %%j in ("%%i") do (
        set /a THREADS+=%%j
    )
)

REM 显示帮助信息
if "%~1"=="-h" goto :show_help
if "%~1"=="--help" goto :show_help

echo =========================================
echo CppCodeGenerator Build Configuration
echo =========================================
echo Build Type: %BUILD_TYPE%
echo Install Directory: %INSTALL_DIR%
echo Parallel Jobs: %THREADS%

if "%USE_CONFIGURE%"=="1" (
    echo Build System: Autotools
) else (
    echo Build System: CMake
)
echo =========================================

REM 清理构建目录
if "%CLEAN_BUILD%"=="1" if exist "%BUILD_DIR%" (
    echo Cleaning build directory: %BUILD_DIR%
    rmdir /s /q "%BUILD_DIR%"
)

REM 创建构建目录
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM 检查依赖
echo Checking dependencies...
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found. Please install CMake 3.16 or later.
    exit /b 1
)

REM 检查编译器
where cl >nul 2>&1
if not errorlevel 1 (
    set COMPILER=MSVC
    echo Found C++ compiler: MSVC
) else (
    where g++ >nul 2>&1
    if not errorlevel 1 (
        set COMPILER=MinGW
        echo Found C++ compiler: MinGW
    ) else (
        echo [ERROR] No C++ compiler found. Please install MSVC or MinGW.
        exit /b 1
    )
)

REM 检查Boost
if not exist "C:\boost\include\boost" (
    if not exist "C:\local\boost\include\boost" (
        if not exist "\boost\include\boost" (
            echo [ERROR] Boost libraries not found. Please install Boost 1.65.0 or later.
            exit /b 1
        )
    )
)

echo Found Boost libraries.

REM 开始构建
if "%USE_CONFIGURE%"=="1" (
    call :build_with_autotools
) else (
    call :build_with_cmake
)

echo =========================================
echo Build completed successfully!
echo Install directory: %INSTALL_DIR%
echo =========================================
goto :eof

REM Autotools构建函数
:build_with_autotools
echo Building with Autotools...
cd "%BUILD_DIR%"

REM 生成configure脚本
if not exist "configure" (
    echo Generating configure script...
    if exist "autogen.sh" (
        bash autogen.sh
    ) else (
        autoreconf -fi
    )
)

REM 配置
set configure_args=--prefix=%CD%\..\%INSTALL_DIR%
if "%COMPILER%"=="MSVC" (
    set configure_args=!configure_args! CXXFLAGS="-std=c++11 -Wall -Wextra"
) else (
    set configure_args=!configure_args! CXXFLAGS="-std=c++11 -Wall -Wextra -pedantic"
)

echo Running configure...
call configure !configure_args!

echo Building...
make -j%THREADS%

echo Installing...
make install

echo Autotools build completed successfully!
goto :eof

REM CMake构建函数
:build_with_cmake
echo Building with CMake...
cd "%BUILD_DIR%"

REM CMake命令
set cmake_cmd=cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX=%CD%\..\%INSTALL_DIR%

REM 添加生成器
if defined GENERATOR (
    set cmake_cmd=!cmake_cmd! -G "%GENERATOR%"
) else (
    where ninja >nul 2>&1
    if not errorlevel 1 (
        set cmake_cmd=!cmake_cmd! -G Ninja
    ) else (
        set cmake_cmd=!cmake_cmd! -G "Visual Studio 16 2019"
    )
)

REM Boost路径
if exist "C:\boost" (
    set cmake_cmd=!cmake_cmd! -DBOOST_ROOT=C:\boost
) else if exist "C:\local\boost" (
    set cmake_cmd=!cmake_cmd! -DBOOST_ROOT=C:\local\boost
)

echo Running CMake...
call !cmake_cmd!

echo Building...
cmake --build . --config %BUILD_TYPE% --parallel %THREADS%
cmake --install . --config %BUILD_TYPE%

echo CMake build completed successfully!
goto :eof

REM 显示帮助信息
:show_help
echo Usage: %~nx0 [OPTIONS] [INSTALL_DIR]
echo.
echo Build script for CppCodeGenerator (Windows)
echo.
echo OPTIONS:
echo   -h, --help           Show this help message
echo   -d, --debug          Build in debug mode
echo   -r, --release        Build in release mode (default)
echo   -c, --configure      Use autotools build system
echo   -m, --cmake          Use CMake build system (default)
echo   -g, --generator GEN  CMake generator (e.g., "Visual Studio 16 2019")
echo   -C, --clean          Clean build directory before building
echo   -j, --jobs N         Number of parallel jobs (default: auto)
echo   --msvc               Use MSVC compiler
echo   --mingw              Use MinGW compiler
echo.
echo EXAMPLES:
echo   %~nx0                 # Build with CMake in release mode
echo   %~nx0 -d              # Build with CMake in debug mode
echo   %~nx0 -c              # Build with autotools
echo   %~nx0 -g "Visual Studio 16 2019"  # Use specific CMake generator
echo   %~nx0 -C -d           # Clean and debug build
echo   %~nx0 --msvc          # Use MSVC compiler
goto :eof

endlocal