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
    AT_DECL_EXPLICIT_DNA
    using VAFlags = DNAMP1::MaterialSet::Material::VAFlags;
    struct Header : BigDNA {
      AT_DECL_DNA
      Value<atUint32> size;
      struct Flags : BigDNA {
        AT_DECL_DNA
        Value<atUint32> flags;
        bool alphaBlending() const { return (flags & 0x8) != 0; }
        void setAlphaBlending(bool enabled) {
          flags &= ~0x8;
          flags |= atUint32(enabled) << 3;
        }
        bool alphaTest() const { return (flags & 0x10) != 0; }
        void setPunchthroughAlpha(bool enabled) {
          flags &= ~0x10;
          flags |= atUint32(enabled) << 4;
        }
        bool additiveBlending() const { return (flags & 0x20) != 0; }
        void setAdditiveBlending(bool enabled) {
          flags &= ~0x20;
          flags |= atUint32(enabled) << 5;
        }
        bool shadowOccluderMesh() const { return (flags & 0x100) != 0; }
        void setShadowOccluderMesh(bool enabled) {
          flags &= ~0x100;
          flags |= atUint32(enabled) << 8;
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

    struct ISection : BigDNAV {
      Delete expl;
      enum class Type : atUint32 { PASS = SBIG('PASS'), CLR = SBIG('CLR '), INT = SBIG('INT ') } m_type;
      ISection(Type type) : m_type(type) {}
      virtual void constructNode(hecl::blender::PyOutStream& out, const PAKRouter<PAKBridge>& pakRouter,
                                 const PAK::Entry& entry, const Material::ISection* prevSection, unsigned idx,
                                 unsigned& texMapIdx, unsigned& texMtxIdx, unsigned& kColorIdx) const = 0;
    };
    struct SectionPASS : ISection {
      SectionPASS() : ISection(ISection::Type::PASS) {}
      static SectionPASS* castTo(ISection* sec) {
        return sec->m_type == Type::PASS ? static_cast<SectionPASS*>(sec) : nullptr;
      }
      AT_DECL_DNAV
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
        Value<atUint16> unk1;
        Value<atUint16> unk2;
        DNAMP1::MaterialSet::Material::UVAnimation anim;
      };
      Vector<UVAnimation, AT_DNA_COUNT(uvAnimSize != 0)> uvAnim;

      void constructNode(hecl::blender::PyOutStream& out, const PAKRouter<PAKBridge>& pakRouter,
                         const PAK::Entry& entry, const Material::ISection* prevSection, unsigned idx,
                         unsigned& texMapIdx, unsigned& texMtxIdx, unsigned& kColorIdx) const override;
    };
    struct SectionCLR : ISection {
      SectionCLR() : ISection(ISection::Type::CLR) {}
      static SectionCLR* castTo(ISection* sec) {
        return sec->m_type == Type::CLR ? static_cast<SectionCLR*>(sec) : nullptr;
      }
      AT_DECL_DNAV
      enum class Subtype : atUint32 { CLR = SBIG('CLR '), DIFB = SBIG('DIFB') };
      DNAFourCC subtype;
      GX::Color color;

      void constructNode(hecl::blender::PyOutStream& out, const PAKRouter<PAKBridge>& pakRouter,
                         const PAK::Entry& entry, const Material::ISection* prevSection, unsigned idx,
                         unsigned& texMapIdx, unsigned& texMtxIdx, unsigned& kColorIdx) const override;
    };
    struct SectionINT : ISection {
      SectionINT() : ISection(ISection::Type::INT) {}
      static SectionINT* castTo(ISection* sec) {
        return sec->m_type == Type::INT ? static_cast<SectionINT*>(sec) : nullptr;
      }
      AT_DECL_DNAV
      enum class Subtype : atUint32 {
        OPAC = SBIG('OPAC'),
        BLOD = SBIG('BLOD'),
        BLOI = SBIG('BLOI'),
        BNIF = SBIG('BNIF'),
        XRBR = SBIG('XRBR')
      };
      DNAFourCC subtype;
      Value<atUint32> value;

      void constructNode(hecl::blender::PyOutStream& out, const PAKRouter<PAKBridge>& pakRouter,
                         const PAK::Entry& entry, const Material::ISection* prevSection, unsigned idx,
                         unsigned& texMapIdx, unsigned& texMtxIdx, unsigned& kColorIdx) const override;
    };
    struct SectionFactory : BigDNA {
      AT_DECL_EXPLICIT_DNA
      std::unique_ptr<ISection> section;
    };
    std::vector<SectionFactory> sections;
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
