#ifndef _DNAMP1_SCLY_HPP_
#define _DNAMP1_SCLY_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "ScriptObjects/IScriptObject.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{
struct SCLY : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    Value<FourCC> fourCC;
    Value<atUint32> version;
    Value<atUint32> layerCount;

    Vector<atUint32, DNA_COUNT(layerCount)> layerSizes;

    struct ScriptLayer : BigDNA
    {
        AT_DECL_EXPLICIT_DNA_YAML
        Value<atUint8> unknown;
        Value<atUint32> objectCount;
        Vector<std::unique_ptr<IScriptObject>, DNA_COUNT(objectCount)> objects;
        void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
                std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const;
        void nameIDs(PAKRouter<PAKBridge>& pakRouter) const;
    };
    Vector<ScriptLayer, DNA_COUNT(layerCount)> layers;

    void exportToLayerDirectories(const PAK::Entry &, PAKRouter<PAKBridge>&, bool) const;
    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const;
    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const;
};
}
#endif
