@echo off
echo Building Ethereum Embedded Signer...

REM Check if GCC is available
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: GCC not found in PATH. Please install MinGW, Cygwin, or MSYS2.
    echo You can install MinGW using: winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT
    exit /b 1
)

REM Create build directory
if not exist build mkdir build

REM Compile the project
gcc -o build\eth_signer.exe src\main.c src\crypto.c src\rlp.c src\transaction.c -Iinclude -std=c11 -Wall -Wextra

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
) else (
    echo Build successful! Executable created at: build\eth_signer.exe
    echo Run the program using: build\eth_signer.exe
)

exit /b 0 