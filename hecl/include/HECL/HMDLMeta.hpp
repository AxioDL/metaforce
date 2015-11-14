#ifndef HMDLMETA_HPP
#define HMDLMETA_HPP

#include <HECL/HECL.hpp>
#include <Athena/DNA.hpp>

namespace HECL
{

enum HMDLTopology : atUint32
{
    TopologyTriangles,
    TopologyTriStrips,
};

#define HECL_HMDL_META_SZ 32

struct HMDLMeta : Athena::io::DNA<Athena::BigEndian>
{
    DECL_DNA
    Value<atUint32> magic = SBIG('TACO');
    Value<HMDLTopology> topology;
    Value<atUint32> vertStride;
    Value<atUint32> vertCount;
    Value<atUint32> indexCount;
    Value<atUint32> colorCount;
    Value<atUint32> uvCount;
    Value<atUint32> weightCount;
};

}

#endif // HMDLMETA_HPP
