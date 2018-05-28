#ifndef _DNAMP1_ANCS_HPP_
#define _DNAMP1_ANCS_HPP_

#include <map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/ANCS.hpp"
#include "CMDLMaterials.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"
#include "ANIM.hpp"
#include "EVNT.hpp"
#include "athena/FileReader.hpp"

namespace DataSpec::DNAMP1
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
            AT_DECL_DNA_YAML
            Delete expl;

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
                String<-1> strB;
            };
            std::vector<Animation> animations;

            struct PASDatabase : BigDNA
            {
                AT_DECL_DNA_YAML
                Value<atUint32> magic;
                Value<atUint32> animStateCount;
                Value<atUint32> defaultState;
                struct AnimState : BigDNA
                {
                    AT_DECL_DNA_YAML
                    Delete expl;
                    atUint32 id;

                    struct ParmInfo : BigDNA
                    {
                        AT_DECL_DNA_YAML
                        Delete expl;
                        enum class DataType
                        {
                            Int32 = 0,
                            UInt32 = 1,
                            Float = 2,
                            Bool = 3,
                            Enum = 4
                        };
                        union Parm
                        {
                            atInt32 int32;
                            atUint32 uint32;
                            float float32;
                            bool bool1;
                            Parm() : int32(0) {}
                            Parm(atInt32 val) : int32(val) {}
                            Parm(atUint32 val) : uint32(val) {}
                            Parm(float val) : float32(val) {}
                            Parm(bool val) : bool1(val) {}
                        };

                        atUint32 parmType;
                        atUint32 weightFunction;
                        float weight;
                        Parm range[2];
                    };
                    std::vector<ParmInfo> parmInfos;

                    struct AnimInfo
                    {
                        atUint32 id;
                        std::vector<ParmInfo::Parm> parmVals;
                    };
                    std::vector<AnimInfo> animInfos;
                };
                Vector<AnimState, AT_DNA_COUNT(animStateCount)> animStates;
            } pasDatabase;

            struct ParticleResData
            {
                std::vector<UniqueID32> part;
                std::vector<UniqueID32> swhc;
                std::vector<UniqueID32> unk;
                std::vector<UniqueID32> elsc;
            } partResData;

            atUint32 unk1 = 0;

            struct ActionAABB : BigDNA
            {
                AT_DECL_DNA_YAML
                String<-1> name;
                Value<atVec3f> aabb[2];
            };
            std::vector<ActionAABB> animAABBs;

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
                    String<-1> locator;
                    Value<float> scale;
                    Value<atUint32> parentMode;
                    Value<atUint32> flags;
                };
                Vector<EffectComponent, AT_DNA_COUNT(compCount)> comps;
            };
            std::vector<Effect> effects;

            UniqueID32Zero cmdlOverlay;
            UniqueID32Zero cskrOverlay;

            std::vector<atUint32> animIdxs;
        };
        Vector<CharacterInfo, AT_DNA_COUNT(characterCount)> characters;
    } characterSet;

    struct AnimationSet : BigDNA
    {
        AT_DECL_DNA_YAML
        Delete expl;

        struct MetaAnimPrimitive;
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
            virtual void gatherPrimitives(PAKRouter<PAKBridge>* pakRouter,
                                          std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)=0;
            virtual bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)=0;
        };
        struct MetaAnimFactory : BigDNA
        {
            AT_DECL_EXPLICIT_DNA_YAML
            std::unique_ptr<IMetaAnim> m_anim;
        };
        struct MetaAnimPrimitive : IMetaAnim
        {
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            MetaAnimPrimitive() : IMetaAnim(Type::Primitive, "Primitive") {}

            UniqueID32 animId;
            Value<atUint32> animIdx;
            String<-1> animName;
            Value<float> unk1;
            Value<atUint32> unk2;

            void gatherPrimitives(PAKRouter<PAKBridge>* pakRouter,
                                  std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out);

            bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)
            {
                return func(*this);
            }
        };
        struct MetaAnimBlend : IMetaAnim
        {
            MetaAnimBlend()
            : IMetaAnim(Type::Blend, "Blend") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            MetaAnimFactory animA;
            MetaAnimFactory animB;
            Value<float> unkFloat;
            Value<atUint8> unk;

            void gatherPrimitives(PAKRouter<PAKBridge>* pakRouter,
                                  std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                animA.m_anim->gatherPrimitives(pakRouter, out);
                animB.m_anim->gatherPrimitives(pakRouter, out);
            }

            bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)
            {
                if (!animA.m_anim->enumeratePrimitives(func))
                    return false;
                if (!animB.m_anim->enumeratePrimitives(func))
                    return false;
                return true;
            }
        };
        struct MetaAnimPhaseBlend : IMetaAnim
        {
            MetaAnimPhaseBlend()
            : IMetaAnim(Type::PhaseBlend, "PhaseBlend") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            MetaAnimFactory animA;
            MetaAnimFactory animB;
            Value<float> unkFloat;
            Value<atUint8> unk;

            void gatherPrimitives(PAKRouter<PAKBridge>* pakRouter,
                                  std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                animA.m_anim->gatherPrimitives(pakRouter, out);
                animB.m_anim->gatherPrimitives(pakRouter, out);
            }

            bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)
            {
                if (!animA.m_anim->enumeratePrimitives(func))
                    return false;
                if (!animB.m_anim->enumeratePrimitives(func))
                    return false;
                return true;
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
                AT_DECL_DNA
                MetaAnimFactory anim;
                Value<atUint32> probability;
            };
            Vector<Child, AT_DNA_COUNT(animCount)> children;

            void gatherPrimitives(PAKRouter<PAKBridge>* pakRouter,
                                  std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                for (const auto& child : children)
                    child.anim.m_anim->gatherPrimitives(pakRouter, out);
            }

            bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)
            {
                for (auto& child : children)
                    if (!child.anim.m_anim->enumeratePrimitives(func))
                        return false;
                return true;
            }
        };
        struct MetaAnimSequence : IMetaAnim
        {
            MetaAnimSequence() : IMetaAnim(Type::Sequence, "Sequence") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            Value<atUint32> animCount;
            Vector<MetaAnimFactory, AT_DNA_COUNT(animCount)> children;

            void gatherPrimitives(PAKRouter<PAKBridge>* pakRouter,
                                  std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                for (const auto& child : children)
                    child.m_anim->gatherPrimitives(pakRouter, out);
            }

            bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)
            {
                for (auto& child : children)
                    if (!child.m_anim->enumeratePrimitives(func))
                        return false;
                return true;
            }
        };

        struct Animation : BigDNA
        {
            AT_DECL_DNA_YAML
            String<-1> name;
            MetaAnimFactory metaAnim;
        };
        std::vector<Animation> animations;

        struct IMetaTrans : BigDNAVYaml
        {
            Delete expl;
            enum class Type
            {
                MetaAnim = 0,
                Trans = 1,
                PhaseTrans = 2,
                NoTrans = 3,
            } m_type;
            const char* m_typeStr;
            IMetaTrans(Type type, const char* typeStr)
            : m_type(type), m_typeStr(typeStr) {}
            virtual void gatherPrimitives(PAKRouter<PAKBridge>* pakRouter,
                                          std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out) {}
            virtual bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func) {return true;}
        };
        struct MetaTransFactory : BigDNA
        {
            AT_DECL_DNA_YAML
            Delete expl;
            std::unique_ptr<IMetaTrans> m_trans;
        };
        struct MetaTransMetaAnim : IMetaTrans
        {
            MetaTransMetaAnim()
            : IMetaTrans(Type::MetaAnim, "MetaAnim") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            MetaAnimFactory anim;

            void gatherPrimitives(PAKRouter<PAKBridge>* pakRouter,
                                  std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                anim.m_anim->gatherPrimitives(pakRouter, out);
            }

            bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)
            {
                return anim.m_anim->enumeratePrimitives(func);
            }
        };
        struct MetaTransTrans : IMetaTrans
        {
            MetaTransTrans()
            : IMetaTrans(Type::Trans, "Trans") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            Value<float> transDurTime;
            Value<atUint32> transDurTimeMode;
            Value<bool> unk2;
            Value<bool> runA;
            Value<atUint32> flags;
        };
        struct MetaTransPhaseTrans : IMetaTrans
        {
            MetaTransPhaseTrans()
            : IMetaTrans(Type::PhaseTrans, "PhaseTrans") {}
            AT_DECL_DNA_YAML
            AT_DECL_DNAV
            Value<float> transDurTime;
            Value<atUint32> transDurTimeMode;
            Value<bool> unk2;
            Value<bool> runA;
            Value<atUint32> flags;
        };

        struct Transition : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<atUint32> unk;
            Value<atUint32> animIdxA;
            Value<atUint32> animIdxB;
            MetaTransFactory metaTrans;
        };
        std::vector<Transition> transitions;
        MetaTransFactory defaultTransition;

        struct AdditiveAnimationInfo : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<atUint32> animIdx;
            Value<float> unk1;
            Value<float> unk2;
        };
        std::vector<AdditiveAnimationInfo> additiveAnims;

        float floatA = 0.0;
        float floatB = 0.0;

        struct HalfTransition : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<atUint32> animIdx;
            MetaTransFactory metaTrans;
        };
        std::vector<HalfTransition> halfTransitions;

        struct AnimationResources : BigDNA
        {
            AT_DECL_DNA_YAML
            UniqueID32 animId;
            UniqueID32 evntId;
        };
        std::vector<AnimationResources> animResources;
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

    void getAnimationResInfo(PAKRouter<PAKBridge>* pakRouter,
                             std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out) const
    {
        out.clear();
        for (const AnimationSet::Animation& ai : animationSet.animations)
            if (AnimationSet::IMetaAnim* anim = ai.metaAnim.m_anim.get())
                anim->gatherPrimitives(pakRouter, out);
        for (const AnimationSet::Transition& ti : animationSet.transitions)
            if (AnimationSet::IMetaTrans* trans = ti.metaTrans.m_trans.get())
                trans->gatherPrimitives(pakRouter, out);
        if (AnimationSet::IMetaTrans* trans = animationSet.defaultTransition.m_trans.get())
            trans->gatherPrimitives(pakRouter, out);
    }

    void enumeratePrimitives(const std::function<bool(AnimationSet::MetaAnimPrimitive& prim)>& func)
    {
        for (const AnimationSet::Animation& ai : animationSet.animations)
            if (AnimationSet::IMetaAnim* anim = ai.metaAnim.m_anim.get())
                anim->enumeratePrimitives(func);
        for (const AnimationSet::Transition& ti : animationSet.transitions)
            if (AnimationSet::IMetaTrans* trans = ti.metaTrans.m_trans.get())
                trans->enumeratePrimitives(func);
        if (AnimationSet::IMetaTrans* trans = animationSet.defaultTransition.m_trans.get())
            trans->enumeratePrimitives(func);
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, int charIdx) const
    {
        auto doCi = [&](const CharacterSet::CharacterInfo& ci)
        {
            for (const auto& id : ci.partResData.part)
                g_curSpec->flattenDependencies(id, pathsOut);
            for (const auto& id : ci.partResData.swhc)
                g_curSpec->flattenDependencies(id, pathsOut);
            for (const auto& id : ci.partResData.unk)
                g_curSpec->flattenDependencies(id, pathsOut);
            for (const auto& id : ci.partResData.elsc)
                g_curSpec->flattenDependencies(id, pathsOut);
        };
        if (charIdx < 0)
            for (const CharacterSet::CharacterInfo& ci : characterSet.characters)
                doCi(ci);
        else if (charIdx < characterSet.characters.size())
            doCi(characterSet.characters[charIdx]);
    }

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged);

    static bool Cook(const hecl::ProjectPath& outPath,
                     const hecl::ProjectPath& inPath,
                     const DNAANCS::Actor& actor);

    static bool CookCINF(const hecl::ProjectPath& outPath,
                         const hecl::ProjectPath& inPath,
                         const DNAANCS::Actor& actor);

    static bool CookCSKR(const hecl::ProjectPath& outPath,
                         const hecl::ProjectPath& inPath,
                         const DNAANCS::Actor& actor,
                         const std::function<bool(const hecl::ProjectPath& modelPath)>& modelCookFunc);
    static bool CookCSKRPC(const hecl::ProjectPath& outPath,
                           const hecl::ProjectPath& inPath,
                           const DNAANCS::Actor& actor,
                           const std::function<bool(const hecl::ProjectPath& modelPath)>& modelCookFunc);

    static bool CookANIM(const hecl::ProjectPath& outPath,
                         const hecl::ProjectPath& inPath,
                         const DNAANCS::Actor& actor,
                         hecl::blender::DataStream& ds,
                         bool pc);
};

}

#endif // _DNAMP1_ANCS_HPP_
