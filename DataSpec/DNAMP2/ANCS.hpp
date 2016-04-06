#ifndef _DNAMP2_ANCS_HPP_
#define _DNAMP2_ANCS_HPP_

#include <map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/ANCS.hpp"
#include "CMDLMaterials.hpp"
#include "BlenderConnection.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"
#include "ANIM.hpp"
#include "../DNAMP1/ANCS.hpp"

namespace DataSpec
{
namespace DNAMP2
{

struct ANCS : BigYAML
{
    using CINFType = CINF;
    using CSKRType = CSKR;
    using ANIMType = ANIM;

    ANCS(const UniqueID32& ancsId) : animationSet(ancsId) {}

    DECL_YAML
    Value<atUint16> version;

    struct CharacterSet : BigYAML
    {
        DECL_YAML
        Value<atUint16> version;
        Value<atUint32> characterCount;
        struct CharacterInfo : BigYAML
        {
            DECL_YAML
            Delete expl;
            using MP1CharacterInfo = DNAMP1::ANCS::CharacterSet::CharacterInfo;

            atUint32 idx;
            std::string name;
            UniqueID32 cmdl;
            AuxiliaryID32 cskr = _S("skin");
            AuxiliaryID32 cinf = {_S("layout"), _S("skin")};

            struct Animation : BigYAML
            {
                DECL_YAML
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

            struct Effect : BigYAML
            {
                DECL_YAML
                String<-1> name;
                Value<atUint32> compCount;
                struct EffectComponent : BigYAML
                {
                    DECL_YAML
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
            AuxiliaryID32 cskrOverlay = _S("skin");

            std::vector<atUint32> animIdxs;

            atUint32 unk4;
            atUint8 unk5;

            struct Extents : BigYAML
            {
                DECL_YAML
                Value<atUint32> animIdx;
                Value<atVec3f> aabb[2];
            };
            std::vector<Extents> extents;
        };
        Vector<CharacterInfo, DNA_COUNT(characterCount)> characters;
    } characterSet;

    struct AnimationSet : BigYAML
    {
        DECL_YAML
        Delete expl;
        const UniqueID32& m_ancsId;
        AnimationSet(const UniqueID32& ancsId) : m_ancsId(ancsId), defaultTransition(ancsId) {}

        using MP1AnimationSet = DNAMP1::ANCS::AnimationSet;

        std::vector<MP1AnimationSet::Animation> animations;

        std::vector<MP1AnimationSet::Transition> transitions;
        MP1AnimationSet::MetaTransFactory defaultTransition;

        std::vector<MP1AnimationSet::AdditiveAnimationInfo> additiveAnims;

        float floatA = 0.0;
        float floatB = 0.0;

        std::vector<MP1AnimationSet::HalfTransition> halfTransitions;

        struct EVNT : BigYAML
        {
            DECL_YAML
            Delete expl;
            atUint32 version;

            struct EventBase : BigYAML
            {
                DECL_YAML
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
                DECL_YAML
                Value<atUint8> flag;
            };
            std::vector<LoopEvent> loopEvents;

            struct UEVTEvent : EventBase
            {
                DECL_YAML
                Value<atUint32> uevtType;
                String<-1> boneName;
            };
            std::vector<UEVTEvent> uevtEvents;

            struct EffectEvent : EventBase
            {
                DECL_YAML
                Value<atUint32> frameCount;
                DNAFourCC effectType;
                UniqueID32 effectId;
                Value<atUint32> boneId;
                Value<float> scale;
                Value<atUint32> parentMode;
            };
            std::vector<EffectEvent> effectEvents;

            struct SFXEvent : EventBase
            {
                DECL_YAML
                Value<atUint32> soundId;
                Value<float> smallNum;
                Value<float> bigNum;
                Value<atUint32> sfxUnk1;
                Value<atUint16> sfxUnk2;
                Value<atUint16> sfxUnk3;
                Value<float> sfxUnk4;
            };
            std::vector<SFXEvent> sfxEvents;
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
    }

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const DNAMP1::PAK::Entry& entry,
                        bool force,
                        hecl::BlenderToken& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        hecl::ProjectPath yamlPath = outPath.getWithExtension(_S(".yaml"));
        hecl::ProjectPath::Type yamlType = yamlPath.getPathType();
        hecl::ProjectPath blendPath = outPath.getWithExtension(_S(".blend"));
        hecl::ProjectPath::Type blendType = blendPath.getPathType();

        if (force ||
            yamlType == hecl::ProjectPath::Type::None ||
            blendType == hecl::ProjectPath::Type::None)
        {
            ANCS ancs(entry.id);
            ancs.read(rs);

            if (force || yamlType == hecl::ProjectPath::Type::None)
            {
                FILE* fp = hecl::Fopen(yamlPath.getAbsolutePath().c_str(), _S("wb"));
                ancs.toYAMLFile(fp);
                fclose(fp);
            }

            if (force || blendType == hecl::ProjectPath::Type::None)
            {
                hecl::BlenderConnection& conn = btok.getBlenderConnection();
                DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, ANCS, MaterialSet, DNACMDL::SurfaceHeader_2, 4>
                        (conn, ancs, blendPath, pakRouter, entry, dataSpec, fileChanged, force);
            }
        }

        return true;
    }
};

}
}

#endif // _DNAMP2_ANCS_HPP_
