#!/bin/bash

ZIP_URL="https://github.com/gfx-rs/wgpu-native/releases/download/v0.19.1.1/wgpu-windows-i686-debug.zip"
DEST_DIR="./wgpu-cmake"
ZIP_FILE="./wgpu-native.zip"

echo "Downloading $ZIP_URL..."
curl -L $ZIP_URL -o "$ZIP_FILE"

echo "Unzipping $ZIP_FILE..."
unzip -q "$ZIP_FILE" -d "$DEST_DIR"

echo "Extraction completed successfully."

echo "Removing downloaded .zip file..."
rm "$ZIP_FILE"
rm "$DEST_DIR/commit-sha"

echo "Cleanup completed successfully."

echo "Setting up wgpuConfig.cmake file"

echo set(WGPU_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include) >> "$DEST_DIR/wgpuConfig.cmake"
echo set(WGPU_LIBRARY_DIR ${CMAKE_CURRENT_LIST_DIR}/lib) >> "$DEST_DIR/wgpuConfig.cmake"
echo if(WGPU_SHARED_LIBS) >> "$DEST_DIR/wgpuConfig.cmake"
echo     set(WGPU_LIBRARY wgpu_native) >> "$DEST_DIR/wgpuConfig.cmake"
echo else() >> "$DEST_DIR/wgpuConfig.cmake"
echo     set(WGPU_LIBRARY wgpu_native.so) >> "$DEST_DIR/wgpuConfig.cmake"
echo endif() >> "$DEST_DIR/wgpuConfig.cmake"
echo if(UNIX AND NOT APPLE) >> "$DEST_DIR/wgpuConfig.cmake"
echo    set(OS_LIBRARIES "-lm" "-ldl") >> "$DEST_DIR/wgpuConfig.cmake"
echo elseif(APPLE) >> "$DEST_DIR/wgpuConfig.cmake"
echo    set(OS_LIBRARIES "-framework CoreFoundation -framework QuartzCore -framework Metal") >> "$DEST_DIR/wgpuConfig.cmake"
echo endif() >> "$DEST_DIR/wgpuConfig.cmake"
echo add_library(wgpu INTERFACE IMPORTED) >> "$DEST_DIR/wgpuConfig.cmake"
echo add_library(WebGPU::WebGPU ALIAS wgpu) >> "$DEST_DIR/wgpuConfig.cmake"
echo set_target_properties(wgpu PROPERTIES INTERFACE_LINK_DIRECTORIES ${WGPU_LIBRARY_DIR}) >> "$DEST_DIR/wgpuConfig.cmake"
echo target_include_directories(wgpu INTERFACE ${WGPU_INCLUDE_DIR}) >> "$DEST_DIR/wgpuConfig.cmake"
echo target_link_libraries(wgpu INTERFACE ${WGPU_LIBRARY} ${OS_LIBRARIES}) >> "$DEST_DIR/wgpuConfig.cmake"
echo install(FILES ${WGPU_LIBRARY} DESTINATION ${CMAKE_BINARY_DIR}) >> "$DEST_DIR/wgpuConfig.cmake"

cmake -B build