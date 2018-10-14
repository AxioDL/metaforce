#pragma once

#include <map>
#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/ANCS.hpp"
#include "CMDLMaterials.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"
#include "ANIM.hpp"
#include "../DNAMP2/ANCS.hpp"

namespace DataSpec::DNAMP3
{

struct CHAR : BigDNA
{
    using CINFType = CINF;
    using CSKRType = CSKR;
    using ANIMType = ANIM;

    AT_DECL_DNA_YAML
    Value<atUint16> version;

    struct CharacterInfo : BigDNA
    {
        AT_DECL_DNA_YAML
        String<-1> name;
        UniqueID64 cmdl;
        UniqueID64 cskr;
        Value<atUint32> overlayCount;
        struct Overlay : BigDNA
        {
            AT_DECL_DNA_YAML
            DNAFourCC type;
            UniqueID64 cmdl;
            UniqueID64 cskr;
        };
        Vector<Overlay, AT_DNA_COUNT(overlayCount)> overlays;
        UniqueID64 cinf;
        UniqueID64 sand;

        using MP1CharacterInfo = DNAMP1::ANCS::CharacterSet::CharacterInfo;
        MP1CharacterInfo::PASDatabase pasDatabase;

        struct ParticleResData : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<atUint32> partCount;
            Vector<UniqueID64, AT_DNA_COUNT(partCount)> part;
            Value<atUint32> swhcCount;
            Vector<UniqueID64, AT_DNA_COUNT(swhcCount)> swhc;
            Value<atUint32> unkCount;
            Vector<UniqueID64, AT_DNA_COUNT(unkCount)> unk;
            Value<atUint32> elscCount;
            Vector<UniqueID64, AT_DNA_COUNT(elscCount)> elsc;
            Value<atUint32> spscCount;
            Vector<UniqueID64, AT_DNA_COUNT(spscCount)> spsc;
            Value<atUint32> unk2Count;
            Vector<UniqueID64, AT_DNA_COUNT(unk2Count)> unk2;
        } partResData;

    } characterInfo;

    struct AnimationInfo : BigDNA
    {
        AT_DECL_DNA_YAML

        struct EVNT : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<atUint32> eventIdx;
            String<-1> eventName;

            struct EventBase : BigDNA
            {
                AT_DECL_DNA_YAML
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
                AT_DECL_DNA_YAML
                DNAFourCC effectType;
                UniqueID64 effectId;
                Value<float> scale;
                Value<atUint32> parentMode;
            };
            Value<atUint32> effectCount;
            Vector<EffectEvent, AT_DNA_COUNT(effectCount)> effectEvents;

            struct SFXEvent : EventBase
            {
                AT_DECL_DNA_YAML
                Delete expl;

                UniqueID64 caudId;
                Value<atUint32> unk1_;
                Value<atUint32> unk2_;
                Value<atUint32> unk3_;
                std::vector<float> unk3Vals;
                Value<atUint32> extraType;
                Value<float> extraFloat;
            };
            Value<atUint32> sfxCount;
            Vector<SFXEvent, AT_DNA_COUNT(sfxCount)> sfxEvents;
        };
        Value<atUint32> evntCount;
        Vector<EVNT, AT_DNA_COUNT(evntCount)> evnts;

        struct IMetaAnim : BigDNAVYaml
        {
            Delete expl;
            enum class Type
            {
                Primitive = 0,
                Blend = 1,
                PhaseBlend = 2,
                Random = 3,
                Sequence = 4
            } m_type;
            const char* m_typeStr;
            IMetaAnim(Type type, const char* typeStr)
            : m_type(type), m_typeStr(typeStr) {}
            virtual void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID64>>& out)=0;
        };
        struct MetaAnimFactory : BigDNA
        {
            AT_DECL_DNA_YAML
            Delete expl;
            std::unique_ptr<IMetaAnim> m_anim;
        };
        struct MetaAnimPrimitive : IMetaAnim
        {
            MetaAnimPrimitive() : IMetaAnim(Type::Primitive, "Primitive") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            UniqueID64 animId;
            Value<atUint32> animIdx;
            String<-1> animName;
            Value<float> unk1;
            Value<atUint32> unk2;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID64>>& out)
            {
                out[animIdx] = {animName, animId, UniqueID64(), false};
            }
        };
        struct MetaAnimBlend : IMetaAnim
        {
            MetaAnimBlend() : IMetaAnim(Type::Blend, "Blend") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            MetaAnimFactory animA;
            MetaAnimFactory animB;
            Value<float> unkFloat;
            Value<atUint8> unk;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID64>>& out)
            {
                animA.m_anim->gatherPrimitives(out);
                animB.m_anim->gatherPrimitives(out);
            }
        };
        struct MetaAnimPhaseBlend : IMetaAnim
        {
            MetaAnimPhaseBlend() : IMetaAnim(Type::PhaseBlend, "PhaseBlend") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            MetaAnimFactory animA;
            MetaAnimFactory animB;
            Value<float> unkFloat;
            Value<atUint8> unk;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID64>>& out)
            {
                animA.m_anim->gatherPrimitives(out);
                animB.m_anim->gatherPrimitives(out);
            }
        };
        struct MetaAnimRandom : IMetaAnim
        {
            MetaAnimRandom() : IMetaAnim(Type::Random, "Random") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            Value<atUint32> animCount;
            struct Child : BigDNA
            {
                AT_DECL_DNA_YAML
                MetaAnimFactory anim;
                Value<atUint32> probability;
            };
            Vector<Child, AT_DNA_COUNT(animCount)> children;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID64>>& out)
            {
                for (const auto& child : children)
                    child.anim.m_anim->gatherPrimitives(out);
            }
        };
        struct MetaAnimSequence : IMetaAnim
        {
            MetaAnimSequence() : IMetaAnim(Type::Sequence, "Sequence") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            Value<atUint32> animCount;
            Vector<MetaAnimFactory, AT_DNA_COUNT(animCount)> children;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID64>>& out)
            {
                for (const auto& child : children)
                    child.m_anim->gatherPrimitives(out);
            }
        };

        struct Animation : BigDNA
        {
            AT_DECL_DNA_YAML
            String<-1> name;
            MetaAnimFactory metaAnim;
        };
        Value<atUint32> animationCount;
        Vector<Animation, AT_DNA_COUNT(animationCount)> animations;

        struct ActionAABB : BigDNA
        {
            AT_DECL_DNA_YAML
            UniqueID64 animId;
            Value<atVec3f> aabb[2];
        };
        Value<atUint32> animAABBCount;
        Vector<ActionAABB, AT_DNA_COUNT(animAABBCount)> animAABBs;

        Value<atUint8> unkByte;

        Value<atUint32> additiveMapCount;
        Vector<bool, AT_DNA_COUNT(additiveMapCount)> additiveMap;

    } animationInfo;

    struct HitboxSet : BigDNA
    {
        AT_DECL_DNA_YAML
        String<-1> name;
        Value<atUint32> hitboxCount;
        struct Hitbox : BigDNA
        {
            AT_DECL_DNA_YAML
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
        Vector<Hitbox, AT_DNA_COUNT(hitboxCount)> hitboxes;
    };
    Value<atUint32> hitboxSetCount;
    Vector<HitboxSet, AT_DNA_COUNT(hitboxSetCount)> hitboxSets;

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
            chOut.overlays.emplace_back(overlay.type.toString(), std::make_pair(overlay.cmdl, overlay.cskr));
    }

    void getAnimationResInfo(PAKRouter<PAKBridge>* pakRouter,
                             std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID64>>& out) const
    {
        out.clear();
        for (const AnimationInfo::Animation& ai : animationInfo.animations)
            ai.metaAnim.m_anim->gatherPrimitives(out);
        for (auto& animRes : out)
            animRes.second.additive = animationInfo.additiveMap.at(animRes.first);
    }

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        hecl::ProjectPath yamlPath = outPath.getWithExtension(_SYS_STR(".yaml"), true);
        hecl::ProjectPath::Type yamlType = yamlPath.getPathType();
        hecl::ProjectPath blendPath = outPath.getWithExtension(_SYS_STR(".blend"), true);
        hecl::ProjectPath::Type blendType = blendPath.getPathType();

        if (force ||
            yamlType == hecl::ProjectPath::Type::None ||
            blendType == hecl::ProjectPath::Type::None)
        {
            CHAR aChar;
            aChar.read(rs);

            if (force || yamlType == hecl::ProjectPath::Type::None)
            {
                athena::io::FileWriter writer(yamlPath.getAbsolutePath());
                athena::io::ToYAMLStream(aChar, writer);
            }

            if (force || blendType == hecl::ProjectPath::Type::None)
            {
                hecl::blender::Connection& conn = btok.getBlenderConnection();
                DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, CHAR, MaterialSet, DNACMDL::SurfaceHeader_3, 4>
                        (conn, aChar, blendPath, pakRouter, entry, dataSpec, fileChanged, force);
            }
        }

        return true;
    }
};

}

