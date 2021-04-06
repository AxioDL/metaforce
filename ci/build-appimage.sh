#!/bin/bash -ex

# Get linuxdeploy
curl -OL https://github.com/encounter/linuxdeploy/releases/download/continuous/linuxdeploy-$(uname -m).AppImage
curl -OL https://github.com/encounter/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-$(uname -m).AppImage
chmod +x linuxdeploy-$(uname -m).AppImage linuxdeploy-plugin-qt-$(uname -m).AppImage

# Build AppImage
mkdir -p appdir/usr/{bin,share/{applications,icons/hicolor}}
cp build/Binaries/{hecl,metaforce-gui,metaforce,visigen} appdir/usr/bin
strip -S appdir/usr/bin/{hecl,metaforce-gui,metaforce,visigen}
cp -r metaforce-gui/platforms/freedesktop/{16x16,32x32,48x48,64x64,128x128,256x256,512x512,1024x1024} appdir/usr/share/icons/hicolor
cp metaforce-gui/platforms/freedesktop/metaforce.desktop appdir/usr/share/applications
VERSION="$METAFORCE_VERSION" ./linuxdeploy-$(uname -m).AppImage --appdir appdir --plugin qt --output appimage
