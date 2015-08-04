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
                         PAKRouter<PAKBridge>& pakRouter,
                         const PAK::Entry& entry)
{
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
    os << "import bpy\n"
          "import bmesh\n"
          "\n"
          "bpy.context.scene.name = '%s'\n"
          "bpy.context.scene.rwk_type = 'MESH'\n"
          "bpy.context.scene.rwk_mesh = bpy.context.scene.name\n"
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
          "# Property to convey original vert indices in overdraw meshes\n"
          "class RWKOriginalIndex(bpy.types.PropertyGroup):\n"
          "    index = bpy.props.IntProperty(name='Original Vertex Index')\n"
          "bpy.utils.register_class(RWKOriginalIndex)\n"
          "bpy.types.Mesh.rwk_orig_verts = bpy.props.CollectionProperty(type=RWKOriginalIndex)\n"
          "\n"
          "def loop_from_facevert(face, vert_idx):\n"
          "    for loop in face.loops:\n"
          "        if loop.vert.index == vert_idx:\n"
          "            return loop\n"
          "\n"
          "def add_triangle(bm, vert_seq, vert_indices, norm_seq, norm_indices, mat_nr, od_list):\n"
          "    if len(set(vert_indices)) != 3:\n"
          "        return None, None\n"
          "\n"
          "    ret_mesh = bm\n"
          "    vert_seq.ensure_lookup_table()\n"
          "    verts = [vert_seq[i] for i in vert_indices]\n"
          "    norms = [norm_seq[i] for i in norm_indices]\n"
          "\n"
          "    # Make the face\n"
          "    face = bm.faces.get(verts)\n"
          "\n"
          "    if face is not None and face.material_index != mat_nr: # Same poly, new material\n"
          "        # Overdraw detected; track copy\n"
          "        od_entry = None\n"
          "        for entry in od_list:\n"
          "            if entry['material'] == mat_nr:\n"
          "                od_entry = entry\n"
          "        if od_entry is None:\n"
          "            bm_cpy = bm.copy()\n"
          "            od_entry = {'material':mat_nr, 'bm':bm_cpy}\n"
          "            bmesh.ops.delete(od_entry['bm'], geom=od_entry['bm'].faces, context=3)\n"
          "            od_list.append(od_entry)\n"
          "        od_entry['bm'].verts.ensure_lookup_table()\n"
          "        verts = [od_entry['bm'].verts[i] for i in vert_indices]\n"
          "        face = od_entry['bm'].faces.get(verts)\n"
          "        if face is None:\n"
          "            face = od_entry['bm'].faces.new(verts)\n"
          "        else: # Probably a double-sided surface\n"
          "            face = face.copy()\n"
          "            face.normal_flip()\n"
          "        ret_mesh = od_entry['bm']\n"
          "\n"
          "    elif face is not None: # Same material, probably double-sided\n"
          "        face = face.copy()\n"
          "        face.normal_flip()\n"
          "\n"
          "    else: \n"
          "        face = bm.faces.new(verts)\n"
          "\n"
          "    # Apply normals\n"
          "    for i in range(3):\n"
          "        verts[i].normal = norms[i]\n"
          "\n"
          "    for i in range(3):\n"
          "        face.verts[i].index = vert_indices[i]\n"
          "    face.material_index = mat_nr\n"
          "    face.smooth = True\n"
          "\n"
          "    return face, ret_mesh\n"
          "\n"
          "# Begin bmesh\n"
          "bm = bmesh.new()\n"
          "\n";

    /* Link master shader library */
    HECL::ProjectPath selfPath = pakRouter.getWorking(&entry, PAKBridge::LookupExtractor(entry));
    std::string masterShaderPath;
    for (int i=0 ; i<selfPath.levelCount() ; ++i)
        masterShaderPath += "../";
    masterShaderPath += ".hecl/RetroMasterShader.blend";
    os.format("# Master shader library\n"
              "with bpy.data.libraries.load('//%s', link=True, relative=True) as (data_from, data_to):\n"
              "    data_to.node_groups = data_from.node_groups\n"
              "\n", masterShaderPath.c_str());

    MaterialSet::RegisterMaterialProps(os);

    for (size_t s=0 ; s<head.secCount ; ++s)
    {
        atUint64 secStart = reader.position();
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
                std::string texName = pakRouter.getBestEntryName(tex);
                HECL::SystemString resPath = pakRouter.getResourceRelativePath(entry, tex);
                HECL::SystemUTF8View resPathView(resPath);
                os.format("if '%s' in bpy.data.textures:\n"
                          "    image = bpy.data.images['%s']\n"
                          "    texture = bpy.data.textures[image.name]\n"
                          "else:\n"
                          "    image = bpy.data.images.load('//%s')\n"
                          "    image.name = '%s'\n"
                          "    texture = bpy.data.textures.new(image.name, 'IMAGE')\n"
                          "    texture.image = image\n"
                          "texmap_list.append(texture)\n"
                          "\n", texName.c_str(), texName.c_str(),
                          resPathView.str().c_str(), texName.c_str());
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
                for (size_t i=0 ; i<vertCount ; ++i)
                {
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
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
                        os.format("normals.append((%f,%f,%f))\n",
                                  reader.readInt16(), reader.readInt16(), reader.readInt16());
                    }
                }
                else
                {
                    size_t normCount = head.secSizes[s] / 12;
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
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
                for (size_t i=0 ; i<uvCount ; ++i)
                {
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
