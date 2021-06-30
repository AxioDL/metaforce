#include "shaderc.hpp"
#include "logvisor/logvisor.hpp"
#include "athena/FileWriter.hpp"
#include "glslang/Public/ShaderLang.h"
#include "hecl/hecl.hpp"
#include <sstream>
#include <nowide/args.hpp>

static logvisor::Module Log("shaderc");

#if _WIN32
#include <d3dcompiler.h>
extern pD3DCompile D3DCompilePROC;
pD3DCompile D3DCompilePROC = nullptr;

static bool FindBestD3DCompile() {
  HMODULE d3dCompilelib = LoadLibraryW(L"D3DCompiler_47.dll");
  if (!d3dCompilelib) {
    d3dCompilelib = LoadLibraryW(L"D3DCompiler_46.dll");
    if (!d3dCompilelib) {
      d3dCompilelib = LoadLibraryW(L"D3DCompiler_45.dll");
      if (!d3dCompilelib) {
        d3dCompilelib = LoadLibraryW(L"D3DCompiler_44.dll");
        if (!d3dCompilelib) {
          d3dCompilelib = LoadLibraryW(L"D3DCompiler_43.dll");
        }
      }
    }
  }
  if (d3dCompilelib) {
    D3DCompilePROC = (pD3DCompile)GetProcAddress(d3dCompilelib, "D3DCompile");
    return D3DCompilePROC != nullptr;
  }
  return false;
}
#endif

int main(int argc, char** argv) {
#if _WIN32
  nowide::args _(argc, argv);
#endif
  logvisor::RegisterConsoleLogger();
  logvisor::RegisterStandardExceptions();

#if _WIN32
  if (!FindBestD3DCompile()) {
    Log.report(logvisor::Info, FMT_STRING("Unable to find D3DCompiler dll"));
    return 1;
  }
#endif

  if (argc == 1) {
    Log.report(logvisor::Info, FMT_STRING("Usage: shaderc -o <out-base> [-D definevar=defineval]... <in-files>..."));
    return 0;
  }

  std::string outPath;
  hecl::shaderc::Compiler c;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'o') {
        if (argv[i][2]) {
          outPath = &argv[i][2];
        } else if (i + 1 < argc) {
          ++i;
          outPath = argv[i];
        } else {
          Log.report(logvisor::Error, FMT_STRING("Invalid -o argument"));
          return 1;
        }
      } else if (argv[i][1] == 'D') {
        const char* define;
        if (argv[i][2]) {
          define = &argv[i][2];
        } else if (i + 1 < argc) {
          ++i;
          define = argv[i];
        } else {
          Log.report(logvisor::Error, FMT_STRING("Invalid -D argument"));
          return 1;
        }
        if (const char* equals = strchr(define, '='))
          c.addDefine(std::string(define, equals - define), equals + 1);
        else
          c.addDefine(define, "");
      } else {
        Log.report(logvisor::Error, FMT_STRING("Unrecognized flag option '{:c}'"), argv[i][1]);
        return 1;
      }
    } else {
      c.addInputFile(argv[i]);
    }
  }

  if (outPath.empty()) {
    Log.report(logvisor::Error, FMT_STRING("-o option is required"));
    return 1;
  }

  std::string_view baseName;
  auto slashPos = outPath.find_last_of("/\\");
  if (slashPos != std::string::npos)
    baseName = outPath.data() + slashPos + 1;
  else
    baseName = outPath;

  if (!glslang::InitializeProcess()) {
    Log.report(logvisor::Error, FMT_STRING("Unable to initialize glslang"));
    return 1;
  }

  std::pair<std::stringstream, std::stringstream> ret;
  if (!c.compile(baseName, ret))
    return 1;

  {
    std::string headerPath = outPath + ".hpp";
    athena::io::FileWriter w(headerPath);
    if (w.hasError()) {
      Log.report(logvisor::Error, FMT_STRING("Error opening '{}' for writing"), headerPath);
      return 1;
    }
    std::string header = ret.first.str();
    w.writeBytes(header.data(), header.size());
  }

  {
    std::string impPath = outPath + ".cpp";
    athena::io::FileWriter w(impPath);
    if (w.hasError()) {
      Log.report(logvisor::Error, FMT_STRING("Error opening '{}' for writing"), impPath);
      return 1;
    }
    std::string source = ret.second.str();
    w.writeBytes(source.data(), source.size());
  }

  return 0;
}
