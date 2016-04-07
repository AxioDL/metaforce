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

            atUint32 idx;
            std::string name;
            UniqueID32 cmdl;
            UniqueID32 _cskrOld;
            UniqueID32 _cinfOld;
            AuxiliaryID32 cskr = _S("skin");
            AuxiliaryID32 cinf = {_S("layout"), _S(".blend")};

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
                    String<-1> name2;
                    Value<float> unk1;
                    Value<atUint32> unk2;
                    Value<atUint32> unk3;
                };
                Vector<EffectComponent, DNA_COUNT(compCount)> comps;
            };
            std::vector<Effect> effects;

            UniqueID32 cmdlOverlay;
            UniqueID32 _cskrOverlayOld;
            AuxiliaryID32 cskrOverlay = _S("skin");

            std::vector<atUint32> animIdxs;
        };
        Vector<CharacterInfo, DNA_COUNT(characterCount)> characters;
    } characterSet;

    struct AnimationSet : BigYAML
    {
        DECL_YAML
        Delete expl;
        const UniqueID32& m_ancsId;
        AnimationSet(const UniqueID32& ancsId)
        : m_ancsId(ancsId), defaultTransition(ancsId) {}

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
            const UniqueID32& m_ancsId;
            IMetaAnim(Type type, const char* typeStr, const UniqueID32& ancsId)
            : m_type(type), m_typeStr(typeStr), m_ancsId(ancsId) {}
            virtual void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)=0;
        };
        struct MetaAnimFactory : BigYAML
        {
            DECL_YAML
            Delete expl;
            const UniqueID32& m_ancsId;
            std::unique_ptr<IMetaAnim> m_anim;
            MetaAnimFactory(const UniqueID32& ancsId) : m_ancsId(ancsId) {}
        };
        struct MetaAnimPrimitive : IMetaAnim
        {
            MetaAnimPrimitive(const UniqueID32& ancsId) : IMetaAnim(Type::Primitive, "Primitive", ancsId) {}

            Delete _d;
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
                
                hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(m_ancsId);
                if (path)
                {
                    hecl::SystemStringView sysView(animName);
                    animId = path.ensureAuxInfo(sysView.sys_str().c_str());
                }
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
        };
        struct MetaAnimBlend : IMetaAnim
        {
            MetaAnimBlend(const UniqueID32& ancsId)
            : IMetaAnim(Type::Blend, "Blend", ancsId), animA(ancsId), animB(ancsId) {}
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
        };
        struct MetaAnimPhaseBlend : IMetaAnim
        {
            MetaAnimPhaseBlend(const UniqueID32& ancsId)
            : IMetaAnim(Type::PhaseBlend, "PhaseBlend", ancsId), animA(ancsId), animB(ancsId) {}
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
        };
        struct MetaAnimRandom : IMetaAnim
        {
            MetaAnimRandom(const UniqueID32& ancsId) : IMetaAnim(Type::Random, "Random", ancsId) {}
            Delete _d;
            Value<atUint32> animCount;
            struct Child : BigYAML
            {
                DECL_YAML
                MetaAnimFactory anim;
                Value<atUint32> probability;
                Child(const UniqueID32& ancsId) : anim(ancsId) {}
            };
            Vector<Child, DNA_COUNT(animCount)> children;

            void read(athena::io::IStreamReader& __dna_reader)
            {
                /* animCount */
                animCount = __dna_reader.readUint32Big();
                /* children */
                children.clear();
                children.reserve(animCount);
                for (size_t i=0 ; i<animCount ; ++i)
                {
                    children.emplace_back(m_ancsId);
                    children.back().read(__dna_reader);
                }
            }

            void write(athena::io::IStreamWriter& __dna_writer) const
            {
                /* animCount */
                __dna_writer.writeUint32Big(animCount);
                /* children */
                __dna_writer.enumerate(children);
            }

            void read(athena::io::YAMLDocReader& __dna_docin)
            {
                /* animCount squelched */
                /* children */
                size_t childCount;
                __dna_docin.enterSubVector("children", childCount);
                animCount = childCount;
                children.clear();
                children.reserve(childCount);
                for (size_t i=0 ; i<childCount ; ++i)
                {
                    children.emplace_back(m_ancsId);
                    __dna_docin.enterSubRecord(nullptr);
                    children.back().read(__dna_docin);
                    __dna_docin.leaveSubRecord();
                }
                __dna_docin.leaveSubVector();
            }

            void write(athena::io::YAMLDocWriter& __dna_docout) const
            {
                /* animCount squelched */
                /* children */
                __dna_docout.enumerate("children", children);
            }

            static const char* DNAType()
            {
                return "DataSpec::DNAMP1::ANCS::AnimationSet::MetaAnimRandom";
            }

            size_t binarySize(size_t __isz) const
            {
                __isz = __EnumerateSize(__isz, children);
                return __isz + 4;
            }

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
            {
                for (const auto& child : children)
                    child.anim.m_anim->gatherPrimitives(out);
            }
        };
        struct MetaAnimSequence : IMetaAnim
        {
            MetaAnimSequence(const UniqueID32& ancsId) : IMetaAnim(Type::Sequence, "Sequence", ancsId) {}
            Delete _d;
            Value<atUint32> animCount;
            Vector<MetaAnimFactory, DNA_COUNT(animCount)> children;

            void read(athena::io::IStreamReader& __dna_reader)
            {
                /* animCount */
                animCount = __dna_reader.readUint32Big();
                /* children */
                children.clear();
                children.reserve(animCount);
                for (size_t i=0 ; i<animCount ; ++i)
                {
                    children.emplace_back(m_ancsId);
                    children.back().read(__dna_reader);
                }
            }

            void write(athena::io::IStreamWriter& __dna_writer) const
            {
                /* animCount */
                __dna_writer.writeUint32Big(animCount);
                /* children */
                __dna_writer.enumerate(children);
            }

            void read(athena::io::YAMLDocReader& __dna_docin)
            {
                /* animCount squelched */
                /* children */
                size_t childCount;
                __dna_docin.enterSubVector("children", childCount);
                animCount = childCount;
                children.clear();
                children.reserve(childCount);
                for (size_t i=0 ; i<childCount ; ++i)
                {
                    children.emplace_back(m_ancsId);
                    __dna_docin.enterSubRecord(nullptr);
                    children.back().read(__dna_docin);
                    __dna_docin.leaveSubRecord();
                }
                __dna_docin.leaveSubVector();
            }

            void write(athena::io::YAMLDocWriter& __dna_docout) const
            {
                /* animCount squelched */
                /* children */
                __dna_docout.enumerate("children", children);
            }

            static const char* DNAType()
            {
                return "DataSpec::DNAMP1::ANCS::AnimationSet::MetaAnimSequence";
            }

            size_t binarySize(size_t __isz) const
            {
                __isz = __EnumerateSize(__isz, children);
                return __isz + 4;
            }

            void gatherPrimitives(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out)
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
            Animation(const UniqueID32& ancsId) : metaAnim(ancsId) {}
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
            const UniqueID32& m_ancsId;
            IMetaTrans(Type type, const char* typeStr, const UniqueID32& ancsId)
            : m_type(type), m_typeStr(typeStr), m_ancsId(ancsId) {}
        };
        struct MetaTransFactory : BigYAML
        {
            DECL_YAML
            Delete expl;
            const UniqueID32& m_ancsId;
            std::unique_ptr<IMetaTrans> m_trans;
            MetaTransFactory(const UniqueID32& ancsId) : m_ancsId(ancsId) {}
        };
        struct MetaTransMetaAnim : IMetaTrans
        {
            MetaTransMetaAnim(const UniqueID32& ancsId)
            : IMetaTrans(Type::MetaAnim, "MetaAnim", ancsId), anim(ancsId) {}
            DECL_YAML
            MetaAnimFactory anim;
        };
        struct MetaTransTrans : IMetaTrans
        {
            MetaTransTrans(const UniqueID32& ancsId)
            : IMetaTrans(Type::Trans, "Trans", ancsId) {}
            DECL_YAML
            Value<float> time;
            Value<atUint32> unk1;
            Value<atUint8> unk2;
            Value<atUint8> unk3;
            Value<atUint32> unk4;
        };
        struct MetaTransPhaseTrans : IMetaTrans
        {
            MetaTransPhaseTrans(const UniqueID32& ancsId)
            : IMetaTrans(Type::PhaseTrans, "PhaseTrans", ancsId) {}
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
            Transition(const UniqueID32& ancsId) : metaTrans(ancsId) {}
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
            HalfTransition(const UniqueID32& ancsId) : metaTrans(ancsId) {}
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
            chOut.cskr = ci._cskrOld;
            chOut.cinf = ci._cinfOld;

            if (ci.cmdlOverlay)
                chOut.overlays.emplace_back(FOURCC('OVER'), std::make_pair(ci.cmdlOverlay, ci._cskrOverlayOld));
        }
    }

    void getAnimationResInfo(std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out) const
    {
        out.clear();
        for (const AnimationSet::Animation& ai : animationSet.animations)
            ai.metaAnim.m_anim->gatherPrimitives(out);
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

    void fixupPaths(const UniqueID32& ancsId)
    {
        for (CharacterSet::CharacterInfo& character : characterSet.characters)
        {
            character._cskrOld = character.cskr;
            character._cinfOld = character.cinf;
            character.cskr = character.cmdl;
            character.cinf = ancsId;
            character._cskrOverlayOld = character.cskrOverlay;
            character.cskrOverlay = character.cmdlOverlay;
        }
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
            ANCS ancs(entry.id);
            ancs.read(rs);
            ancs.fixupPaths(entry.id);

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
        ANCS ancs(UniqueID32::kInvalidId);
        ancs.read(yamlReader);


        return true;
    }
};

}
}

#endif // _DNAMP1_ANCS_HPP_
