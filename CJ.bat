@echo off
gcc Main.c -o Compiler.exe

if %errorlevel% neq 0 (
    exit /b
) else (
    .\Compiler.exe
    del Compiler.exe

    nasm -f win64 Main.asm -o Main.obj

    if %errorlevel% neq 0 (
        exit /b
    )
    GoLink /console /entry main Main.obj kernel32.dll >nul

    if %errorlevel% neq 0 (
        exit /b
    )

    del Main.obj
    rem del Main.asm
)
