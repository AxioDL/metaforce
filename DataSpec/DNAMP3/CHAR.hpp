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
    Delete expl;
    atUint16 version;

    std::string name;
    UniqueID64 cmdl;
    UniqueID64 cskr;
    atUint32 unk1;
    atUint32 unk2;
    UniqueID64 cmdl2;
    UniqueID64 cskr2;
    UniqueID64 cinf;
    UniqueID64 sand;

    using MP1CharacterInfo = DNAMP1::ANCS::CharacterSet::CharacterInfo;
    MP1CharacterInfo::PASDatabase pasDatabase;

    struct ParticleResData
    {
        std::vector<UniqueID64> part;
        std::vector<UniqueID64> swhc;
        std::vector<UniqueID64> unk;
        std::vector<UniqueID64> elsc;
        std::vector<UniqueID64> spsc;
        std::vector<UniqueID64> unk2;
    } partResData;

    struct AnimationSet : BigYAML
    {
        DECL_YAML
        Delete expl;

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
        std::vector<Animation> animations;

        struct IMetaTrans : BigYAML
        {
            Delete expl;
            enum Type
            {
                MTMetaAnim = 0,
                MTTrans = 1,
                MTPhaseTrans = 2,
                MTNoTrans = 3,
            } m_type;
            const char* m_typeStr;
            IMetaTrans(Type type, const char* typeStr)
            : m_type(type), m_typeStr(typeStr) {}
        };
        struct MetaTransFactory : BigYAML
        {
            DECL_YAML
            Delete expl;
            std::unique_ptr<IMetaTrans> m_trans;
        };
        struct MetaTransMetaAnim : IMetaTrans
        {
            MetaTransMetaAnim() : IMetaTrans(MTMetaAnim, "MetaAnim") {}
            DECL_YAML
            MetaAnimFactory anim;
        };
        struct MetaTransTrans : IMetaTrans
        {
            MetaTransTrans() : IMetaTrans(MTTrans, "Trans") {}
            DECL_YAML
            Value<float> time;
            Value<atUint32> unk1;
            Value<atUint8> unk2;
            Value<atUint8> unk3;
            Value<atUint32> unk4;
        };
        struct MetaTransPhaseTrans : IMetaTrans
        {
            MetaTransPhaseTrans() : IMetaTrans(MTPhaseTrans, "PhaseTrans") {}
            DECL_YAML
            Value<float> time;
            Value<atUint32> unk1;
            Value<atUint8> unk2;
            Value<atUint8> unk3;
            Value<atUint32> unk4;
        };

        struct Transition : BigYAML
        {
            DECL_YAML
            Value<atUint32> unk;
            Value<atUint32> animIdxA;
            Value<atUint32> animIdxB;
            MetaTransFactory metaTrans;
        };
        std::vector<Transition> transitions;
        MetaTransFactory defaultTransition;

        struct AdditiveAnimationInfo : BigYAML
        {
            DECL_YAML
            Value<atUint32> animIdx;
            Value<float> unk1;
            Value<float> unk2;
        };
        std::vector<AdditiveAnimationInfo> additiveAnims;

        float floatA = 0.0;
        float floatB = 0.0;

        struct HalfTransition : BigYAML
        {
            DECL_YAML
            Value<atUint32> animIdx;
            MetaTransFactory metaTrans;
        };
        std::vector<HalfTransition> halfTransitions;

        struct AnimationResources : BigYAML
        {
            DECL_YAML
            UniqueID64 animId;
            UniqueID64 evntId;
        };
        std::vector<AnimationResources> animResources;
    } animationSet;

    struct ActionAABB : BigYAML
    {
        DECL_YAML
        UniqueID64 animId;
        Value<atVec3f> aabb[2];
    };
    std::vector<ActionAABB> animAABBs;

    atUint32 unk3 = 0;


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

    UniqueID32 cmdlOverride;
    UniqueID32 cskrOverride;

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




    void getCharacterResInfo(std::vector<DNAANCS::CharacterResInfo<UniqueID64>>& out) const
    {
        out.clear();
        out.reserve(1);
        out.emplace_back();
        DNAANCS::CharacterResInfo<UniqueID64>& chOut = out.back();
        chOut.name = name;
        chOut.cmdl = cmdl;
        chOut.cskr = cskr;
        chOut.cinf = cinf;
    }

    void getAnimationResInfo(std::map<atUint32, std::pair<std::string, UniqueID64>>& out) const
    {
        out.clear();
        for (const AnimationSet::Animation& ai : animationSet.animations)
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
                DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, CHAR, MaterialSet, 4>
                        (conn, aChar, blendPath, pakRouter, entry, dataSpec, fileChanged, force);
            }
        }

        return true;
    }
};

}
}

#endif // _DNAMP3_CHAR_HPP_
