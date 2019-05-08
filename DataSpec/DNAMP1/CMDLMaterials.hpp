#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/GX.hpp"
#include "DataSpec/DNACommon/CMDL.hpp"
#include "DNAMP1.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP1 {

struct MaterialSet : BigDNA {
  static constexpr bool OneSection() { return false; }

  AT_DECL_DNA
  struct MaterialSetHead : BigDNA {
    AT_DECL_DNA
    Value<atUint32> textureCount = 0;
    Vector<UniqueID32, AT_DNA_COUNT(textureCount)> textureIDs;
    Value<atUint32> materialCount = 0;
    Vector<atUint32, AT_DNA_COUNT(materialCount)> materialEndOffs;

    void addTexture(const UniqueID32& id) {
      textureIDs.push_back(id);
      ++textureCount;
    }
    void addMaterialEndOff(atUint32 off) {
      materialEndOffs.push_back(off);
      ++materialCount;
    }

    template <class PAKBRIDGE>
    void ensureTexturesExtracted(PAKRouter<PAKBRIDGE>& pakRouter) const {
      for (const auto& id : textureIDs) {
        const nod::Node* node;
        const PAK::Entry* texEntry = pakRouter.lookupEntry(id, &node);
        if (!texEntry)
          continue;
        hecl::ProjectPath txtrPath = pakRouter.getWorking(texEntry);
        if (txtrPath.isNone()) {
          txtrPath.makeDirChain(false);
          PAKEntryReadStream rs = texEntry->beginReadStream(*node);
          TXTR::Extract(rs, txtrPath);
        }
      }
    }
  } head;

  struct Material : BigDNA {
    AT_DECL_DNA
    struct Flags : BigDNA {
      AT_DECL_DNA
      Value<atUint32> flags = 0;
      bool konstValuesEnabled() const { return (flags & 0x8) != 0; }
      void setKonstValuesEnabled(bool enabled) {
        flags &= ~0x8;
        flags |= atUint32(enabled) << 3;
      }
      bool depthSorting() const { return (flags & 0x10) != 0; }
      void setDepthSorting(bool enabled) {
        flags &= ~0x10;
        flags |= atUint32(enabled) << 4;
      }
      bool alphaTest() const { return (flags & 0x20) != 0; }
      void setAlphaTest(bool enabled) {
        flags &= ~0x20;
        flags |= atUint32(enabled) << 5;
      }
      bool samusReflection() const { return (flags & 0x40) != 0; }
      void setSamusReflection(bool enabled) {
        flags &= ~0x40;
        flags |= atUint32(enabled) << 6;
      }
      bool depthWrite() const { return (flags & 0x80) != 0; }
      void setDepthWrite(bool enabled) {
        flags &= ~0x80;
        flags |= atUint32(enabled) << 7;
      }
      bool samusReflectionSurfaceEye() const { return (flags & 0x100) != 0; }
      void setSamusReflectionSurfaceEye(bool enabled) {
        flags &= ~0x100;
        flags |= atUint32(enabled) << 8;
      }
      bool shadowOccluderMesh() const { return (flags & 0x200) != 0; }
      void setShadowOccluderMesh(bool enabled) {
        flags &= ~0x200;
        flags |= atUint32(enabled) << 9;
      }
      bool samusReflectionIndirectTexture() const { return (flags & 0x400) != 0; }
      void setSamusReflectionIndirectTexture(bool enabled) {
        flags &= ~0x400;
        flags |= atUint32(enabled) << 10;
      }
      bool lightmap() const { return (flags & 0x800) != 0; }
      void setLightmap(bool enabled) {
        flags &= ~0x800;
        flags |= atUint32(enabled) << 11;
      }
      bool lightmapUVArray() const { return (flags & 0x2000) != 0; }
      void setLightmapUVArray(bool enabled) {
        flags &= ~0x2000;
        flags |= atUint32(enabled) << 13;
      }
      atUint16 textureSlots() const { return (flags >> 16) != 0; }
      void setTextureSlots(atUint16 texslots) {
        flags &= ~0xffff0000;
        flags |= atUint32(texslots) << 16;
      }
    } flags;
    const Flags& getFlags() const { return flags; }

    Value<atUint32> textureCount = 0;
    Vector<atUint32, AT_DNA_COUNT(textureCount)> textureIdxs;
    struct VAFlags : BigDNA {
      AT_DECL_DNA
      Value<atUint32> vaFlags = 0;
      GX::AttrType position() const { return GX::AttrType(vaFlags & 0x3); }
      void setPosition(GX::AttrType val) {
        vaFlags &= ~0x3;
        vaFlags |= atUint32(val);
      }
      GX::AttrType normal() const { return GX::AttrType(vaFlags >> 2 & 0x3); }
      void setNormal(GX::AttrType val) {
        vaFlags &= ~0xC;
        vaFlags |= atUint32(val) << 2;
      }
      GX::AttrType color0() const { return GX::AttrType(vaFlags >> 4 & 0x3); }
      void setColor0(GX::AttrType val) {
        vaFlags &= ~0x30;
        vaFlags |= atUint32(val) << 4;
      }
      GX::AttrType color1() const { return GX::AttrType(vaFlags >> 6 & 0x3); }
      void setColor1(GX::AttrType val) {
        vaFlags &= ~0xC0;
        vaFlags |= atUint32(val) << 6;
      }
      GX::AttrType tex0() const { return GX::AttrType(vaFlags >> 8 & 0x3); }
      void setTex0(GX::AttrType val) {
        vaFlags &= ~0x300;
        vaFlags |= atUint32(val) << 8;
      }
      GX::AttrType tex1() const { return GX::AttrType(vaFlags >> 10 & 0x3); }
      void setTex1(GX::AttrType val) {
        vaFlags &= ~0xC00;
        vaFlags |= atUint32(val) << 10;
      }
      GX::AttrType tex2() const { return GX::AttrType(vaFlags >> 12 & 0x3); }
      void setTex2(GX::AttrType val) {
        vaFlags &= ~0x3000;
        vaFlags |= atUint32(val) << 12;
      }
      GX::AttrType tex3() const { return GX::AttrType(vaFlags >> 14 & 0x3); }
      void setTex3(GX::AttrType val) {
        vaFlags &= ~0xC000;
        vaFlags |= atUint32(val) << 14;
      }
      GX::AttrType tex4() const { return GX::AttrType(vaFlags >> 16 & 0x3); }
      void setTex4(GX::AttrType val) {
        vaFlags &= ~0x30000;
        vaFlags |= atUint32(val) << 16;
      }
      GX::AttrType tex5() const { return GX::AttrType(vaFlags >> 18 & 0x3); }
      void setTex5(GX::AttrType val) {
        vaFlags &= ~0xC0000;
        vaFlags |= atUint32(val) << 18;
      }
      GX::AttrType tex6() const { return GX::AttrType(vaFlags >> 20 & 0x3); }
      void setTex6(GX::AttrType val) {
        vaFlags &= ~0x300000;
        vaFlags |= atUint32(val) << 20;
      }
      GX::AttrType pnMatIdx() const { return GX::AttrType(vaFlags >> 24 & 0x1); }
      void setPnMatIdx(GX::AttrType val) {
        vaFlags &= ~0x1000000;
        vaFlags |= atUint32(val & 0x1) << 24;
      }
      GX::AttrType tex0MatIdx() const { return GX::AttrType(vaFlags >> 25 & 0x1); }
      void setTex0MatIdx(GX::AttrType val) {
        vaFlags &= ~0x2000000;
        vaFlags |= atUint32(val & 0x1) << 25;
      }
      GX::AttrType tex1MatIdx() const { return GX::AttrType(vaFlags >> 26 & 0x1); }
      void setTex1MatIdx(GX::AttrType val) {
        vaFlags &= ~0x4000000;
        vaFlags |= atUint32(val & 0x1) << 26;
      }
      GX::AttrType tex2MatIdx() const { return GX::AttrType(vaFlags >> 27 & 0x1); }
      void setTex2MatIdx(GX::AttrType val) {
        vaFlags &= ~0x8000000;
        vaFlags |= atUint32(val & 0x1) << 27;
      }
      GX::AttrType tex3MatIdx() const { return GX::AttrType(vaFlags >> 28 & 0x1); }
      void setTex3MatIdx(GX::AttrType val) {
        vaFlags &= ~0x10000000;
        vaFlags |= atUint32(val & 0x1) << 28;
      }
      GX::AttrType tex4MatIdx() const { return GX::AttrType(vaFlags >> 29 & 0x1); }
      void setTex4MatIdx(GX::AttrType val) {
        vaFlags &= ~0x20000000;
        vaFlags |= atUint32(val & 0x1) << 29;
      }
      GX::AttrType tex5MatIdx() const { return GX::AttrType(vaFlags >> 30 & 0x1); }
      void setTex5MatIdx(GX::AttrType val) {
        vaFlags &= ~0x40000000;
        vaFlags |= atUint32(val & 0x1) << 30;
      }
      GX::AttrType tex6MatIdx() const { return GX::AttrType(vaFlags >> 31 & 0x1); }
      void setTex6MatIdx(GX::AttrType val) {
        vaFlags &= ~0x80000000;
        vaFlags |= atUint32(val & 0x1) << 31;
      }

      size_t vertDLSize() const {
        static size_t ATTR_SZ[] = {0, 1, 1, 2};
        size_t ret = 0;
        ret += ATTR_SZ[position()];
        ret += ATTR_SZ[normal()];
        ret += ATTR_SZ[color0()];
        ret += ATTR_SZ[color1()];
        ret += ATTR_SZ[tex0()];
        ret += ATTR_SZ[tex1()];
        ret += ATTR_SZ[tex2()];
        ret += ATTR_SZ[tex3()];
        ret += ATTR_SZ[tex4()];
        ret += ATTR_SZ[tex5()];
        ret += ATTR_SZ[tex6()];
        ret += ATTR_SZ[pnMatIdx()];
        ret += ATTR_SZ[tex0MatIdx()];
        ret += ATTR_SZ[tex1MatIdx()];
        ret += ATTR_SZ[tex2MatIdx()];
        ret += ATTR_SZ[tex3MatIdx()];
        ret += ATTR_SZ[tex4MatIdx()];
        ret += ATTR_SZ[tex5MatIdx()];
        ret += ATTR_SZ[tex6MatIdx()];
        return ret;
      }
    } vaFlags;
    const VAFlags& getVAFlags() const { return vaFlags; }
    Value<atUint32> uniqueIdx;

    Vector<atUint32, AT_DNA_COUNT(flags.konstValuesEnabled())> konstCount;
    Vector<GX::Color, AT_DNA_COUNT(flags.konstValuesEnabled() ? konstCount[0] : 0)> konstColors;

    using BlendFactor = GX::BlendFactor;
    Value<BlendFactor> blendDstFac;
    Value<BlendFactor> blendSrcFac;
    Vector<atUint32, AT_DNA_COUNT(flags.samusReflectionIndirectTexture())> indTexSlot;

    Value<atUint32> colorChannelCount = 0;
    struct ColorChannel : BigDNA {
      AT_DECL_DNA
      Value<atUint32> flags = 0;
      bool lighting() const { return (flags & 0x1) != 0; }
      void setLighting(bool enabled) {
        flags &= ~0x1;
        flags |= atUint32(enabled);
      }
      bool useAmbient() const { return (flags & 0x2) != 0; }
      void setUseAmbient(bool enabled) {
        flags &= ~0x2;
        flags |= atUint32(enabled) << 1;
      }
      bool useMaterial() const { return (flags & 0x4) != 0; }
      void setUseMaterial(bool enabled) {
        flags &= ~0x4;
        flags |= atUint32(enabled) << 2;
      }
      atUint8 lightmask() const { return atUint8(flags >> 3 & 0xff); }
      void setLightmask(atUint8 mask) {
        flags &= ~0x7f8;
        flags |= atUint32(mask) << 3;
      }
      GX::DiffuseFn diffuseFn() const { return GX::DiffuseFn(flags >> 11 & 0x3); }
      void setDiffuseFn(GX::DiffuseFn fn) {
        flags &= ~0x1800;
        flags |= atUint32(fn) << 11;
      }
      GX::AttnFn attenuationFn() const { return GX::AttnFn(flags >> 13 & 0x3); }
      void setAttenuationFn(GX::AttnFn fn) {
        flags &= ~0x6000;
        flags |= atUint32(fn) << 13;
      }
    };
    Vector<ColorChannel, AT_DNA_COUNT(colorChannelCount)> colorChannels;

    Value<atUint32> tevStageCount = 0;
    struct TEVStage : BigDNA {
      AT_DECL_DNA
      Value<atUint32> ciFlags = 0;
      Value<atUint32> aiFlags = 0;
      Value<atUint32> ccFlags = 0;
      Value<atUint32> acFlags = 0;
      Value<atUint8> pad = 0;
      Value<atUint8> kaInput = 0;
      Value<atUint8> kcInput = 0;
      Value<atUint8> rascInput = 0;

      GX::TevColorArg colorInA() const { return GX::TevColorArg(ciFlags & 0xf); }
      void setColorInA(GX::TevColorArg val) {
        ciFlags &= ~0x1f;
        ciFlags |= atUint32(val);
      }
      GX::TevColorArg colorInB() const { return GX::TevColorArg(ciFlags >> 5 & 0xf); }
      void setColorInB(GX::TevColorArg val) {
        ciFlags &= ~0x3e0;
        ciFlags |= atUint32(val) << 5;
      }
      GX::TevColorArg colorInC() const { return GX::TevColorArg(ciFlags >> 10 & 0xf); }
      void setColorInC(GX::TevColorArg val) {
        ciFlags &= ~0x7c00;
        ciFlags |= atUint32(val) << 10;
      }
      GX::TevColorArg colorInD() const { return GX::TevColorArg(ciFlags >> 15 & 0xf); }
      void setColorInD(GX::TevColorArg val) {
        ciFlags &= ~0xf8000;
        ciFlags |= atUint32(val) << 15;
      }

      GX::TevAlphaArg alphaInA() const { return GX::TevAlphaArg(aiFlags & 0x7); }
      void setAlphaInA(GX::TevAlphaArg val) {
        aiFlags &= ~0x1f;
        aiFlags |= atUint32(val);
      }
      GX::TevAlphaArg alphaInB() const { return GX::TevAlphaArg(aiFlags >> 5 & 0x7); }
      void setAlphaInB(GX::TevAlphaArg val) {
        aiFlags &= ~0x3e0;
        aiFlags |= atUint32(val) << 5;
      }
      GX::TevAlphaArg alphaInC() const { return GX::TevAlphaArg(aiFlags >> 10 & 0x7); }
      void setAlphaInC(GX::TevAlphaArg val) {
        aiFlags &= ~0x7c00;
        aiFlags |= atUint32(val) << 10;
      }
      GX::TevAlphaArg alphaInD() const { return GX::TevAlphaArg(aiFlags >> 15 & 0x7); }
      void setAlphaInD(GX::TevAlphaArg val) {
        aiFlags &= ~0xf8000;
        aiFlags |= atUint32(val) << 15;
      }

      GX::TevOp colorOp() const { return GX::TevOp(ccFlags & 0xf); }
      void setColorOp(GX::TevOp val) {
        ccFlags &= ~0x1;
        ccFlags |= atUint32(val);
      }
      GX::TevBias colorOpBias() const { return GX::TevBias(ccFlags >> 4 & 0x3); }
      void setColorOpBias(GX::TevBias val) {
        ccFlags &= ~0x30;
        ccFlags |= atUint32(val) << 4;
      }
      GX::TevScale colorOpScale() const { return GX::TevScale(ccFlags >> 6 & 0x3); }
      void setColorOpScale(GX::TevScale val) {
        ccFlags &= ~0xc0;
        ccFlags |= atUint32(val) << 6;
      }
      bool colorOpClamp() const { return ccFlags >> 8 & 0x1; }
      void setColorOpClamp(bool val) {
        ccFlags &= ~0x100;
        ccFlags |= atUint32(val) << 8;
      }
      GX::TevRegID colorOpOutReg() const { return GX::TevRegID(ccFlags >> 9 & 0x3); }
      void setColorOpOutReg(GX::TevRegID val) {
        ccFlags &= ~0x600;
        ccFlags |= atUint32(val) << 9;
      }

      GX::TevOp alphaOp() const { return GX::TevOp(acFlags & 0xf); }
      void setAlphaOp(GX::TevOp val) {
        acFlags &= ~0x1;
        acFlags |= atUint32(val);
      }
      GX::TevBias alphaOpBias() const { return GX::TevBias(acFlags >> 4 & 0x3); }
      void setAlphaOpBias(GX::TevBias val) {
        acFlags &= ~0x30;
        acFlags |= atUint32(val) << 4;
      }
      GX::TevScale alphaOpScale() const { return GX::TevScale(acFlags >> 6 & 0x3); }
      void setAlphaOpScale(GX::TevScale val) {
        acFlags &= ~0xc0;
        acFlags |= atUint32(val) << 6;
      }
      bool alphaOpClamp() const { return acFlags >> 8 & 0x1; }
      void setAlphaOpClamp(bool val) {
        acFlags &= ~0x100;
        acFlags |= atUint32(val) << 8;
      }
      GX::TevRegID alphaOpOutReg() const { return GX::TevRegID(acFlags >> 9 & 0x3); }
      void setAlphaOpOutReg(GX::TevRegID val) {
        acFlags &= ~0x600;
        acFlags |= atUint32(val) << 9;
      }

      GX::TevKColorSel kColorIn() const { return GX::TevKColorSel(kcInput); }
      void setKColorIn(GX::TevKColorSel val) { kcInput = val; }
      GX::TevKAlphaSel kAlphaIn() const { return GX::TevKAlphaSel(kaInput); }
      void setKAlphaIn(GX::TevKAlphaSel val) { kaInput = val; }

      GX::ChannelID rasIn() const { return GX::ChannelID(rascInput); }
      void setRASIn(GX::ChannelID id) { rascInput = id; }
    };
    Vector<TEVStage, AT_DNA_COUNT(tevStageCount)> tevStages;
    struct TEVStageTexInfo : BigDNA {
      AT_DECL_DNA
      Value<atUint16> pad = 0;
      Value<atUint8> texSlot = 0xff;
      Value<atUint8> tcgSlot = 0xff;
    };
    Vector<TEVStageTexInfo, AT_DNA_COUNT(tevStageCount)> tevStageTexInfo;

    Value<atUint32> tcgCount = 0;
    struct TexCoordGen : BigDNA {
      AT_DECL_DNA
      Value<atUint32> flags = 0;

      GX::TexGenType type() const { return GX::TexGenType(flags & 0xf); }
      void setType(GX::TexGenType val) {
        flags &= ~0xf;
        flags |= atUint32(val);
      }
      GX::TexGenSrc source() const { return GX::TexGenSrc(flags >> 4 & 0x1f); }
      void setSource(GX::TexGenSrc val) {
        flags &= ~0x1f0;
        flags |= atUint32(val) << 4;
      }
      GX::TexMtx mtx() const { return GX::TexMtx((flags >> 9 & 0x1f) + 30); }
      void setMtx(GX::TexMtx val) {
        flags &= ~0x3e00;
        flags |= (atUint32(val) - 30) << 9;
      }
      bool normalize() const { return flags >> 14 & 0x1; }
      void setNormalize(bool val) {
        flags &= ~0x4000;
        flags |= atUint32(val) << 14;
      }
      GX::PTTexMtx postMtx() const { return GX::PTTexMtx((flags >> 15 & 0x3f) + 64); }
      void setPostMtx(GX::PTTexMtx val) {
        flags &= ~0x1f8000;
        flags |= (atUint32(val) - 64) << 15;
      }
    };
    Vector<TexCoordGen, AT_DNA_COUNT(tcgCount)> tcgs;

    Value<atUint32> uvAnimsSize = 4;
    Value<atUint32> uvAnimsCount = 0;
    struct UVAnimation : BigDNA {
      AT_DECL_EXPLICIT_DNA
      enum class Mode {
        MvInvNoTranslation,
        MvInv,
        Scroll,
        Rotation,
        HStrip,
        VStrip,
        Model,
        CylinderEnvironment,
        Eight
      } mode;
      float vals[9];

      UVAnimation() = default;
      UVAnimation(const std::string& gameFunction, const std::vector<atVec4f>& gameArgs);
    };
    Vector<UVAnimation, AT_DNA_COUNT(uvAnimsCount)> uvAnims;

    static void AddTexture(hecl::blender::PyOutStream& out, GX::TexGenSrc type, int mtxIdx, uint32_t texIdx,
                           bool diffuse);
    static void AddTextureAnim(hecl::blender::PyOutStream& out, MaterialSet::Material::UVAnimation::Mode type,
                               unsigned idx, const float* vals);
    static void AddKcolor(hecl::blender::PyOutStream& out, const GX::Color& col, unsigned idx);
    static void AddDynamicColor(hecl::blender::PyOutStream& out, unsigned idx);
    static void AddDynamicAlpha(hecl::blender::PyOutStream& out, unsigned idx);

    Material() = default;
    Material(const hecl::blender::Material& material,
             std::vector<hecl::ProjectPath>& texPathsOut,
             int colorCount, bool lightmapUVs, bool matrixSkinning);
  };
  Vector<Material, AT_DNA_COUNT(head.materialCount)> materials;

  static void RegisterMaterialProps(hecl::blender::PyOutStream& out);
  static void ConstructMaterial(hecl::blender::PyOutStream& out, const MaterialSet::Material& material,
                                unsigned groupIdx, unsigned matIdx);

  void readToBlender(hecl::blender::PyOutStream& os, const PAKRouter<PAKBridge>& pakRouter,
                     const PAKRouter<PAKBridge>::EntryType& entry, unsigned setIdx) {
    DNACMDL::ReadMaterialSetToBlender_1_2(os, *this, pakRouter, entry, setIdx);
  }

  template <class PAKRouter>
  void nameTextures(PAKRouter& pakRouter, const char* prefix, int setIdx) const {
    int matIdx = 0;
    for (const Material& mat : materials) {
      int stageIdx = 0;
      for (const Material::TEVStage& stage : mat.tevStages) {
        (void)stage;
        const Material::TEVStageTexInfo& texInfo = mat.tevStageTexInfo[stageIdx];
        if (texInfo.texSlot == 0xff) {
          ++stageIdx;
          continue;
        }
        const nod::Node* node;
        typename PAKRouter::EntryType* texEntry = (typename PAKRouter::EntryType*)pakRouter.lookupEntry(
            head.textureIDs[mat.textureIdxs[texInfo.texSlot]], &node);
        if (texEntry->name.size()) {
          if (texEntry->name.size() < 5 || texEntry->name.compare(0, 5, "mult_"))
            texEntry->name = "mult_" + texEntry->name;
          ++stageIdx;
          continue;
        }
        if (setIdx < 0)
          texEntry->name = hecl::Format("%s_%d_%d", prefix, matIdx, stageIdx);
        else
          texEntry->name = hecl::Format("%s_%d_%d_%d", prefix, setIdx, matIdx, stageIdx);

        if (mat.flags.lightmap() && stageIdx == 0) {
          texEntry->name += "light";
          ++stageIdx;
          continue;
        }

        ++stageIdx;
      }
      ++matIdx;
    }
  }

  void ensureTexturesExtracted(PAKRouter<PAKBridge>& pakRouter) const { head.ensureTexturesExtracted(pakRouter); }
};

struct HMDLMaterialSet : BigDNA {
  static constexpr bool OneSection() { return false; }

  AT_DECL_DNA
  Value<atUint32> materialCount = 0;
  Vector<atUint32, AT_DNA_COUNT(materialCount)> materialEndOffs;

  struct Material : BigDNA {
    AT_DECL_DNA
    MaterialSet::Material::Flags flags;

    using BlendMaterial = hecl::blender::Material;

    struct PASS : hecl::TypedRecordBigDNA<BlendMaterial::ChunkType::TexturePass> {
      AT_DECL_EXPLICIT_DNA
      Value<BlendMaterial::PassType> type;
      UniqueID32 texId;
      Value<BlendMaterial::TexCoordSource> source;
      Value<BlendMaterial::UVAnimType> uvAnimType;
      Value<float> uvAnimParms[9] = {};
      Value<bool> alpha;
      PASS() = default;
      explicit PASS(const BlendMaterial::PASS& pass)
      : type(pass.type), texId(pass.tex), source(pass.source), uvAnimType(pass.uvAnimType), alpha(pass.alpha) {
        std::copy(pass.uvAnimParms.begin(), pass.uvAnimParms.end(), std::begin(uvAnimParms));
      }
      bool shouldNormalizeUv() const {
        switch (uvAnimType) {
        case BlendMaterial::UVAnimType::MvInvNoTranslation:
        case BlendMaterial::UVAnimType::MvInv:
        case BlendMaterial::UVAnimType::Model:
        case BlendMaterial::UVAnimType::CylinderEnvironment:
          return true;
        default:
          return false;
        }
      }
      size_t uvAnimParamsCount() const {
        switch (uvAnimType) {
        default:
          return 0;
        case BlendMaterial::UVAnimType::Scroll:
        case BlendMaterial::UVAnimType::HStrip:
        case BlendMaterial::UVAnimType::VStrip:
          return 4;
        case BlendMaterial::UVAnimType::Rotation:
        case BlendMaterial::UVAnimType::CylinderEnvironment:
          return 2;
        case BlendMaterial::UVAnimType::Eight:
          return 9;
        }
      }
    };
    struct CLR : hecl::TypedRecordBigDNA<BlendMaterial::ChunkType::ColorPass> {
      AT_DECL_DNA
      Value<BlendMaterial::PassType> type;
      Value<atVec4f> color;
      CLR() = default;
      explicit CLR(const BlendMaterial::CLR& clr) : type(clr.type), color(clr.color.val) {}
    };
    using Chunk = hecl::TypedVariantBigDNA<PASS, CLR>;

    static unsigned TexMapIdx(BlendMaterial::PassType type) {
      switch (type) {
      case BlendMaterial::PassType::Lightmap:
        return 0;
      case BlendMaterial::PassType::Diffuse:
        return 1;
      case BlendMaterial::PassType::Emissive:
        return 2;
      case BlendMaterial::PassType::Specular:
        return 3;
      case BlendMaterial::PassType::ExtendedSpecular:
        return 4;
      case BlendMaterial::PassType::Reflection:
        return 5;
      case BlendMaterial::PassType::Alpha:
        return 6;
      case BlendMaterial::PassType::IndirectTex:
        return 7;
      default:
        assert(false && "Unknown pass type");
        return 0;
      }
    }

    Value<atUint64> hash;
    Value<BlendMaterial::ShaderType> shaderType;
    Value<atUint32> chunkCount;
    Vector<Chunk, AT_DNA_COUNT(chunkCount)> chunks;
    Value<BlendMaterial::BlendMode> blendMode = BlendMaterial::BlendMode::Opaque;

    std::pair<hecl::Backend::BlendFactor, hecl::Backend::BlendFactor>
    blendFactors() const {
      switch (blendMode) {
      case BlendMaterial::BlendMode::Opaque:
      default:
        return {hecl::Backend::BlendFactor::One, hecl::Backend::BlendFactor::Zero};
      case BlendMaterial::BlendMode::Alpha:
        return {hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::InvSrcAlpha};
      case BlendMaterial::BlendMode::Additive:
        return {hecl::Backend::BlendFactor::SrcAlpha, hecl::Backend::BlendFactor::One};
      }
    }

    Material() = default;
    Material(const hecl::blender::Material& mat);
  };
  Vector<Material, AT_DNA_COUNT(materialCount)> materials;
};

} // namespace DataSpec::DNAMP1

AT_SPECIALIZE_TYPED_VARIANT_BIGDNA(DataSpec::DNAMP1::HMDLMaterialSet::Material::PASS,
                                   DataSpec::DNAMP1::HMDLMaterialSet::Material::CLR)
