#ifndef _DNACOMMON_ANCS_HPP_
#define _DNACOMMON_ANCS_HPP_

#include "DNACommon.hpp"
#include "BlenderConnection.hpp"
#include "CMDL.hpp"

namespace Retro
{
namespace DNAANCS
{

template <typename IDTYPE>
struct CharacterResInfo
{
    std::string name;
    IDTYPE cmdl;
    IDTYPE cskr;
    IDTYPE cinf;
};

template <class PAKRouter, class ANCSDNA, class MaterialSet, atUint32 CMDLVersion>
bool ReadANCSToBlender(HECL::BlenderConnection& conn,
                       const ANCSDNA& ancs,
                       const HECL::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       const HECL::ProjectPath& masterShader,
                       bool force=false)
{
    /* Extract characters first */
    std::vector<CharacterResInfo<typename PAKRouter::IDType>> chResInfo;
    ancs.getCharacterResInfo(chResInfo);
    for (const auto& info : chResInfo)
    {
        const NOD::DiscBase::IPartition::Node* node;
        const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(info.cmdl, &node);
        HECL::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
        if (force || cmdlPath.getPathType() == HECL::ProjectPath::PT_NONE)
        {
            if (!conn.createBlend(cmdlPath.getAbsolutePath()))
                return false;
            PAKEntryReadStream rs = cmdlE->beginReadStream(*node);
            DNACMDL::ReadCMDLToBlender<PAKRouter, MaterialSet, CMDLVersion>
                    (conn, rs, pakRouter, entry, masterShader);

            const typename PAKRouter::EntryType* cskrE = pakRouter.lookupEntry(info.cskr);
            const typename PAKRouter::EntryType* cinfE = pakRouter.lookupEntry(info.cinf);

            conn.saveBlend();
        }
    }

    /* Establish ANCS blend */
    if (!conn.createBlend(outPath.getAbsolutePath() + ".blend"))
        return false;
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);

    /* Get animation primitives */
    std::unordered_set<typename PAKRouter::IDType> animResInfo;
    ancs.getAnimationResInfo(animResInfo);
    for (const auto& id : animResInfo)
    {
        const typename PAKRouter::EntryType* animE = pakRouter.lookupEntry(id);
    }

    return true;
}

}
}

#endif // _DNACOMMON_ANCS_HPP_
