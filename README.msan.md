## Instrumenting URDE with MemorySanitizer

*For Linux and OpenGL only.*

To enable checking for uninitialized memory accesses with
[MemorySanitizer](https://clang.llvm.org/docs/MemorySanitizer.html), pass
`-DURDE_MSAN=On` to the CMake command line.
URDE and boo will conditionally disable certain features when MemorySanitizer
is enabled. Audio will not be processed in order to avoid the significant
CPU expense required for mixing. Disabling audio also avoids the need to
track allocations made by PulseAudio. Additionally, DBus functionality is
disabled to avoid tracking its allocations.

MemorySanitizer ideally requires all code in the process be compiled for
instrumentation. In practice this is not easy since several system and
multimedia libraries are dynamically linked by URDE.
Two key system additions/modifications are required to successfully instrument.

### libc++

The C++ runtime has numerous instances of memory allocation. The easiest way
to ensure these are instrumented is to build libc++ and libc++abi with
MemorySanitizer enabled. Follow
[these instructions](https://github.com/google/sanitizers/wiki/MemorySanitizerBootstrappingClang)
up to the "Build libc++ and libc++abi with MemorySanitizer" step.
Once built, install them onto your system.

### Mesa

*Tested with Radeon SI only. Might work with Intel and Nouveau as well.*

There is not much to do with URDE in the absence of a graphics acceleration
library. Unfortunately, Mesa is a beast of a library that does not handle
MemorySanitizer instrumentation well. As an alternative, boo uses MSan's
API to contextually disable memory tracking when frequently-used OpenGL
functions are called.

Unfortunately, the Radeon SI driver (and probably others) spin up a work
queue thread that URDE has no visibility of. Memory allocations in this
thread will generate an unmanageable quantity of reports. Mesa has a convenient
thread entry implementing a generic work queue for drivers to make use of.

`#include <dlfcn.h>` and add the following code to the beginning of
`util_queue_thread_func` in `src/util/u_queue.c`:
```cpp
typedef void (*msan_function)(void);
msan_function func = dlsym(RTLD_NEXT, "__msan_scoped_disable_interceptor_checks");
if (!func)
    func = dlsym(RTLD_DEFAULT, "__msan_scoped_disable_interceptor_checks");
if (func)
    func();
```

Graphics driver processing should now be totally silent. There will likely
still be a large quantity of reports during application launch and exit,
but once the actual URDE mainloop begins processing, reports should be mostly
limited to the game code itself.
