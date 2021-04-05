#!/bin/bash -ex

# Get linuxdeploy
curl -OL https://github.com/encounter/linuxdeploy/releases/download/continuous/linuxdeploy-$(uname -m).AppImage
curl -OL https://github.com/encounter/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-$(uname -m).AppImage
chmod +x linuxdeploy-$(uname -m).AppImage linuxdeploy-plugin-qt-$(uname -m).AppImage

# Build AppImage
mkdir -p appdir/usr/{bin,share/{applications,icons/hicolor}}
cp build/Binaries/{hecl,hecl-gui,urde,visigen} appdir/usr/bin
strip -S appdir/usr/bin/{hecl,hecl-gui,urde,visigen}
cp -r Editor/platforms/freedesktop/{16x16,32x32,48x48,64x64,128x128,256x256,512x512,1024x1024} appdir/usr/share/icons/hicolor
cp Editor/platforms/freedesktop/urde.desktop appdir/usr/share/applications
sed -i 's/Exec=.*/Exec=hecl-gui/' appdir/usr/share/applications/urde.desktop
VERSION="$URDE_VERSION" ./linuxdeploy-$(uname -m).AppImage --appdir appdir --plugin qt --output appimage
