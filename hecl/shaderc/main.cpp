#include "shaderc.hpp"
#include "logvisor/logvisor.hpp"
#include "athena/FileWriter.hpp"
#include "glslang/Public/ShaderLang.h"
#include "hecl/hecl.hpp"

static logvisor::Module Log("shaderc");

#if _WIN32
#include <d3dcompiler.h>
extern pD3DCompile D3DCompilePROC;
pD3DCompile D3DCompilePROC = nullptr;

static bool FindBestD3DCompile()
{
    HMODULE d3dCompilelib = LoadLibraryW(L"D3DCompiler_47.dll");
    if (!d3dCompilelib)
    {
        d3dCompilelib = LoadLibraryW(L"D3DCompiler_46.dll");
        if (!d3dCompilelib)
        {
            d3dCompilelib = LoadLibraryW(L"D3DCompiler_45.dll");
            if (!d3dCompilelib)
            {
                d3dCompilelib = LoadLibraryW(L"D3DCompiler_44.dll");
                if (!d3dCompilelib)
                {
                    d3dCompilelib = LoadLibraryW(L"D3DCompiler_43.dll");
                }
            }
        }
    }
    if (d3dCompilelib)
    {
        D3DCompilePROC = (pD3DCompile)GetProcAddress(d3dCompilelib, "D3DCompile");
        return D3DCompilePROC != nullptr;
    }
    return false;
}

int wmain(int argc, const hecl::SystemChar** argv)
#else
int main(int argc, const hecl::SystemChar** argv)
#endif
{
    logvisor::RegisterConsoleLogger();
    logvisor::RegisterStandardExceptions();

#if _WIN32
    if (!FindBestD3DCompile())
    {
        Log.report(logvisor::Info, "Unable to find D3DCompiler dll");
        return 1;
    }
#endif

    if (argc == 1)
    {
        Log.report(logvisor::Info, "Usage: shaderc -o <out-base> [-D definevar=defineval]... <in-files>...");
        return 0;
    }

    hecl::SystemString outPath;
    hecl::shaderc::Compiler c;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'o')
            {
                if (argv[i][2])
                {
                    outPath = &argv[i][2];
                }
                else if (i + 1 < argc)
                {
                    ++i;
                    outPath = argv[i];
                }
                else
                {
                    Log.report(logvisor::Error, "Invalid -o argument");
                    return 1;
                }
            }
            else if (argv[i][1] == 'D')
            {
                const hecl::SystemChar* define;
                if (argv[i][2])
                {
                    define = &argv[i][2];
                }
                else if (i + 1 < argc)
                {
                    ++i;
                    define = argv[i];
                }
                else
                {
                    Log.report(logvisor::Error, "Invalid -D argument");
                    return 1;
                }
                hecl::SystemUTF8Conv conv(define);
                const char* defineU8 = conv.c_str();
                if (const char* equals = strchr(defineU8, '='))
                    c.addDefine(std::string(defineU8, equals - defineU8), equals + 1);
                else
                    c.addDefine(defineU8, "");
            }
            else
            {
                Log.report(logvisor::Error, "Unrecognized flag option '%c'", argv[i][1]);
                return 1;
            }
        }
        else
        {
            c.addInputFile(argv[i]);
        }
    }

    if (outPath.empty())
    {
        Log.report(logvisor::Error, "-o option is required");
        return 1;
    }

    hecl::SystemStringView baseName;
    auto slashPos = outPath.find_last_of(_SYS_STR("/\\"));
    if (slashPos != hecl::SystemString::npos)
        baseName = outPath.data() + slashPos + 1;
    else
        baseName = outPath;

    if (!glslang::InitializeProcess())
    {
        Log.report(logvisor::Error, "Unable to initialize glslang");
        return 1;
    }

    hecl::SystemUTF8Conv conv(baseName);
    std::pair<std::string, std::string> ret;
    if (!c.compile(conv.str(), ret))
        return 1;

    {
        hecl::SystemString headerPath = outPath + _SYS_STR(".hpp");
        athena::io::FileWriter w(headerPath);
        if (w.hasError())
        {
            Log.report(logvisor::Error, _SYS_STR("Error opening '%s' for writing"), headerPath.c_str());
            return 1;
        }
        w.writeBytes(ret.first.data(), ret.first.size());
    }

    {
        hecl::SystemString impPath = outPath + _SYS_STR(".cpp");
        athena::io::FileWriter w(impPath);
        if (w.hasError())
        {
            Log.report(logvisor::Error, _SYS_STR("Error opening '%s' for writing"), impPath.c_str());
            return 1;
        }
        w.writeBytes(ret.second.data(), ret.second.size());
    }

    return 0;
}
