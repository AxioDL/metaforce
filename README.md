## Metaforce
#### Formerly known as URDE

**Status:** Metroid Prime 1 In-Game (all retail GC & Wii versions)

**Official Discord Channel:** https://discord.gg/AMBVFuf

![Metaforce screenshot](assets/metaforce-screen1.png)

### Download
This project is currently in **Alpha** state, so expect bugs.  
Builds available at [https://releases.axiodl.com](https://releases.axiodl.com).

### Platform Support
* Windows 10 (64-bit, D3D11 / Vulkan)
* macOS 10.15+ (Metal)
* Linux (Vulkan)
    * Follow [this guide](https://github.com/lutris/docs/blob/master/InstallingDrivers.md) to set up Vulkan & appropriate drivers for your distro.

### Usage (GUI)

Windows:
- Open `metaforce-gui.exe`

macOS:
- Open `Metaforce.app`

Linux: 
- Ensure AppImage is marked as executable: `chmod +x Metaforce-*.AppImage`
- Open `Metaforce-*.AppImage`

### CLI usage (GC versions)

* Extract ISO: `hecl extract [path].iso -o mp1`
  * `mp1` can be substituted with the directory name of your choice
* Repackage game for Metaforce: `cd mp1; hecl package`
* Run Metaforce: `metaforce mp1/out`

### CLI usage (Wii versions)

**IMPORTANT**: Trilogy main menu currently doesn't work, and requires the `--warp 1 0` command line arguments to get in-game.  

NFS files dumped from Metroid Prime Trilogy on Wii U VC can be used directly without converting to ISO.

* Extract ISO or NFS: `hecl extract [path].[iso/nfs] -o mpt`
  * `mpt` can be substituted with the directory name of your choice
* Repackage game for Metaforce: `cd mpt; hecl package MP1`
  * The `MP1` parameter is important here.
* Run Metaforce: `metaforce mpt/out --warp 1 0`

#### Metaforce options (non-exhaustive)

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
      libudev-dev libpng-dev libncurses5-dev cmake libx11-xcb-dev python3 python-is-python3 qt5-default
      ```
     * Arch Linux packages
       ```
       base-devel cmake ninja llvm vulkan-headers python3 qt6 clang lld alsa-lib libpulse libxrandr
       ```
     * Fedora packages
       ```
       cmake vulkan-headers ninja-build clang-devel llvm-devel libpng-devel qt6-qtbase-devel 
       qt6-linguist qt6-qttools-devel qt6-qtscxml-devel qt6-qtsvg-devel qt6-qt5compat-devel
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

#### Optional Debug Models
We provide custom debug models for use to visualize certain aspects of the game such as lighting, in order to use 
these models you may download them from https://axiodl.com/files/debug_models.zip and extract to `MP1/URDE` in an 
existing HECL project (assuming paths are relative), then run the following command:

```sh
hecl package MP1/URDE
```
This will cook and package the debug models and will automatically enable rendering of lights in a debug build of Metaforce.
