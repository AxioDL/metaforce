#include <cstddef>
#include "CMDL.hpp"
#include "DNAMP1.hpp"
#include "CMDLMaterials.hpp"

namespace Retro
{
namespace DNAMP1
{

bool CMDL::ReadToBlender(HECL::BlenderConnection& conn,
                         Athena::io::IStreamReader& reader,
                         PAKRouter<PAKBridge>& pakRouter)
{
    return true;
    reader.setEndian(Athena::BigEndian);

    CMDL::Header head;
    head.read(reader);

    if (head.magic != 0xDEADBABE)
    {
        Log.report(LogVisor::Error, "invalid CMDL magic");
        return false;
    }

    if (head.version != 2)
    {
        Log.report(LogVisor::Error, "invalid CMDL version for MP1");
        return false;
    }

    /* Open Py Stream */
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut();
    os << "import bmesh\n"
          "import bpy\n"
          "bm = bmesh.new()\n";
    MaterialSet::RegisterMaterialProps(os);

    for (size_t s=0 ; s<head.secCount ; ++s)
    {
        atUint64 secStart = reader.position();

        /*
        std::unique_ptr<atVec3f[]> vertPos;
        std::unique_ptr<atVec3f[]> vertNorm;
        typedef atInt16 ShortVec3[3];
        std::unique_ptr<ShortVec3[]> vertNormShort;
        std::unique_ptr<atVec2f[]> vertUVs;
        typedef atInt16 ShortVec2[2];
        std::unique_ptr<ShortVec2[]> vertUVsShort;
        */
        std::vector<std::vector<unsigned>> matUVCounts;
        matUVCounts.reserve(head.matSetCount);
        bool visitedDLOffsets = false;
        if (s < head.matSetCount)
        {
            MaterialSet matSet;
            matSet.read(reader);

            /* Texmaps */
            os << "texmap_list = []\n";
            for (const UniqueID32& tex : matSet.head.textureIDs)
            {
                os.format("if '%s' in bpy.data.textures:\n"
                          "    image = bpy.data.images['%s']\n"
                          "    texture = bpy.data.textures[image.name]\n"
                          "else:\n"
                          "    image_path = os.path.relpath('../../%s/textures/%s.png')\n"
                          "    print(os.getcwd()+image_path)\n"
                          "    image = bpy.data.images.load('//' + image_path)\n"
                          "    image.name = '%s'\n"
                          "    texture = bpy.data.textures.new(image.name, 'IMAGE')\n"
                          "    texture.image = image\n"
                          "texmap_list.append(texture)\n"
                          "\n");
            }

            matUVCounts.emplace_back();
            std::vector<unsigned>& uvCounts = matUVCounts.back();
            uvCounts.reserve(matSet.head.materialCount);

            unsigned m=0;
            for (const MaterialSet::Material& mat : matSet.materials)
            {
                uvCounts.emplace_back();
                unsigned& uvCount = uvCounts.back();
                MaterialSet::ConstructMaterial(os, mat, s, m++, uvCount);
            }
        }
        else
        {
            switch (s-head.matSetCount)
            {
            case 0:
            {
                /* Positions */
                size_t vertCount = head.secSizes[s] / 12;
                //vertPos.reset(new atVec3f[vertCount]);
                for (size_t i=0 ; i<vertCount ; ++i)
                {
                    //vertPos[i] = reader.readVec3f();
                    atVec3f pos = reader.readVec3f();
                    os.format("bm.verts.new(co=(%f,%f,%f))\n",
                              pos.vec[0], pos.vec[1], pos.vec[2]);
                }
                break;
            }
            case 1:
            {
                /* Normals */
                os << "normals = []\n";
                if (head.flags.shortNormals())
                {
                    size_t normCount = head.secSizes[s] / 6;
                    //vertNormShort.reset(new ShortVec3[normCount]);
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
                        //vertNormShort[i][0] = reader.readInt16();
                        //vertNormShort[i][1] = reader.readInt16();
                        //vertNormShort[i][2] = reader.readInt16();
                        os.format("normals.append((%f,%f,%f))\n",
                                  reader.readInt16(), reader.readInt16(), reader.readInt16());
                    }
                }
                else
                {
                    size_t normCount = head.secSizes[s] / 12;
                    //vertNorm.reset(new atVec3f[normCount]);
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
                        //vertNorm[i] = reader.readVec3f();
                        atVec3f norm = reader.readVec3f();
                        os.format("normals.append((%f,%f,%f))\n",
                                  norm.vec[0], norm.vec[1], norm.vec[2]);
                    }
                }
                break;
            }
            case 2:
            {
                /* Colors */
                break;
            }
            case 3:
            {
                /* Float UVs */
                os << "uv_list = []\n";
                size_t uvCount = head.secSizes[s] / 8;
                //vertUVs.reset(new atVec2f[uvCount]);
                for (size_t i=0 ; i<uvCount ; ++i)
                {
                    //vertUVs[i] = reader.readVec2f();
                    atVec2f uv = reader.readVec2f();
                    os.format("uv_list.append((%f,%f))\n",
                              uv.vec[0], uv.vec[1]);
                }
                break;
            }
            case 4:
            {
                /* Short UVs */
                os << "suv_list = []\n";
                if (head.flags.shortUVs())
                {
                    size_t uvCount = head.secSizes[s] / 4;
                    //vertUVsShort.reset(new ShortVec2[uvCount]);
                    for (size_t i=0 ; i<uvCount ; ++i)
                    {
                        os.format("suv_list.append((%f,%f))\n",
                                  reader.readInt16(), reader.readInt16());
                    }
                    break;
                }

                /* DL Offsets (here or next section) */
                visitedDLOffsets = true;
                break;
            }
            default:
            {
                if (!visitedDLOffsets)
                {
                    visitedDLOffsets = true;
                    break;
                }

                /* GX Display List (surface) */
                SurfaceHeader sHead;
                sHead.read(reader);
            }
            }
        }

        if (s < head.secCount - 1)
            reader.seek(secStart + head.secSizes[s], Athena::Begin);
    }

    return true;
}

}
}
