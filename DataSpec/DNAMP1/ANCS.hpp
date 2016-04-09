#ifndef _DNAMP1_ANCS_HPP_
#define _DNAMP1_ANCS_HPP_

#include <map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/ANCS.hpp"
#include "CMDLMaterials.hpp"
#include "BlenderConnection.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"
#include "ANIM.hpp"
#include "athena/FileReader.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct ANCS : BigYAML
{
    using CINFType = CINF;
    using CSKRType = CSKR;
    using ANIMType = ANIM;

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

            atUint32 idx;
            std::string name;
            UniqueID32 cmdl;
            UniqueID32 cskr;
            UniqueID32 cinf;

            struct Animation : BigYAML
            {
                DECL_YAML
                Value<atUint32> animIdx;
                String<-1> strA;
                String<-1> strB;
            };
            std::vector<Animation> animations;

            struct PASDatabase : BigYAML
            {
                DECL_YAML
                Value<atUint32> magic;
                Value<atUint32> animStateCount;
                Value<atUint32> defaultState;
                struct AnimState : BigYAML
                {
                    DECL_YAML
                    Delete expl;
                    atUint32 id;

                    struct ParmInfo : BigYAML
                    {
                        DECL_YAML
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
                        atUint32 unk1;
                        float unk2;
                        Parm parmVals[2];
                    };
                    std::vector<ParmInfo> parmInfos;

                    struct AnimInfo
                    {
                        atUint32 id;
                        std::vector<ParmInfo::Parm> parmVals;
                    };
                    std::vector<AnimInfo> animInfos;
                };
                Vector<AnimState, DNA_COUNT(animStateCount)> animStates;
            } pasDatabase;

            struct ParticleResData
            {
                std::vector<UniqueID32> part;
                std::vector<UniqueID32> swhc;
                std::vector<UniqueID32> unk;
                std::vector<UniqueID32> elsc;
            } partResData;

            atUint32 unk1 = 0;

            struct ActionAABB : BigYAML
            {
                DECL_YAML
                String<-1> name;
                Value<atVec3f> aabb[2];
            };
            std::vector<ActionAABB> animAABBs;

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
                    String<-1> locator;
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
        };
        Vector<CharacterInfo, DNA_COUNT(characterCount)> characters;
    } characterSet;

    struct AnimationSet : BigYAML
    {
        DECL_YAML
        Delete expl;

        struct MetaAnimPrimitive;
        struct IMetaAnim : BigYAML
        {
            Delete expl;
            virtual ~IMetaAnim() {}
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
            virtual void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)=0;
            virtual bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)=0;
        };
        struct MetaAnimFactory : BigYAML
        {
            DECL_YAML
            Delete expl;
            std::unique_ptr<IMetaAnim> m_anim;
        };
        struct MetaAnimPrimitive : IMetaAnim
        {
            Delete expl;

            MetaAnimPrimitive() : IMetaAnim(Type::Primitive, "Primitive") {}

            UniqueID32 animId;
            Value<atUint32> animIdx;
            String<-1> animName;
            Value<float> unk1;
            Value<atUint32> unk2;

            void read(athena::io::IStreamReader& __dna_reader)
            {
                /* animId */
                animId.read(__dna_reader);
                /* animIdx */
                animIdx = __dna_reader.readUint32Big();
                /* animName */
                animName = __dna_reader.readString(-1);
                /* unk1 */
                unk1 = __dna_reader.readFloatBig();
                /* unk2 */
                unk2 = __dna_reader.readUint32Big();
            }

            void write(athena::io::IStreamWriter& __dna_writer) const
            {
                /* animId */
                animId.write(__dna_writer);
                /* animIdx */
                __dna_writer.writeUint32Big(animIdx);
                /* animName */
                __dna_writer.writeString(animName, -1);
                /* unk1 */
                __dna_writer.writeFloatBig(unk1);
                /* unk2 */
                __dna_writer.writeUint32Big(unk2);
            }

            void read(athena::io::YAMLDocReader& __dna_docin)
            {
                /* animIdx */
                animIdx = __dna_docin.readUint32("animIdx");
                /* animName */
                animName = __dna_docin.readString("animName");
                /* unk1 */
                unk1 = __dna_docin.readFloat("unk1");
                /* unk2 */
                unk2 = __dna_docin.readUint32("unk2");
            }

            void write(athena::io::YAMLDocWriter& __dna_docout) const
            {
                /* animIdx */
                __dna_docout.writeUint32("animIdx", animIdx);
                /* animName */
                __dna_docout.writeString("animName", animName);
                /* unk1 */
                __dna_docout.writeFloat("unk1", unk1);
                /* unk2 */
                __dna_docout.writeUint32("unk2", unk2);
            }

            static const char* DNAType()
            {
                return "DataSpec::DNAMP1::ANCS::AnimationSet::MetaAnimPrimitive";
            }

            size_t binarySize(size_t __isz) const
            {
                __isz = animId.binarySize(__isz);
                __isz += animName.size() + 1;
                return __isz + 12;
            }

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                out[animIdx] = {animName, animId, UniqueID32(), false};
            }

            bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)
            {
                return func(*this);
            }
        };
        struct MetaAnimBlend : IMetaAnim
        {
            MetaAnimBlend()
            : IMetaAnim(Type::Blend, "Blend") {}
            DECL_YAML
            MetaAnimFactory animA;
            MetaAnimFactory animB;
            Value<float> unkFloat;
            Value<atUint8> unk;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                animA.m_anim->gatherPrimitives(out);
                animB.m_anim->gatherPrimitives(out);
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
            DECL_YAML
            MetaAnimFactory animA;
            MetaAnimFactory animB;
            Value<float> unkFloat;
            Value<atUint8> unk;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                animA.m_anim->gatherPrimitives(out);
                animB.m_anim->gatherPrimitives(out);
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
            DECL_YAML
            Value<atUint32> animCount;
            struct Child : BigYAML
            {
                DECL_YAML
                MetaAnimFactory anim;
                Value<atUint32> probability;
            };
            Vector<Child, DNA_COUNT(animCount)> children;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                for (const auto& child : children)
                    child.anim.m_anim->gatherPrimitives(out);
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
            DECL_YAML
            Value<atUint32> animCount;
            Vector<MetaAnimFactory, DNA_COUNT(animCount)> children;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                for (const auto& child : children)
                    child.m_anim->gatherPrimitives(out);
            }

            bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func)
            {
                for (auto& child : children)
                    if (!child.m_anim->enumeratePrimitives(func))
                        return false;
                return true;
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
            virtual ~IMetaTrans() {}
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
            virtual void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out) {}
            virtual bool enumeratePrimitives(const std::function<bool(MetaAnimPrimitive& prim)>& func) {return true;}
        };
        struct MetaTransFactory : BigYAML
        {
            DECL_YAML
            Delete expl;
            std::unique_ptr<IMetaTrans> m_trans;
        };
        struct MetaTransMetaAnim : IMetaTrans
        {
            MetaTransMetaAnim()
            : IMetaTrans(Type::MetaAnim, "MetaAnim") {}
            DECL_YAML
            MetaAnimFactory anim;

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                anim.m_anim->gatherPrimitives(out);
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
            DECL_YAML
            Value<float> time;
            Value<atUint32> unk1;
            Value<atUint8> unk2;
            Value<atUint8> unk3;
            Value<atUint32> unk4;
        };
        struct MetaTransPhaseTrans : IMetaTrans
        {
            MetaTransPhaseTrans()
            : IMetaTrans(Type::PhaseTrans, "PhaseTrans") {}
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

    void getAnimationResInfo(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out) const
    {
        out.clear();
        for (const AnimationSet::Animation& ai : animationSet.animations)
            ai.metaAnim.m_anim->gatherPrimitives(out);
        for (const AnimationSet::Transition& ti : animationSet.transitions)
            if (ti.metaTrans.m_trans)
                ti.metaTrans.m_trans->gatherPrimitives(out);
        if (animationSet.defaultTransition.m_trans)
            animationSet.defaultTransition.m_trans->gatherPrimitives(out);
        for (auto& anim : out)
        {
            for (const AnimationSet::AnimationResources& res : animationSet.animResources)
            {
                if (res.animId == anim.second.animId)
                {
                    anim.second.evntId = res.evntId;
                    break;
                }
            }
        }
    }

    void enumeratePrimitives(const std::function<bool(AnimationSet::MetaAnimPrimitive& prim)>& func)
    {
        for (const AnimationSet::Animation& ai : animationSet.animations)
            ai.metaAnim.m_anim->enumeratePrimitives(func);
        for (const AnimationSet::Transition& ti : animationSet.transitions)
            ti.metaTrans.m_trans->enumeratePrimitives(func);
        animationSet.defaultTransition.m_trans->enumeratePrimitives(func);
    }

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
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
            ANCS ancs;
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
                DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, ANCS, MaterialSet, DNACMDL::SurfaceHeader_1, 2>
                        (conn, ancs, blendPath, pakRouter, entry, dataSpec, fileChanged, force);
            }
        }

        return true;
    }

    static bool Cook(const hecl::ProjectPath& outPath,
                     const hecl::ProjectPath& inPath,
                     const DNAANCS::Actor& actor)
    {
        /* Search for yaml */
        hecl::ProjectPath yamlPath = inPath.getWithExtension(_S(".yaml"), true);
        if (yamlPath.getPathType() != hecl::ProjectPath::Type::File)
            Log.report(logvisor::Fatal, _S("'%s' not found as file"),
                       yamlPath.getRelativePath().c_str());

        athena::io::FileReader yamlReader(yamlPath.getAbsolutePath());
        if (!BigYAML::ValidateFromYAMLFile<ANCS>(yamlReader))
            Log.report(logvisor::Fatal, _S("'%s' is not urde::DNAMP1::ANCS type"),
                       yamlPath.getRelativePath().c_str());
        ANCS ancs;
        ancs.read(yamlReader);

        /* Set Character Resource IDs */
        for (ANCS::CharacterSet::CharacterInfo& ch : ancs.characterSet.characters)
        {
            hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(ch.cmdl);
            if (path)
                ch.cskr = path.ensureAuxInfo(_S("skin"));

            for (const DNAANCS::Actor::Subtype& sub : actor.subtypes)
            {
                if (!sub.name.compare(ch.name))
                {
                    if (sub.armature >= 0)
                    {
                        const DNAANCS::Actor::Armature& arm = actor.armatures[sub.armature];
                        hecl::SystemStringView chSysName(arm.name);
                        ch.cinf = inPath.ensureAuxInfo(chSysName.c_str());
                        break;
                    }
                }
            }

            path = UniqueIDBridge::TranslatePakIdToPath(ch.cmdlOverlay);
            if (path)
                ch.cskrOverlay = path.ensureAuxInfo(_S("skin"));
        }

        /* Set Animation Resource IDs */
        ancs.enumeratePrimitives([&](AnimationSet::MetaAnimPrimitive& prim) -> bool
        {
            hecl::SystemStringView sysStr(prim.animName);
            prim.animId = inPath.ensureAuxInfo(sysStr.c_str());
            return true;
        });

        /* Write out CINF resources */
        for (const DNAANCS::Actor::Armature& arm : actor.armatures)
        {
            hecl::SystemStringView sysStr(arm.name);
            hecl::ProjectPath pathOut = inPath.getWithExtension(sysStr.c_str(), true);
            athena::io::FileWriter w(pathOut.getAbsolutePath(), true, false);
            if (w.hasError())
                Log.report(logvisor::Fatal, _S("unable to open '%s' for writing"), pathOut.getRelativePath().c_str());
            CINF cinf(arm);
            cinf.write(w);
        }

        return true;
    }
};

}
}

#endif // _DNAMP1_ANCS_HPP_
