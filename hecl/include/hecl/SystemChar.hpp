#pragma once

#ifndef _WIN32
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
#include <string>
#include <string_view>
#include <cstring>
#include <algorithm>

namespace hecl {

} // namespace hecl
