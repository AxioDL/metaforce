#ifndef _DNAMP1_CSKR_HPP_
#define _DNAMP1_CSKR_HPP_

#include "../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP1
{

struct CSKR : BigDNA
{
    DECL_DNA
    Value<atUint32> skinningRuleCount;
    struct SkinningRule : BigDNA
    {
        DECL_DNA
        Value<atUint32> weightCount;
        struct Weight : BigDNA
        {
            DECL_DNA
            Value<atUint32> boneId;
            Value<float> weight;
        };
        Vector<Weight, DNA_COUNT(weightCount)> weights;
        Value<atUint32> vertCount;
    };
    Vector<SkinningRule, DNA_COUNT(skinningRuleCount)> skinningRules;
};

}
}

#endif // _DNAMP1_CSKR_HPP_
