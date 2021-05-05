#include "../version.h"
#include "VISIRendererMetal.hh"
#include "athena/Global.hpp"
#include "logvisor/logvisor.hpp"
#include <AppKit/AppKit.h>
#include <MetalKit/MetalKit.h>
#include <thread>

#if !__has_feature(objc_arc)
#error ARC Required
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
  VISIRendererMetal renderer(argc, argv);
  @autoreleasepool {
    renderer.Run(nullptr);
  }
  return renderer.ReturnVal();
}
