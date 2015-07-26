#ifndef _DNAMP2_CMDL_MATERIALS_HPP_
#define _DNAMP2_CMDL_MATERIALS_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/GX.hpp"
#include "../DNAMP1/CMDLMaterials.hpp"

namespace Retro
{
namespace DNAMP2
{

/* Structurally identical to DNAMP1::MaterialSet except unk0 and unk1 fields */
struct MaterialSet : BigDNA
{
    DECL_DNA
    DNAMP1::MaterialSet::MaterialSetHead head;

    struct Material : BigDNA
    {
        DECL_DNA
        DNAMP1::MaterialSet::Material::Flags flags;

        Value<atUint32> textureCount;
        Vector<UniqueID32, DNA_COUNT(textureCount)> texureIdxs;
        DNAMP1::MaterialSet::Material::VAFlags vaFlags;
        Value<atUint32> unk0; /* MP2 only */
        Value<atUint32> unk1; /* MP2 only */
        Value<atUint32> groupIdx;

        Value<atUint32> konstCount;
        Vector<GX::Color, DNA_COUNT(konstCount)> konstColors;

        Value<atUint16> _blendDstFac;
        using BlendFactor = DNAMP1::MaterialSet::Material::BlendFactor;
        inline BlendFactor blendDestFactor() const {return BlendFactor(_blendDstFac);}
        inline void setBlendDestFactor(BlendFactor fac) {_blendDstFac = fac;}
        Value<atUint16> _blendSrcFac;
        inline BlendFactor blendSrcFactor() const {return BlendFactor(_blendSrcFac);}
        inline void setBlendSrcFactor(BlendFactor fac) {_blendSrcFac = fac;}
        Vector<atUint32, DNA_COUNT(flags.samusReflectionIndirectTexture())> indTexSlot;

        Value<atUint32> colorChannelCount;
        Vector<DNAMP1::MaterialSet::Material::ColorChannel, DNA_COUNT(colorChannelCount)> colorChannels;

        Value<atUint32> tevStageCount;
        Vector<DNAMP1::MaterialSet::Material::TEVStage, DNA_COUNT(tevStageCount)> tevStages;
        Vector<DNAMP1::MaterialSet::Material::TEVStageTexInfo, DNA_COUNT(tevStageCount)> tevStageTexInfo;

        Value<atUint32> tcgCount;
        Vector<DNAMP1::MaterialSet::Material::TexCoordGen, DNA_COUNT(tcgCount)> tgcs;

        Value<atUint32> uvAnimsSize;
        Value<atUint32> uvAnimsCount;
        Vector<DNAMP1::MaterialSet::Material::UVAnimation, DNA_COUNT(uvAnimsCount)> uvAnims;
    };
    Vector<Material, DNA_COUNT(head.materialCount)> materials;

};

}
}

#endif // _DNAMP2_CMDL_MATERIALS_HPP_
