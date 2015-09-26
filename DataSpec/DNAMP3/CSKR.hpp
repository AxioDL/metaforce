#ifndef _DNAMP3_CSKR_HPP_
#define _DNAMP3_CSKR_HPP_

#include "hecl/blender/BlenderConnection.hpp"
#include "../DNACommon/DNACommon.hpp"
#include "CINF.hpp"
#include "../DNAMP2/CSKR.hpp"

namespace Retro
{
namespace DNAMP3
{

struct CSKR : BigDNA
{
    DECL_DNA
    DNAFourCC magic;
    Value<atUint32> version;
    DNAMP2::CSKR data;
    Value<atUint32> matrixCount;
    struct MatrixBindings : BigDNA
    {
        DECL_DNA
        Value<atInt16> mtxs[10];
    };
    Vector<MatrixBindings, DNA_COUNT(matrixCount / 10)> mtxBindings;

    Value<atUint32> unkCount1;
    Vector<atUint8, DNA_COUNT(unkCount1)> unk1;
    Value<atUint32> unkCount2;
    Vector<atUint8, DNA_COUNT(unkCount2)> unk2;
    Value<atUint32> unkCount3;
    Vector<atUint8, DNA_COUNT(unkCount3)> unk3;
    Value<atUint32> unkCount4;
    Vector<atUint8, DNA_COUNT(unkCount4)> unk4;
    Value<atUint32> unkCount5;
    Vector<atUint8, DNA_COUNT(unkCount5)> unk5;

    const atInt16* getMatrixBank(size_t idx) const
    {
        return mtxBindings.at(idx).mtxs;
    }

    void weightVertex(HECL::BlenderConnection::PyOutStream& os, const CINF& cinf, atInt16 skinIdx) const
    {
        if (skinIdx < 0)
            return;
        const DNAMP2::CSKR::SkinningRule& rule = data.skinningRules[skinIdx];
        for (const DNAMP2::CSKR::SkinningRule::Weight& weight : rule.weights)
            os.format("vert[dvert_lay][%u] = %f\n",
                      cinf.getBoneIdxFromId(weight.boneId),
                      weight.weight);
    }
};

}
}

#endif // _DNAMP3_CSKR_HPP_
