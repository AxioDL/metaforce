#include "hecl/Compilers.hpp"

#include <cstring>
#include <utility>

#include <boo/graphicsdev/GLSLMacros.hpp>
#include <logvisor/logvisor.hpp>

#include <glslang/Public/ShaderLang.h>
#include <StandAlone/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/disassemble.h>

#if _WIN32
#include <d3dcompiler.h>
extern pD3DCompile D3DCompilePROC;
#endif

#if __APPLE__
#include <unistd.h>
#include <memory>
#endif

namespace hecl {
logvisor::Module Log("hecl::Compilers");

template <typename P>
struct ShaderCompiler {};

template <>
struct ShaderCompiler<PlatformType::OpenGL> {
  template <typename S>
  static std::pair<StageBinaryData, size_t> Compile(std::string_view text) {
    std::string str = "#version 330\n";
    str += BOO_GLSL_BINDING_HEAD;
    str += text;
    std::pair<StageBinaryData, size_t> ret(MakeStageBinaryData(str.size() + 1), str.size() + 1);
    memcpy(ret.first.get(), str.data(), ret.second);
    return ret;
  }
};

template <>
struct ShaderCompiler<PlatformType::Vulkan> {
  static constexpr EShLanguage ShaderTypes[] = {EShLangVertex, /* Invalid */
                                                EShLangVertex,      EShLangFragment,      EShLangGeometry,
                                                EShLangTessControl, EShLangTessEvaluation};

  template <typename S>
  static std::pair<StageBinaryData, size_t> Compile(std::string_view text) {
    EShLanguage lang = ShaderTypes[int(S::Enum)];
    const EShMessages messages = EShMessages(EShMsgSpvRules | EShMsgVulkanRules);
    glslang::TShader shader(lang);
    const char* strings[] = {"#version 330\n", BOO_GLSL_BINDING_HEAD, text.data()};
    shader.setStrings(strings, 3);
    if (!shader.parse(&glslang::DefaultTBuiltInResource, 110, false, messages)) {
      fmt::print(FMT_STRING("{}\n"), text);
      Log.report(logvisor::Fatal, FMT_STRING("unable to compile shader\n{}"), shader.getInfoLog());
      return {};
    }

    glslang::TProgram prog;
    prog.addShader(&shader);
    if (!prog.link(messages)) {
      Log.report(logvisor::Fatal, FMT_STRING("unable to link shader program\n{}"), prog.getInfoLog());
      return {};
    }

    std::vector<unsigned int> out;
    glslang::GlslangToSpv(*prog.getIntermediate(lang), out);
    std::pair<StageBinaryData, size_t> ret(MakeStageBinaryData(out.size() * 4), out.size() * 4);
    memcpy(ret.first.get(), out.data(), ret.second);
    return ret;
  }
};

#if _WIN32
static const char* D3DShaderTypes[] = {nullptr, "vs_5_0", "ps_5_0", "gs_5_0", "hs_5_0", "ds_5_0"};
template <>
struct ShaderCompiler<PlatformType::D3D11> {
#if _DEBUG && 0
#define BOO_D3DCOMPILE_FLAG D3DCOMPILE_DEBUG | D3DCOMPILE_OPTIMIZATION_LEVEL0
#else
#define BOO_D3DCOMPILE_FLAG D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
  template <typename S>
  static std::pair<StageBinaryData, size_t> Compile(std::string_view text) {
    ComPtr<ID3DBlob> errBlob;
    ComPtr<ID3DBlob> blobOut;
    if (FAILED(D3DCompilePROC(text.data(), text.size(), "Boo HLSL Source", nullptr, nullptr, "main",
                              D3DShaderTypes[int(S::Enum)], BOO_D3DCOMPILE_FLAG, 0, &blobOut, &errBlob))) {
      fmt::print(FMT_STRING("{}\n"), text);
      Log.report(logvisor::Fatal, FMT_STRING("error compiling shader: {}"), (char*)errBlob->GetBufferPointer());
      return {};
    }
    std::pair<StageBinaryData, size_t> ret(MakeStageBinaryData(blobOut->GetBufferSize()), blobOut->GetBufferSize());
    memcpy(ret.first.get(), blobOut->GetBufferPointer(), blobOut->GetBufferSize());
    return ret;
  }
};
#endif

#if __APPLE__
template <>
struct ShaderCompiler<PlatformType::Metal> {
  static bool m_didCompilerSearch;
  static bool m_hasCompiler;

  static bool SearchForCompiler() {
    m_didCompilerSearch = true;
    const char* no_metal_compiler = getenv("HECL_NO_METAL_COMPILER");
    if (no_metal_compiler && atoi(no_metal_compiler))
      return false;

    pid_t pid = fork();
    if (!pid) {
      execlp("xcrun", "xcrun", "-sdk", "macosx", "metal", "--version", nullptr);
      /* xcrun returns 72 if metal command not found;
       * emulate that if xcrun not found */
      exit(72);
    }

    int status, ret;
    while ((ret = waitpid(pid, &status, 0)) < 0 && errno == EINTR) {}
    if (ret < 0)
      return false;
    return WEXITSTATUS(status) == 0;
  }

  template <typename S>
  static std::pair<StageBinaryData, size_t> Compile(std::string_view text) {
    if (!m_didCompilerSearch)
      m_hasCompiler = SearchForCompiler();

    std::string str =
        "#include <metal_stdlib>\n"
        "using namespace metal;\n";
    str += text;
    std::pair<StageBinaryData, size_t> ret;

    if (!m_hasCompiler) {
      /* First byte unset to indicate source data */
      ret.first = MakeStageBinaryData(str.size() + 2);
      ret.first.get()[0] = 0;
      ret.second = str.size() + 2;
      memcpy(&ret.first.get()[1], str.data(), str.size() + 1);
    } else {
      int compilerOut[2];
      int compilerIn[2];
      pipe(compilerOut);
      pipe(compilerIn);

      pid_t pid = getpid();
      const char* tmpdir = getenv("TMPDIR");
      std::string libFile = fmt::format(FMT_STRING("{}boo_metal_shader{}.metallib"), tmpdir, pid);

      /* Pipe source write to compiler */
      pid_t compilerPid = fork();
      if (!compilerPid) {
        dup2(compilerIn[0], STDIN_FILENO);
        dup2(compilerOut[1], STDOUT_FILENO);

        close(compilerOut[0]);
        close(compilerOut[1]);
        close(compilerIn[0]);
        close(compilerIn[1]);

        execlp("xcrun", "xcrun", "-sdk", "macosx", "metal", "-o", "/dev/stdout", "-Wno-unused-variable",
               "-Wno-unused-const-variable", "-Wno-unused-function", "-c", "-x", "metal",
#ifndef NDEBUG
               "-gline-tables-only", "-MO",
#endif
               "-", nullptr);
        fmt::print(stderr, FMT_STRING("execlp fail {}\n"), strerror(errno));
        exit(1);
      }
      close(compilerIn[0]);
      close(compilerOut[1]);

      /* Pipe compiler to linker */
      pid_t linkerPid = fork();
      if (!linkerPid) {
        dup2(compilerOut[0], STDIN_FILENO);

        close(compilerOut[0]);
        close(compilerIn[1]);

        /* metallib doesn't like outputting to a pipe, so temp file will have to do */
        execlp("xcrun", "xcrun", "-sdk", "macosx", "metallib", "-", "-o", libFile.c_str(), nullptr);
        fmt::print(stderr, FMT_STRING("execlp fail {}\n"), strerror(errno));
        exit(1);
      }
      close(compilerOut[0]);

      /* Stream in source */
      const char* inPtr = str.data();
      size_t inRem = str.size();
      while (inRem) {
        ssize_t writeRes = write(compilerIn[1], inPtr, inRem);
        if (writeRes < 0) {
          fmt::print(stderr, FMT_STRING("write fail {}\n"), strerror(errno));
          break;
        }
        inPtr += writeRes;
        inRem -= writeRes;
      }
      close(compilerIn[1]);

      /* Wait for completion */
      int compilerStat, linkerStat;
      while (waitpid(compilerPid, &compilerStat, 0) < 0) {
        if (errno == EINTR)
          continue;
        Log.report(logvisor::Fatal, FMT_STRING("waitpid fail {}"), strerror(errno));
        return {};
      }

      if (WEXITSTATUS(compilerStat)) {
        Log.report(logvisor::Fatal, FMT_STRING("compile fail"));
        return {};
      }

      while (waitpid(linkerPid, &linkerStat, 0) < 0) {
        if (errno == EINTR)
          continue;
        Log.report(logvisor::Fatal, FMT_STRING("waitpid fail {}"), strerror(errno));
        return {};
      }

      if (WEXITSTATUS(linkerStat)) {
        Log.report(logvisor::Fatal, FMT_STRING("link fail"));
        return {};
      }

      /* Copy temp file into buffer with first byte set to indicate binary data */
      FILE* fin = fopen(libFile.c_str(), "rb");
      fseek(fin, 0, SEEK_END);
      long libLen = ftell(fin);
      fseek(fin, 0, SEEK_SET);
      ret.first = MakeStageBinaryData(libLen + 1);
      ret.first.get()[0] = 1;
      ret.second = libLen + 1;
      fread(&ret.first.get()[1], 1, libLen, fin);
      fclose(fin);
      unlink(libFile.c_str());
    }

    return ret;
  }
};
bool ShaderCompiler<PlatformType::Metal>::m_didCompilerSearch = false;
bool ShaderCompiler<PlatformType::Metal>::m_hasCompiler = false;
#endif

#if HECL_NOUVEAU_NX
template <>
struct ShaderCompiler<PlatformType::NX> {
  template <typename S>
  static std::pair<std::shared_ptr<uint8_t[]>, size_t> Compile(std::string_view text) {
    std::string str = "#version 330\n";
    str += BOO_GLSL_BINDING_HEAD;
    str += text;
    std::pair<std::shared_ptr<uint8_t[]>, size_t> ret(new uint8_t[str.size() + 1], str.size() + 1);
    memcpy(ret.first.get(), str.data(), str.size() + 1);
    return ret;
  }
};
#endif

template <typename P, typename S>
std::pair<StageBinaryData, size_t> CompileShader(std::string_view text) {
  return ShaderCompiler<P>::template Compile<S>(text);
}
#define SPECIALIZE_COMPILE_SHADER(P)                                                                                   \
  template std::pair<StageBinaryData, size_t> CompileShader<P, PipelineStage::Vertex>(std::string_view text);          \
  template std::pair<StageBinaryData, size_t> CompileShader<P, PipelineStage::Fragment>(std::string_view text);        \
  template std::pair<StageBinaryData, size_t> CompileShader<P, PipelineStage::Geometry>(std::string_view text);        \
  template std::pair<StageBinaryData, size_t> CompileShader<P, PipelineStage::Control>(std::string_view text);         \
  template std::pair<StageBinaryData, size_t> CompileShader<P, PipelineStage::Evaluation>(std::string_view text);
SPECIALIZE_COMPILE_SHADER(PlatformType::OpenGL)
SPECIALIZE_COMPILE_SHADER(PlatformType::Vulkan)
#if _WIN32
SPECIALIZE_COMPILE_SHADER(PlatformType::D3D11)
#endif
#if __APPLE__
SPECIALIZE_COMPILE_SHADER(PlatformType::Metal)
#endif
#if HECL_NOUVEAU_NX
SPECIALIZE_COMPILE_SHADER(PlatformType::NX)
#endif

} // namespace hecl
