#pragma once

#include <borealis/log.hpp>

#define REQUIRE(valid, ...)                                                                        \
  do {                                                                                             \
    if (!(valid)) {                                                                                \
      Log.fatal(__VA_ARGS__);                                                                      \
    }                                                                                              \
  } while (false)
