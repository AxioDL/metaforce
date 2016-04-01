#ifndef _DNAMP1_CMDL_HPP_
#define _DNAMP1_CMDL_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/CMDL.hpp"
#include "CMDLMaterials.hpp"
#include "DNAMP1.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"

#include <athena/FileReader.hpp>

namespace DataSpec
{
namespace DNAMP1
{

struct CMDL
{
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        hecl::BlenderToken& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        /* Check for RigPair */
        const PAKRouter<PAKBridge>::RigPair* rp = pakRouter.lookupCMDLRigPair(entry.id);
        CINF cinf;
        CSKR cskr;
        std::pair<CSKR*,CINF*> loadRp(nullptr, nullptr);
        if (rp)
        {
            pakRouter.lookupAndReadDNA(rp->first, cskr);
            pakRouter.lookupAndReadDNA(rp->second, cinf);
            loadRp.first = &cskr;
            loadRp.second = &cinf;
        }

        /* Do extract */
        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        if (!conn.createBlend(outPath, hecl::BlenderConnection::BlendType::Mesh))
            return false;
        DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, std::pair<CSKR*,CINF*>, DNACMDL::SurfaceHeader_1, 2>
                (conn, rs, pakRouter, entry, dataSpec, loadRp);
        conn.saveBlend();

#if 0
        /* Cook and re-extract test */
        hecl::ProjectPath tempOut = outPath.getWithExtension(_S(".recook"), true);
        hecl::BlenderConnection::DataStream ds = conn.beginData();
        DNACMDL::Mesh mesh = ds.compileMesh(hecl::TopologyTriStrips, -1);
        ds.close();
        DNACMDL::WriteCMDL<MaterialSet, DNACMDL::SurfaceHeader_1_2, 2>(tempOut, outPath, mesh);

        athena::io::FileReader reader(tempOut.getAbsolutePath());
        hecl::ProjectPath tempBlend = outPath.getWithExtension(_S(".recook.blend"), true);
        if (!conn.createBlend(tempBlend, hecl::BlenderConnection::TypeMesh))
            return false;
        DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, std::pair<CSKR*,CINF*>, DNACMDL::SurfaceHeader_1_2, 2>
                (conn, reader, pakRouter, entry, dataSpec, loadRp);
        return conn.saveBlend();
#elif 0
        /* HMDL cook test */
        hecl::ProjectPath tempOut = outPath.getWithExtension(_S(".recook"), true);
        hecl::BlenderConnection::DataStream ds = conn.beginData();
        DNACMDL::Mesh mesh = ds.compileMesh(hecl::HMDLTopology::TriStrips, 16);
        ds.close();
        DNACMDL::WriteHMDLCMDL<HMDLMaterialSet, DNACMDL::SurfaceHeader_1, 2>(tempOut, outPath, mesh);
#endif

        return true;
    }

    static void Name(const SpecBase& dataSpec,
                     PAKEntryReadStream& rs,
                     PAKRouter<PAKBridge>& pakRouter,
                     PAK::Entry& entry)
    {
        DNACMDL::NameCMDL<PAKRouter<PAKBridge>, MaterialSet>(rs, pakRouter, entry, dataSpec);
    }

    static bool Cook(const hecl::ProjectPath& outPath,
                     const hecl::ProjectPath& inPath,
                     const DNACMDL::Mesh& mesh)
    {
        if (mesh.skins.size())
        {
            DNACMDL::Mesh skinMesh = mesh.getContiguousSkinningVersion();
            if (!DNACMDL::WriteCMDL<MaterialSet, DNACMDL::SurfaceHeader_1, 2>(outPath, inPath, skinMesh))
                return false;

            /* Output skinning intermediate */
            auto vertCountIt = skinMesh.contiguousSkinVertCounts.cbegin();
            athena::io::FileWriter writer(outPath.getWithExtension(_S(".skin")).getAbsolutePath());
            writer.writeUint32Big(skinMesh.skins.size());
            for (const std::vector<DNACMDL::Mesh::SkinBind> skin : skinMesh.skins)
            {
                writer.writeUint32Big(skin.size());
                for (const DNACMDL::Mesh::SkinBind& bind : skin)
                {
                    writer.writeUint32Big(bind.boneIdx);
                    writer.writeFloatBig(bind.weight);
                }
                writer.writeUint32Big(*vertCountIt++);
            }
            writer.writeUint32Big(skinMesh.pos.size());
            writer.writeUint32Big(skinMesh.boneNames.size());
            for (const std::string& boneName : skinMesh.boneNames)
                writer.writeString(boneName);
        }
        else if (!DNACMDL::WriteCMDL<MaterialSet, DNACMDL::SurfaceHeader_1, 2>(outPath, inPath, mesh))
            return false;
        return true;
    }

    static bool HMDLCook(const hecl::ProjectPath& outPath,
                         const hecl::ProjectPath& inPath,
                         const DNACMDL::Mesh& mesh)
    {
        if (mesh.skins.size())
        {
            if (!DNACMDL::WriteHMDLCMDL<HMDLMaterialSet, DNACMDL::SurfaceHeader_1, 2>(outPath, inPath, mesh))
                return false;

            /* Output skinning intermediate */
            athena::io::FileWriter writer(outPath.getWithExtension(_S(".skin")).getAbsolutePath());
            writer.writeUint32Big(mesh.skinBanks.banks.size());
            for (const DNACMDL::Mesh::SkinBanks::Bank& sb : mesh.skinBanks.banks)
            {
                writer.writeUint32Big(sb.m_boneIdxs.size());
                for (uint32_t bind : sb.m_boneIdxs)
                    writer.writeUint32Big(bind);
            }
            writer.writeUint32Big(mesh.boneNames.size());
            for (const std::string& boneName : mesh.boneNames)
                writer.writeString(boneName);
        }
        else if (!DNACMDL::WriteHMDLCMDL<HMDLMaterialSet, DNACMDL::SurfaceHeader_1, 2>(outPath, inPath, mesh))
            return false;
        return true;
    }
};

}
}

#endif // _DNAMP1_CMDL_HPP_
