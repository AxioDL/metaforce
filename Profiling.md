# Profiling

Metaforce has integrated support for [Optick](https://github.com/bombomby/optick), a lightweight profiler focused on games. This profiler measures how long each frame takes, as well as which functions consumed time on each given frame, both details extremely important when profiling a game.

## Building with Optick

Just set CMake option `OPTICK_ENABLED` to `ON`.
* On command line, add `-DOPTICK_ENABLED=ON` to the `cmake` call.
* On Visual Studio, go `Project -> CMake Settings for metaforce`. In "CMake variables and cache", find OPTICK_ENABLED and toggle to ON. Otherwise, open `CMakeSettings.json` directly and add it the `variables` section.

## Running and profiling

1. Download https://github.com/bombomby/optick/releases/download/1.3.1.0/Optick_1.3.1.zip, extract and open

2. Open a command prompt as administrator (non-windows can run in their own users. admin is for windows-only extra metrics)

3. Run `metaforce.exe <your_extract>` and any extra arguments you want. It can be found at `<git root>\out\build\x64-MSVC-Release\Binaries\metaforce.exe`

4. When the game is in a spot you want to start profiling, go to Optick (you opened in 1) and press "Start Profiling Session", the play button on top left. Game might lag a bit at this point.

    a. You might need to change the target in the combo to the right of the trash icon. You can also configure an arbitrary IP address to profile another computer.

5. When you're done, go to Optick and press the same button again, but it's now a red square button. The game might lag a bit and optick will start processing stuff. After it's finished, you have juicy data to look at.

6. Press the save button to save the data you just collected for later or sharing.


## Tutorials

This video has tips on how to look at the data:

https://www.youtube.com/watch?v=p57TV5342fo