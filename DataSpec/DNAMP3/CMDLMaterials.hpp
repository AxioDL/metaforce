#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/GX.hpp"
#include "../DNAMP1/CMDLMaterials.hpp"
#include "DNAMP3.hpp"

namespace DataSpec::DNAMP3 {

struct MaterialSet : BigDNA {
  static constexpr bool OneSection() { return true; }

  AT_DECL_DNA
  Value<atUint32> materialCount;

  /* Dummy methods from MP1/2 */
  void addTexture(const UniqueID32&) {}
  void addMaterialEndOff(atUint32) { ++materialCount; }

  struct Material : BigDNA {
    enum class SwapColorComponent { Red, Green, Blue, Alpha };
    enum class UVAnimationUVSource : atUint16 { Position, Normal, UV };
    enum class UVAnimationMatrixConfig : atUint16 { NoMtxNoPost, MtxNoPost, NoMtxPost, MtxPost };

    AT_DECL_EXPLICIT_DNA
    using VAFlags = DNAMP1::MaterialSet::Material::VAFlags;
    struct Header : BigDNA {
      AT_DECL_DNA
      Value<atUint32> size;
      struct Flags : BigDNA {
        AT_DECL_DNA
        Value<atUint32> flags;
        bool enableBloom() const { return (flags & 0x1) != 0; }
        void setEnableBloom(bool enabled) {
          flags &= ~0x1;
          flags |= atUint32(enabled) << 0;
        }
        bool forceLightingStage() const { return (flags & 0x4) != 0; }
        void setForceLightingStage(bool enabled) {
          flags &= ~0x4;
          flags |= atUint32(enabled) << 2;
        }
        bool preIncaTransparency() const { return (flags & 0x8) != 0; }
        void setPreIncaTransparency(bool enabled) {
          flags &= ~0x8;
          flags |= atUint32(enabled) << 3;
        }
        bool alphaTest() const { return (flags & 0x10) != 0; }
        void setPunchthroughAlpha(bool enabled) {
          flags &= ~0x10;
          flags |= atUint32(enabled) << 4;
        }
        bool additiveIncandecence() const { return (flags & 0x20) != 0; }
        void setAdditiveIncandecence(bool enabled) {
          flags &= ~0x20;
          flags |= atUint32(enabled) << 5;
        }
        bool shadowOccluderMesh() const { return (flags & 0x100) != 0; }
        void setShadowOccluderMesh(bool enabled) {
          flags &= ~0x100;
          flags |= atUint32(enabled) << 8;
        }
        bool justWhite() const { return (flags & 0x200) != 0; }
        void setJustWhite(bool enabled) {
          flags &= ~0x200;
          flags |= atUint32(enabled) << 9;
        }
        bool reflectionAlphaTarget() const { return (flags & 0x400) != 0; }
        void setReflectionAlphaTarget(bool enabled) {
          flags &= ~0x400;
          flags |= atUint32(enabled) << 10;
        }
        bool justSolidColor() const { return (flags & 0x800) != 0; }
        void setJustSolidColor(bool enabled) {
          flags &= ~0x800;
          flags |= atUint32(enabled) << 11;
        }
        bool excludeFromScanVisor() const { return (flags & 0x4000) != 0; }
        void setExcludeFromScanVisor(bool enabled) {
          flags &= ~0x4000;
          flags |= atUint32(enabled) << 14;
        }
        bool xrayOpaque() const { return (flags & 0x8000) != 0; }
        void setXRayOpaque(bool enabled) {
          flags &= ~0x8000;
          flags |= atUint32(enabled) << 15;
        }
        bool xrayAlphaTarget() const { return (flags & 0x10000) != 0; }
        void setXRayAlphaTarget(bool enabled) {
          flags &= ~0x10000;
          flags |= atUint32(enabled) << 16;
        }
        bool lightmapUVArray() const { return false; } /* For polymorphic compatibility with MP1/2 */
      } flags;
      Value<atUint32> uniqueIdx;
      Value<atUint32> unk1;
      VAFlags vaFlags;
      Value<atUint32> unk2;
      Value<atUint32> unk3;
      Value<atUint32> unk4;
    } header;
    const Header::Flags& getFlags() const { return header.flags; }
    const VAFlags& getVAFlags() const { return header.vaFlags; }

    enum class ChunkType : atUint32 {
      PASS = 'PASS', CLR = 'CLR ', INT = 'INT ', END = 'END '
    };

    struct PASS : hecl::TypedRecordBigDNA<ChunkType::PASS> {
      AT_DECL_DNA
      Value<atUint32> size;
      enum class Subtype : atUint32 {
        DIFF = SBIG('DIFF'),
        RIML = SBIG('RIML'),
        BLOL = SBIG('BLOL'),
        BLOD = SBIG('BLOD'),
        CLR = SBIG('CLR '),
        TRAN = SBIG('TRAN'),
        INCA = SBIG('INCA'),
        RFLV = SBIG('RFLV'),
        RFLD = SBIG('RFLD'),
        LRLD = SBIG('LRLD'),
        LURD = SBIG('LURD'),
        BLOI = SBIG('BLOI'),
        XRAY = SBIG('XRAY'),
        TOON = SBIG('TOON')
      };
      DNAFourCC subtype;
      struct Flags : BigDNA {
        AT_DECL_DNA
        Value<atUint32> flags;
        SwapColorComponent swapColorComponent() const { return SwapColorComponent(flags & 0x3); }
        void setSwapColorComponent(SwapColorComponent comp) {
          flags &= ~0x3;
          flags |= atUint32(comp) << 0;
        }
        bool alphaContribution() const { return (flags & 0x4) != 0; }
        void setAlphaContribution(bool enabled) {
          flags &= ~0x4;
          flags |= atUint32(enabled) << 2;
        }
        bool INCAColorMod() const { return (flags & 0x8) != 0; }
        void setINCAColorMod(bool enabled) {
          flags &= ~0x8;
          flags |= atUint32(enabled) << 3;
        }
        bool TRANInvert() const { return (flags & 0x10) != 0; }
        void setTRANInvert(bool enabled) {
          flags &= ~0x10;
          flags |= atUint32(enabled) << 4;
        }
      } flags;
      UniqueID64 txtrId;
      Value<atUint32> uvSrc;
      Value<atUint32> uvAnimSize;
      struct UVAnimation : BigDNA {
        AT_DECL_DNA
        Value<UVAnimationUVSource> uvSource;
        Value<UVAnimationMatrixConfig> mtxConfig;
        DNAMP1::MaterialSet::Material::UVAnimation anim;
      };
      Vector<UVAnimation, AT_DNA_COUNT(uvAnimSize != 0)> uvAnim;
    };
    struct CLR : hecl::TypedRecordBigDNA<ChunkType::CLR> {
      AT_DECL_DNA
      enum class Subtype : atUint32 { CLR = SBIG('CLR '), DIFB = SBIG('DIFB') };
      DNAFourCC subtype;
      GX::Color color;
      CLR() = default;
    };
    struct INT : hecl::TypedRecordBigDNA<ChunkType::INT> {
      AT_DECL_DNA
      enum class Subtype : atUint32 {
        OPAC = SBIG('OPAC'),
        BLOD = SBIG('BLOD'),
        BLOI = SBIG('BLOI'),
        BNIF = SBIG('BNIF'),
        XRBR = SBIG('XRBR')
      };
      DNAFourCC subtype;
      Value<atUint32> value;
    };
    struct END : hecl::TypedRecordBigDNA<ChunkType::END> {
      AT_DECL_DNA
    };
    using Chunk = hecl::TypedVariantBigDNA<PASS, CLR, INT, END>;
    std::vector<Chunk> chunks;
  };
  Vector<Material, AT_DNA_COUNT(materialCount)> materials;

  static void RegisterMaterialProps(hecl::blender::PyOutStream& out);
  static void ConstructMaterial(hecl::blender::PyOutStream& out, const PAKRouter<PAKBridge>& pakRouter,
                                const PAK::Entry& entry, const MaterialSet::Material& material, unsigned groupIdx,
                                unsigned matIdx);

  void readToBlender(hecl::blender::PyOutStream& os, const PAKRouter<PAKBridge>& pakRouter,
                     const PAKRouter<PAKBridge>::EntryType& entry, unsigned setIdx) {
    DNACMDL::ReadMaterialSetToBlender_3(os, *this, pakRouter, entry, setIdx);
  }

  void ensureTexturesExtracted(PAKRouter<PAKBridge>& pakRouter) const {}
};

} // namespace DataSpec::DNAMP3
