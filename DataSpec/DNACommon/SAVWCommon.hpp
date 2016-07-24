#ifndef __COMMON_SAVWCOMMON_HPP__
#define __COMMON_SAVWCOMMON_HPP__
#include "DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec
{
namespace SAVWCommon
{
enum class EScanCategory
{
    None,
    Data,
    Lore,
    Creature,
    Research
};

struct Header : BigYAML
{
    DECL_YAML
    Value<atUint32> magic;
    Value<atUint32> version;
    Value<atUint32> areaCount;
};

struct EnvironmentVariable : BigYAML
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unk1;
    Value<atUint32> unk2;
    Value<atUint32> unk3;
};

struct Layer : BigYAML
{
    DECL_YAML
    Value<atUint32> areaId;
    Value<atUint32> layer;
};

template <class SAVW>
static bool ExtractSAVW(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    SAVW savw;
    savw.read(rs);
    FILE* fp = hecl::Fopen(outPath.getAbsolutePath().c_str(), _S("wb"));
    savw.toYAMLFile(fp);
    fclose(fp);
    return true;
}

}
}

#endif // __COMMON_SAVWCOMMON_HPP__
