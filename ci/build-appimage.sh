#!/bin/bash -ex
shopt -s extglob

# Get linuxdeploy
cd "$RUNNER_WORKSPACE"
curl -OL https://github.com/encounter/linuxdeploy/releases/download/continuous/linuxdeploy-$(uname -m).AppImage
curl -OL https://github.com/encounter/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-$(uname -m).AppImage
chmod +x linuxdeploy-$(uname -m).AppImage linuxdeploy-plugin-qt-$(uname -m).AppImage

# Build AppImage
cd "$GITHUB_WORKSPACE"
mkdir -p build/appdir/usr/{bin,share/{applications,icons/hicolor}}
cp build/install/!(*.*) build/appdir/usr/bin
cp -r metaforce-gui/platforms/freedesktop/{16x16,32x32,48x48,64x64,128x128,256x256,512x512,1024x1024} build/appdir/usr/share/icons/hicolor
cp metaforce-gui/platforms/freedesktop/metaforce.desktop build/appdir/usr/share/applications

cd build/install
VERSION="$METAFORCE_VERSION" NO_STRIP=1 "$RUNNER_WORKSPACE"/linuxdeploy-$(uname -m).AppImage \
  --appdir "$GITHUB_WORKSPACE"/build/appdir \
  --plugin qt --output appimage
