#ifndef _DNAMP2_CSKR_HPP_
#define _DNAMP2_CSKR_HPP_

#include "BlenderConnection.hpp"
#include "../DNACommon/DNACommon.hpp"
#include "CINF.hpp"
#include "../DNAMP1/CSKR.hpp"

namespace DataSpec
{
namespace DNAMP2
{

struct CSKR : DNAMP1::CSKR
{
    Delete expl;

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

#endif // _DNAMP2_CSKR_HPP_
