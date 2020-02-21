## URDE
**Status:** In-Game with 0-00 (v1.088) only

**Official Discord Channel:** https://discord.gg/AMBVFuf

### Download
Precompiled builds of the command-line extraction utility (`hecl`) with embedded dataspec libraries are available at https://github.com/AxioDL/urde/releases. This will give you intermediate dumps of original formats as *blender* and *yaml* representations.

Everything else is much too experimental to make portable/stable release builds (for now)

### Platform Support
* Windows 7+ (64-bit support only)
* macOS 10.11+
* Linux
    * Arch is known to function with [`glx` vendor setup instructions](https://wiki.archlinux.org/index.php/Category:Graphics) *(main development/testing OS)*
    * **[WIP]** Vulkan loader detection is also integrated into the cmake for Linux
* **[Coming Soon]** FreeBSD
    * Much multimedia functionality is in place, but not fully tested

### Build Prerequisites:
* [CMake 3.13+](https://cmake.org)
* [Python 3+](https://python.org)
* LLVM development package *(headers and libs)*
    * [Specialized Windows Package](https://axiodl.com/files/LLVM-9.0.1-win64.exe)
    * [Specialized macOS Package](https://axiodl.com/files/LLVM-9.0.1-Darwin.tar.xz)
* **[Windows]** [Visual Studio 2017/2019 and Windows SDK](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx)
* **[macOS]** [Xcode Tools](https://developer.apple.com/xcode/download/)
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
make -j$(nproc)
```

CMake options:
- Build release optimized (better runtime performance): `-DCMAKE_BUILD_TYPE=Release`
- Use clang+lld (faster linking): `-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++`
- Optimize for current CPU (resulting binaries are not portable): `-DURDE_VECTOR_ISA=native`

#### Qt Creator
*(main development / debugging IDE)*

Open the repository's `CMakeLists.txt` via File > Open File or Project.

Configure the desired CMake targets to build in the *Projects* area of the IDE.

Build / Debug / Run on Windows, macOS and Linux in a unified way.

#### Visual Studio

Open the `urde` directory in Visual Studio (automatically imports CMake configuration)

Follow these instructions to use clang-cl for the build process https://docs.microsoft.com/en-us/cpp/build/clang-support-cmake?view=vs-2019

#### Xcode

```sh
cmake -G Xcode -DCMAKE_BUILD_TYPE=Debug -DLLVM_ROOT_DIR=<path-to-llvm-dev-package> ../urde
```

Then open `urde.xcodeproj`
