#include "shaderc.hpp"
#include "athena/FileReader.hpp"
#include "logvisor/logvisor.hpp"
#include "hecl/hecl.hpp"
#include "hecl/PipelineBase.hpp"
#include <algorithm>
#include <regex>
#include <unordered_map>
#include <set>
#include <bitset>

using namespace std::literals;

namespace hecl::shaderc
{
static logvisor::Module Log("shaderc");

static constexpr std::regex::flag_type RegexFlags = std::regex::ECMAScript|std::regex::optimize;

#if __GNUC__
__attribute__((__format__ (__printf__, 1, 2)))
#endif
static std::string Format(const char* format, ...)
{
    char resultBuf[FORMAT_BUF_SZ];
    va_list va;
    va_start(va, format);
    int printSz = vsnprintf(resultBuf, FORMAT_BUF_SZ, format, va);
    va_end(va);
    return std::string(resultBuf, printSz);
}

const std::string* Compiler::getFileContents(SystemStringView path)
{
    auto search = m_fileContents.find(path.data());
    if (search == m_fileContents.end())
    {
        athena::io::FileReader r(path);
        if (r.hasError())
            return nullptr;
        auto len = r.length();
        auto data = r.readBytes(len);
        search = m_fileContents.insert(std::make_pair(path.data(), std::string((char*)data.get(), len))).first;
    }
    return &search->second;
}

void Compiler::addInputFile(SystemStringView file)
{
    if (std::find(m_inputFiles.begin(), m_inputFiles.end(), file) == m_inputFiles.end())
        m_inputFiles.emplace_back(file);
}

void Compiler::addDefine(std::string_view var, std::string_view val)
{
    m_defines[var.data()] = val;
}

static const char* ShaderHeaderTemplate =
"class Shader_%s : public hecl::GeneralShader\n"
"{\n"
"public:\n"
"    static const boo::VertexFormatInfo VtxFmt;\n"
"    static const boo::AdditionalPipelineInfo PipelineInfo;\n"
"    static constexpr bool HasHash = true;\n"
"    static constexpr uint64_t Hash() { return 0x%016llX; }\n"
"};\n\n";

static const char* StageObjectHeaderTemplate =
"template<typename P, typename S>\n"
"class StageObject_%s : public hecl::StageBinary<P, S>\n"
"{\n"
"    static const hecl::StageBinary<P, S> Prototype;\n"
"public:\n"
"    StageObject_%s(hecl::StageConverter<P, S>& conv, hecl::FactoryCtx& ctx, const Shader_%s& in)\n"
"    : hecl::StageBinary<P, S>(Prototype) {}\n"
"};\n"
"STAGEOBJECT_PROTOTYPE_DECLARATIONS(StageObject_%s)\n\n";

static const char* StageObjectImplTemplate =
"template<>\n"
"const hecl::StageBinary<hecl::PlatformType::%s, hecl::PipelineStage::%s>\n"
"StageObject_%s<hecl::PlatformType::%s, hecl::PipelineStage::%s>::Prototype = \n"
"{%s_%s_%s_data, sizeof(%s_%s_%s_data)};\n\n";

struct CompileSubStageAction
{
    template<typename P, typename S>
    static bool Do(const std::string& name, const std::string& basename, const std::string& stage, std::string& implOut)
    {
        implOut += Format(StageObjectImplTemplate, P::Name, S::Name, name.c_str(), P::Name, S::Name,
                          basename.c_str(), P::Name, S::Name, basename.c_str(), P::Name, S::Name);

        return true;
    }
};

struct CompileStageAction
{
    template<typename P, typename S>
    static bool Do(const std::string& name, const std::string& basename, const std::string& stage, std::string& implOut)
    {
        std::pair<std::shared_ptr<uint8_t[]>, size_t> data = CompileShader<P, S>(stage);
        if (data.second == 0)
            return false;

        implOut += Format("static const uint8_t %s_%s_%s_data[] = {\n", name.c_str(), P::Name, S::Name);
        for (size_t i = 0; i < data.second; )
        {
            implOut += "    ";
            for (int j = 0; j < 10 && i < data.second ; ++i, ++j)
                implOut += Format("0x%02X, ", data.first[i]);
            implOut += "\n";
        }
        implOut += "};\n\n";
        implOut += Format(StageObjectImplTemplate, P::Name, S::Name, name.c_str(), P::Name, S::Name,
                          name.c_str(), P::Name, S::Name, name.c_str(), P::Name, S::Name);

        return true;
    }
};

template<typename Action, typename P>
bool Compiler::StageAction(StageType type,
                           const std::string& name, const std::string& basename, const std::string& stage,
                           std::string& implOut)
{
    switch (type)
    {
    case StageType::Vertex:
        return Action::template Do<P, PipelineStage::Vertex>(name, basename, stage, implOut);
    case StageType::Fragment:
        return Action::template Do<P, PipelineStage::Fragment>(name, basename, stage, implOut);
    case StageType::Geometry:
        return Action::template Do<P, PipelineStage::Geometry>(name, basename, stage, implOut);
    case StageType::Control:
        return Action::template Do<P, PipelineStage::Control>(name, basename, stage, implOut);
    case StageType::Evaluation:
        return Action::template Do<P, PipelineStage::Evaluation>(name, basename, stage, implOut);
    default:
        break;
    }
    Log.report(logvisor::Error, "Unknown stage type");
    return false;
}

static const std::regex regWord(R"((\w+))", RegexFlags);

template<typename Action>
bool Compiler::StageAction(const std::string& platforms, StageType type,
                           const std::string& name, const std::string& basename, const std::string& stage,
                           std::string& implOut)
{
    std::smatch match;
    auto begin = platforms.cbegin();
    auto end = platforms.cend();
    while (std::regex_search(begin, end, match, regWord))
    {
        std::string plat = match[1].str();
        std::transform(plat.begin(), plat.end(), plat.begin(), ::tolower);
        if (plat == "glsl")
        {
            if (!StageAction<Action, PlatformType::OpenGL>(type, name, basename, stage, implOut) ||
                !StageAction<Action, PlatformType::Vulkan>(type, name, basename, stage, implOut)
#if HECL_NOUVEAU_NX
                || !StageAction<Action, PlatformType::NX>(type, name, basename, stage, implOut)
#endif
                )
                return false;
        }
        else if (plat == "opengl")
        {
            if (!StageAction<Action, PlatformType::OpenGL>(type, name, basename, stage, implOut))
                return false;
        }
        else if (plat == "vulkan")
        {
            if (!StageAction<Action, PlatformType::Vulkan>(type, name, basename, stage, implOut))
                return false;
        }
        else if (plat == "nx")
        {
#if HECL_NOUVEAU_NX
            if (!StageAction<Action, PlatformType::NX>(type, name, basename, stage, implOut))
                return false;
#endif
        }
        else if (plat == "d3d11" || plat == "hlsl")
        {
#if _WIN32
            if (!StageAction<Action, PlatformType::D3D11>(type, name, basename, stage, implOut))
                return false;
#endif
        }
        else if (plat == "metal")
        {
#if __APPLE__
            if (!StageAction<Action, PlatformType::Metal>(type, name, basename, stage, implOut))
                return false;
#endif
        }
        else
        {
            Log.report(logvisor::Error, "Unknown platform '%s'", plat.c_str());
            return false;
        }
        begin = match.suffix().first;
    }

    return true;
}

static const std::regex regInclude(R"(#\s*include\s+\"(.*)\")", RegexFlags);
static const std::regex regDefine(R"(#\s*define\s+(\w+)\s*(.*))", RegexFlags);
static const std::regex regShaderEx(R"(#\s*shader\s+(\w+)\s*:\s*(\w+))", RegexFlags);
static const std::regex regShader(R"(#\s*shader\s+(\w+))", RegexFlags);
static const std::regex regAttributeEx(R"(#\s*attribute\s+(\w+)\s+([0-9]+))", RegexFlags);
static const std::regex regAttribute(R"(#\s*attribute\s+(\w+))", RegexFlags);
static const std::regex regInstAttributeEx(R"(#\s*instattribute\s+(\w+)\s+([0-9]+))", RegexFlags);
static const std::regex regInstAttribute(R"(#\s*instattribute\s+(\w+))", RegexFlags);
static const std::regex regSrcFac(R"(#\s*srcfac\s+(\w+))", RegexFlags);
static const std::regex regDstFac(R"(#\s*dstfac\s+(\w+))", RegexFlags);
static const std::regex regPrim(R"(#\s*primitive\s+(\w+))", RegexFlags);
static const std::regex regZTest(R"(#\s*depthtest\s+(\w+))", RegexFlags);
static const std::regex regDepthWrite(R"(#\s*depthwrite\s+(\w+))", RegexFlags);
static const std::regex regColorWrite(R"(#\s*colorwrite\s+(\w+))", RegexFlags);
static const std::regex regAlphaWrite(R"(#\s*alphawrite\s+(\w+))", RegexFlags);
static const std::regex regCulling(R"(#\s*culling\s+(\w+))", RegexFlags);
static const std::regex regPatchSize(R"(#\s*patchsize\s+(\w+))", RegexFlags);
static const std::regex regOverwriteAlpha(R"(#\s*overwritealpha\s+(\w+))", RegexFlags);
static const std::regex regDepthAttachment(R"(#\s*depthattachment\s+(\w+))", RegexFlags);
static const std::regex regVertex(R"(#\s*vertex\s+(.*))", RegexFlags);
static const std::regex regFragment(R"(#\s*fragment\s+(.*))", RegexFlags);
static const std::regex regGeometry(R"(#\s*geometry\s+(.*))", RegexFlags);
static const std::regex regControl(R"(#\s*control\s+(.*))", RegexFlags);
static const std::regex regEvaluation(R"(#\s*evaluation\s+(.*))", RegexFlags);

bool Compiler::includeFile(SystemStringView file, std::string& out, int depth)
{
    if (depth > 32)
    {
        Log.report(logvisor::Error, _SYS_STR("Too many levels of includes (>32) at '%s'"), file.data());
        return false;
    }

    const std::string* data = getFileContents(file);
    if (!data)
    {
        Log.report(logvisor::Error, _SYS_STR("Unable to access '%s'"), file.data());
        return false;
    }
    const std::string& sdata = *data;

    SystemString directory;
    auto slashPos = file.find_last_of(_SYS_STR("/\\"));
    if (slashPos != SystemString::npos)
        directory = SystemString(file.begin(), file.begin() + slashPos);
    else
        directory = _SYS_STR(".");

    auto begin = sdata.cbegin();
    auto end = sdata.cend();
    while (begin != end)
    {
        std::string::const_iterator nextBegin;
        auto findPos = sdata.find('\n', begin - sdata.begin());
        if (findPos == std::string::npos)
            nextBegin = end;
        else
            nextBegin = sdata.begin() + findPos + 1;
        
        std::smatch subMatch;
        if (std::regex_search(begin, nextBegin, subMatch, regInclude))
        {
            std::string path = subMatch[1].str();
            if (path.empty())
            {
                Log.report(logvisor::Error, _SYS_STR("Empty path provided to include in '%s'"), file.data());
                return false;
            }

            hecl::SystemString pathStr(hecl::SystemStringConv(path).sys_str());
            if (!hecl::IsAbsolute(pathStr))
                pathStr = directory + _SYS_STR('/') + pathStr;
            if (!includeFile(pathStr, out, depth + 1))
                return false;
        }
        else
        {
            out.insert(out.end(), begin, nextBegin);
        }

        begin = nextBegin;
    }
    return true;
}

static std::string_view BlendFactorToStr(boo::BlendFactor fac)
{
    switch (fac)
    {
    case boo::BlendFactor::Zero:
    default:
        return "boo::BlendFactor::Zero"sv;
    case boo::BlendFactor::One:
        return "boo::BlendFactor::One"sv;
    case boo::BlendFactor::SrcColor:
        return "boo::BlendFactor::SrcColor"sv;
    case boo::BlendFactor::InvSrcColor:
        return "boo::BlendFactor::InvSrcColor"sv;
    case boo::BlendFactor::DstColor:
        return "boo::BlendFactor::DstColor"sv;
    case boo::BlendFactor::InvDstColor:
        return "boo::BlendFactor::InvDstColor"sv;
    case boo::BlendFactor::SrcAlpha:
        return "boo::BlendFactor::SrcAlpha"sv;
    case boo::BlendFactor::InvSrcAlpha:
        return "boo::BlendFactor::InvSrcAlpha"sv;
    case boo::BlendFactor::DstAlpha:
        return "boo::BlendFactor::DstAlpha"sv;
    case boo::BlendFactor::InvDstAlpha:
        return "boo::BlendFactor::InvDstAlpha"sv;
    case boo::BlendFactor::SrcColor1:
        return "boo::BlendFactor::SrcColor1"sv;
    case boo::BlendFactor::InvSrcColor1:
        return "boo::BlendFactor::InvSrcColor1"sv;
    case boo::BlendFactor::Subtract:
        return "boo::BlendFactor::Subtract"sv;
    }
}

static bool StrToBlendFactor(std::string str, boo::BlendFactor& fac)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (str == "zero")
        fac = boo::BlendFactor::Zero;
    else if (str == "one")
        fac = boo::BlendFactor::One;
    else if (str == "srccolor")
        fac = boo::BlendFactor::SrcColor;
    else if (str == "invsrccolor")
        fac = boo::BlendFactor::InvSrcColor;
    else if (str == "dstcolor")
        fac = boo::BlendFactor::DstColor;
    else if (str == "invdstcolor")
        fac = boo::BlendFactor::InvDstColor;
    else if (str == "srcalpha")
        fac = boo::BlendFactor::SrcAlpha;
    else if (str == "invsrcalpha")
        fac = boo::BlendFactor::InvSrcAlpha;
    else if (str == "dstalpha")
        fac = boo::BlendFactor::DstAlpha;
    else if (str == "invdstalpha")
        fac = boo::BlendFactor::InvDstAlpha;
    else if (str == "srccolor1")
        fac = boo::BlendFactor::SrcColor1;
    else if (str == "invsrccolor1")
        fac = boo::BlendFactor::InvSrcColor1;
    else if (str == "subtract")
        fac = boo::BlendFactor::Subtract;
    else
    {
        Log.report(logvisor::Error, "Unrecognized blend mode '%s'", str.c_str());
        return false;
    }
    return true;
}

static std::string_view PrimitiveToStr(boo::Primitive prim)
{
    switch (prim)
    {
    case boo::Primitive::Triangles:
    default:
        return "boo::Primitive::Triangles"sv;
    case boo::Primitive::TriStrips:
        return "boo::Primitive::TriStrips"sv;
    case boo::Primitive::Patches:
        return "boo::Primitive::Patches"sv;
    }
}

static bool StrToPrimitive(std::string str, boo::Primitive& prim)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (str == "triangles")
        prim = boo::Primitive::Triangles;
    else if (str == "tristrips")
        prim = boo::Primitive::TriStrips;
    else if (str == "patches")
        prim = boo::Primitive::Patches;
    else
    {
        Log.report(logvisor::Error, "Unrecognized primitive '%s'", str.c_str());
        return false;
    }
    return true;
}

static std::string_view ZTestToStr(boo::ZTest ztest)
{
    switch (ztest)
    {
    case boo::ZTest::None:
    default:
        return "boo::ZTest::None"sv;
    case boo::ZTest::LEqual:
        return "boo::ZTest::LEqual"sv;
    case boo::ZTest::Greater:
        return "boo::ZTest::Greater"sv;
    case boo::ZTest::GEqual:
        return "boo::ZTest::GEqual"sv;
    case boo::ZTest::Equal:
        return "boo::ZTest::Equal"sv;
    }
}

static bool StrToZTest(std::string str, boo::ZTest& ztest)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (str == "none")
        ztest = boo::ZTest::None;
    else if (str == "lequal")
        ztest = boo::ZTest::LEqual;
    else if (str == "greater")
        ztest = boo::ZTest::Greater;
    else if (str == "gequal")
        ztest = boo::ZTest::GEqual;
    else if (str == "equal")
        ztest = boo::ZTest::Equal;
    else
    {
        Log.report(logvisor::Error, "Unrecognized ztest '%s'", str.c_str());
        return false;
    }
    return true;
}

static std::string_view CullModeToStr(boo::CullMode cull)
{
    switch (cull)
    {
    case boo::CullMode::None:
    default:
        return "boo::CullMode::None"sv;
    case boo::CullMode::Backface:
        return "boo::CullMode::Backface"sv;
    case boo::CullMode::Frontface:
        return "boo::CullMode::Frontface"sv;
    }
}

static bool StrToCullMode(std::string str, boo::CullMode& cull)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (str == "none")
        cull = boo::CullMode::None;
    else if (str == "backface")
        cull = boo::CullMode::Backface;
    else if (str == "frontface")
        cull = boo::CullMode::Frontface;
    else
    {
        Log.report(logvisor::Error, "Unrecognized cull mode '%s'", str.c_str());
        return false;
    }
    return true;
}

static std::string_view BoolToStr(bool b)
{
    return b ? "true"sv : "false"sv;
}

static bool StrToBool(std::string str, bool& b)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (strtol(str.c_str(), nullptr, 0))
        b = true;
    else if (str == "true")
        b = true;
    else if (str == "false")
        b = false;
    else
    {
        Log.report(logvisor::Error, "Unrecognized bool '%s'", str.c_str());
        return false;
    }
    return true;
}

bool Compiler::compileFile(SystemStringView file, std::string_view baseName, std::pair<std::string, std::string>& out)
{
    std::string includesPass;
    if (!includeFile(file, includesPass))
        return false;

    std::string shaderName(baseName);
    std::string shaderBase;
    std::vector<std::pair<boo::VertexSemantic, int>> shaderAttributes;
    bool shaderAttributesReset = false;
    boo::AdditionalPipelineInfo shaderInfo = {};
    std::string stagePlatforms;
    StageType stageType;
    auto stageBegin = includesPass.cend();
    auto stageEnd = includesPass.cend();
    std::unordered_map<std::string, std::pair<std::bitset<6>, std::set<std::string>>> shaderStageUses;
    std::unordered_map<std::string, std::string> shaderBases;

    auto _DoCompile = [&]()
    {
        if (stageBegin == includesPass.end())
            return true;

        if (shaderName.empty())
        {
            Log.report(logvisor::Error, "`#shader <name>` must be issued before stages");
            return false;
        }
        std::string stage(stageBegin, stageEnd);
        for (const auto& define : m_defines)
        {
            std::string::size_type pos = 0;
            while ((pos = stage.find(define.first, pos)) != std::string::npos)
            {
                stage = std::string(stage.begin(), stage.begin() + pos) + define.second +
                        std::string(stage.begin() + pos + define.first.size(), stage.end());
            }
        }
        stageBegin = includesPass.end();
        std::pair<std::bitset<6>, std::set<std::string>>& uses = shaderStageUses[shaderName];
        uses.first.set(size_t(stageType));
        uses.second.insert(stagePlatforms);
        return StageAction<CompileStageAction>(stagePlatforms, stageType, shaderName,
                                               shaderBase, stage, out.second);
    };
    auto DoCompile = [&](std::string platform, StageType type,
                         std::string::const_iterator end, std::string::const_iterator begin)
    {
        stageEnd = end;
        bool ret = _DoCompile();
        stagePlatforms = std::move(platform);
        stageType = type;
        stageBegin = begin;
        return ret;
    };
    auto DoShader = [&]()
    {
        if (shaderBase.empty() && shaderStageUses.find(shaderName) == shaderStageUses.cend())
            return true;
        std::pair<std::bitset<6>, std::set<std::string>>& uses = shaderStageUses[shaderName];
        if (uses.first.test(5))
            return true;

        out.first += Format(ShaderHeaderTemplate, shaderName.c_str(),
                            XXH64(shaderName.c_str(), shaderName.size(), 0));
        out.first += Format(StageObjectHeaderTemplate, shaderName.c_str(), shaderName.c_str(),
                            shaderName.c_str(), shaderName.c_str());
        
        if (!shaderBase.empty())
        {
            shaderBases[shaderName] = shaderBase;

            for (int i = 0; i < 5; ++i)
            {
                if (uses.first.test(size_t(i)))
                    continue;

                std::string useBase = shaderBase;
                std::unordered_map<std::string, std::pair<std::bitset<6>, std::set<std::string>>>::const_iterator
                baseUses = shaderStageUses.find(useBase);
                while (baseUses == shaderStageUses.cend() || !baseUses->second.first.test(size_t(i)))
                {
                    auto search = shaderBases.find(useBase);
                    if (search == shaderBases.cend())
                        break;
                    useBase = search->second;
                    baseUses = shaderStageUses.find(useBase);
                }
                if (baseUses == shaderStageUses.cend() || !baseUses->second.first.test(size_t(i)))
                    continue;
                for (const std::string& basePlatforms : baseUses->second.second)
                {
                    StageAction<CompileSubStageAction>(basePlatforms, StageType(i), shaderName,
                                                       useBase, {}, out.second);
                }
            }
            shaderBase.clear();
        }

        out.second += Format("static const boo::VertexElementDescriptor %s_vtxfmtelems[] = {\n", shaderName.c_str());
        for (const auto& attr : shaderAttributes)
        {
            const char* fmt;
            switch (attr.first & boo::VertexSemantic::SemanticMask)
            {
            case boo::VertexSemantic::Position3:
                fmt = "{boo::VertexSemantic::Position3%s, %d},\n";
                break;
            case boo::VertexSemantic::Position4:
                fmt = "{boo::VertexSemantic::Position4%s, %d},\n";
                break;
            case boo::VertexSemantic::Normal3:
                fmt = "{boo::VertexSemantic::Normal3%s, %d},\n";
                break;
            case boo::VertexSemantic::Normal4:
                fmt = "{boo::VertexSemantic::Normal4%s, %d},\n";
                break;
            case boo::VertexSemantic::Color:
                fmt = "{boo::VertexSemantic::Color%s, %d},\n";
                break;
            case boo::VertexSemantic::ColorUNorm:
                fmt = "{boo::VertexSemantic::ColorUNorm%s, %d},\n";
                break;
            case boo::VertexSemantic::UV2:
                fmt = "{boo::VertexSemantic::UV2%s, %d},\n";
                break;
            case boo::VertexSemantic::UV4:
                fmt = "{boo::VertexSemantic::UV4%s, %d},\n";
                break;
            case boo::VertexSemantic::Weight:
                fmt = "{boo::VertexSemantic::Weight%s, %d},\n";
                break;
            case boo::VertexSemantic::ModelView:
                fmt = "{boo::VertexSemantic::ModelView%s, %d},\n";
                break;
            default:
                fmt = "{boo::VertexSemantic::None%s, %d},\n";
                break;
            }
            out.second += Format(fmt,
            (attr.first & boo::VertexSemantic::Instanced) != boo::VertexSemantic::None ?
            " | boo::VertexSemantic::Instanced" : "", attr.second);
        }
        out.second += "};\n";
        out.second += Format("const boo::VertexFormatInfo Shader_%s::VtxFmt = { %s_vtxfmtelems };\n\n",
                             shaderName.c_str(), shaderName.c_str());
        out.second += Format("const boo::AdditionalPipelineInfo Shader_%s::PipelineInfo = {\n", shaderName.c_str());
        out.second += BlendFactorToStr(shaderInfo.srcFac); out.second += ", ";
        out.second += BlendFactorToStr(shaderInfo.dstFac); out.second += ", ";
        out.second += PrimitiveToStr(shaderInfo.prim); out.second += ", ";
        out.second += ZTestToStr(shaderInfo.depthTest); out.second += ",\n";
        out.second += BoolToStr(shaderInfo.depthWrite); out.second += ", ";
        out.second += BoolToStr(shaderInfo.colorWrite); out.second += ", ";
        out.second += BoolToStr(shaderInfo.alphaWrite); out.second += ", ";
        out.second += CullModeToStr(shaderInfo.culling); out.second += ", ";
        out.second += Format("%d, ", shaderInfo.patchSize);
        out.second += BoolToStr(shaderInfo.overwriteAlpha); out.second += ", ";
        out.second += BoolToStr(shaderInfo.depthAttachment); out.second += ", ";
        out.second += "};\n\n";

        uses.first.set(5);

        return true;
    };
    auto AddAttribute = [&](std::string semantic, std::string idx, bool inst)
    {
        if (shaderAttributesReset)
        {
            shaderAttributes.clear();
            shaderAttributesReset = false;
        }
        boo::VertexSemantic orsem = inst ? boo::VertexSemantic::Instanced : boo::VertexSemantic::None;
        int idxNum = int(strtoul(idx.c_str(), nullptr, 10));
        std::transform(semantic.begin(), semantic.end(), semantic.begin(), ::tolower);
        if (semantic == "position3")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::Position3 | orsem, idxNum));
        else if (semantic == "position4")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::Position4 | orsem, idxNum));
        else if (semantic == "normal3")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::Normal3 | orsem, idxNum));
        else if (semantic == "normal4")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::Normal4 | orsem, idxNum));
        else if (semantic == "color")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::Color | orsem, idxNum));
        else if (semantic == "colorunorm")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::ColorUNorm | orsem, idxNum));
        else if (semantic == "uv2")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::UV2 | orsem, idxNum));
        else if (semantic == "uv4")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::UV4 | orsem, idxNum));
        else if (semantic == "weight")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::Weight | orsem, idxNum));
        else if (semantic == "modelview")
            shaderAttributes.push_back(std::make_pair(boo::VertexSemantic::ModelView | orsem, idxNum));
        else
        {
            Log.report(logvisor::Error, "Unrecognized vertex semantic '%s'", semantic.c_str());
            return false;
        }
        return true;
    };

    auto begin = includesPass.cbegin();
    auto end = includesPass.cend();
    std::string* defineContinue = nullptr;
    while (begin != end)
    {
        std::string::const_iterator nextBegin;
        auto findPos = includesPass.find('\n', begin - includesPass.cbegin());
        if (findPos == std::string::npos)
            nextBegin = end;
        else
            nextBegin = includesPass.cbegin() + findPos + 1;
        
        std::smatch subMatch;
        if (defineContinue)
        {
            std::string extraLine;
            if (findPos == std::string::npos)
                extraLine = std::string(begin, end);
            else
                extraLine = std::string(begin, includesPass.cbegin() + findPos);
            *defineContinue += extraLine;
            if (!defineContinue->empty() && defineContinue->back() == '\\')
                defineContinue->pop_back();
            else
                defineContinue = nullptr;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regDefine))
        {
            std::string& defOut = m_defines[subMatch[1].str()];
            defOut = subMatch[2].str();
            if (!defOut.empty() && defOut.back() == '\\')
            {
                defOut.pop_back();
                defineContinue = &defOut;
            }
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regShaderEx))
        {
            stageEnd = begin;
            if (!_DoCompile() || !DoShader())
                return false;
            shaderName = subMatch[1].str();
            shaderBase = subMatch[2].str();
            shaderAttributesReset = true;
            //shaderAttributes.clear();
            //shaderInfo = boo::AdditionalPipelineInfo();
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regShader))
        {
            stageEnd = begin;
            if (!_DoCompile() || !DoShader())
                return false;
            shaderName = subMatch[1].str();
            shaderAttributesReset = true;
            //shaderAttributes.clear();
            //shaderInfo = boo::AdditionalPipelineInfo();
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regAttributeEx))
        {
            if (!AddAttribute(subMatch[1].str(), subMatch[2].str(), false))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regAttribute))
        {
            if (!AddAttribute(subMatch[1].str(), "0", false))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regInstAttributeEx))
        {
            if (!AddAttribute(subMatch[1].str(), subMatch[2].str(), true))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regInstAttribute))
        {
            if (!AddAttribute(subMatch[1].str(), "0", true))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regSrcFac))
        {
            if (!StrToBlendFactor(subMatch[1].str(), shaderInfo.srcFac))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regDstFac))
        {
            if (!StrToBlendFactor(subMatch[1].str(), shaderInfo.dstFac))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regPrim))
        {
            if (!StrToPrimitive(subMatch[1].str(), shaderInfo.prim))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regZTest))
        {
            if (!StrToZTest(subMatch[1].str(), shaderInfo.depthTest))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regDepthWrite))
        {
            if (!StrToBool(subMatch[1].str(), shaderInfo.depthWrite))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regColorWrite))
        {
            if (!StrToBool(subMatch[1].str(), shaderInfo.colorWrite))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regAlphaWrite))
        {
            if (!StrToBool(subMatch[1].str(), shaderInfo.alphaWrite))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regCulling))
        {
            if (!StrToCullMode(subMatch[1].str(), shaderInfo.culling))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regPatchSize))
        {
            auto str = subMatch[1].str();
            char* endptr;
            shaderInfo.patchSize = uint32_t(strtoul(str.c_str(), &endptr, 0));
            if (endptr == str.c_str())
            {
                Log.report(logvisor::Error, "Non-unsigned-integer value for #patchsize directive");
                return false;
            }
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regOverwriteAlpha))
        {
            if (!StrToBool(subMatch[1].str(), shaderInfo.overwriteAlpha))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regDepthAttachment))
        {
            if (!StrToBool(subMatch[1].str(), shaderInfo.depthAttachment))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regVertex))
        {
            if (!DoCompile(subMatch[1].str(), StageType::Vertex, begin, nextBegin))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regFragment))
        {
            if (!DoCompile(subMatch[1].str(), StageType::Fragment, begin, nextBegin))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regGeometry))
        {
            if (!DoCompile(subMatch[1].str(), StageType::Geometry, begin, nextBegin))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regControl))
        {
            if (!DoCompile(subMatch[1].str(), StageType::Control, begin, nextBegin))
                return false;
        }
        else if (std::regex_search(begin, nextBegin, subMatch, regEvaluation))
        {
            if (!DoCompile(subMatch[1].str(), StageType::Evaluation, begin, nextBegin))
                return false;
        }
        
        begin = nextBegin;
    }

    stageEnd = begin;
    if (!_DoCompile() || !DoShader())
        return false;

    out.first += "#define UNIVERSAL_PIPELINES_";
    out.first += baseName;
    for (const auto& shader : shaderStageUses)
    {
        out.first += " \\\n";
        out.first += "::Shader_";
        out.first += shader.first;
    }
    out.first += "\n";

    out.first += "#define STAGES_";
    out.first += baseName;
    out.first += "(P, S)";
    for (const auto& shader : shaderStageUses)
    {
        out.first += " \\\n";
        out.first += "::StageObject_";
        out.first += shader.first;
        out.first += "<P, S>,";
    }
    out.first += "\n";

    return true;
}

bool Compiler::compile(std::string_view baseName, std::pair<std::string, std::string>& out)
{
    out =
    {
        "#pragma once\n"
        "#include \"hecl/PipelineBase.hpp\"\n\n",
        Format("#include \"%s.hpp\"\n\n", baseName.data())
    };

    for (const auto& file : m_inputFiles)
        if (!compileFile(file, baseName, out))
            return false;

    return true;
}

}
