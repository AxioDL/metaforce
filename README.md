Metroid Prime  
[![Build Status]][actions] [![Code Progress]][progress] [![Data Progress]][progress] [![Discord Badge]][discord]
=============

[Build Status]: https://github.com/PrimeDecomp/prime/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/PrimeDecomp/prime/actions/workflows/build.yml

[Code Progress]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=code&label=Code
[Data Progress]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=data&label=Data

[Code Progress GM8E01_00]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=code&label=Code&version=GM8E01_00
[Data Progress GM8E01_00]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=data&label=Data&version=GM8E01_00
[Progress GM8E01_00]: https://decomp.dev/PrimeDecomp/prime?version=GM8E01_00

[Code Progress GM8E01_01]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=code&label=Code&version=GM8E01_01
[Data Progress GM8E01_01]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=data&label=Data&version=GM8E01_01
[Progress GM8E01_01]: https://decomp.dev/PrimeDecomp/prime?version=GM8E01_01

[Code Progress GM8E01_02]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=code&label=Code&version=GM8E01_02
[Data Progress GM8E01_02]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=data&label=Data&version=GM8E01_02
[Progress GM8E01_02]: https://decomp.dev/PrimeDecomp/prime?version=GM8E01_02

[Code Progress GM8P01_00]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=code&label=Code&version=GM8P01_00
[Data Progress GM8P01_00]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=data&label=Code&version=GM8P01_00
[Progress GM8P01_00]: https://decomp.dev/PrimeDecomp/prime?version=GM8P01_00

[Code Progress GM8E01_48]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=code&label=Code&version=GM8E01_48
[Data Progress GM8E01_48]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=data&label=Data&version=GM8E01_48
[Progress GM8E01_48]: https://decomp.dev/PrimeDecomp/prime?version=GM8E01_48

[Code Progress GM8J01_00]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=code&label=Code&version=GM8J01_00
[Data Progress GM8J01_00]: https://decomp.dev/PrimeDecomp/prime.svg?mode=shield&measure=data&label=Data&version=GM8J01_00
[Progress GM8J01_00]: https://decomp.dev/PrimeDecomp/prime?version=GM8J01_00

[progress]: https://decomp.dev/PrimeDecomp/prime
[Discord Badge]: https://img.shields.io/discord/727908905392275526?color=%237289DA&logo=discord&logoColor=%23FFFFFF
[discord]: https://discord.gg/hKx3FJJgrV

A work-in-progress decompilation of Metroid Prime.

This repository does **not** contain any game assets or assembly whatsoever. An existing copy of the game is required.

The following game versions are supported:

- `GM8E01_00` (USA v1.088)
  * [![Code Progress GM8E01_00]][Progress GM8E01_00][![Data Progress GM8E01_00]][Progress GM8E01_00]
- `GM8E01_01` (USA v1.093)
  * [![Code Progress GM8E01_01]][Progress GM8E01_01][![Data Progress GM8E01_01]][Progress GM8E01_01]
- `GM8E01_48` (KOR v1.097)
  * [![Code Progress GM8E01_48]][Progress GM8E01_48][![Data Progress GM8E01_48]][Progress GM8E01_48]
- `GM8P01_00` (PAL v1.110)
  * [![Code Progress GM8P01_00]][Progress GM8P01_00][![Data Progress GM8P01_00]][Progress GM8P01_00]
- `GM8E01_02` (USA v1.111)
  * [![Code Progress GM8E01_02]][Progress GM8E01_02][![Data Progress GM8E01_02]][Progress GM8E01_02]
- `GM8J01_00` (JPN v1.111)
  * [![Code Progress GM8J01_00]][Progress GM8J01_00][![Data Progress GM8J01_00]][Progress GM8J01_00]



If you'd like to contribute, see [CONTRIBUTING.md](CONTRIBUTING.md).

Dependencies
============

Windows
--------

On Windows, it's **highly recommended** to use native tooling. WSL or msys2 are **not** required.  
When running under WSL, [objdiff](#diffing) is unable to get filesystem notifications for automatic rebuilds.

- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
  - Also available from the [Windows Store](https://apps.microsoft.com/store/detail/python-311/9NRWMJP3717K).
- Download [ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
  - Quick install via pip: `pip install ninja`

macOS
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages):

  ```sh
  brew install ninja
  ```

- Install [wine-crossover](https://github.com/Gcenx/homebrew-wine):

  ```sh
  brew install --cask --no-quarantine gcenx/wine/wine-crossover
  ```

After OS upgrades, if macOS complains about `Wine Crossover.app` being unverified, you can unquarantine it using:

```sh
sudo xattr -rd com.apple.quarantine '/Applications/Wine Crossover.app'
```

Linux
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).
- For non-x86(_64) platforms: Install wine from your package manager.
  - For x86(_64), [wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Building
========

- Clone the repository:

  ```sh
  git clone https://github.com/PrimeDecomp/prime.git
  ```

- Update and Initialize submodules:

  ```sh
  git submodule update --init --recursive
  ```

- Copy your game's disc image to `orig/GM8E01_00` (or the appropriate version).
  - Supported formats: ISO (GCM), RVZ, WIA, WBFS, CISO, NFS, GCZ, TGC
  - After the initial build, the disc image can be deleted to save space.

- Configure:

  ```sh
  python configure.py
  ```

  To use a version other than `GM8E01_00` (USA), specify `--version GM8E01_01` or similar.
- Build:

  ```sh
  ninja
  ```

Diffing
=======

Once the initial build succeeds, an `objdiff.json` should exist in the project root.

Download the latest release from [encounter/objdiff](https://github.com/encounter/objdiff). Under project settings, set `Project directory`. The configuration should be loaded automatically.

Select an object from the left sidebar to begin diffing. Changes to the project will rebuild automatically: changes to source files, headers, `configure.py`, `splits.txt` or `symbols.txt`.

![](assets/objdiff.png)
