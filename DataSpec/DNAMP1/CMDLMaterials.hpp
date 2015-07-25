#ifndef _DNAMP1_CMDL_MATERIALS_HPP_
#define _DNAMP1_CMDL_MATERIALS_HPP_

#include "../DNACommon/DNACommon.hpp"

namespace Retro
{
namespace DNAMP1
{

struct CMDLMaterial : BigDNA
{
    Delete expl;

    struct MaterialSetHead : BigDNA
    {
        DECL_DNA
        Value<atUint32> textureCount;
        Vector<UniqueID32, DNA_COUNT(textureCount)> textureIDs;
        Value<atUint32> materialCount;
        Vector<UniqueID32, DNA_COUNT(materialCount)> materialEndOffs;
    } head;

    struct Material : BigDNA
    {
        DECL_DNA
        struct Flags : BigDNA
        {
            DECL_DNA
            Value<atUint32> flags;
            inline bool konstValuesEnabled() const {return flags & 0x8;}
            inline void setKonstValuesEnabled(bool enabled) {flags &= ~0x8; flags |= enabled << 3;}
            inline bool depthSorting() const {return flags & 0x10;}
            inline void setDepthSorting(bool enabled) {flags &= ~0x10; flags |= enabled << 4;}
            inline bool punchthroughAlpha() const {return flags & 0x20;}
            inline void setPunchthroughAlpha(bool enabled) {flags &= ~0x20; flags |= enabled << 5;}
            inline bool samusReflection() const {return flags & 0x40;}
            inline void setSamusReflection(bool enabled) {flags &= ~0x40; flags |= enabled << 6;}
            inline bool depthWrite() const {return flags & 0x80;}
            inline void setDepthWrite(bool enabled) {flags &= ~0x80; flags |= enabled << 7;}
            inline bool samusReflectionSurfaceEye() const {return flags & 0x100;}
            inline void setSamusReflectionSurfaceEye(bool enabled) {flags &= ~0x100; flags |= enabled << 8;}
            inline bool shadowOccluderMesh() const {return flags & 0x200;}
            inline void setShadowOccluderMesh(bool enabled) {flags &= ~0x200; flags |= enabled << 9;}
            inline bool samusReflectionIndirectTexture() const {return flags & 0x400;}
            inline void setSamusReflectionIndirectTexture(bool enabled) {flags &= ~0x400; flags |= enabled << 10;}
            inline bool lightmap() const {return flags & 0x800;}
            inline void setLightmap(bool enabled) {flags &= ~0x800; flags |= enabled << 11;}
            inline bool lightmapUVArray() const {return flags & 0x2000;}
            inline void setLightmapUVArray(bool enabled) {flags &= ~0x2000; flags |= enabled << 13;}
            inline atUint16 textureSlots() const {return flags >> 16;}
            inline void setTextureSlots(atUint16 texslots) {flags &= ~0xffff0000; flags |= (atUint32)texslots << 16;}
        } flags;
        Value<atUint32> textureCount;
        Vector<UniqueID32, DNA_COUNT(textureCount)> texureIdxs;
        struct VAFlags : BigDNA
        {
            DECL_DNA
            Value<atUint32> vaFlags;
            inline GXAttrType position() const {return GXAttrType(vaFlags & 0x3);}
            inline void setPosition(GXAttrType val) {vaFlags &= ~0x3; vaFlags |= val;}
            inline GXAttrType normal() const {return GXAttrType(vaFlags >> 2 & 0x3);}
            inline void setNormal(GXAttrType val) {vaFlags &= ~0xC; vaFlags |= val << 2;}
            inline GXAttrType color0() const {return GXAttrType(vaFlags >> 4 & 0x3);}
            inline void setColor0(GXAttrType val) {vaFlags &= ~0x30; vaFlags |= val << 4;}
            inline GXAttrType color1() const {return GXAttrType(vaFlags >> 6 & 0x3);}
            inline void setColor1(GXAttrType val) {vaFlags &= ~0xC0; vaFlags |= val << 6;}
            inline GXAttrType tex0() const {return GXAttrType(vaFlags >> 8 & 0x3);}
            inline void setTex0(GXAttrType val) {vaFlags &= ~0x300; vaFlags |= val << 8;}
            inline GXAttrType tex1() const {return GXAttrType(vaFlags >> 10 & 0x3);}
            inline void setTex1(GXAttrType val) {vaFlags &= ~0xC00; vaFlags |= val << 10;}
            inline GXAttrType tex2() const {return GXAttrType(vaFlags >> 12 & 0x3);}
            inline void setTex2(GXAttrType val) {vaFlags &= ~0x3000; vaFlags |= val << 12;}
            inline GXAttrType tex3() const {return GXAttrType(vaFlags >> 14 & 0x3);}
            inline void setTex3(GXAttrType val) {vaFlags &= ~0xC000; vaFlags |= val << 14;}
            inline GXAttrType tex4() const {return GXAttrType(vaFlags >> 16 & 0x3);}
            inline void setTex4(GXAttrType val) {vaFlags &= ~0x30000; vaFlags |= val << 16;}
            inline GXAttrType tex5() const {return GXAttrType(vaFlags >> 18 & 0x3);}
            inline void setTex5(GXAttrType val) {vaFlags &= ~0xC0000; vaFlags |= val << 18;}
            inline GXAttrType tex6() const {return GXAttrType(vaFlags >> 20 & 0x3);}
            inline void setTex6(GXAttrType val) {vaFlags &= ~0x300000; vaFlags |= val << 20;}
        } vaFlags;
        Value<atUint32> groupIdx;
        Value<atUint32> konstCount;
        Vector<GXColor, DNA_COUNT(konstCount)> konstColors;
        enum GXBlendFactor
        {
            GX_BL_ZERO,
            GX_BL_ONE,
            GX_BL_SRCCLR,
            GX_BL_INVSRCCLR,
            GX_BL_SRCALPHA,
            GX_BL_INVSRCALPHA,
            GX_BL_DSTALPHA,
            GX_BL_INVDSTALPHA
        };
        Value<atUint16> blendDstFac;
        Value<atUint16> blendSrcFac;
        Vector<atUint32, DNA_COUNT(flags.samusReflectionIndirectTexture())> indTexSlot;
        Value<atUint32> colorChannelCount;
        struct ColorChannel : BigDNA
        {
            DECL_DNA
            Value<atUint32> flags;
            inline bool lighting() const {return flags & 0x1;}
            inline void setLighting(bool enabled) {flags &= ~0x1; flags |= enabled;}
            inline bool useAmbient() const {return flags & 0x2;}
            inline void setUseAmbient(bool enabled) {flags &= ~0x2; flags |= enabled << 1;}
            inline bool useMaterial() const {return flags & 0x4;}
            inline void setUseMaterial(bool enabled) {flags &= ~0x4; flags |= enabled << 2;}
            inline bool lightmask() const {return flags >> 3 & 0xff;}
            inline void setLightmask(atUint8 mask) {flags &= ~0x7f8; flags |= (atUint32)mask << 3;}
            inline bool diffuseFn() const {return flags >> 11 & 0x3;}
            inline void setDiffuseFn(atUint8 fn) {flags &= ~0x1800; flags |= (atUint32)fn << 11;}
            inline bool attenuationFn() const {return flags >> 13 & 0x3;}
            inline void setAttenuationFn(atUint8 fn) {flags &= ~0x6000; flags |= (atUint32)fn << 13;}
        };
        Vector<ColorChannel, DNA_COUNT(colorChannelCount)> colorChannels;
        Value<atUint32> tevStageCount;
        struct TEVStage : BigDNA
        {
            DECL_DNA

        };
        Vector<TEVStage, DNA_COUNT(tevStageCount)> tevStages;
    };

    void read(Athena::io::IStreamReader& reader)
    {
        head.read(reader);
    }
    void write(Athena::io::IStreamWriter& writer) const
    {
        head.write(writer);
    }
};

}
}

#endif // _DNAMP1_CMDL_MATERIALS_HPP_
