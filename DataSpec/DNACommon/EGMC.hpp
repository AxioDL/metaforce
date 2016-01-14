#ifndef EGMC_HPP
#define EGMC_HPP

#include "DNACommon.hpp"

namespace Retro
{
namespace DNACommon
{
struct EGMC : public BigDNA
{
    DECL_DNA
    Value<atUint32> count;

    struct Object : BigDNA
    {
        DECL_DNA
        Value<atUint32> mesh;
        Value<atUint32> instanceId;
    };

    Vector<Object, DNA_COUNT(count)> objects;
};
}
}
#endif // EGMC_HPP
