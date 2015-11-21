#ifndef _DNAMP1_SCAN_HPP_
#define _DNAMP1_SCAN_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace Retro
{
namespace DNAMP1
{
struct SCAN : BigYAML
{
    DECL_YAML
    Value<atUint32> version;
    Value<atUint32> magic;
    UniqueID32      frame;
    UniqueID32      string;

    enum class ScanSpeed : atUint32
    { Normal, Slow };
    Value<ScanSpeed> scanSpeed;

    enum class Category : atUint32
    {
        None,
        SpacePirateData,
        ChozoLore,
        Creatures,
        Research
    };

    Value<Category> category;

    Value<bool> isImportant;

    struct Texture : BigYAML
    {
        DECL_YAML
        UniqueID32 texture;
        Value<float> appearanceRange;
        enum class Position : atInt32
        {
            Pane0,
            Pane1,
            Pane2,
            Pane3,
            Pane01,
            Pane12,
            Pane23,
            Pane012,
            Pane123,
            Pane0123,
            Pane4,
            Pane5,
            Pane6,
            Pane7,
            Pane45,
            Pane56,
            Pane67,
            Pane456,
            Pane567,
            Pane4567,
            Invalid = -1
        };
        Value<Position> position;
        Value<atUint32> width;        // width of animation cell
        Value<atUint32> height;       // height of animation cell
        Value<float>    interval;     // 0.0 - 1.0
        Value<float>    fadeDuration; // 0.0 - 1.0
    };

    Texture textures[4];

    static bool Extract(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
    {
        SCAN scan;
        scan.read(rs);
        FILE* fp = HECL::Fopen(outPath.getAbsolutePath().c_str(), _S("wb"));
        scan.toYAMLFile(fp);
        fclose(fp);
        return true;
    }

    static void Name(const SpecBase& dataSpec,
                     PAKEntryReadStream& rs,
                     PAKRouter<PAKBridge>& pakRouter,
                     PAK::Entry& entry)
    {
        SCAN scan;
        scan.read(rs);
        if (scan.string)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(scan.string);
            ent->name = HECL::Format("SCAN_%s_strg", entry.id.toString().c_str());
        }
        for (int i=0 ; i<4 ; ++i)
        {
            const Texture& tex = scan.textures[i];
            if (tex.texture)
            {
                PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(tex.texture);
                ent->name = HECL::Format("SCAN_%s_tex%d", entry.id.toString().c_str(), i+1);
            }
        }
    }
};
}
}

#endif
