#ifndef _DNACOMMON_ANCS_HPP_
#define _DNACOMMON_ANCS_HPP_

#include <unordered_set>
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
                       const SpecBase& dataspec,
                       std::function<void(const HECL::SystemChar*)> fileChanged,
                       bool force=false)
{    
    /* Extract character CMDL/CSKR first */
    std::vector<CharacterResInfo<typename PAKRouter::IDType>> chResInfo;
    ancs.getCharacterResInfo(chResInfo);
    for (const auto& info : chResInfo)
    {
        const NOD::DiscBase::IPartition::Node* node;
        const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(info.cmdl, &node);
        if (cmdlE)
        {
            HECL::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
            if (force || cmdlPath.getPathType() == HECL::ProjectPath::PT_NONE)
            {
                if (!conn.createBlend(cmdlPath.getAbsolutePath()))
                    return false;

                HECL::SystemStringView bestNameView(pakRouter.getBestEntryName(*cmdlE));
                fileChanged(bestNameView.sys_str().c_str());

                typename ANCSDNA::CSKRType cskr;
                pakRouter.lookupAndReadDNA(info.cskr, cskr);
                typename ANCSDNA::CINFType cinf;
                pakRouter.lookupAndReadDNA(info.cinf, cinf);
                using RIGPair = std::pair<typename ANCSDNA::CSKRType*, typename ANCSDNA::CINFType*>;
                RIGPair rigPair(&cskr, &cinf);

                PAKEntryReadStream rs = cmdlE->beginReadStream(*node);
                DNACMDL::ReadCMDLToBlender<PAKRouter, MaterialSet, RIGPair, CMDLVersion>
                        (conn, rs, pakRouter, *cmdlE, dataspec, rigPair);

                conn.saveBlend();
            }
        }
    }

    HECL::SystemStringView bestNameView(pakRouter.getBestEntryName(entry));
    fileChanged(bestNameView.sys_str().c_str());

    /* Establish ANCS blend */
    if (!conn.createBlend(outPath.getAbsolutePath()))
        return false;
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);

    os.format("import bpy\n"
              "from mathutils import Vector\n"
              "bpy.context.scene.name = '%s'\n"
              "bpy.context.scene.hecl_type = 'ACTOR'\n"
              "bpy.context.scene.hecl_mesh_obj = bpy.context.scene.name\n"
              "\n"
              "# Using 'Blender Game'\n"
              "bpy.context.scene.render.engine = 'BLENDER_GAME'\n"
              "\n"
              "# Clear Scene\n"
              "for ob in bpy.data.objects:\n"
              "    if ob.type != 'LAMP':\n"
              "        bpy.context.scene.objects.unlink(ob)\n"
              "        bpy.data.objects.remove(ob)\n"
              "\n"
              "actor_data = bpy.context.scene.hecl_sact_data\n",
              pakRouter.getBestEntryName(entry).c_str());

    typename ANCSDNA::CINFType cinf;
    std::unordered_set<typename PAKRouter::IDType> cinfsDone;
    for (const auto& info : chResInfo)
    {
        /* Provide data to add-on */
        os.format("actor_subtype = actor_data.subtypes.add()\n"
                  "actor_subtype.name = '%s'\n\n",
                  info.name.c_str());

        /* Build CINF if needed */
        if (cinfsDone.find(info.cinf) == cinfsDone.end())
        {
            pakRouter.lookupAndReadDNA(info.cinf, cinf);
            cinf.sendCINFToBlender(os, info.cinf);
            cinfsDone.insert(info.cinf);
        }
        else
            os.format("arm_obj = bpy.data.objects['CINF_%08X']\n", info.cinf.toUint32());
        os << "actor_subtype.linked_armature = arm_obj.name\n";

        /* Link CMDL */
        const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(info.cmdl);
        if (cmdlE)
        {
            HECL::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
            os.linkBlend(cmdlPath.getAbsolutePathUTF8(), pakRouter.getBestEntryName(*cmdlE), true);

            /* Attach CMDL to CINF */
            os << "if obj.name not in bpy.context.scene.objects:\n"
                  "    bpy.context.scene.objects.link(obj)\n"
                  "obj.parent = arm_obj\n"
                  "obj.parent_type = 'ARMATURE'\n"
                  "actor_subtype.linked_mesh = obj.name\n\n";
        }
    }

    /* Get animation primitives */
    std::map<atUint32, std::pair<std::string, typename PAKRouter::IDType>> animResInfo;
    ancs.getAnimationResInfo(animResInfo);
    for (const auto& id : animResInfo)
    {
        typename ANCSDNA::ANIMType anim;
        if (pakRouter.lookupAndReadDNA(id.second.second, anim))
        {
            os.format("act = bpy.data.actions.new('%s')\n"
                      "act.use_fake_user = True\n", id.second.first.c_str());
            anim.sendANIMToBlender(os, cinf);
        }

        os.format("actor_action = actor_data.actions.add()\n"
                  "actor_action.name = '%s'\n", id.second.first.c_str());
    }

    os.close();
    conn.saveBlend();
    return true;
}

}
}

#endif // _DNACOMMON_ANCS_HPP_
