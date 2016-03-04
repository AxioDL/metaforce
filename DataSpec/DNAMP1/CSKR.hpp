#ifndef _DNAMP1_CSKR_HPP_
#define _DNAMP1_CSKR_HPP_

#include "BlenderConnection.hpp"
#include "../DNACommon/DNACommon.hpp"
#include "CINF.hpp"

namespace DataSpec
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

    const atInt16* getMatrixBank(size_t) const
    {
        return nullptr;
    }

    void weightVertex(hecl::BlenderConnection::PyOutStream& os, const CINF& cinf, atUint32 idx) const
    {
        atUint32 accum = 0;
        for (const SkinningRule& rule : skinningRules)
        {
            if (idx >= accum && idx < accum + rule.vertCount)
                for (const SkinningRule::Weight& weight : rule.weights)
                    os.format("vert[dvert_lay][%u] = %f\n",
                              cinf.getBoneIdxFromId(weight.boneId),
                              weight.weight);
            accum += rule.vertCount;
        }
    }
};

}
}

#endif // _DNAMP1_CSKR_HPP_
