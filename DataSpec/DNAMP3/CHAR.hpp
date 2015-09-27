#ifndef _DNAMP3_CHAR_HPP_
#define _DNAMP3_CHAR_HPP_

#include <map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/ANCS.hpp"
#include "CMDLMaterials.hpp"
#include "BlenderConnection.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"
#include "ANIM.hpp"
#include "../DNAMP2/ANCS.hpp"

namespace Retro
{
namespace DNAMP3
{

struct CHAR : BigYAML
{
    using CINFType = CINF;
    using CSKRType = CSKR;
    using ANIMType = ANIM;

    DECL_YAML
    Value<atUint16> version;

    struct CharacterInfo : BigYAML
    {
        DECL_YAML
        String<-1> name;
        UniqueID64 cmdl;
        UniqueID64 cskr;
        Value<atUint32> overlayCount;
        struct Overlay : BigYAML
        {
            DECL_YAML
            DNAFourCC type;
            UniqueID64 cmdl;
            UniqueID64 cskr;
        };
        Vector<Overlay, DNA_COUNT(overlayCount)> overlays;
        UniqueID64 cinf;
        UniqueID64 sand;

        using MP1CharacterInfo = DNAMP1::ANCS::CharacterSet::CharacterInfo;
        MP1CharacterInfo::PASDatabase pasDatabase;

        struct ParticleResData : BigYAML
        {
            DECL_YAML
            Value<atUint32> partCount;
            Vector<UniqueID64, DNA_COUNT(partCount)> part;
            Value<atUint32> swhcCount;
            Vector<UniqueID64, DNA_COUNT(swhcCount)> swhc;
            Value<atUint32> unkCount;
            Vector<UniqueID64, DNA_COUNT(unkCount)> unk;
            Value<atUint32> elscCount;
            Vector<UniqueID64, DNA_COUNT(elscCount)> elsc;
            Value<atUint32> spscCount;
            Vector<UniqueID64, DNA_COUNT(spscCount)> spsc;
            Value<atUint32> unk2Count;
            Vector<UniqueID64, DNA_COUNT(unk2Count)> unk2;
        } partResData;

    } characterInfo;

    struct AnimationInfo : BigYAML
    {
        DECL_YAML

        struct EVNT : BigYAML
        {
            DECL_YAML
            Value<atUint32> eventIdx;
            String<-1> eventName;

            struct EventBase : BigYAML
            {
                DECL_YAML
                String<-1> name1;
                Value<atUint16> unk0;
                String<-1> name2;
                Value<atUint16> type;
                Value<atUint32> unk1;
                Value<atUint32> unk2;
                Value<atUint32> unk3;
                Value<atUint32> unk4;
                Value<atUint8> unk5;
                Value<float> unk6;
                Value<atUint32> unk7;
                Value<atUint32> unk8;
                Value<atUint32> unk9;
                Value<atUint32> unk10;
                Value<atUint32> unk11;
                Value<atUint32> unk12;
                Value<atUint32> unk13;
            };

            struct EffectEvent : EventBase
            {
                DECL_YAML
                DNAFourCC effectType;
                UniqueID64 effectId;
                Value<float> scale;
                Value<atUint32> parentMode;
            };
            Value<atUint32> effectCount;
            Vector<EffectEvent, DNA_COUNT(effectCount)> effectEvents;

            struct SFXEvent : EventBase
            {
                DECL_YAML
                Delete expl;

                UniqueID64 caudId;
                Value<atUint32> unk1;
                Value<atUint32> unk2;
                Value<atUint32> unk3;
                std::vector<float> unk3Vals;
                Value<atUint32> extraType;
                Value<float> extraFloat;
            };
            Value<atUint32> sfxCount;
            Vector<SFXEvent, DNA_COUNT(sfxCount)> sfxEvents;
        };
        Value<atUint32> evntCount;
        Vector<EVNT, DNA_COUNT(evntCount)> evnts;

        struct IMetaAnim : BigYAML
        {
            Delete expl;
            enum Type
            {
                MAPrimitive = 0,
                MABlend = 1,
                MAPhaseBlend = 2,
                MARandom = 3,
                MASequence = 4
            } m_type;
            const char* m_typeStr;
            IMetaAnim(Type type, const char* typeStr)
            : m_type(type), m_typeStr(typeStr) {}
            virtual void gatherPrimitives(std::map<atUint32, std::pair<std::string, UniqueID64>>& out)=0;
        };
        struct MetaAnimFactory : BigYAML
        {
            DECL_YAML
            Delete expl;
            std::unique_ptr<IMetaAnim> m_anim;
        };
        struct MetaAnimPrimitive : IMetaAnim
        {
            MetaAnimPrimitive() : IMetaAnim(MAPrimitive, "Primitive") {}
            DECL_YAML
            UniqueID64 animId;
            Value<atUint32> animIdx;
            String<-1> animName;
            Value<float> unk1;
            Value<atUint32> unk2;

            void gatherPrimitives(std::map<atUint32, std::pair<std::string, UniqueID64>>& out)
            {
                out[animIdx] = std::make_pair(animName, animId);
            }
        };
        struct MetaAnimBlend : IMetaAnim
        {
            MetaAnimBlend() : IMetaAnim(MABlend, "Blend") {}
            DECL_YAML
            MetaAnimFactory animA;
            MetaAnimFactory animB;
            Value<float> unkFloat;
            Value<atUint8> unk;

            void gatherPrimitives(std::map<atUint32, std::pair<std::string, UniqueID64>>& out)
            {
                animA.m_anim->gatherPrimitives(out);
                animB.m_anim->gatherPrimitives(out);
            }
        };
        struct MetaAnimPhaseBlend : IMetaAnim
        {
            MetaAnimPhaseBlend() : IMetaAnim(MAPhaseBlend, "PhaseBlend") {}
            DECL_YAML
            MetaAnimFactory animA;
            MetaAnimFactory animB;
            Value<float> unkFloat;
            Value<atUint8> unk;

            void gatherPrimitives(std::map<atUint32, std::pair<std::string, UniqueID64>>& out)
            {
                animA.m_anim->gatherPrimitives(out);
                animB.m_anim->gatherPrimitives(out);
            }
        };
        struct MetaAnimRandom : IMetaAnim
        {
            MetaAnimRandom() : IMetaAnim(MARandom, "Random") {}
            DECL_YAML
            Value<atUint32> animCount;
            struct Child : BigYAML
            {
                DECL_YAML
                MetaAnimFactory anim;
                Value<atUint32> probability;
            };
            Vector<Child, DNA_COUNT(animCount)> children;

            void gatherPrimitives(std::map<atUint32, std::pair<std::string, UniqueID64>>& out)
            {
                for (const auto& child : children)
                    child.anim.m_anim->gatherPrimitives(out);
            }
        };
        struct MetaAnimSequence : IMetaAnim
        {
            MetaAnimSequence() : IMetaAnim(MASequence, "Sequence") {}
            DECL_YAML
            Value<atUint32> animCount;
            Vector<MetaAnimFactory, DNA_COUNT(animCount)> children;

            void gatherPrimitives(std::map<atUint32, std::pair<std::string, UniqueID64>>& out)
            {
                for (const auto& child : children)
                    child.m_anim->gatherPrimitives(out);
            }
        };

        struct Animation : BigYAML
        {
            DECL_YAML
            String<-1> name;
            MetaAnimFactory metaAnim;
        };
        Value<atUint32> animationCount;
        Vector<Animation, DNA_COUNT(animationCount)> animations;

        struct ActionAABB : BigYAML
        {
            DECL_YAML
            UniqueID64 animId;
            Value<atVec3f> aabb[2];
        };
        Value<atUint32> animAABBCount;
        Vector<ActionAABB, DNA_COUNT(animAABBCount)> animAABBs;

        Value<atUint8> unkByte;

        Value<atUint32> additiveMapCount;
        Vector<bool, DNA_COUNT(additiveMapCount)> additiveMap;

    } animationInfo;

    struct HitboxSet : BigYAML
    {
        DECL_YAML
        String<-1> name;
        Value<atUint32> hitboxCount;
        struct Hitbox : BigYAML
        {
            DECL_YAML
            Value<atUint32> unk1;
            Value<atUint32> unk2;
            Value<atUint32> unk3;
            Value<atUint32> unk4;
            Value<atUint32> unk5;
            Value<float> unk6;
            Value<float> unk7;
            Value<float> unk8;
            Value<float> unk9;
            Value<float> unk10;
            Value<float> unk11;
            Value<float> unk12;
            Value<float> unk13;
            String<-1> boneName;
            Value<float> unk14;
        };
        Vector<Hitbox, DNA_COUNT(hitboxCount)> hitboxes;
    };
    Value<atUint32> hitboxSetCount;
    Vector<HitboxSet, DNA_COUNT(hitboxSetCount)> hitboxSets;

    void getCharacterResInfo(std::vector<DNAANCS::CharacterResInfo<UniqueID64>>& out) const
    {
        out.clear();
        out.reserve(1);
        out.emplace_back();
        DNAANCS::CharacterResInfo<UniqueID64>& chOut = out.back();
        chOut.name = characterInfo.name;
        chOut.cmdl = characterInfo.cmdl;
        chOut.cskr = characterInfo.cskr;
        chOut.cinf = characterInfo.cinf;

        for (const CharacterInfo::Overlay& overlay : characterInfo.overlays)
            chOut.overlays.emplace_back(overlay.type, std::make_pair(overlay.cmdl, overlay.cskr));
    }

    void getAnimationResInfo(std::map<atUint32, std::pair<std::string, UniqueID64>>& out) const
    {
        out.clear();
        for (const AnimationInfo::Animation& ai : animationInfo.animations)
            ai.metaAnim.m_anim->gatherPrimitives(out);
    }

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        std::function<void(const HECL::SystemChar*)> fileChanged)
    {
        HECL::ProjectPath yamlPath = outPath.getWithExtension(_S(".yaml"));
        HECL::ProjectPath::PathType yamlType = yamlPath.getPathType();
        HECL::ProjectPath blendPath = outPath.getWithExtension(_S(".blend"));
        HECL::ProjectPath::PathType blendType = blendPath.getPathType();

        if (force ||
            yamlType == HECL::ProjectPath::PT_NONE ||
            blendType == HECL::ProjectPath::PT_NONE)
        {
            CHAR aChar;
            aChar.read(rs);

            if (force || yamlType == HECL::ProjectPath::PT_NONE)
            {
                FILE* fp = HECL::Fopen(yamlPath.getAbsolutePath().c_str(), _S("wb"));
                aChar.toYAMLFile(fp);
                fclose(fp);
            }

            if (force || blendType == HECL::ProjectPath::PT_NONE)
            {
                HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
                DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, CHAR, MaterialSet, DNACMDL::SurfaceHeader_3, 4>
                        (conn, aChar, blendPath, pakRouter, entry, dataSpec, fileChanged, force);
            }
        }

        return true;
    }
};

}
}

#endif // _DNAMP3_CHAR_HPP_
