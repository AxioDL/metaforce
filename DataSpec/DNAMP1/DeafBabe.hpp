#ifndef _DNAMP1_DEAFBABE_HPP_
#define _DNAMP1_DEAFBABE_HPP_

#include "../DNACommon/DeafBabe.hpp"

namespace Retro
{
namespace DNAMP1
{

struct DeafBabe : BigDNA
{
    DECL_DNA
    enum class BspNodeType : atUint32
    {
        Invalid,
        Branch,
        Leaf
    };

    struct Material : BigDNA
    {
        DECL_DNA
        Value<atUint32> material;
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
        void setType(Type t) {material &= ~0xff; material |= atUint32(t);}
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
