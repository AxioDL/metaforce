#ifndef _DNAMP1_SCAN_HPP_
#define _DNAMP1_SCAN_HPP_

#include <athena/FileWriter.hpp>
#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec
{
namespace DNAMP1
{
static const std::vector<std::string> PaneNames =
{
    "imagepane_pane0",  "imagepane_pane1",   "imagepane_pane2",   "imagepane_pane3",   "imagepane_pane01",
    "imagepane_pane12", "imagepane_pane23",  "imagepane_pane012", "imagepane_pane123", "imagepane_pane0123",
    "imagepane_pane4",  "imagepane_pane5",   "imagepane_pane6",   "imagepane_pane7",   "imagepane_pane45",
    "imagepane_pane56", "imagepane_pane67",  "imagepane_pane456", "imagepane_pane567", "imagepane_pane4567"
};

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
        Delete __delete;
        UniqueID32 texture;
        Value<float> appearanceRange;
        enum class Position : atInt32
        {
            Pane0,   Pane1,    Pane2,   Pane3, Pane01,  Pane12,   Pane23,  Pane012,
            Pane123, Pane0123, Pane4,   Pane5, Pane6,   Pane7,    Pane45,  Pane56,
            Pane67,  Pane456,  Pane567, Pane4567, Invalid = -1
        };
        Value<Position> position;
        Value<atUint32> width;        // width of animation cell
        Value<atUint32> height;       // height of animation cell
        Value<float>    interval;     // 0.0 - 1.0
        Value<float>    fadeDuration; // 0.0 - 1.0

        void read(athena::io::IStreamReader& __dna_reader)
        {
            /* texture */
            texture.read(__dna_reader);
            /* appearanceRange */
            appearanceRange = __dna_reader.readFloatBig();
            /* position */
            position = Position(__dna_reader.readUint32Big());
            /* width */
            width = __dna_reader.readUint32Big();
            /* height */
            height = __dna_reader.readUint32Big();
            /* interval */
            interval = __dna_reader.readFloatBig();
            /* fadeDuration */
            fadeDuration = __dna_reader.readFloatBig();
        }

        void write(athena::io::IStreamWriter& __dna_writer) const
        {
            /* texture */
            texture.write(__dna_writer);
            /* appearanceRange */
            __dna_writer.writeFloatBig(appearanceRange);
            /* position */
            __dna_writer.writeUint32Big(atUint32(position));
            /* width */
            __dna_writer.writeUint32Big(width);
            /* height */
            __dna_writer.writeUint32Big(height);
            /* interval */
            __dna_writer.writeFloatBig(interval);
            /* fadeDuration */
            __dna_writer.writeFloatBig(fadeDuration);
        }

        void read(athena::io::YAMLDocReader& __dna_docin)
        {
            /* texture */
            __dna_docin.enumerate("texture", texture);
            /* appearanceRange */
            appearanceRange = __dna_docin.readFloat("appearanceRange");
            /* position */
            std::string tmp = __dna_docin.readString("position");

            auto idx = std::find(PaneNames.begin(), PaneNames.end(), tmp);
            if (idx != PaneNames.end())
                position = Position(idx - PaneNames.begin());
            else
                position = Position::Invalid;

            /* width */
            width = __dna_docin.readUint32("width");
            /* height */
            height = __dna_docin.readUint32("height");
            /* interval */
            interval = __dna_docin.readFloat("interval");
            /* fadeDuration */
            fadeDuration = __dna_docin.readFloat("fadeDuration");
        }

        void write(athena::io::YAMLDocWriter& __dna_docout) const
        {
            /* texture */
            __dna_docout.enumerate("texture", texture);
            /* appearanceRange */
            __dna_docout.writeFloat("appearanceRange", appearanceRange);
            /* position */
            if (position != Position::Invalid)
                __dna_docout.writeString("position", PaneNames.at(atUint32(position)));
            else
                __dna_docout.writeString("position", "undefined");
            /* width */
            __dna_docout.writeUint32("width", width);
            /* height */
            __dna_docout.writeUint32("height", height);
            /* interval */
            __dna_docout.writeFloat("interval", interval);
            /* fadeDuration */
            __dna_docout.writeFloat("fadeDuration", fadeDuration);
        }

        const char* DNAType() { return "urde::DNAMP1::SCAN::Texture"; }
        size_t binarySize(size_t __isz) const
        {
            __isz = texture.binarySize(__isz);
            return __isz + 24;
        }

    };

    Texture textures[4];

    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
    {
        SCAN scan;
        scan.read(rs);
        athena::io::FileWriter writer(outPath.getAbsolutePath());
        scan.toYAMLStream(writer);
        return true;
    }

    static bool Cook(const SCAN& scan, const hecl::ProjectPath& outPath)
    {
        athena::io::FileWriter ws(outPath.getAbsolutePath());
        scan.write(ws);
        return true;
    }

    static Category GetCategory(const hecl::ProjectPath& inPath)
    {
        SCAN scan;
        athena::io::FileReader reader(inPath.getAbsolutePath());
        if (reader.hasError())
            return Category::None;
        if (!scan.fromYAMLStream(reader))
            return Category::None;
        return scan.category;
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
            ent->name = hecl::Format("SCAN_%s_strg", entry.id.toString().c_str());
        }
        for (int i=0 ; i<4 ; ++i)
        {
            const Texture& tex = scan.textures[i];
            if (tex.texture)
            {
                PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(tex.texture);
                ent->name = hecl::Format("SCAN_%s_tex%d", entry.id.toString().c_str(), i+1);
            }
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut)
    {
        for (int i = 0; i < 4; ++i)
            g_curSpec->flattenDependencies(textures[i].texture, pathsOut);
    }
};
}
}

#endif
