#include "hecl/Compilers.hpp"
#include "boo/graphicsdev/GLSLMacros.hpp"
#include "logvisor/logvisor.hpp"
#if BOO_HAS_VULKAN
#include <glslang/Public/ShaderLang.h>
#include <StandAlone/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/disassemble.h>
#endif

namespace hecl
{
logvisor::Module Log("hecl::Compilers");

template<typename P> struct ShaderCompiler {};

template<> struct ShaderCompiler<PlatformType::OpenGL>
{
    template<typename S>
    static std::pair<std::shared_ptr<uint8_t[]>, size_t> Compile(std::string_view text)
    {
        std::string str = "#version 330\n";
        str += BOO_GLSL_BINDING_HEAD;
        str += text;
        std::pair<std::shared_ptr<uint8_t[]>, size_t> ret(new uint8_t[str.size() + 1], str.size() + 1);
        memcpy(ret.first.get(), str.data(), str.size() + 1);
        return ret;
    }
};

template<> struct ShaderCompiler<PlatformType::Vulkan>
{
    static constexpr EShLanguage ShaderTypes[] =
    {
        EShLangVertex, /* Invalid */
        EShLangVertex,
        EShLangFragment,
        EShLangGeometry,
        EShLangTessControl,
        EShLangTessEvaluation
    };

    template<typename S>
    static std::pair<std::shared_ptr<uint8_t[]>, size_t> Compile(std::string_view text)
    {
        EShLanguage lang = ShaderTypes[int(S::Enum)];
        const EShMessages messages = EShMessages(EShMsgSpvRules | EShMsgVulkanRules);
        glslang::TShader shader(lang);
        const char* strings[] = { "#version 330\n", BOO_GLSL_BINDING_HEAD, text.data() };
        shader.setStrings(strings, 3);
        if (!shader.parse(&glslang::DefaultTBuiltInResource, 110, false, messages))
        {
            printf("%s\n", text.data());
            Log.report(logvisor::Fatal, "unable to compile shader\n%s", shader.getInfoLog());
            return {};
        }

        glslang::TProgram prog;
        prog.addShader(&shader);
        if (!prog.link(messages))
        {
            Log.report(logvisor::Fatal, "unable to link shader program\n%s", prog.getInfoLog());
            return {};
        }

        std::vector<unsigned int> out;
        glslang::GlslangToSpv(*prog.getIntermediate(lang), out);
        std::pair<std::shared_ptr<uint8_t[]>, size_t> ret(new uint8_t[out.size() * 4], out.size() * 4);
        memcpy(ret.first.get(), out.data(), ret.second);
        return ret;
    }
};

template<> struct ShaderCompiler<PlatformType::NX>
{
    template<typename S>
    static std::pair<std::shared_ptr<uint8_t[]>, size_t> Compile(std::string_view text)
    {
        std::string str = "#version 330\n";
        str += BOO_GLSL_BINDING_HEAD;
        str += text;
        std::pair<std::shared_ptr<uint8_t[]>, size_t> ret(new uint8_t[str.size() + 1], str.size() + 1);
        memcpy(ret.first.get(), str.data(), str.size() + 1);
        return ret;
    }
};

template<typename P, typename S>
std::pair<std::shared_ptr<uint8_t[]>, size_t> CompileShader(std::string_view text)
{
    return ShaderCompiler<P>::template Compile<S>(text);
}
#define SPECIALIZE_COMPILE_SHADER(P) \
template std::pair<std::shared_ptr<uint8_t[]>, size_t> CompileShader<P, PipelineStage::Vertex>(std::string_view text); \
template std::pair<std::shared_ptr<uint8_t[]>, size_t> CompileShader<P, PipelineStage::Fragment>(std::string_view text); \
template std::pair<std::shared_ptr<uint8_t[]>, size_t> CompileShader<P, PipelineStage::Geometry>(std::string_view text); \
template std::pair<std::shared_ptr<uint8_t[]>, size_t> CompileShader<P, PipelineStage::Control>(std::string_view text); \
template std::pair<std::shared_ptr<uint8_t[]>, size_t> CompileShader<P, PipelineStage::Evaluation>(std::string_view text);
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

#if _WIN32
static const char* ShaderTypes[] =
{
    "vs_5_0",
    "ps_5_0",
    "gs_5_0",
    "hs_5_0",
    "ds_5_0"
};
template<>
std::vector<uint8_t> CompileShader<PlatformType::D3D11>(std::string_view text, PipelineStage stage)
{
    ComPtr<ID3DBlob> errBlob;
    ComPtr<ID3DBlob> blobOut;
    if (FAILED(D3DCompilePROC(text.data(), text.size(), "Boo HLSL Source", nullptr, nullptr, "main",
                              ShaderTypes[int(stage)], BOO_D3DCOMPILE_FLAG, 0, &blobOut, &errBlob)))
    {
        printf("%s\n", source);
        Log.report(logvisor::Fatal, "error compiling shader: %s", errBlob->GetBufferPointer());
        return {};
    }
    std::vector<uint8_t> ret(blobOut.GetBufferSize());
    memcpy(ret.data(), blobOut.GetBufferPointer(), blobOut.GetBufferSize());
    return ret;
};
#endif

#if BOO_HAS_METAL
static int HasMetalCompiler = -1;

static void CheckForMetalCompiler()
{
    pid_t pid = fork();
    if (!pid)
    {
        execlp("xcrun", "xcrun", "-sdk", "macosx", "metal", NULL);
        /* xcrun returns 72 if metal command not found;
         * emulate that if xcrun not found */
        exit(72);
    }

    int status, ret;
    while ((ret = waitpid(pid, &status, 0)) < 0 && errno == EINTR) {}
    if (ret < 0)
        HasMetalCompiler = 0;
    else
        HasMetalCompiler = WEXITSTATUS(status) == 1;
}

template<>
std::vector<uint8_t> CompileShader<PlatformType::Metal>(std::string_view text, PipelineStage stage)
{
    if (HasMetalCompiler == -1)
        CheckForMetalCompiler();

    std::vector<uint8_t> blobOut;
    if (!HasMetalCompiler)
    {
        /* Cache the source if there's no compiler */
        size_t sourceLen = strlen(source);

        /* First byte unset to indicate source data */
        blobOut.resize(sourceLen + 2);
        memcpy(&blobOut[1], source, sourceLen);
    }
    else
    {
        /* Cache the binary otherwise */
        int compilerOut[2];
        int compilerIn[2];
        pipe(compilerOut);
        pipe(compilerIn);

        /* Pipe source write to compiler */
        pid_t compilerPid = fork();
        if (!compilerPid)
        {
            dup2(compilerIn[0], STDIN_FILENO);
            dup2(compilerOut[1], STDOUT_FILENO);

            close(compilerOut[0]);
            close(compilerOut[1]);
            close(compilerIn[0]);
            close(compilerIn[1]);

            execlp("xcrun", "xcrun", "-sdk", "macosx", "metal", "-o", "/dev/stdout", "-Wno-unused-variable",
                   "-Wno-unused-const-variable", "-Wno-unused-function", "-x", "metal", "-", NULL);
            fprintf(stderr, "execlp fail %s\n", strerror(errno));
            exit(1);
        }
        close(compilerIn[0]);
        close(compilerOut[1]);

        /* Pipe compiler to linker */
        pid_t linkerPid = fork();
        if (!linkerPid)
        {
            dup2(compilerOut[0], STDIN_FILENO);

            close(compilerOut[0]);
            close(compilerIn[1]);

            /* metallib doesn't like outputting to a pipe, so temp file will have to do */
            execlp("xcrun", "xcrun", "-sdk", "macosx", "metallib", "-", "-o", m_libfile, NULL);
            fprintf(stderr, "execlp fail %s\n", strerror(errno));
            exit(1);
        }
        close(compilerOut[0]);

        /* Stream in source */
        const char* inPtr = source;
        size_t inRem = strlen(source);
        while (inRem)
        {
            ssize_t writeRes = write(compilerIn[1], inPtr, inRem);
            if (writeRes < 0)
            {
                fprintf(stderr, "write fail %s\n", strerror(errno));
                break;
            }
            inPtr += writeRes;
            inRem -= writeRes;
        }
        close(compilerIn[1]);

        /* Wait for completion */
        int compilerStat, linkerStat;
        if (waitpid(compilerPid, &compilerStat, 0) < 0 || waitpid(linkerPid, &linkerStat, 0) < 0)
        {
            fprintf(stderr, "waitpid fail %s\n", strerror(errno));
            return {};
        }

        if (WEXITSTATUS(compilerStat) || WEXITSTATUS(linkerStat))
            return {};

        /* Copy temp file into buffer with first byte set to indicate binary data */
        FILE* fin = fopen(m_libfile, "rb");
        fseek(fin, 0, SEEK_END);
        long libLen = ftell(fin);
        fseek(fin, 0, SEEK_SET);
        blobOut.resize(libLen + 1);
        blobOut[0] = 1;
        fread(&blobOut[1], 1, libLen, fin);
        fclose(fin);
    }
    return blobOut;
}
#endif

}
