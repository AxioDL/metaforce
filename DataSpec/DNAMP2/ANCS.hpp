#ifndef _DNAMP2_ANCS_HPP_
#define _DNAMP2_ANCS_HPP_

#include <map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/ANCS.hpp"
#include "CMDLMaterials.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"
#include "ANIM.hpp"
#include "../DNAMP1/ANCS.hpp"

namespace DataSpec::DNAMP2
{

struct ANCS : BigDNA
{
    using CINFType = CINF;
    using CSKRType = CSKR;
    using ANIMType = ANIM;

    AT_DECL_DNA_YAML
    Value<atUint16> version;

    struct CharacterSet : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint16> version;
        Value<atUint32> characterCount;
        struct CharacterInfo : BigDNA
        {
            AT_DECL_EXPLICIT_DNA_YAML
            using MP1CharacterInfo = DNAMP1::ANCS::CharacterSet::CharacterInfo;

            atUint32 idx;
            std::string name;
            UniqueID32 cmdl;
            UniqueID32 cskr;
            UniqueID32 cinf;

            struct Animation : BigDNA
            {
                AT_DECL_DNA_YAML
                Value<atUint32> animIdx;
                String<-1> strA;
            };
            std::vector<Animation> animations;

            MP1CharacterInfo::PASDatabase pasDatabase;
            struct ParticleResData
            {
                std::vector<UniqueID32> part;
                std::vector<UniqueID32> swhc;
                std::vector<UniqueID32> unk;
                std::vector<UniqueID32> elsc;
                std::vector<UniqueID32> spsc;
                std::vector<UniqueID32> unk2;
            } partResData;

            atUint32 unk1 = 0;

            std::vector<MP1CharacterInfo::ActionAABB> animAABBs;

            struct Effect : BigDNA
            {
                AT_DECL_DNA_YAML
                String<-1> name;
                Value<atUint32> compCount;
                struct EffectComponent : BigDNA
                {
                    AT_DECL_DNA_YAML
                    String<-1> name;
                    DNAFourCC type;
                    UniqueID32 id;
                    Value<atUint32> unkMP2;
                    Value<float> unk1;
                    Value<atUint32> unk2;
                    Value<atUint32> unk3;
                };
                Vector<EffectComponent, DNA_COUNT(compCount)> comps;
            };
            std::vector<Effect> effects;

            UniqueID32 cmdlOverlay;
            UniqueID32 cskrOverlay;

            std::vector<atUint32> animIdxs;

            atUint32 unk4;
            atUint8 unk5;

            struct Extents : BigDNA
            {
                AT_DECL_DNA_YAML
                Value<atUint32> animIdx;
                Value<atVec3f> aabb[2];
            };
            std::vector<Extents> extents;
        };
        Vector<CharacterInfo, DNA_COUNT(characterCount)> characters;
    } characterSet;

    struct AnimationSet : BigDNA
    {
        AT_DECL_DNA_YAML
        Delete expl;

        using MP1AnimationSet = DNAMP1::ANCS::AnimationSet;

        std::vector<MP1AnimationSet::Animation> animations;

        std::vector<MP1AnimationSet::Transition> transitions;
        MP1AnimationSet::MetaTransFactory defaultTransition;

        std::vector<MP1AnimationSet::AdditiveAnimationInfo> additiveAnims;

        float floatA = 0.0;
        float floatB = 0.0;

        std::vector<MP1AnimationSet::HalfTransition> halfTransitions;

        struct EVNT : BigDNA
        {
            AT_DECL_EXPLICIT_DNA_YAML
            atUint32 version;

            struct EventBase : BigDNA
            {
                AT_DECL_DNA_YAML
                Value<atUint16> unk0;
                String<-1> name;
                Value<atUint16> type;
                Value<float> startTime;
                Value<atUint32> unk1;
                Value<atUint32> idx;
                Value<atUint8> unk2;
                Value<float> unk3;
                Value<atUint32> unk4;
                Value<atUint32> unk5;
            };

            struct LoopEvent : EventBase
            {
                AT_DECL_DNA_YAML
                Value<atUint8> flag;
            };
            Value<atUint32> loopEventCount;
            Vector<LoopEvent, DNA_COUNT(loopEventCount)> loopEvents;

            struct UEVTEvent : EventBase
            {
                AT_DECL_DNA_YAML
                Value<atUint32> uevtType;
                String<-1> boneName;
            };
            Value<atUint32> uevtEventCount;
            Vector<UEVTEvent, DNA_COUNT(uevtEventCount)> uevtEvents;

            struct EffectEvent : EventBase
            {
                AT_DECL_DNA_YAML
                Value<atUint32> frameCount;
                DNAFourCC effectType;
                UniqueID32 effectId;
                Value<atUint32> boneId;
                Value<float> scale;
                Value<atUint32> parentMode;
            };
            Value<atUint32> effectEventCount;
            Vector<EffectEvent, DNA_COUNT(effectEventCount)> effectEvents;

            struct SFXEvent : EventBase
            {
                AT_DECL_DNA_YAML
                Value<atUint32> soundId;
                Value<float> smallNum;
                Value<float> bigNum;
                Value<atUint32> sfxUnk1;
                Value<atUint16> sfxUnk2;
                Value<atUint16> sfxUnk3;
                Value<float> sfxUnk4;
            };
            Value<atUint32> sfxEventCount;
            Vector<SFXEvent, DNA_COUNT(sfxEventCount)> sfxEvents;
        };
        std::vector<EVNT> evnts;
    } animationSet;

    void getCharacterResInfo(std::vector<DNAANCS::CharacterResInfo<UniqueID32>>& out) const
    {
        out.clear();
        out.reserve(characterSet.characters.size());
        for (const CharacterSet::CharacterInfo& ci : characterSet.characters)
        {
            out.emplace_back();
            DNAANCS::CharacterResInfo<UniqueID32>& chOut = out.back();
            chOut.name = ci.name;
            chOut.cmdl = ci.cmdl;
            chOut.cskr = ci.cskr;
            chOut.cinf = ci.cinf;

            if (ci.cmdlOverlay)
                chOut.overlays.emplace_back(FOURCC('OVER'), std::make_pair(ci.cmdlOverlay, ci.cskrOverlay));
        }
    }

    void getAnimationResInfo(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out) const
    {
        out.clear();
        for (const DNAMP1::ANCS::AnimationSet::Animation& ai : animationSet.animations)
            ai.metaAnim.m_anim->gatherPrimitives(out);
        for (const DNAMP1::ANCS::AnimationSet::Transition& ti : animationSet.transitions)
            if (ti.metaTrans.m_trans)
                ti.metaTrans.m_trans->gatherPrimitives(out);
        if (animationSet.defaultTransition.m_trans)
            animationSet.defaultTransition.m_trans->gatherPrimitives(out);
    }

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const DNAMP1::PAK::Entry& entry,
                        bool force,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        hecl::ProjectPath yamlPath = outPath.getWithExtension(_S(".yaml"), true);
        hecl::ProjectPath::Type yamlType = yamlPath.getPathType();
        hecl::ProjectPath blendPath = outPath.getWithExtension(_S(".blend"), true);
        hecl::ProjectPath::Type blendType = blendPath.getPathType();

        if (force ||
            yamlType == hecl::ProjectPath::Type::None ||
            blendType == hecl::ProjectPath::Type::None)
        {
            ANCS ancs;
            ancs.read(rs);

            if (force || yamlType == hecl::ProjectPath::Type::None)
            {
                athena::io::FileWriter writer(yamlPath.getAbsolutePath());
                athena::io::ToYAMLStream(ancs, writer);
            }

            if (force || blendType == hecl::ProjectPath::Type::None)
            {
                hecl::blender::Connection& conn = btok.getBlenderConnection();
                DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, ANCS, MaterialSet, DNACMDL::SurfaceHeader_2, 4>
                        (conn, ancs, blendPath, pakRouter, entry, dataSpec, fileChanged, force);
            }
        }

        return true;
    }
};

}

#endif // _DNAMP2_ANCS_HPP_
