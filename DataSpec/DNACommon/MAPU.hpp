#pragma once

#include "DNACommon.hpp"

namespace DataSpec::DNAMAPU
{
struct MAPU : BigDNA
{
    AT_DECL_DNA
    Value<uint32_t> magic;
    Value<uint32_t> version;
    UniqueID32 hexMapa;
    Value<uint32_t> worldCount;
    struct Transform : BigDNA
    {
        AT_DECL_DNA
        Value<atVec4f> xf[3];
    };
    struct World : BigDNA
    {
        AT_DECL_DNA
        String<-1> name;
        UniqueID32 mlvl;
        Transform transform;
        Value<uint32_t> hexCount;
        Vector<Transform, AT_DNA_COUNT(hexCount)> hexTransforms;
        DNAColor hexColor;
    };
    Vector<World, AT_DNA_COUNT(worldCount)> worlds;

    static bool Cook(const hecl::blender::MapUniverse& mapu, const hecl::ProjectPath& out);
};

template <typename PAKRouter>
bool ReadMAPUToBlender(hecl::blender::Connection& conn,
                       const MAPU& mapu,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force);

}

