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

namespace Retro
{
namespace DNAMP2
{

struct ANCS : BigYAML
{
    using CINFType = CINF;
    using CSKRType = CSKR;
    using ANIMType = ANIM;

    DECL_YAML
    Value<atUint16> version;

    DNAMP1::ANCS::CharacterSet characterSet;
    DNAMP1::ANCS::AnimationSet animationSet;

    void getCharacterResInfo(std::vector<DNAANCS::CharacterResInfo<UniqueID32>>& out) const
    {
        out.clear();
        out.reserve(characterSet.characters.size());
        for (const DNAMP1::ANCS::CharacterSet::CharacterInfo& ci : characterSet.characters)
        {
            out.emplace_back();
            DNAANCS::CharacterResInfo<UniqueID32>& chOut = out.back();
            chOut.name = ci.name;
            chOut.cmdl = ci.cmdl;
            chOut.cskr = ci.cskr;
            chOut.cinf = ci.cinf;
        }
    }

    void getAnimationResInfo(std::map<atUint32, std::pair<std::string, UniqueID32>>& out) const
    {
        out.clear();
        for (const DNAMP1::ANCS::AnimationSet::Animation& ai : animationSet.animations)
            ai.metaAnim.m_anim->gatherPrimitives(out);
    }

    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const DNAMP1::PAK::Entry& entry,
                        bool force)
    {
        ANCS ancs;
        ancs.read(rs);

        HECL::ProjectPath yamlPath = outPath.getWithExtension(_S(".yaml"));
        if (force || yamlPath.getPathType() == HECL::ProjectPath::PT_NONE)
        {
            FILE* fp = HECL::Fopen(yamlPath.getAbsolutePath().c_str(), _S("wb"));
            ancs.toYAMLFile(fp);
            fclose(fp);
        }

        HECL::ProjectPath blendPath = outPath.getWithExtension(_S(".blend"));
        if (force || blendPath.getPathType() == HECL::ProjectPath::PT_NONE)
        {
            HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
            DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, ANCS, MaterialSet, 4>
                    (conn, ancs, blendPath, pakRouter, entry, dataSpec.getMasterShaderPath(), force);
            return conn.saveBlend();
        }

        return true;
    }
};

}
}

#endif // _DNAMP2_ANCS_HPP_
