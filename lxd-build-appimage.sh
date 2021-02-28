#!/bin/bash -ex

###############################################################
# Uses LXD to create an Ubuntu focal container and produce   #
# a reasonably portable AppImage of URDE.                    #
###############################################################

VERSION=${VERSION:-local}
CONTAINER_NAME=urde-ci

# Set up container, deleting existing if necessary
if lxc info $CONTAINER_NAME >& /dev/null
then
    lxc delete $CONTAINER_NAME --force
fi
lxc init ubuntu:20.04 $CONTAINER_NAME

# Inject build script
lxc file push - $CONTAINER_NAME/root/dobuild.sh <<END
set -e

# URDE build script for Ubuntu 20.04 LTS (Focal)

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
rm -rf urde{-build,-appdir,}

# Clone repository
git clone https://github.com/AxioDL/urde.git --recursive

# Build
mkdir -p urde{-build,-appdir}
pushd urde-build
cmake -GNinja -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
	            -DURDE_DLPACKAGE=urde-$VERSION-linux-$(uname -m) -DURDE_VECTOR_ISA=sse41 \
              /urde
ninja hecl hecl-gui urde visigen
popd

mkdir -p /urde-appdir/usr/{bin,share/{applications,icons/hicolor}}
cp /urde-build/Binaries/{hecl,hecl-gui,urde,visigen} /urde-appdir/usr/bin
strip -s /urde-appdir/usr/bin/{hecl,hecl-gui,urde,visigen}
cp -r /urde/Editor/platforms/freedesktop/{16x16,32x32,48x48,64x64,128x128,256x256,512x512,1024x1024} /urde-appdir/usr/share/icons/hicolor
cp /urde/Editor/platforms/freedesktop/urde.desktop /urde-appdir/usr/share/applications
sed -i 's/Exec=.*/Exec=hecl-gui/' /urde-appdir/usr/share/applications/urde.desktop
VERSION=$VERSION /linuxdeploy-$(uname -m).AppImage --appdir /urde-appdir --plugin qt --output appimage
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
