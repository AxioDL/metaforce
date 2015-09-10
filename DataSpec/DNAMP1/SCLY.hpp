#ifndef _DNAMP1_SCLY_HPP_
#define _DNAMP1_SCLY_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "ScriptObjects/IScriptObject.hpp"
#include "DNAMP1.hpp"

namespace Retro
{
namespace DNAMP1
{
struct SCLY : BigYAML
{
    DECL_EXPLICIT_YAML
    Delete _d;
    Value<FourCC> fourCC;
    Value<atUint32> version;
    Value<atUint32> layerCount;

    Vector<atUint32, DNA_COUNT(layerCount)> layerSizes;

    struct ScriptLayer : BigYAML
    {
        DECL_EXPLICIT_YAML
        Delete _d;
        Value<atUint8> unknown;
        Value<atUint32> objectCount;
        Vector<std::shared_ptr<IScriptObject>, DNA_COUNT(objectCount)> objects;
        void read(Athena::io::IStreamReader &rs);
        void write(Athena::io::IStreamWriter &ws) const;
    };
    Vector<ScriptLayer, DNA_COUNT(layerCount)> layers;

    void read(Athena::io::IStreamReader &rs);
    void write(Athena::io::IStreamWriter &ws) const;

    void exportToLayerDirectories(const PAK::Entry &, PAKRouter<PAKBridge>&, bool);
};
}
}
#endif
