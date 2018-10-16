#include "ANCS.hpp"
#include "DataSpec/DNAMP1/DNAMP1.hpp"
#include "DataSpec/DNAMP1/ANCS.hpp"
#include "DataSpec/DNAMP2/DNAMP2.hpp"
#include "DataSpec/DNAMP2/ANCS.hpp"
#include "DataSpec/DNAMP3/DNAMP3.hpp"
#include "DataSpec/DNAMP3/CHAR.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAANCS
{

template <class PAKRouter, class ANCSDNA, class MaterialSet, class SurfaceHeader, atUint32 CMDLVersion>
bool ReadANCSToBlender(hecl::blender::Connection& conn,
                       const ANCSDNA& ancs,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       const SpecBase& dataspec,
                       std::function<void(const hecl::SystemChar*)> fileChanged,
                       bool force)
{
    /* Extract character CMDL/CSKR first */
    std::vector<CharacterResInfo<typename PAKRouter::IDType>> chResInfo;
    ancs.getCharacterResInfo(chResInfo);
    for (const auto& info : chResInfo)
    {
        const nod::Node* node;
        const typename PAKRouter::EntryType* cmdlE =
            pakRouter.lookupEntry(info.cmdl, &node, true, false);
        if (cmdlE)
        {
            hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
            if (force || cmdlPath.isNone())
            {
                if (!conn.createBlend(cmdlPath, hecl::blender::BlendType::Mesh))
                    return false;

                std::string bestName = pakRouter.getBestEntryName(*cmdlE);
                hecl::SystemStringConv bestNameView(bestName);
                fileChanged(bestNameView.c_str());

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

    /* Extract attachment CMDL/CSKRs first */
    auto attRange = pakRouter.lookupCharacterAttachmentRigs(entry.id);
    for (auto it = attRange.first; it != attRange.second; ++it)
    {
        auto cmdlid = it->second.first.second;

        const nod::Node* node;
        const typename PAKRouter::EntryType* cmdlE =
            pakRouter.lookupEntry(cmdlid, &node, true, false);
        if (cmdlE)
        {
            hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
            if (force || cmdlPath.isNone())
            {
                if (!conn.createBlend(cmdlPath, hecl::blender::BlendType::Mesh))
                    return false;

                std::string bestName = pakRouter.getBestEntryName(*cmdlE);
                hecl::SystemStringConv bestNameView(bestName);
                fileChanged(bestNameView.c_str());

                const auto* rp = pakRouter.lookupCMDLRigPair(cmdlid);
                typename ANCSDNA::CSKRType cskr;
                pakRouter.lookupAndReadDNA(rp->first, cskr);
                typename ANCSDNA::CINFType cinf;
                pakRouter.lookupAndReadDNA(rp->second, cinf);
                using RigPair = std::pair<typename ANCSDNA::CSKRType*, typename ANCSDNA::CINFType*>;
                RigPair rigPair(&cskr, &cinf);

                PAKEntryReadStream rs = cmdlE->beginReadStream(*node);
                DNACMDL::ReadCMDLToBlender<PAKRouter, MaterialSet, RigPair, SurfaceHeader, CMDLVersion>
                    (conn, rs, pakRouter, *cmdlE, dataspec, rigPair);

                conn.saveBlend();
            }
        }
    }

    std::string bestName = pakRouter.getBestEntryName(entry);
    hecl::SystemStringConv bestNameView(bestName);
    fileChanged(bestNameView.c_str());

    /* Establish ANCS blend */
    if (!conn.createBlend(outPath, hecl::blender::BlendType::Actor))
        return false;

    std::string firstName;
    typename ANCSDNA::CINFType firstCinf;
    {
        hecl::blender::PyOutStream os = conn.beginPythonOut(true);

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
                  "    if ob.type != 'LAMP' and ob.type != 'CAMERA':\n"
                  "        bpy.context.scene.objects.unlink(ob)\n"
                  "        bpy.data.objects.remove(ob)\n"
                  "\n"
                  "actor_data = bpy.context.scene.hecl_sact_data\n"
                  "arm_obj = None\n",
                  pakRouter.getBestEntryName(entry).c_str());

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
                typename ANCSDNA::CINFType cinf;
                pakRouter.lookupAndReadDNA(info.cinf, cinf);
                cinf.sendCINFToBlender(os, info.cinf);
                if (cinfsDone.empty())
                {
                    firstName = ANCSDNA::CINFType::GetCINFArmatureName(info.cinf);
                    firstCinf = cinf;
                }
                cinfsDone.insert(info.cinf);
            }
            else
                os.format("arm_obj = bpy.data.objects['CINF_%s']\n", info.cinf.toString().c_str());
            os << "actor_subtype.linked_armature = arm_obj.name\n";

            /* Link CMDL */
            const typename PAKRouter::EntryType* cmdlE =
                pakRouter.lookupEntry(info.cmdl, nullptr, true, false);
            if (cmdlE)
            {
                hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
                os.linkBlend(cmdlPath.getAbsolutePathUTF8().data(),
                             pakRouter.getBestEntryName(*cmdlE).data(), true);

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
                os.format("overlay.name = '%s'\n", overlay.first.c_str());

                /* Link CMDL */
                const typename PAKRouter::EntryType* cmdlE =
                    pakRouter.lookupEntry(overlay.second.first, nullptr, true, false);
                if (cmdlE)
                {
                    hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
                    os.linkBlend(cmdlPath.getAbsolutePathUTF8().data(),
                                 pakRouter.getBestEntryName(*cmdlE).data(), true);

                    /* Attach CMDL to CINF */
                    os << "if obj.name not in bpy.context.scene.objects:\n"
                        "    bpy.context.scene.objects.link(obj)\n"
                        "obj.parent = arm_obj\n"
                        "obj.parent_type = 'ARMATURE'\n"
                        "overlay.linked_mesh = obj.name\n\n";
                }
            }
        }

        /* Link attachments */
        for (auto it = attRange.first; it != attRange.second; ++it)
        {
            os << "attachment = actor_data.attachments.add()\n";
            os.format("attachment.name = '%s'\n", it->second.second.c_str());

            auto cinfid = it->second.first.first;
            auto cmdlid = it->second.first.second;

            if (cinfid)
            {
                /* Build CINF if needed */
                if (cinfsDone.find(cinfid) == cinfsDone.end())
                {
                    typename ANCSDNA::CINFType cinf;
                    pakRouter.lookupAndReadDNA(cinfid, cinf);
                    cinf.sendCINFToBlender(os, cinfid);
                    if (cinfsDone.empty())
                    {
                        firstName = ANCSDNA::CINFType::GetCINFArmatureName(cinfid);
                        firstCinf = cinf;
                    }
                    cinfsDone.insert(cinfid);
                }
                else
                    os.format("arm_obj = bpy.data.objects['CINF_%s']\n", cinfid.toString().c_str());
                os << "attachment.linked_armature = arm_obj.name\n";
            }

            /* Link CMDL */
            const typename PAKRouter::EntryType* cmdlE =
                pakRouter.lookupEntry(cmdlid, nullptr, true, false);
            if (cmdlE)
            {
                hecl::ProjectPath cmdlPath = pakRouter.getWorking(cmdlE);
                os.linkBlend(cmdlPath.getAbsolutePathUTF8().data(),
                             pakRouter.getBestEntryName(*cmdlE).data(), true);

                /* Attach CMDL to CINF */
                os << "if obj.name not in bpy.context.scene.objects:\n"
                      "    bpy.context.scene.objects.link(obj)\n"
                      "obj.parent = arm_obj\n"
                      "obj.parent_type = 'ARMATURE'\n"
                      "attachment.linked_mesh = obj.name\n\n";
            }
        }
    }

    {
        hecl::blender::DataStream ds = conn.beginData();
        std::unordered_map<std::string,
            hecl::blender::Matrix3f> matrices = ds.getBoneMatrices(firstName);
        ds.close();
        DNAANIM::RigInverter<typename ANCSDNA::CINFType> inverter(firstCinf, matrices);

        hecl::blender::PyOutStream os = conn.beginPythonOut(true);
        os << "import bpy\n"
              "actor_data = bpy.context.scene.hecl_sact_data\n";

        /* Get animation primitives */
        std::map<atUint32, AnimationResInfo<typename PAKRouter::IDType>> animResInfo;
        ancs.getAnimationResInfo(&pakRouter, animResInfo);
        for (const auto& id : animResInfo)
        {
            typename ANCSDNA::ANIMType anim;
            if (pakRouter.lookupAndReadDNA(id.second.animId, anim, true))
            {
                os.format("act = bpy.data.actions.new('%s')\n"
                              "act.use_fake_user = True\n", id.second.name.c_str());
                anim.sendANIMToBlender(os, inverter, id.second.additive);
            }

            os.format("actor_action = actor_data.actions.add()\n"
                          "actor_action.name = '%s'\n", id.second.name.c_str());
        }
    }
    conn.saveBlend();
    return true;
}

template bool ReadANCSToBlender<PAKRouter<DNAMP1::PAKBridge>, DNAMP1::ANCS, DNAMP1::MaterialSet, DNACMDL::SurfaceHeader_1, 2>
    (hecl::blender::Connection& conn,
     const DNAMP1::ANCS& ancs,
     const hecl::ProjectPath& outPath,
     PAKRouter<DNAMP1::PAKBridge>& pakRouter,
     const typename PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
     const SpecBase& dataspec,
     std::function<void(const hecl::SystemChar*)> fileChanged,
     bool force);
template bool ReadANCSToBlender<PAKRouter<DNAMP2::PAKBridge>, DNAMP2::ANCS, DNAMP2::MaterialSet, DNACMDL::SurfaceHeader_2, 4>
    (hecl::blender::Connection& conn,
     const DNAMP2::ANCS& ancs,
     const hecl::ProjectPath& outPath,
     PAKRouter<DNAMP2::PAKBridge>& pakRouter,
     const typename PAKRouter<DNAMP2::PAKBridge>::EntryType& entry,
     const SpecBase& dataspec,
     std::function<void(const hecl::SystemChar*)> fileChanged,
     bool force);
template bool ReadANCSToBlender<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::CHAR, DNAMP3::MaterialSet, DNACMDL::SurfaceHeader_3, 4>
    (hecl::blender::Connection& conn,
     const DNAMP3::CHAR& ancs,
     const hecl::ProjectPath& outPath,
     PAKRouter<DNAMP3::PAKBridge>& pakRouter,
     const typename PAKRouter<DNAMP3::PAKBridge>::EntryType& entry,
     const SpecBase& dataspec,
     std::function<void(const hecl::SystemChar*)> fileChanged,
     bool force);

}
