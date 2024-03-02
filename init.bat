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
echo set(WGPU_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include) >> .\%DEST_DIR%\wgpuConfig.cmake
echo set(WGPU_LIBRARY_DIR ${CMAKE_CURRENT_LIST_DIR}/lib) >> .\%DEST_DIR%\wgpuConfig.cmake
echo if(WGPU_SHARED_LIBS) >> .\%DEST_DIR%\wgpuConfig.cmake
echo     set(WGPU_LIBRARY wgpu_native) >> .\%DEST_DIR%\wgpuConfig.cmake
echo else() >> .\%DEST_DIR%\wgpuConfig.cmake
echo     set(WGPU_LIBRARY wgpu_native.dll) >> .\%DEST_DIR%\wgpuConfig.cmake
echo endif() >> .\%DEST_DIR%\wgpuConfig.cmake
echo add_library(wgpu INTERFACE IMPORTED) >> .\%DEST_DIR%\wgpuConfig.cmake
echo add_library(WebGPU::WebGPU ALIAS wgpu) >> .\%DEST_DIR%\wgpuConfig.cmake
echo set_target_properties(wgpu PROPERTIES INTERFACE_LINK_DIRECTORIES ${WGPU_LIBRARY_DIR}) >> .\%DEST_DIR%\wgpuConfig.cmake
echo target_include_directories(wgpu INTERFACE ${WGPU_INCLUDE_DIR}) >> .\%DEST_DIR%\wgpuConfig.cmake
echo target_link_libraries(wgpu INTERFACE ${WGPU_LIBRARY} d3dcompiler ws2_32 userenv bcrypt ntdll d3d12 dxgi dxguid opengl32) >> .\%DEST_DIR%\wgpuConfig.cmake
echo install(FILES ${WGPU_LIBRARY} DESTINATION ${CMAKE_BINARY_DIR}) >> .\%DEST_DIR%\wgpuConfig.cmake

REM Generate build files...
cmake -B build -T host=x86 -A win32

echo Installation completed successfully!
