#ifndef _DNAMP1_CSKR_HPP_
#define _DNAMP1_CSKR_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "CINF.hpp"

namespace DataSpec::DNAMP1
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

    const atInt16* getMatrixBank(size_t) const
    {
        return nullptr;
    }

    void weightVertex(hecl::blender::PyOutStream& os, const CINF& cinf, atUint32 idx) const;
};

}

#endif // _DNAMP1_CSKR_HPP_
