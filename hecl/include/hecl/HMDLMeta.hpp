#pragma once

#include "hecl/hecl.hpp"
#include "athena/DNA.hpp"

namespace hecl
{

enum class HMDLTopology : atUint32
{
    Triangles,
    TriStrips,
};

#define HECL_HMDL_META_SZ 32

struct HMDLMeta : athena::io::DNA<athena::Big>
{
    AT_DECL_DNA
    Value<atUint32> magic = 'TACO';
    Value<HMDLTopology> topology;
    Value<atUint32> vertStride;
    Value<atUint32> vertCount;
    Value<atUint32> indexCount;
    Value<atUint32> colorCount;
    Value<atUint32> uvCount;
    Value<atUint16> weightCount;
    Value<atUint16> bankCount;
};

}

