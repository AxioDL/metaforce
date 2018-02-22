#ifndef __COMMON_SAVWCOMMON_HPP__
#define __COMMON_SAVWCOMMON_HPP__
#include "DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec::SAVWCommon
{
enum class EScanCategory
{
    None,
    Data,
    Lore,
    Creature,
    Research,
    Artifact
};

struct Header : BigDNA
{
    AT_DECL_DNA_YAML
    Value<atUint32> magic;
    Value<atUint32> version;
    Value<atUint32> areaCount;
};

struct EnvironmentVariable : BigDNA
{
    AT_DECL_DNA_YAML
    String<-1> name;
    Value<atUint32> unk1;
    Value<atUint32> unk2;
    Value<atUint32> unk3;
};

struct Layer : BigDNA
{
    AT_DECL_DNA_YAML
    Value<atUint32> areaId;
    Value<atUint32> layer;
};

template <class SAVW>
static bool ExtractSAVW(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    SAVW savw;
    savw.read(rs);
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    athena::io::ToYAMLStream(savw, writer);
    return true;
}

}

#endif // __COMMON_SAVWCOMMON_HPP__
