#ifndef _DNAMP3_HINT_HPP_
#define _DNAMP3_HINT_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec::DNAMP3
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
        UniqueID64 stringID;
        Value<atUint32> unknown2;
        struct Location : BigYAML
        {
            DECL_YAML
            UniqueID64 worldAssetID;
            UniqueID64 areaAssetID;
            Value<atUint32> areaID;
            UniqueID64 stringID;
            Value<atUint32> unknown[3];
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
};
}
#endif // _DNAMP1_HINT_HPP_
