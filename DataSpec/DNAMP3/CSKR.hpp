#ifndef _DNAMP3_CSKR_HPP_
#define _DNAMP3_CSKR_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "CINF.hpp"
#include "../DNAMP2/CSKR.hpp"

namespace DataSpec::DNAMP3
{

struct CSKR : BigDNA
{
    AT_DECL_DNA
    DNAFourCC magic;
    Value<atUint32> version;
    DNAMP2::CSKR data;
    Value<atUint32> matrixCount;
    struct MatrixBindings : BigDNA
    {
        AT_DECL_DNA
        Value<atInt16> mtxs[10];
    };
    Vector<MatrixBindings, AT_DNA_COUNT(matrixCount / 10)> mtxBindings;

    Value<atUint32> unkCount1;
    Vector<atUint8, AT_DNA_COUNT(unkCount1)> unk1;
    Value<atUint32> unkCount2;
    Vector<atUint8, AT_DNA_COUNT(unkCount2)> unk2;
    Value<atUint32> unkCount3;
    Vector<atUint8, AT_DNA_COUNT(unkCount3)> unk3;
    Value<atUint32> unkCount4;
    Vector<atUint8, AT_DNA_COUNT(unkCount4)> unk4;
    Value<atUint32> unkCount5;
    Vector<atUint8, AT_DNA_COUNT(unkCount5)> unk5;

    const atInt16* getMatrixBank(size_t idx) const
    {
        return mtxBindings.at(idx).mtxs;
    }

    void weightVertex(hecl::blender::PyOutStream& os, const CINF& cinf, atInt16 skinIdx) const;
};

}

#endif // _DNAMP3_CSKR_HPP_
