#include "MAPA.hpp"
#include "../DNAMP1/DNAMP1.hpp"
#include "../DNAMP2/DNAMP2.hpp"
#include "../DNAMP3/DNAMP3.hpp"
#include "zeus/CTransform.hpp"

namespace DataSpec
{
namespace DNAMAPA
{

void MAPA::read(athena::io::IStreamReader& __dna_reader)
{
    /* magic */
    magic = __dna_reader.readUint32Big();
    if (magic != 0xDEADD00D)
    {
        LogDNACommon.report(logvisor::Error, "invalid MAPA magic");
        return;
    }
    /* version */
    version = __dna_reader.readUint32Big();
    if (version == 2)
        header.reset(new HeaderMP1);
    else if (version == 3)
        header.reset(new HeaderMP2);
    else if (version == 5)
        header.reset(new HeaderMP3);
    else
    {
        LogDNACommon.report(logvisor::Error, "invalid MAPA version");
        return;
    }

    header->read(__dna_reader);

    for (atUint32 i = 0; i < header->mappableObjectCount(); i++)
    {
        std::unique_ptr<IMappableObject> mo = nullptr;
        if (version != 5)
            mo.reset(new MappableObjectMP1_2);
        else
            mo.reset(new MappableObjectMP3);
        mo->read(__dna_reader);
        mappableObjects.push_back(std::move(mo));
    }

    /* vertices */
    __dna_reader.enumerateBig(vertices, header->vertexCount());
    /* surfaceHeaders */
    __dna_reader.enumerate(surfaceHeaders, header->surfaceCount());
    /* surfaces */
    __dna_reader.enumerate(surfaces, header->surfaceCount());
}

void MAPA::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* magic */
    __dna_writer.writeUint32Big(magic);
    /* version */
    __dna_writer.writeUint32Big(version);
    header->write(__dna_writer);

    /* mappableObjects */
    for (const std::unique_ptr<IMappableObject>& mo : mappableObjects)
        mo->write(__dna_writer);
    /* vertices */
    __dna_writer.enumerateBig(vertices);
    /* surfaceHeaders */
    __dna_writer.enumerate(surfaceHeaders);
    /* surfaces */
    __dna_writer.enumerate(surfaces);
}

size_t MAPA::binarySize(size_t __isz) const
{
    __isz = header->binarySize(__isz);

    for (const std::unique_ptr<IMappableObject>& mo : mappableObjects)
        __isz = mo->binarySize(__isz);

    __isz += vertices.size() * 12;
    __isz = __EnumerateSize(__isz, surfaceHeaders);
    __isz = __EnumerateSize(__isz, surfaces);
    return __isz + 8;
}

template <typename PAKRouter>
bool ReadMAPAToBlender(hecl::BlenderConnection& conn,
                       const MAPA& mapa,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force)
{
    if (!force && outPath.isFile())
        return true;

    if (!conn.createBlend(outPath, hecl::BlenderConnection::BlendType::MapArea))
        return false;
    hecl::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);

    os << "import bpy, bmesh\n"
          "from mathutils import Matrix\n"
          "\n"
          "bpy.types.Object.retro_mappable_type = bpy.props.IntProperty(name='Retro: MAPA object type', default=-1)\n"
          "bpy.types.Object.retro_mappable_unk = bpy.props.IntProperty(name='Retro: MAPA object unk')\n"
          "bpy.types.Object.retro_mappable_sclyid = bpy.props.StringProperty(name='Retro: MAPA object SCLY ID')\n"
          "\n"
          "for ar in bpy.context.screen.areas:\n"
          "    for sp in ar.spaces:\n"
          "        if sp.type == 'VIEW_3D':\n"
          "            sp.viewport_shade = 'SOLID'\n"
          "\n"
          "# Clear Scene\n"
          "for ob in bpy.data.objects:\n"
          "    if ob.type != 'CAMERA':\n"
          "        bpy.context.scene.objects.unlink(ob)\n"
          "        bpy.data.objects.remove(ob)\n"
          "\n"
          "def add_triangle(bm, verts):\n"
          "    verts = [bm.verts[vi] for vi in verts]\n"
          "    face = bm.faces.get(verts)\n"
          "    if face:\n"
          "        face = face.copy()\n"
          "        bm.verts.ensure_lookup_table()\n"
          "        face.normal_flip()\n"
          "    else:\n"
          "        bm.faces.new(verts)\n"
          "\n"
          "def add_border(bm, verts):\n"
          "    verts = [bm.verts[vi] for vi in verts]\n"
          "    edge = bm.edges.get(verts)\n"
          "    if not edge:\n"
          "        edge = bm.edges.new(verts)\n"
          "    edge.seam = True\n"
          "\n";

    os.format("bpy.context.scene.name = 'MAPA_%s'\n",
              entry.id.toString().c_str());

    /* Add empties representing MappableObjects */
    int moIdx = 0;
    for (const std::unique_ptr<MAPA::IMappableObject>& mo : mapa.mappableObjects)
    {
        const MAPA::MappableObjectMP1_2* moMP12 = dynamic_cast<const MAPA::MappableObjectMP1_2*>(mo.get());
        if (moMP12)
        {
            os.format("obj = bpy.data.objects.new('MAPOBJ_%02d', None)\n"
                      "bpy.context.scene.objects.link(obj)\n"
                      "obj.retro_mappable_type = %d\n"
                      "obj.retro_mappable_unk = %d\n"
                      "obj.retro_mappable_sclyid = '%08X'\n"
                      "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
                      "mtxd = mtx.decompose()\n"
                      "obj.rotation_mode = 'QUATERNION'\n"
                      "obj.location = mtxd[0]\n"
                      "obj.rotation_quaternion = mtxd[1]\n"
                      "obj.scale = mtxd[2]\n",
                      moIdx, moMP12->type, moMP12->unknown1, moMP12->sclyId,
                      moMP12->transformMtx[0].vec[0], moMP12->transformMtx[0].vec[1], moMP12->transformMtx[0].vec[2], moMP12->transformMtx[0].vec[3],
                      moMP12->transformMtx[1].vec[0], moMP12->transformMtx[1].vec[1], moMP12->transformMtx[1].vec[2], moMP12->transformMtx[1].vec[3],
                      moMP12->transformMtx[2].vec[0], moMP12->transformMtx[2].vec[1], moMP12->transformMtx[2].vec[2], moMP12->transformMtx[2].vec[3]);
            ++moIdx;
            continue;
        }
        const MAPA::MappableObjectMP3* moMP3 = dynamic_cast<const MAPA::MappableObjectMP3*>(mo.get());
        if (moMP3)
        {
            os.format("obj = bpy.data.objects.new('MAPOBJ_%02d', None)\n"
                      "bpy.context.scene.objects.link(obj)\n"
                      "obj.retro_mappable_type = %d\n"
                      "obj.retro_mappable_unk = %d\n"
                      "obj.retro_mappable_sclyid = '%08X'\n"
                      "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
                      "mtxd = mtx.decompose()\n"
                      "obj.rotation_mode = 'QUATERNION'\n"
                      "obj.location = mtxd[0]\n"
                      "obj.rotation_quaternion = mtxd[1]\n"
                      "obj.scale = mtxd[2]\n",
                      moIdx, moMP3->type, moMP3->unknown1, moMP3->sclyId,
                      moMP3->transformMtx[0].vec[0], moMP3->transformMtx[0].vec[1], moMP3->transformMtx[0].vec[2], moMP3->transformMtx[0].vec[3],
                      moMP3->transformMtx[1].vec[0], moMP3->transformMtx[1].vec[1], moMP3->transformMtx[1].vec[2], moMP3->transformMtx[1].vec[3],
                      moMP3->transformMtx[2].vec[0], moMP3->transformMtx[2].vec[1], moMP3->transformMtx[2].vec[2], moMP3->transformMtx[2].vec[3]);
            ++moIdx;
            continue;
        }
    }

    os << "# Begin bmesh\n"
          "bm = bmesh.new()\n"
          "\n";

    /* Read in verts */
    for (const atVec3f& vert : mapa.vertices)
        os.format("bm.verts.new((%f,%f,%f))\n",
                  vert.vec[0], vert.vec[1], vert.vec[2]);
    os << "bm.verts.ensure_lookup_table()\n";

    /* Read in surfaces */
    for (const typename MAPA::Surface& surf : mapa.surfaces)
    {
        for (const typename MAPA::Surface::Primitive& prim : surf.primitives)
        {
            auto iit = prim.indices.cbegin();

            /* 3 Prim Verts to start */
            int c = 0;
            unsigned int primVerts[3] =
            {
                *iit++,
                *iit++,
                *iit++
            };

            if (GX::Primitive(prim.type) == GX::TRIANGLESTRIP)
            {
                atUint8 flip = 0;
                for (size_t v=0 ; v<prim.indexCount-2 ; ++v)
                {
                    if (flip)
                    {
                        os.format("add_triangle(bm, (%u,%u,%u))\n",
                                  primVerts[c%3],
                                  primVerts[(c+2)%3],
                                  primVerts[(c+1)%3]);
                    }
                    else
                    {
                        os.format("add_triangle(bm, (%u,%u,%u))\n",
                                  primVerts[c%3],
                                  primVerts[(c+1)%3],
                                  primVerts[(c+2)%3]);
                    }
                    flip ^= 1;

                    /* Break if done */
                    if (iit == prim.indices.cend())
                        break;

                    bool peek = (v >= prim.indexCount - 3);

                    /* Advance one prim vert */
                    if (peek)
                        primVerts[c%3] = *iit;
                    else
                        primVerts[c%3] = *iit++;
                    ++c;
                }
            }
            else if (GX::Primitive(prim.type) == GX::TRIANGLES)
            {
                for (size_t v=0 ; v<prim.indexCount ; v+=3)
                {
                    os.format("add_triangle(bm, (%u,%u,%u))\n",
                              primVerts[0],
                              primVerts[1],
                              primVerts[2]);

                    /* Break if done */
                    if (v+3 >= prim.indexCount)
                        break;

                    /* Advance 3 Prim Verts */
                    for (int pv=0 ; pv<3 ; ++pv)
                        primVerts[pv] = *iit++;
                }
            }
        }

        for (const typename MAPA::Surface::Border& border : surf.borders)
        {
            auto iit = border.indices.cbegin();
            for (size_t i=0 ; i<border.indexCount-1 ; ++i)
            {
                os.format("add_border(bm, (%u,%u))\n",
                          *iit, *(iit+1));
                ++iit;
            }
        }
    }


    os << "mesh = bpy.data.meshes.new('MAP')\n"
          "obj = bpy.data.objects.new(mesh.name, mesh)\n"
          "bm.to_mesh(mesh)\n"
          "bpy.context.scene.objects.link(obj)\n"
          "bm.free()\n";

    const zeus::CMatrix4f* tmpMtx = pakRouter.lookupMAPATransform(entry.id);
    const zeus::CMatrix4f& mtx = tmpMtx ? *tmpMtx : zeus::CMatrix4f::skIdentityMatrix4f;
    os.format("mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
              "mtxd = mtx.decompose()\n"
              "obj.rotation_mode = 'QUATERNION'\n"
              "obj.location = mtxd[0]\n"
              "obj.rotation_quaternion = mtxd[1]\n"
              "obj.scale = mtxd[2]\n",
              mtx[0][0], mtx[1][0], mtx[2][0], mtx[3][0],
              mtx[0][1], mtx[1][1], mtx[2][1], mtx[3][1],
              mtx[0][2], mtx[1][2], mtx[2][2], mtx[3][2]);

    /* World background */
    hecl::ProjectPath worldBlend(outPath.getParentPath().getParentPath(), "!world.blend");
    if (worldBlend.isFile())
        os.linkBackground("//../!world.blend", "World");

    os.centerView();
    os.close();
    conn.saveBlend();
    return true;
}

template bool ReadMAPAToBlender<PAKRouter<DNAMP1::PAKBridge>>
(hecl::BlenderConnection& conn,
 const MAPA& mapa,
 const hecl::ProjectPath& outPath,
 PAKRouter<DNAMP1::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
 bool force);

template bool ReadMAPAToBlender<PAKRouter<DNAMP2::PAKBridge>>
(hecl::BlenderConnection& conn,
 const MAPA& mapa,
 const hecl::ProjectPath& outPath,
 PAKRouter<DNAMP2::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP2::PAKBridge>::EntryType& entry,
 bool force);

template bool ReadMAPAToBlender<PAKRouter<DNAMP3::PAKBridge>>
(hecl::BlenderConnection& conn,
 const MAPA& mapa,
 const hecl::ProjectPath& outPath,
 PAKRouter<DNAMP3::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry,
 bool force);

}
}
