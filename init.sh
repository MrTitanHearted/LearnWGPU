#!/bin/bash

ZIP_URL="https://github.com/gfx-rs/wgpu-native/releases/download/v0.19.3.1/wgpu-windows-x86_64-debug.zip"
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
cp scripts/unix.cmake "$DEST_DIR/wgpuConfig.cmake"

cmake -B build