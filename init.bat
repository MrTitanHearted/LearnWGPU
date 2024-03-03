@echo off

set ZIP_URL=https://github.com/gfx-rs/wgpu-native/releases/download/v0.19.1.1/wgpu-windows-i686-debug.zip
set DEST_DIR=.\wgpu-cmake
set ZIP_FILE=.\wgpu-native.zip

curl -o %ZIP_FILE% -L "%ZIP_URL%"

REM Unzip the file
powershell.exe -nologo -noprofile -command "Expand-Archive -Path %ZIP_FILE% -DestinationPath '.\%DEST_DIR%'"

mkdir ".\%DEST_DIR%\include"
mkdir ".\%DEST_DIR%\include\wgpu"
mkdir ".\%DEST_DIR%\lib"
move ".\%DEST_DIR%\*.h" ".\%DEST_DIR%\include\wgpu"
move ".\%DEST_DIR%\*.lib" ".\%DEST_DIR%\lib"
move ".\%DEST_DIR%\*.pdb" ".\%DEST_DIR%\lib"
move ".\%DEST_DIR%\*.dll" ".\%DEST_DIR%\lib"

REM Clean up
del %ZIP_FILE%
del ".\%DEST_DIR%\commit-sha"

REM Set up wgpuConfig.cmake file
copy scripts\windows.cmake "%DEST_DIR%\wgpuConfig.cmake"

REM Generate build files...
cmake -B build -T host=x86 -A win32

echo Installation completed successfully!
