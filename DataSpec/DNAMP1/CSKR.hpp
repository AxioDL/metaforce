#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "CINF.hpp"

namespace DataSpec::DNAMP1
{

struct CSKR : BigDNA
{
    AT_DECL_DNA
    Value<atUint32> skinningRuleCount;
    struct SkinningRule : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> weightCount;
        struct Weight : BigDNA
        {
            AT_DECL_DNA
            Value<atUint32> boneId;
            Value<float> weight;
        };
        Vector<Weight, AT_DNA_COUNT(weightCount)> weights;
        Value<atUint32> vertCount;
    };
    Vector<SkinningRule, AT_DNA_COUNT(skinningRuleCount)> skinningRules;

    const atInt16* getMatrixBank(size_t) const
    {
        return nullptr;
    }

    void weightVertex(hecl::blender::PyOutStream& os, const CINF& cinf, atUint32 idx) const;
};

}

