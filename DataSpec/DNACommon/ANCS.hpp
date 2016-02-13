#ifndef _DNACOMMON_ANCS_HPP_
#define _DNACOMMON_ANCS_HPP_

#include <unordered_set>
#include "DNACommon.hpp"
#include "BlenderConnection.hpp"
#include "CMDL.hpp"

namespace DataSpec
{
namespace DNAANCS
{

using Actor = HECL::BlenderConnection::DataStream::Actor;

template <typename IDTYPE>
struct CharacterResInfo
{
    std::string name;
    IDTYPE cmdl;
    IDTYPE cskr;
    IDTYPE cinf;
    std::vector<std::pair<HECL::FourCC, std::pair<IDTYPE, IDTYPE>>> overlays;
};

template <typename IDTYPE>
struct AnimationResInfo
{
    std::string name;
    IDTYPE animId;
    IDTYPE evntId;
    bool additive;
};

template <class PAKRouter, class ANCSDNA, class MaterialSet, class SurfaceHeader, atUint32 CMDLVersion>
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
        const NOD::Node* node;
        const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(info.cmdl, &node, true, true);
        if (cmdlE)
        {
            HECL::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
            if (force || cmdlPath.getPathType() == HECL::ProjectPath::Type::None)
            {
                if (!conn.createBlend(cmdlPath, HECL::BlenderConnection::BlendType::Mesh))
                    return false;

                HECL::SystemStringView bestNameView(pakRouter.getBestEntryName(*cmdlE));
                fileChanged(bestNameView.sys_str().c_str());

                typename ANCSDNA::CSKRType cskr;
                pakRouter.lookupAndReadDNA(info.cskr, cskr);
                typename ANCSDNA::CINFType cinf;
                pakRouter.lookupAndReadDNA(info.cinf, cinf);
                using RigPair = std::pair<typename ANCSDNA::CSKRType*, typename ANCSDNA::CINFType*>;
                RigPair rigPair(&cskr, &cinf);

                PAKEntryReadStream rs = cmdlE->beginReadStream(*node);
                DNACMDL::ReadCMDLToBlender<PAKRouter, MaterialSet, RigPair, SurfaceHeader, CMDLVersion>
                        (conn, rs, pakRouter, *cmdlE, dataspec, rigPair);

                conn.saveBlend();
            }
        }
    }

    HECL::SystemStringView bestNameView(pakRouter.getBestEntryName(entry));
    fileChanged(bestNameView.sys_str().c_str());

    /* Establish ANCS blend */
    if (!conn.createBlend(outPath, HECL::BlenderConnection::BlendType::Actor))
        return false;
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);

    os.format("import bpy\n"
              "from mathutils import Vector\n"
              "bpy.context.scene.name = '%s'\n"
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
            os.format("arm_obj = bpy.data.objects['CINF_%s']\n", info.cinf.toString().c_str());
        os << "actor_subtype.linked_armature = arm_obj.name\n";

        /* Link CMDL */
        const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(info.cmdl, nullptr, true, true);
        if (cmdlE)
        {
            HECL::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
            os.linkBlend(cmdlPath.getAbsolutePathUTF8().c_str(),
                         pakRouter.getBestEntryName(*cmdlE).c_str(), true);

            /* Attach CMDL to CINF */
            os << "if obj.name not in bpy.context.scene.objects:\n"
                  "    bpy.context.scene.objects.link(obj)\n"
                  "obj.parent = arm_obj\n"
                  "obj.parent_type = 'ARMATURE'\n"
                  "actor_subtype.linked_mesh = obj.name\n\n";
        }

        /* Link overlays */
        for (const auto& overlay : info.overlays)
        {
            os << "overlay = actor_subtype.overlays.add()\n";
            os.format("overlay.name = '%s'\n", overlay.first.toString().c_str());

            /* Link CMDL */
            const typename PAKRouter::EntryType* cmdlE = pakRouter.lookupEntry(overlay.second.first, nullptr, true, true);
            if (cmdlE)
            {
                HECL::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
                os.linkBlend(cmdlPath.getAbsolutePathUTF8().c_str(),
                             pakRouter.getBestEntryName(*cmdlE).c_str(), true);

                /* Attach CMDL to CINF */
                os << "if obj.name not in bpy.context.scene.objects:\n"
                      "    bpy.context.scene.objects.link(obj)\n"
                      "obj.parent = arm_obj\n"
                      "obj.parent_type = 'ARMATURE'\n"
                      "overlay.linked_mesh = obj.name\n\n";
            }
        }
    }

    /* Get animation primitives */
    std::map<atUint32, AnimationResInfo<typename PAKRouter::IDType>> animResInfo;
    ancs.getAnimationResInfo(animResInfo);
    for (const auto& id : animResInfo)
    {
        typename ANCSDNA::ANIMType anim;
        if (pakRouter.lookupAndReadDNA(id.second.animId, anim, true))
        {
            os.format("act = bpy.data.actions.new('%s')\n"
                      "act.use_fake_user = True\n", id.second.name.c_str());
            anim.sendANIMToBlender(os, cinf, id.second.additive);
        }

        os.format("actor_action = actor_data.actions.add()\n"
                  "actor_action.name = '%s'\n", id.second.name.c_str());
    }

    os.close();
    conn.saveBlend();
    return true;
}

}
}

#endif // _DNACOMMON_ANCS_HPP_
