#ifndef _DNAMP2_DEAFBABE_HPP_
#define _DNAMP2_DEAFBABE_HPP_

#include "../DNAMP1/DeafBabe.hpp"

namespace DataSpec
{
namespace DNAMP2
{

struct DeafBabe : BigDNA
{
    DECL_DNA
    using BspNodeType = DataSpec::BspNodeType;

    struct Material : BigDNA
    {
        DECL_DNA
        Value<atUint64> material;
        bool fireThrough() const {return material >> 18 & 0x1;}
        void setFireThrough(bool v) {material &= ~0x40000; material |= v << 18;}

        enum class Type
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
        void setType(Type t) {material &= ~0xff; material |= atUint64(t);}
    };

    using Edge = DNAMP1::DeafBabe::Edge;
    using Triangle = DNAMP1::DeafBabe::Triangle;

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
    Value<atUint32> noClimbEdgeCount;
    Vector<atInt16, DNA_COUNT(noClimbEdgeCount)> noClimbEdges;
    Value<atUint32> vertCount;
    Vector<atVec3f, DNA_COUNT(vertCount)> verts;

    static void BlenderInit(hecl::BlenderConnection::PyOutStream& os)
    {
        DNAMP1::DeafBabe::BlenderInit(os);
    }
    void insertNoClimb(hecl::BlenderConnection::PyOutStream& os) const
    {
        for (atInt16 edgeIdx : noClimbEdges)
        {
            if (edgeIdx == -1)
                continue;
            const Edge& edge = edgeVertConnections[edgeIdx];
            os.format("edge = col_bm.edges.get((col_bm.verts[%u], col_bm.verts[%u]))\n"
                      "if edge:\n"
                      "    edge.seam = True\n",
                      edge.verts[0], edge.verts[1]);
        }
    }
    void sendToBlender(hecl::BlenderConnection::PyOutStream& os) const
    {
        DeafBabeSendToBlender(os, *this);
    }
};

}
}

#endif // _DNAMP2_DEAFBABE_HPP_
