mkdir build
clang -o build/build.exe bootstrap/build.c -g -march=native -std=gnu2x -Wall -Wextra -Wpedantic -Wno-nested-anon-types -Wno-keyword-macro -Wno-gnu-auto-type -Wno-auto-decl-extensions -Wno-gnu-empty-initializer -Wno-fixed-enum-extension -pedantic -fno-exceptions -fno-stack-protector -Wl,/INCREMENTAL:no
SET clang_exit_code=%errorlevel%
echo Clang exit code: %clang_exit_code%
if %clang_exit_code% neq 0 exit /b %clang_exit_code%
.\build\build.exe %*
SET builder_exit_code=%errorlevel%
echo Builder exit code: %builder_exit_code%
if %builder_exit_code% neq 0 exit /b %builder_exit_code%
