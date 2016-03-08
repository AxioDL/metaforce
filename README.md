## URDE
**Status:** Extract Only

### Download
Precompiled builds are available at https://github.com/AxioDL/urde/releases

### Platform Support
* Windows 7+ (64-bit support only)
* Mac OS X 10.9+
* Linux
    * With development packages of recent udev, xorg+xcb, xinput, glx, asound
    * Arch is known to function with [vendor setup instructions](https://wiki.archlinux.org/index.php/Category:Graphics) *(main development/testing OS)*
* **[Coming Soon]** FreeBSD

### Build Prerequisites:
* [CMake 3+](https://cmake.org)
* [Python 3+](https://python.org)
* LLVM
    * [Specialized Windows Package](https://www.dropbox.com/s/4u0rckvh3d3tvr1/LLVM-3.7.0-win64.exe)
    * [OS X Package](http://llvm.org/releases/3.7.0/clang+llvm-3.7.0-x86_64-apple-darwin.tar.xz)
* [Visual Studio 2015 and Windows SDK](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx) (Windows builders)
* [Xcode Tools](https://developer.apple.com/xcode/download/) (Mac builders)

### Build Directions (Release)

#### make

```sh
git clone https://github.com/AxioDL/urde.git
mkdir urde-build
cd urde
git submodule update --init --recursive
cd ../urde-build
cmake -DCMAKE_BUILD_TYPE=Release ../urde
make
```

#### Visual Studio

Using *Git Shell* with [GitHub for Windows](https://desktop.github.com/)

```sh
git clone https://github.com/AxioDL/urde.git
mkdir urde-build
cd urde
git submodule update --init --recursive
cd ../urde-build
```
Windows 7/8 users should then run:

```sh
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release ../urde
```

Windows 10 users should then run:

```sh
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_SYSTEM_VERSION=10.0 -DCMAKE_BUILD_TYPE=Release ../urde
```

Then open `urde.sln`

#### Xcode

```sh
git clone https://github.com/AxioDL/urde.git
mkdir urde-build
cd urde
git submodule update --init --recursive
cd ../urde-build
cmake -G Xcode -DCMAKE_BUILD_TYPE=Release ../urde
```

Then open `urde.xcodeproj`
