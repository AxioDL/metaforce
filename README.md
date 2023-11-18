## Metaforce [![Build Status]][actions] [![Discord Badge]][discord]

[Build Status]: https://github.com/AxioDL/metaforce/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/AxioDL/metaforce/actions
[Discord Badge]: https://dcbadge.vercel.app/api/server/AMBVFuf?style=flat
[discord]: https://discord.gg/AMBVFuf

A reverse-engineered, native reimplementation of Metroid Prime.

This project is currently in **alpha** state.  
Builds are currently unavailable while the project undergoes large changes.

Separately, a [matching decompilation](https://github.com/PrimeDecomp/prime) of Metroid Prime is currently underway. Contributions are welcome.  
Progress on the decompilation benefits Metaforce with bug fixes and new implementations.

![Metaforce screenshot](assets/metaforce-screen1.png)

### Platform Support
* Windows 10+ (64-bit, D3D12 / Vulkan / OpenGL)
* macOS 10.15+ (Metal)
* Linux (Vulkan / OpenGL)
    * Follow [this guide](https://github.com/lutris/docs/blob/master/InstallingDrivers.md) to set up Vulkan & appropriate drivers for your distro.

### Usage

Windows:
- Open `metaforce.exe`

macOS:
- Open `Metaforce.app`

Linux: 
- Ensure AppImage is marked as executable: `chmod +x Metaforce-*.AppImage`
- Open `Metaforce-*.AppImage`

#### CLI options (non-exhaustive)

* `-l`: Enable console logging
* `--warp [worldid] [areaid]`: Warp to a specific world/area. Example: `--warp 2 2`
* `+developer=1`: Enable developer UI

### Build Prerequisites:
* [CMake 3.15+](https://cmake.org)
    * Windows: Install `CMake Tools` in Visual Studio
    * macOS: `brew install cmake`
* [Python 3+](https://python.org)
    * Windows: [Microsoft Store](https://go.microsoft.com/fwlink?linkID=2082640)
        * Verify it's added to `%PATH%` by typing `python` in `cmd`.
    * macOS: `brew install python@3`
* **[Windows]** [Visual Studio 2019 Community](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx)
    * Select `C++ Development` and verify the following packages are included:
        * `Windows 10 SDK`
        * `CMake Tools`
        * `C++ Clang Compiler`
        * `C++ Clang-cl`
* **[macOS]** [Xcode 11.5+](https://developer.apple.com/xcode/download/)
* **[Linux]** Actively tested on Ubuntu 20.04, Arch Linux & derivatives.
    * Ubuntu 20.04+ packages
      ```
      build-essential curl git ninja-build clang lld zlib1g-dev libcurl4-openssl-dev \
      libglu1-mesa-dev libdbus-1-dev libvulkan-dev libxi-dev libxrandr-dev libasound2-dev libpulse-dev \
      libudev-dev libpng-dev libncurses5-dev cmake libx11-xcb-dev python3 python-is-python3 \
      libclang-dev libfreetype-dev libxinerama-dev libxcursor-dev python3-markupsafe libgtk-3-dev
      ```
       * Optional Ubuntu 22.04+ packages (currently only needed for amuse-gui, can be ignored)
         ```
         libqt6svg6-dev libqt6core5compat6-dev qt6-tools-dev-tools
         ```
     * Arch Linux packages
       ```
       base-devel cmake ninja llvm vulkan-headers python python-markupsafe clang lld alsa-lib libpulse libxrandr freetype2
       ```
     * Fedora packages
       ```
       cmake vulkan-headers ninja-build clang-devel llvm-devel libpng-devel
       ```
         * It's also important that you install the developer tools and libraries
           ```
           sudo dnf groupinstall "Development Tools" "Development Libraries"
           ```
### Prep Directions

```sh
git clone --recursive https://github.com/AxioDL/metaforce.git
cd metaforce
```

### Update Directions

```sh
cd metaforce
git pull
git submodule update --recursive
```

### Build Directions

For Windows, it's recommended to use Visual Studio. See below.

#### ninja (Windows/macOS/Linux)

Builds using `RelWithDebInfo` by default.

```sh
cmake -B out -G Ninja # add extra options here
cmake --build out --target metaforce hecl visigen
```

#### CMake configure options
- Build in debug mode (slower runtime speed, better backtraces): `-DCMAKE_BUILD_TYPE=Debug`
- Use clang+lld (faster linking): `-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`
- Optimize for current CPU (resulting binaries are not portable): `-DMETAFORCE_VECTOR_ISA=native`

#### CLion (Windows/macOS/Linux)
*(main development / debugging IDE)*

Open the repository's `CMakeLists.txt`.

Optionally configure CMake options via `File` > `Settings` > `Build, Execution, Deployment` > `CMake`.

#### Qt Creator (Windows/macOS/Linux)

Open the repository's `CMakeLists.txt` via File > Open File or Project.

Configure the desired CMake targets to build in the *Projects* area of the IDE.

#### Visual Studio (Windows)

Verify all required VS packages are installed from the above **Build Prerequisites** section.

Open the `metaforce` directory in Visual Studio (imports CMake configuration).

MSVC and clang-cl configurations should import automatically.

#### Xcode (macOS)

```sh
cmake -G Xcode ../metaforce
```

Then open `metaforce.xcodeproj`
