#!/bin/bash -ex

###############################################################
# Uses LXD to create an Ubuntu focal container and produce   #
# a reasonably portable AppImage of Metaforce.               #
###############################################################

VERSION=${VERSION:-local}
CONTAINER_NAME=metaforce-ci

# Set up container, deleting existing if necessary
if lxc info $CONTAINER_NAME >& /dev/null
then
    lxc delete $CONTAINER_NAME --force
fi
lxc init ubuntu:20.04 $CONTAINER_NAME

# Inject build script
lxc file push - $CONTAINER_NAME/root/dobuild.sh <<END
set -e

# Metaforce build script for Ubuntu 20.04 LTS (Focal)

# Install build dependencies
apt update
apt -y install build-essential curl git ninja-build llvm-dev libclang-dev clang lld zlib1g-dev \
       libglu1-mesa-dev libdbus-1-dev libvulkan-dev libxi-dev libxrandr-dev libasound2-dev libpulse-dev \
       libudev-dev libpng-dev libncurses5-dev cmake libx11-xcb-dev python3 python-is-python3 qt5-default

# Doing everything in root is fine
cd /

# Get linuxdeploy
curl -OL https://github.com/encounter/linuxdeploy/releases/download/continuous/linuxdeploy-$(uname -m).AppImage
curl -OL https://github.com/encounter/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-$(uname -m).AppImage
chmod +x linuxdeploy-$(uname -m).AppImage linuxdeploy-plugin-qt-$(uname -m).AppImage

# Cleanup
rm -rf metaforce{-build,-appdir,}

# Clone repository
git clone https://github.com/AxioDL/metaforce.git --recursive

# Build
mkdir -p metaforce{-build,-appdir}
pushd metaforce-build
cmake -GNinja -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
              /metaforce
ninja hecl metaforce-gui metaforce visigen
popd

mkdir -p /metaforce-appdir/usr/{bin,share/{applications,icons/hicolor}}
cp /metaforce-build/Binaries/{hecl,metaforce-gui,metaforce,visigen} /metaforce-appdir/usr/bin
strip -s /metaforce-appdir/usr/bin/{hecl,metaforce-gui,metaforce,visigen}
cp -r /metaforce/Editor/platforms/freedesktop/{16x16,32x32,48x48,64x64,128x128,256x256,512x512,1024x1024} /metaforce-appdir/usr/share/icons/hicolor
cp /metaforce/Editor/platforms/freedesktop/metaforce.desktop /metaforce-appdir/usr/share/applications
sed -i 's/Exec=.*/Exec=metaforce-gui/' /metaforce-appdir/usr/share/applications/metaforce.desktop
VERSION=$VERSION /linuxdeploy-$(uname -m).AppImage --appdir /metaforce-appdir --plugin qt --output appimage
END

# Start container
lxc start $CONTAINER_NAME

# Wait for network
lxc exec $CONTAINER_NAME -- bash -c "while ! systemctl status network.target &>/dev/null; do echo 'Waiting for network...'; sleep 1; done"

# Run build script
lxc exec $CONTAINER_NAME -t -- bash /root/dobuild.sh

# Retrieve AppImage 
lxc file pull $CONTAINER_NAME/URDE-$VERSION-$(uname -m).AppImage .

# Cleanup
lxc delete $CONTAINER_NAME --force
