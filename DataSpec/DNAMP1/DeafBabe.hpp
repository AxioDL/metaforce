#ifndef _DNAMP1_DEAFBABE_HPP_
#define _DNAMP1_DEAFBABE_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "BlenderConnection.hpp"

namespace Retro
{
namespace DNAMP1
{

template<class DEAFBABE>
void DeafBabeSendToBlender(HECL::BlenderConnection::PyOutStream& os, const DEAFBABE& db)
{
    os << "material_index = []\n"
          "col_bm = bmesh.new()\n";
    for (const atVec3f& vert : db.verts)
        os.format("col_bm.verts.new((%f,%f,%f))\n",
                  vert.vec[0],
                  vert.vec[1],
                  vert.vec[2]);

    os << "col_bm.verts.ensure_lookup_table()\n";

    int triIdx = 0;
    for (const typename DEAFBABE::Triangle& tri : db.triangleEdgeConnections)
    {
        const typename DEAFBABE::Material& triMat = db.materials[db.triMats[triIdx++]];
        const typename DEAFBABE::Edge& edge0 = db.edgeVertConnections[tri.edges[0]];
        const typename DEAFBABE::Edge& edge1 = db.edgeVertConnections[tri.edges[1]];
        const typename DEAFBABE::Edge& edge2 = db.edgeVertConnections[tri.edges[2]];
        if (!edge0.verts[0] && !edge1.verts[0] && !edge2.verts[0])
            break;
        os << "tri_verts = []\n";
        os.format("tri_verts.append(col_bm.verts[%u])\n", edge0.findCommon(edge1));
        os.format("tri_verts.append(col_bm.verts[%u])\n", edge1.findCommon(edge2));
        os.format("tri_verts.append(col_bm.verts[%u])\n", edge2.findCommon(edge0));

        os.format("face = col_bm.faces.get(tri_verts)\n"
                  "if face is None:\n"
                  "    face = col_bm.faces.new(tri_verts)\n"
                  "else:\n"
                  "    face = face.copy()\n"
                  "    for i in range(3):\n"
                  "        face.verts[i].co = tri_verts[i].co\n"
                  "    col_bm.verts.ensure_lookup_table()\n"
                  "face.material_index = select_material(0x%08X)\n"
                  "face.smooth = False\n"
                  "\n",
                  atUint32(triMat.material));
    }

    db.insertNoClimb(os);

    os << "col_mesh = bpy.data.meshes.new('CMESH')\n"
          "col_bm.to_mesh(col_mesh)\n"
          "col_mesh_obj = bpy.data.objects.new(col_mesh.name, col_mesh)\n"
          "\n"
          "for mat_name in material_index:\n"
          "    mat = material_dict[mat_name]\n"
          "    col_mesh.materials.append(mat)\n"
          "\n"
          "bpy.context.scene.objects.link(col_mesh_obj)\n"
          "col_mesh_obj.layers[1] = True\n"
          "col_mesh_obj.layers[0] = False\n"
          "col_mesh_obj.draw_type = 'SOLID'\n"
          "col_mesh_obj.game.physics_type = 'STATIC'\n"
          "\n";
}

struct DeafBabe : BigDNA
{
    DECL_DNA
    enum BspNodeType : atUint32
    {
        BspNodeInvalid,
        BspNodeBranch,
        BspNodeLeaf
    };

    struct Material : BigDNA
    {
        DECL_DNA
        Value<atUint32> material;
        bool fireThrough() const {return material >> 18 & 0x1;}
        void setFireThrough(bool v) {material &= 0x40000; material |= v << 18;}

        enum Type
        {
            Mat0,
            MatGround,
            Mat2,
            MatStone,
            MatHardStone,
            MatMetal,
            Mat6,
            Mat7,
            Mat8,
            MatLeaves
        };
        Type type() const {return Type(material & 0xff);}
        void setType(Type t) {material &= 0xff; material |= t;}
    };

    struct Edge : BigDNA
    {
        DECL_DNA
        Value<atUint16> verts[2];
        atUint16 findCommon(const Edge& other) const
        {
            if (verts[0] == other.verts[0] || verts[0] == other.verts[1])
                return verts[0];
            if (verts[1] == other.verts[0] || verts[1] == other.verts[1])
                return verts[1];
            return -1;
        }
    };

    struct Triangle : BigDNA
    {
        DECL_DNA
        Value<atUint16> edges[3];
    };

    Value<atUint32> unk1;
    Value<atUint32> length;
    Value<atUint32> magic;
    Value<atUint32> version;
    Value<atVec3f> aabb[2];
    Value<BspNodeType> rootNodeType;
    Value<atUint32> bspSize;
    Buffer<DNA_COUNT(bspSize)> bspTree;
    Value<atUint32> materialCount;
    Vector<Material, DNA_COUNT(materialCount)> materials;
    Value<atUint32> vertMatsCount;
    Vector<atUint8, DNA_COUNT(vertMatsCount)> vertMats;
    Value<atUint32> edgeMatsCount;
    Vector<atUint8, DNA_COUNT(edgeMatsCount)> edgeMats;
    Value<atUint32> triMatsCount;
    Vector<atUint8, DNA_COUNT(triMatsCount)> triMats;
    Value<atUint32> edgeVertsCount;
    Vector<Edge, DNA_COUNT(edgeVertsCount)> edgeVertConnections;
    Value<atUint32> triangleEdgesCount;
    Vector<Triangle, DNA_COUNT(triangleEdgesCount / 3)> triangleEdgeConnections;
    Value<atUint32> vertCount;
    Vector<atVec3f, DNA_COUNT(vertCount)> verts;

    /* Dummy MP2 member */
    void insertNoClimb(HECL::BlenderConnection::PyOutStream&) const {}

    static void BlenderInit(HECL::BlenderConnection::PyOutStream& os);
    void sendToBlender(HECL::BlenderConnection::PyOutStream& os) const
    {
        DeafBabeSendToBlender(os, *this);
    }
};

}
}

#endif // _DNAMP1_DEAFBABE_HPP_
