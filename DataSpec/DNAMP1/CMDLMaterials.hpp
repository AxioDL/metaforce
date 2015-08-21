#ifndef _DNAMP1_CMDL_MATERIALS_HPP_
#define _DNAMP1_CMDL_MATERIALS_HPP_

#include <BlenderConnection.hpp>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/GX.hpp"
#include "../DNACommon/CMDL.hpp"
#include "DNAMP1.hpp"

namespace Retro
{
namespace DNAMP1
{

struct MaterialSet : BigDNA
{
    DECL_DNA
    struct MaterialSetHead : BigDNA
    {
        DECL_DNA
        Value<atUint32> textureCount;
        Vector<UniqueID32, DNA_COUNT(textureCount)> textureIDs;
        Value<atUint32> materialCount;
        Vector<atUint32, DNA_COUNT(materialCount)> materialEndOffs;
    } head;

    struct Material : BigDNA
    {
        DECL_DNA
        struct Flags : BigDNA
        {
            DECL_DNA
            Value<atUint32> flags;
            inline bool konstValuesEnabled() const {return (flags & 0x8) != 0;}
            inline void setKonstValuesEnabled(bool enabled) {flags &= ~0x8; flags |= atUint32(enabled) << 3;}
            inline bool depthSorting() const {return (flags & 0x10) != 0;}
            inline void setDepthSorting(bool enabled) {flags &= ~0x10; flags |= atUint32(enabled) << 4;}
            inline bool punchthroughAlpha() const {return (flags & 0x20) != 0;}
            inline void setPunchthroughAlpha(bool enabled) {flags &= ~0x20; flags |= atUint32(enabled) << 5;}
            inline bool samusReflection() const {return (flags & 0x40) != 0;}
            inline void setSamusReflection(bool enabled) {flags &= ~0x40; flags |= atUint32(enabled) << 6;}
            inline bool depthWrite() const {return (flags & 0x80) != 0;}
            inline void setDepthWrite(bool enabled) {flags &= ~0x80; flags |= atUint32(enabled) << 7;}
            inline bool samusReflectionSurfaceEye() const {return (flags & 0x100) != 0;}
            inline void setSamusReflectionSurfaceEye(bool enabled) {flags &= ~0x100; flags |= atUint32(enabled) << 8;}
            inline bool shadowOccluderMesh() const {return (flags & 0x200) != 0;}
            inline void setShadowOccluderMesh(bool enabled) {flags &= ~0x200; flags |= atUint32(enabled) << 9;}
            inline bool samusReflectionIndirectTexture() const {return (flags & 0x400) != 0;}
            inline void setSamusReflectionIndirectTexture(bool enabled) {flags &= ~0x400; flags |= atUint32(enabled) << 10;}
            inline bool lightmap() const {return (flags & 0x800) != 0;}
            inline void setLightmap(bool enabled) {flags &= ~0x800; flags |= atUint32(enabled) << 11;}
            inline bool lightmapUVArray() const {return (flags & 0x2000) != 0;}
            inline void setLightmapUVArray(bool enabled) {flags &= ~0x2000; flags |= atUint32(enabled) << 13;}
            inline atUint16 textureSlots() const {return (flags >> 16) != 0;}
            inline void setTextureSlots(atUint16 texslots) {flags &= ~0xffff0000; flags |= atUint32(texslots) << 16;}
        } flags;

        Value<atUint32> textureCount;
        Vector<atUint32, DNA_COUNT(textureCount)> texureIdxs;
        struct VAFlags : BigDNA
        {
            DECL_DNA
            Value<atUint32> vaFlags;
            inline GX::AttrType position() const {return GX::AttrType(vaFlags & 0x3);}
            inline void setPosition(GX::AttrType val) {vaFlags &= ~0x3; vaFlags |= atUint32(val);}
            inline GX::AttrType normal() const {return GX::AttrType(vaFlags >> 2 & 0x3);}
            inline void setNormal(GX::AttrType val) {vaFlags &= ~0xC; vaFlags |= atUint32(val) << 2;}
            inline GX::AttrType color0() const {return GX::AttrType(vaFlags >> 4 & 0x3);}
            inline void setColor0(GX::AttrType val) {vaFlags &= ~0x30; vaFlags |= atUint32(val) << 4;}
            inline GX::AttrType color1() const {return GX::AttrType(vaFlags >> 6 & 0x3);}
            inline void setColor1(GX::AttrType val) {vaFlags &= ~0xC0; vaFlags |= atUint32(val) << 6;}
            inline GX::AttrType tex0() const {return GX::AttrType(vaFlags >> 8 & 0x3);}
            inline void setTex0(GX::AttrType val) {vaFlags &= ~0x300; vaFlags |= atUint32(val) << 8;}
            inline GX::AttrType tex1() const {return GX::AttrType(vaFlags >> 10 & 0x3);}
            inline void setTex1(GX::AttrType val) {vaFlags &= ~0xC00; vaFlags |= atUint32(val) << 10;}
            inline GX::AttrType tex2() const {return GX::AttrType(vaFlags >> 12 & 0x3);}
            inline void setTex2(GX::AttrType val) {vaFlags &= ~0x3000; vaFlags |= atUint32(val) << 12;}
            inline GX::AttrType tex3() const {return GX::AttrType(vaFlags >> 14 & 0x3);}
            inline void setTex3(GX::AttrType val) {vaFlags &= ~0xC000; vaFlags |= atUint32(val) << 14;}
            inline GX::AttrType tex4() const {return GX::AttrType(vaFlags >> 16 & 0x3);}
            inline void setTex4(GX::AttrType val) {vaFlags &= ~0x30000; vaFlags |= atUint32(val) << 16;}
            inline GX::AttrType tex5() const {return GX::AttrType(vaFlags >> 18 & 0x3);}
            inline void setTex5(GX::AttrType val) {vaFlags &= ~0xC0000; vaFlags |= atUint32(val) << 18;}
            inline GX::AttrType tex6() const {return GX::AttrType(vaFlags >> 20 & 0x3);}
            inline void setTex6(GX::AttrType val) {vaFlags &= ~0x300000; vaFlags |= atUint32(val) << 20;}
            inline GX::AttrType pnMatIdx() const {return GX::AttrType(vaFlags >> 24 & 0x1);}
            inline void setPnMatIdx(GX::AttrType val) {vaFlags &= ~0x1000000; vaFlags |= atUint32(val & 0x1) << 24;}
            inline GX::AttrType tex0MatIdx() const {return GX::AttrType(vaFlags >> 25 & 0x1);}
            inline void setTex0MatIdx(GX::AttrType val) {vaFlags &= ~0x2000000; vaFlags |= atUint32(val & 0x1) << 25;}
            inline GX::AttrType tex1MatIdx() const {return GX::AttrType(vaFlags >> 26 & 0x1);}
            inline void setTex1MatIdx(GX::AttrType val) {vaFlags &= ~0x4000000; vaFlags |= atUint32(val & 0x1) << 26;}
            inline GX::AttrType tex2MatIdx() const {return GX::AttrType(vaFlags >> 27 & 0x1);}
            inline void setTex2MatIdx(GX::AttrType val) {vaFlags &= ~0x8000000; vaFlags |= atUint32(val & 0x1) << 27;}
            inline GX::AttrType tex3MatIdx() const {return GX::AttrType(vaFlags >> 28 & 0x1);}
            inline void setTex3MatIdx(GX::AttrType val) {vaFlags &= ~0x10000000; vaFlags |= atUint32(val & 0x1) << 28;}
            inline GX::AttrType tex4MatIdx() const {return GX::AttrType(vaFlags >> 29 & 0x1);}
            inline void setTex4MatIdx(GX::AttrType val) {vaFlags &= ~0x20000000; vaFlags |= atUint32(val & 0x1) << 29;}
            inline GX::AttrType tex5MatIdx() const {return GX::AttrType(vaFlags >> 30 & 0x1);}
            inline void setTex5MatIdx(GX::AttrType val) {vaFlags &= ~0x40000000; vaFlags |= atUint32(val & 0x1) << 30;}
            inline GX::AttrType tex6MatIdx() const {return GX::AttrType(vaFlags >> 31 & 0x1);}
            inline void setTex6MatIdx(GX::AttrType val) {vaFlags &= ~0x80000000; vaFlags |= atUint32(val & 0x1) << 31;}
        } vaFlags;
        inline const VAFlags& getVAFlags() const {return vaFlags;}
        Value<atUint32> groupIdx;

        Vector<atUint32, DNA_COUNT(flags.konstValuesEnabled())> konstCount;
        Vector<GX::Color, DNA_COUNT(flags.konstValuesEnabled() ? konstCount[0] : 0)> konstColors;

        /** Slightly modified blend enums in Retro's implementation */
        enum BlendFactor : atUint16
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
        Value<BlendFactor> blendDstFac;
        Value<BlendFactor> blendSrcFac;
        Vector<atUint32, DNA_COUNT(flags.samusReflectionIndirectTexture())> indTexSlot;

        Value<atUint32> colorChannelCount;
        struct ColorChannel : BigDNA
        {
            DECL_DNA
            Value<atUint32> flags;
            inline bool lighting() const {return (flags & 0x1) != 0;}
            inline void setLighting(bool enabled) {flags &= ~0x1; flags |= atUint32(enabled);}
            inline bool useAmbient() const {return (flags & 0x2) != 0;}
            inline void setUseAmbient(bool enabled) {flags &= ~0x2; flags |= atUint32(enabled) << 1;}
            inline bool useMaterial() const {return (flags & 0x4) != 0;}
            inline void setUseMaterial(bool enabled) {flags &= ~0x4; flags |= atUint32(enabled) << 2;}
            inline atUint8 lightmask() const {return atUint8(flags >> 3 & 0xff);}
            inline void setLightmask(atUint8 mask) {flags &= ~0x7f8; flags |= atUint32(mask) << 3;}
            inline atUint8 diffuseFn() const {return atUint8(flags >> 11 & 0x3);}
            inline void setDiffuseFn(atUint8 fn) {flags &= ~0x1800; flags |= atUint32(fn) << 11;}
            inline atUint8 attenuationFn() const {return atUint8(flags >> 13 & 0x3);}
            inline void setAttenuationFn(atUint8 fn) {flags &= ~0x6000; flags |= atUint32(fn) << 13;}
        };
        Vector<ColorChannel, DNA_COUNT(colorChannelCount)> colorChannels;

        Value<atUint32> tevStageCount;
        struct TEVStage : BigDNA
        {
            DECL_DNA
            Value<atUint32> ciFlags;
            Value<atUint32> aiFlags;
            Value<atUint32> ccFlags;
            Value<atUint32> acFlags;
            Value<atUint8> pad;
            Value<atUint8> kaInput;
            Value<atUint8> kcInput;
            Value<atUint8> rascInput;

            inline GX::TevColorArg colorInA() const {return GX::TevColorArg(ciFlags & 0xf);}
            inline void setColorInA(GX::TevColorArg val) {ciFlags &= ~0x1f; ciFlags |= atUint32(val);}
            inline GX::TevColorArg colorInB() const {return GX::TevColorArg(ciFlags >> 5 & 0xf);}
            inline void setColorInB(GX::TevColorArg val) {ciFlags &= ~0x3e0; ciFlags |= atUint32(val) << 5;}
            inline GX::TevColorArg colorInC() const {return GX::TevColorArg(ciFlags >> 10 & 0xf);}
            inline void setColorInC(GX::TevColorArg val) {ciFlags &= ~0x7c00; ciFlags |= atUint32(val) << 10;}
            inline GX::TevColorArg colorInD() const {return GX::TevColorArg(ciFlags >> 15 & 0xf);}
            inline void setColorInD(GX::TevColorArg val) {ciFlags &= ~0xf8000; ciFlags |= atUint32(val) << 15;}

            inline GX::TevAlphaArg alphaInA() const {return GX::TevAlphaArg(aiFlags & 0x7);}
            inline void setAlphaInA(GX::TevAlphaArg val) {aiFlags &= ~0x1f; aiFlags |= atUint32(val);}
            inline GX::TevAlphaArg alphaInB() const {return GX::TevAlphaArg(aiFlags >> 5 & 0x7);}
            inline void setAlphaInB(GX::TevAlphaArg val) {aiFlags &= ~0x3e0; aiFlags |= atUint32(val) << 5;}
            inline GX::TevAlphaArg alphaInC() const {return GX::TevAlphaArg(aiFlags >> 10 & 0x7);}
            inline void setAlphaInC(GX::TevAlphaArg val) {aiFlags &= ~0x7c00; aiFlags |= atUint32(val) << 10;}
            inline GX::TevAlphaArg alphaInD() const {return GX::TevAlphaArg(aiFlags >> 15 & 0x7);}
            inline void setAlphaInD(GX::TevAlphaArg val) {aiFlags &= ~0xf8000; aiFlags |= atUint32(val) << 15;}

            inline GX::TevOp colorOp() const {return GX::TevOp(ccFlags & 0xf);}
            inline void setColorOp(GX::TevOp val) {ccFlags &= ~0x1; ccFlags |= atUint32(val);}
            inline GX::TevBias colorOpBias() const {return GX::TevBias(ccFlags >> 4 & 0x3);}
            inline void setColorOpBias(GX::TevBias val) {ccFlags &= ~0x30; ccFlags |= atUint32(val) << 4;}
            inline GX::TevScale colorOpScale() const {return GX::TevScale(ccFlags >> 6 & 0x3);}
            inline void setColorOpScale(GX::TevScale val) {ccFlags &= ~0xc0; ccFlags |= atUint32(val) << 6;}
            inline bool colorOpClamp() const {return ccFlags >> 8 & 0x1;}
            inline void setColorOpClamp(bool val) {ccFlags &= ~0x100; ccFlags |= atUint32(val) << 8;}
            inline GX::TevRegID colorOpOutReg() const {return GX::TevRegID(ccFlags >> 9 & 0x3);}
            inline void setColorOpOutReg(GX::TevRegID val) {ccFlags &= ~0x600; ccFlags |= atUint32(val) << 9;}

            inline GX::TevOp alphaOp() const {return GX::TevOp(acFlags & 0xf);}
            inline void setAlphaOp(GX::TevOp val) {acFlags &= ~0x1; acFlags |= atUint32(val);}
            inline GX::TevBias alphaOpBias() const {return GX::TevBias(acFlags >> 4 & 0x3);}
            inline void setAlphaOpBias(GX::TevBias val) {acFlags &= ~0x30; acFlags |= atUint32(val) << 4;}
            inline GX::TevScale alphaOpScale() const {return GX::TevScale(acFlags >> 6 & 0x3);}
            inline void setAlphaOpScale(GX::TevScale val) {acFlags &= ~0xc0; acFlags |= atUint32(val) << 6;}
            inline bool alphaOpClamp() const {return acFlags >> 8 & 0x1;}
            inline void setAlphaOpClamp(bool val) {acFlags &= ~0x100; acFlags |= atUint32(val) << 8;}
            inline GX::TevRegID alphaOpOutReg() const {return GX::TevRegID(acFlags >> 9 & 0x3);}
            inline void setAlphaOpOutReg(GX::TevRegID val) {acFlags &= ~0x600; acFlags |= atUint32(val) << 9;}

            inline GX::TevKColorSel kColorIn() const {return GX::TevKColorSel(kcInput);}
            inline GX::TevKAlphaSel kAlphaIn() const {return GX::TevKAlphaSel(kaInput);}
        };
        Vector<TEVStage, DNA_COUNT(tevStageCount)> tevStages;
        struct TEVStageTexInfo : BigDNA
        {
            DECL_DNA
            Value<atUint16> pad;
            Value<atUint8> texSlot;
            Value<atUint8> tcgSlot;
        };
        Vector<TEVStageTexInfo, DNA_COUNT(tevStageCount)> tevStageTexInfo;

        Value<atUint32> tcgCount;
        struct TexCoordGen : BigDNA
        {
            DECL_DNA
            Value<atUint32> flags;

            inline GX::TexGenType type() const {return GX::TexGenType(flags & 0xf);}
            inline void setType(GX::TexGenType val) {flags &= ~0xf; flags |= atUint32(val);}
            inline GX::TexGenSrc source() const {return GX::TexGenSrc(flags >> 4 & 0x1f);}
            inline void setSource(GX::TexGenSrc val) {flags &= ~0x1f0; flags |= atUint32(val) << 4;}
            inline GX::TexMtx mtx() const {return GX::TexMtx(flags >> 9 & 0x1f + 30);}
            inline void setMtx(GX::TexMtx val) {flags &= ~0x3e00; flags |= (atUint32(val)-30) << 9;}
            inline bool normalize() const {return flags >> 14 & 0x1;}
            inline void setNormalize(bool val) {flags &= ~0x4000; flags |= atUint32(val) << 14;}
            inline GX::PTTexMtx postMtx() const {return GX::PTTexMtx(flags >> 15 & 0x3f + 64);}
            inline void setPostMtx(GX::PTTexMtx val) {flags &= ~0x1f8000; flags |= (atUint32(val)-64) << 15;}
        };
        Vector<TexCoordGen, DNA_COUNT(tcgCount)> tcgs;

        Value<atUint32> uvAnimsSize;
        Value<atUint32> uvAnimsCount;
        struct UVAnimation : BigDNA
        {
            Delete expl;
            enum Mode : atUint32
            {
                ANIM_MV_INV_NOTRANS,
                ANIM_MV_INV,
                ANIM_SCROLL,
                ANIM_ROTATION,
                ANIM_HSTRIP,
                ANIM_VSTRIP,
                ANIM_MODEL,
                ANIM_MODE_WHO_MUST_NOT_BE_NAMED
            } mode;
            float vals[4];
            void read(Athena::io::IStreamReader& reader)
            {
                mode = Mode(reader.readUint32Big());
                switch (mode)
                {
                case ANIM_MV_INV_NOTRANS:
                case ANIM_MV_INV:
                case ANIM_MODEL:
                    break;
                case ANIM_SCROLL:
                case ANIM_HSTRIP:
                case ANIM_VSTRIP:
                    vals[0] = reader.readFloatBig();
                    vals[1] = reader.readFloatBig();
                    vals[2] = reader.readFloatBig();
                    vals[3] = reader.readFloatBig();
                    break;
                case ANIM_ROTATION:
                case ANIM_MODE_WHO_MUST_NOT_BE_NAMED:
                    vals[0] = reader.readFloatBig();
                    vals[1] = reader.readFloatBig();
                    break;
                }
            }
            void write(Athena::io::IStreamWriter& writer) const
            {
                writer.writeUint32Big(mode);
                switch (mode)
                {
                case ANIM_MV_INV_NOTRANS:
                case ANIM_MV_INV:
                case ANIM_MODEL:
                    break;
                case ANIM_SCROLL:
                case ANIM_HSTRIP:
                case ANIM_VSTRIP:
                    writer.writeFloatBig(vals[0]);
                    writer.writeFloatBig(vals[1]);
                    writer.writeFloatBig(vals[2]);
                    writer.writeFloatBig(vals[3]);
                    break;
                case ANIM_ROTATION:
                case ANIM_MODE_WHO_MUST_NOT_BE_NAMED:
                    writer.writeFloatBig(vals[0]);
                    writer.writeFloatBig(vals[1]);
                    break;
                }
            }
        };
        Vector<UVAnimation, DNA_COUNT(uvAnimsCount)> uvAnims;
    };
    Vector<Material, DNA_COUNT(head.materialCount)> materials;

    static void RegisterMaterialProps(HECL::BlenderConnection::PyOutStream& out);
    static void ConstructMaterial(HECL::BlenderConnection::PyOutStream& out,
                                  const MaterialSet::Material& material,
                                  unsigned groupIdx, unsigned matIdx);

    inline void readToBlender(HECL::BlenderConnection::PyOutStream& os,
                              const PAKRouter<PAKBridge>& pakRouter,
                              const typename PAKRouter<PAKBridge>::EntryType& entry,
                              unsigned setIdx,
                              const SpecBase& dataspec)
    {
        DNACMDL::ReadMaterialSetToBlender_1_2(os, *this, pakRouter, entry, setIdx, dataspec);
    }
};

}
}

#endif // _DNAMP1_CMDL_MATERIALS_HPP_
