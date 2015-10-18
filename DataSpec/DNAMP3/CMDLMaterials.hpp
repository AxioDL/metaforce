#ifndef _DNAMP3_CMDL_MATERIALS_HPP_
#define _DNAMP3_CMDL_MATERIALS_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/GX.hpp"
#include "../DNAMP1/CMDLMaterials.hpp"
#include "DNAMP3.hpp"

namespace Retro
{
namespace DNAMP3
{

struct MaterialSet : BigDNA
{
    static constexpr bool OneSection() {return true;}

    DECL_DNA
    Value<atUint32> materialCount;

    struct Material : BigDNA
    {
        Delete expl;
        using VAFlags = DNAMP1::MaterialSet::Material::VAFlags;
        struct Header : BigDNA
        {
            DECL_DNA
            Value<atUint32> size;
            struct Flags : BigDNA
            {
                DECL_DNA
                Value<atUint32> flags;
                bool alphaBlending() const {return (flags & 0x8) != 0;}
                void setAlphaBlending(bool enabled) {flags &= ~0x8; flags |= atUint32(enabled) << 3;}
                bool punchthroughAlpha() const {return (flags & 0x10) != 0;}
                void setPunchthroughAlpha(bool enabled) {flags &= ~0x10; flags |= atUint32(enabled) << 4;}
                bool additiveBlending() const {return (flags & 0x20) != 0;}
                void setAdditiveBlending(bool enabled) {flags &= ~0x20; flags |= atUint32(enabled) << 5;}
                bool shadowOccluderMesh() const {return (flags & 0x100) != 0;}
                void setShadowOccluderMesh(bool enabled) {flags &= ~0x100; flags |= atUint32(enabled) << 8;}
                bool lightmapUVArray() const {return false;} /* For polymorphic compatibility with MP1/2 */
            } flags;
            Value<atUint32> groupIdx;
            Value<atUint32> unk1;
            VAFlags vaFlags;
            Value<atUint32> unk2;
            Value<atUint32> unk3;
            Value<atUint32> unk4;
        } header;
        const Header::Flags& getFlags() const {return header.flags;}
        const VAFlags& getVAFlags() const {return header.vaFlags;}

        struct ISection : BigDNA
        {
            Delete expl;
            enum Type
            {
                PASS = SBIG('PASS'),
                CLR = SBIG('CLR '),
                INT = SBIG('INT ')
            } m_type;
            ISection(Type type) : m_type(type) {}
            virtual void constructNode(HECL::BlenderConnection::PyOutStream& out,
                                       const PAKRouter<PAKBridge>& pakRouter,
                                       const PAK::Entry& entry,
                                       const Material::ISection* prevSection,
                                       unsigned idx,
                                       unsigned& texMapIdx,
                                       unsigned& texMtxIdx,
                                       unsigned& kColorIdx) const=0;
        };
        struct SectionPASS : ISection
        {
            SectionPASS() : ISection(ISection::PASS) {}
            DECL_DNA
            Value<atUint32> size;
            enum Subtype
            {
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
            struct Flags : BigDNA
            {
                DECL_DNA
                Value<atUint32> flags;
                bool TRANInvert() const {return (flags & 0x10) != 0;}
                void setTRANInvert(bool enabled) {flags &= ~0x10; flags |= atUint32(enabled) << 4;}
            } flags;
            UniqueID64 txtrId;
            Value<atUint32> uvSrc;
            Value<atUint32> uvAnimSize;
            struct UVAnimation : BigDNA
            {
                DECL_DNA
                Value<atUint16> unk1;
                Value<atUint16> unk2;
                DNAMP1::MaterialSet::Material::UVAnimation anim;
            };
            Vector<UVAnimation, DNA_COUNT(uvAnimSize != 0)> uvAnim;

            void constructNode(HECL::BlenderConnection::PyOutStream& out,
                               const PAKRouter<PAKBridge>& pakRouter,
                               const PAK::Entry& entry,
                               const Material::ISection* prevSection,
                               unsigned idx,
                               unsigned& texMapIdx,
                               unsigned& texMtxIdx,
                               unsigned& kColorIdx) const;
        };
        struct SectionCLR : ISection
        {
            SectionCLR() : ISection(ISection::CLR) {}
            DECL_DNA
            enum Subtype
            {
                CLR = SBIG('CLR '),
                DIFB = SBIG('DIFB')
            };
            DNAFourCC subtype;
            GX::Color color;

            void constructNode(HECL::BlenderConnection::PyOutStream& out,
                               const PAKRouter<PAKBridge>& pakRouter,
                               const PAK::Entry& entry,
                               const Material::ISection* prevSection,
                               unsigned idx,
                               unsigned& texMapIdx,
                               unsigned& texMtxIdx,
                               unsigned& kColorIdx) const;
        };
        struct SectionINT : ISection
        {
            SectionINT() : ISection(ISection::INT) {}
            DECL_DNA
            enum Subtype
            {
                OPAC = SBIG('OPAC'),
                BLOD = SBIG('BLOD'),
                BLOI = SBIG('BLOI'),
                BNIF = SBIG('BNIF'),
                XRBR = SBIG('XRBR')
            };
            DNAFourCC subtype;
            Value<atUint32> value;

            void constructNode(HECL::BlenderConnection::PyOutStream& out,
                               const PAKRouter<PAKBridge>& pakRouter,
                               const PAK::Entry& entry,
                               const Material::ISection* prevSection,
                               unsigned idx,
                               unsigned& texMapIdx,
                               unsigned& texMtxIdx,
                               unsigned& kColorIdx) const;
        };
        struct SectionFactory : BigDNA
        {
            Delete expl;
            std::unique_ptr<ISection> section;
            void read(Athena::io::IStreamReader& reader)
            {
                DNAFourCC type;
                type.read(reader);
                switch (type)
                {
                case ISection::PASS:
                    section.reset(new struct SectionPASS);
                    section->read(reader);
                    break;
                case ISection::CLR:
                    section.reset(new struct SectionCLR);
                    section->read(reader);
                    break;
                case ISection::INT:
                    section.reset(new struct SectionINT);
                    section->read(reader);
                    break;
                default:
                    section.reset(nullptr);
                    break;
                }
            }
            void write(Athena::io::IStreamWriter& writer) const
            {
                if (!section)
                    return;
                writer.writeUBytes((atUint8*)&section->m_type, 4);
                section->write(writer);
            }
            size_t binarySize(size_t __isz) const
            {
                return section->binarySize(__isz + 4);
            }
        };
        std::vector<SectionFactory> sections;
        void read(Athena::io::IStreamReader& reader)
        {
            header.read(reader);
            sections.clear();
            do {
                sections.emplace_back();
                sections.back().read(reader);
            } while (sections.back().section);
            sections.pop_back();
        }
        void write(Athena::io::IStreamWriter& writer) const
        {
            header.write(writer);
            for (const SectionFactory& section : sections)
                section.write(writer);
            writer.writeUBytes((atUint8*)"END ", 4);
        }
        size_t binarySize(size_t __isz) const
        {
            __isz = header.binarySize(__isz);
            for (const SectionFactory& section : sections)
                __isz = section.binarySize(__isz);
            return __isz + 4;
        }
    };
    Vector<Material, DNA_COUNT(materialCount)> materials;

    static void RegisterMaterialProps(HECL::BlenderConnection::PyOutStream& out);
    static void ConstructMaterial(HECL::BlenderConnection::PyOutStream& out,
                                  const PAKRouter<PAKBridge>& pakRouter,
                                  const PAK::Entry& entry,
                                  const MaterialSet::Material& material,
                                  unsigned groupIdx, unsigned matIdx);

    void readToBlender(HECL::BlenderConnection::PyOutStream& os,
                       const PAKRouter<PAKBridge>& pakRouter,
                       const PAKRouter<PAKBridge>::EntryType& entry,
                       unsigned setIdx)
    {
        DNACMDL::ReadMaterialSetToBlender_3(os, *this, pakRouter, entry, setIdx);
    }
};

}
}

#endif // _DNAMP3_CMDL_MATERIALS_HPP_
