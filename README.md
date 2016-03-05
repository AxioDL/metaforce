## URDE
**Status:** Extract Only

### Download
Precompiled builds are available at https://github.com/AxioDL/urde/releases

### Platform Support
* Windows 7+ (64-bit support only)
* Mac OS X 10.9+
* Linux (with recent udev, xcb, xinput, glx)
* **[Coming Soon]** FreeBSD

### Build Prerequisites:
* [CMake 3+](https://cmake.org)
* [Python 3+](https://python.org)
* LLVM
    * [Specialized Windows Package](https://www.dropbox.com/s/4u0rckvh3d3tvr1/LLVM-3.7.0-win64.exe)
    * [OS X Package](http://llvm.org/releases/3.7.0/clang+llvm-3.7.0-x86_64-apple-darwin.tar.xz)
* [Visual Studio 2015 and Windows SDK](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx) (windows users)

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
cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release ../urde
```

Then open `urde.sln`
