#ifndef _DNAMP2_CMDL_MATERIALS_HPP_
#define _DNAMP2_CMDL_MATERIALS_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/GX.hpp"
#include "../DNAMP1/CMDLMaterials.hpp"
#include "DNAMP2.hpp"

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
        Vector<atUint32, DNA_COUNT(textureCount)> texureIdxs;
        using VAFlags = DNAMP1::MaterialSet::Material::VAFlags;
        DNAMP1::MaterialSet::Material::VAFlags vaFlags;
        inline const VAFlags& getVAFlags() const {return vaFlags;}
        Value<atUint32> unk0; /* MP2 only */
        Value<atUint32> unk1; /* MP2 only */
        Value<atUint32> groupIdx;

        Vector<atUint32, DNA_COUNT(flags.konstValuesEnabled())> konstCount;
        Vector<GX::Color, DNA_COUNT(flags.konstValuesEnabled() ? konstCount[0] : 0)> konstColors;

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
        Vector<DNAMP1::MaterialSet::Material::TexCoordGen, DNA_COUNT(tcgCount)> tcgs;

        Value<atUint32> uvAnimsSize;
        Value<atUint32> uvAnimsCount;
        Vector<DNAMP1::MaterialSet::Material::UVAnimation, DNA_COUNT(uvAnimsCount)> uvAnims;
    };
    Vector<Material, DNA_COUNT(head.materialCount)> materials;

    static inline void RegisterMaterialProps(HECL::BlenderConnection::PyOutStream& out)
    {
        DNAMP1::MaterialSet::RegisterMaterialProps(out);
    }
    static void ConstructMaterial(HECL::BlenderConnection::PyOutStream& out,
                                  const MaterialSet::Material& material,
                                  unsigned groupIdx, unsigned matIdx);

    inline void readToBlender(HECL::BlenderConnection::PyOutStream& os,
                              const PAKRouter<PAKBridge>& pakRouter,
                              const typename PAKRouter<PAKBridge>::EntryType& entry,
                              std::vector<DNACMDL::VertexAttributes>& attributesOut,
                              unsigned setIdx)
    {
        DNACMDL::ReadMaterialSetToBlender_1_2(os, *this, pakRouter, entry, attributesOut, setIdx);
    }
};

}
}

#endif // _DNAMP2_CMDL_MATERIALS_HPP_
