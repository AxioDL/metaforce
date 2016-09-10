#ifndef _DNAMP1_HINT_HPP_
#define _DNAMP1_HINT_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/PAK.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct HINT : BigYAML
{
    DECL_YAML
    Value<atUint32> magic;
    Value<atUint32> version;

    struct Hint : BigYAML
    {
        DECL_YAML
        String<-1> name;
        Value<float> unknown1;
        Value<float> fadeInTime;
        UniqueID32 stringID;
        Value<atUint32> unknown2;
        struct Location : BigYAML
        {
            DECL_YAML
            UniqueID32 worldAssetID;
            UniqueID32 areaAssetID;
            Value<atUint32> areaID;
            UniqueID32 stringID;
        };

        Value<atUint32> locationCount;
        Vector<Location, DNA_COUNT(locationCount)> locations;
    };
    Value<atUint32> hintCount;
    Vector<Hint, DNA_COUNT(hintCount)> hints;


    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
    {
        HINT hint;
        hint.read(rs);
        athena::io::FileWriter writer(outPath.getAbsolutePath());
        hint.toYAMLStream(writer);
        return true;
    }

    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath)
    {
        HINT hint;
        athena::io::FileReader reader(inPath.getAbsolutePath());
        hint.fromYAMLStream(reader);
        athena::io::FileWriter ws(outPath.getAbsolutePath());
        hint.write(ws);
        return true;
    }
};
}
}
#endif // _DNAMP1_HINT_HPP_
