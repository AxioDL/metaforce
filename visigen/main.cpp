#include "../version.h"
#include "athena/Global.hpp"
#include "logvisor/logvisor.hpp"
#ifdef __APPLE__
#include "metal/VISIRendererMetal.hh"
#else
#include "vulkan/VISIRendererVulkan.hpp"
#endif

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char * /*file*/, const char *, int /*line*/,
                      fmt::string_view fmt, fmt::format_args args) {
  AthenaLog.vreport(logvisor::Level(level), fmt, args);
}

int main(int argc, const char **argv) {
  if (argc > 1 && !strcmp(argv[1], "--dlpackage")) {
    fmt::print(FMT_STRING("{}\n"), METAFORCE_DLPACKAGE);
    return 100;
  }

  logvisor::RegisterStandardExceptions();
  logvisor::RegisterConsoleLogger();
  atSetExceptionHandler(AthenaExc);
#ifdef __APPLE__
  VISIRendererMetal renderer(argc, argv);
#else
  VISIRendererVulkan renderer(argc, argv);
#endif
  renderer.Run(nullptr);
  return renderer.ReturnVal();
}
