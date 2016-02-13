#ifndef _DNAMP1_SCLY_HPP_
#define _DNAMP1_SCLY_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "ScriptObjects/IScriptObject.hpp"
#include "DNAMP1.hpp"

namespace DataSpec
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
        Vector<std::unique_ptr<IScriptObject>, DNA_COUNT(objectCount)> objects;
        void read(Athena::io::IStreamReader &rs);
        void write(Athena::io::IStreamWriter &ws) const;
        size_t binarySize(size_t __isz) const;
        void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
                std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const;
        void nameIDs(PAKRouter<PAKBridge>& pakRouter) const;
    };
    Vector<ScriptLayer, DNA_COUNT(layerCount)> layers;

    void read(Athena::io::IStreamReader &rs);
    void write(Athena::io::IStreamWriter &ws) const;
    size_t binarySize(size_t __isz) const;

    void exportToLayerDirectories(const PAK::Entry &, PAKRouter<PAKBridge>&, bool) const;
    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const;
    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const;
};
}
}
#endif
