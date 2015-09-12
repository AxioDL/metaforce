#ifndef _DNAMP2_DEAFBABE_HPP_
#define _DNAMP2_DEAFBABE_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "BlenderConnection.hpp"
#include "../DNAMP1/DeafBabe.hpp"

namespace Retro
{
namespace DNAMP2
{

struct DeafBabe : BigDNA
{
    DECL_DNA
    using BspNodeType = DNAMP1::DeafBabe::BspNodeType;

    struct Material : BigDNA
    {
        DECL_DNA
        Value<atUint64> material;
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
    Value<atUint32> triangleEdges2Count;
    Vector<Triangle, DNA_COUNT(triangleEdges2Count / 3)> triangleEdgeConnections2;
    Value<atUint32> vertCount;
    Vector<atVec3f, DNA_COUNT(vertCount)> verts;

    static void BlenderInit(HECL::BlenderConnection::PyOutStream& os)
    {
        DNAMP1::DeafBabe::BlenderInit(os);
    }
    void sendToBlender(HECL::BlenderConnection::PyOutStream& os) const
    {
        DNAMP1::DeafBabeSendToBlender(os, *this);
    }
};

}
}

#endif // _DNAMP2_DEAFBABE_HPP_
