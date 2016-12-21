## URDE
**Status:** Extract Only
[![Build status](https://ci.appveyor.com/api/projects/status/avx3mpu1gd75ju1b?svg=true)](https://ci.appveyor.com/project/jackoalan/urde)

### Download
Precompiled builds of the command-line extraction utility (`hecl`) with embedded dataspec libraries are available at https://github.com/AxioDL/urde/releases. This will give you intermediate dumps of original formats as *blender* and *yaml* representations.

Everything else is much too experimental to make portable/stable release builds (for now)

### Platform Support
* Windows 7+ (64-bit support only)
* Mac OS X 10.9+
* Linux
    * Arch is known to function with [`glx` vendor setup instructions](https://wiki.archlinux.org/index.php/Category:Graphics) *(main development/testing OS)*
    * **[WIP]** Vulkan loader detection is also integrated into the cmake for Linux
* **[Coming Soon]** FreeBSD
    * Much multimedia functionality is in place, but not fully tested

### Build Prerequisites:
* [CMake 3+](https://cmake.org)
* [Python 3+](https://python.org)
* LLVM development package *(headers and libs)*
    * [Specialized Windows Package](https://www.dropbox.com/s/8vz8ogsxjhhxoi2/LLVM-4.0.0svn-win64-athena.exe?dl=1)
    * [OS X Package](http://llvm.org/releases/3.9.0/clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz)
* **[Windows]** [Visual Studio 2015 and Windows SDK](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx)
* **[OS X]** [Xcode Tools](https://developer.apple.com/xcode/download/)
* **[Linux]** recent development packages of `udev`, `x11`, `xcb`, `xinput`, `glx`, `asound`

### Prep Directions

```sh
git clone --recurse-submodules https://github.com/AxioDL/urde.git
mkdir urde-build
cd urde-build
```

### Build Directions (Debug)

#### make

```sh
cmake -DCMAKE_BUILD_TYPE=Debug ../urde
make
```

#### Qt Creator
*(main development / debugging IDE)*

Open the repository's `CMakeLists.txt` via File > Open File or Project.

Configure the desired CMake targets to build in the *Projects* area of the IDE.

Build / Debug / Run on Windows, OS X and Linux in a unified way.

#### Visual Studio

Using *Git Shell* with [GitHub for Windows](https://desktop.github.com/)

Windows 7/8 users should run:

```sh
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Debug ../urde
```

Windows 10 users should run:

```sh
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_SYSTEM_VERSION=10.0 -DCMAKE_BUILD_TYPE=Debug ../urde
```

Then open `urde.sln`

#### Xcode

```sh
cmake -G Xcode -DCMAKE_BUILD_TYPE=Debug -DLLVM_ROOT_DIR=<path-to-llvm-dev-package> ../urde
```

Then open `urde.xcodeproj`
