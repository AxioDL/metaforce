#ifndef _DNAMP1_DEAFBABE_HPP_
#define _DNAMP1_DEAFBABE_HPP_

#include "../DNACommon/DeafBabe.hpp"

namespace DataSpec::DNAMP1
{

struct DeafBabe : BigDNA
{
    AT_DECL_DNA
    using BspNodeType = DataSpec::BspNodeType;

    struct Material : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> material = 0;
        bool unknown() const { return material & 1; }
        void setUnknown(bool v) { material &= ~1; material |= int(v); }
        bool surfaceStone() const { return (material >> 1) & 1; }
        void setSurfaceStone(bool v) { material &= ~(1ull << 1); material |= (v << 1); }
        bool surfaceMetal() const { return (material >> 2) & 1; }
        void setSurfaceMetal(bool v) { material &= ~(1ull << 2); material |= (v << 2); }
        bool surfaceGrass() const { return (material >> 3) & 1; }
        void setSurfaceGrass(bool v) { material &= ~(1ull << 3); material |= (v << 3); }
        bool surfaceIce() const { return (material >> 4) & 1; }
        void setSurfaceIce(bool v) { material &= ~(1ull << 4); material |= (v << 4); }
        bool pillar() const { return (material >> 5) & 1; }
        void setPillar(bool v) { material &= ~(1ull << 5); material |= (v << 5); }
        bool surfaceMetalGrating() const { return (material >> 6) & 1; }
        void setSurfaceMetalGrating(bool v) { material &= ~(1ull << 6); material |= (v << 6); }
        bool surfacePhazon() const { return (material >> 7) & 1; }
        void setSurfacePhazon(bool v) { material &= ~(1ull << 7); material |= (v << 7); }
        bool surfaceDirt() const { return (material >> 8) & 1; }
        void setSurfaceDirt(bool v) { material &= ~(1ull << 8); material |= (v << 8); }
        bool surfaceLava() const {return (material >> 9) & 1; }
        void setSurfaceLava(bool v) { material &= ~(1ull << 9); material |= (v << 9); }
        bool surfaceStoneRock() const { return (material >> 10) & 1; }
        void setSurfaceStoneRock(bool v) { material &= ~(1ull << 10); material |= (v << 10); }
        bool surfaceSnow() const { return (material >> 11) & 1; }
        void setSurfaceSnow(bool v) { material &= ~(1ull << 11); material |= (v << 11); }
        bool surfaceMudSlow() const { return (material >> 12) & 1; }
        void setSurfaceMudSlow(bool v) { material &= ~(1ull << 12); material |= (v << 12); }
        bool halfPipe() const { return (material >> 13) & 1; }
        void setHalfPipe(bool v) { material &= ~(1ull << 13); material |= (v << 13); }
        bool surfaceMud() const { return (material >> 14) & 1; }
        void setSurfaceMud(bool v) { material &= ~(1ull << 14); material |= (v << 14); }
        bool surfaceGlass() const { return (material >> 15) & 1; }
        void setSurfaceGlass(bool v) { material &= ~(1ull << 15); material |= (v << 15); }
        bool surfaceShield() const { return (material >> 16) & 1; }
        void setSurfaceShield(bool v) { material &= ~(1ull << 16); material |= (v << 16); }
        bool surfaceSand() const { return (material >> 17) & 1; }
        void setSurfaceSand(bool v) { material &= ~(1ull << 17); material |= (v << 17); }
        bool projectilePassthrough() const { return (material >> 18) & 1; }
        void setProjectilePassthrough(bool v) { material &= ~(1ull << 18); material |= (v << 18); }
        bool solid() const { return (material >> 19) & 1; }
        void setSolid(bool v) { material &= ~(1ull << 19); material |= (v << 19); }
        bool noPlatformCollision() const { return (material >> 20) & 1; }
        void setNoPlatformCollision(bool v) { material &= ~(1ull << 20); material |= (v << 20); }
        bool cameraPassthrough() const { return (material >> 21) & 1; }
        void setCameraPassthrough(bool v) { material &= ~(1ull << 21); material |= (v << 21); }
        bool surfaceWood() const { return (material >> 22) & 1; }
        void setSurfaceWood(bool v) { material &= ~(1ull << 22); material |= (v << 22); }
        bool surfaceOrganic() const { return (material >> 23) & 1; }
        void setSurfaceOrganic(bool v) { material &= ~(1ull << 23); material |= (v << 23); }
        bool noEdgeCollision() const { return (material >> 24) & 1; }
        void setNoEdgeCollision(bool v) { material &= ~(1ull << 24); material |= (v << 24); }
        bool flipFace() const { return (material >> 25) & 1; }
        void setFlipFace(bool v) { material &= ~(1ull << 25); material |= (v << 25); }
        bool seeThrough() const { return (material >> 26) & 1; }
        void setSeeThrough(bool v) { material &= ~(1ull << 26); material |= (v << 26); }
        bool scanPassthrough() const { return (material >> 27) & 1; }
        void setScanPassthrough(bool v) { material &= ~(1ull << 27); material |= (v << 27); }
        bool aiPassthrough() const { return (material >> 28) & 1; }
        void setAiPassthrough(bool v) { material &= ~(1ull << 28); material |= (v << 28); }
        bool ceiling() const { return (material >> 29) & 1; }
        void setCeiling(bool v) { material &= ~(1ull << 29); material |= (v << 29); }
        bool wall() const { return (material >> 30) & 1; }
        void setWall(bool v) { material &= ~(1ull << 30); material |= (v << 30); }
        bool floor() const { return (material >> 31) & 1; }
        void setFloor(bool v) { material &= ~(1ull << 31); material |= (v << 31); }

        /* Dummies for later games */
        bool surfaceSPMetal() const { return false; }
        void setSurfaceSPMetal(bool v) {  }
        bool surfaceFabric() const { return false; }
        void setSurfaceFabric(bool v) { }
        bool surfaceRubber() const { return false; }
        void setSurfaceRubber(bool v) { }
        bool surfaceMothOrSeedOrganics() const { return false; }
        void setSurfaceMothOrSeedOrganics(bool v) { }
        bool surfaceWeb() const { return false; }
        void setSurfaceWeb(bool v) { }
        bool unused3() const  { return false; }
        void setUnused3(bool v) {}
        bool unused4() const  { return false; }
        void setUnused4(bool v) {}
        bool aiBlock() const { return false; }
        void setAiBlock(bool v) { }
        bool jumpNotAllowed() const { return false; }
        void setJumpNotAllowed(bool v) {  }
        bool spiderBall() const { return false; }
        void setSpiderBall(bool v) { }
        bool screwAttackWallJump() const { return false; }
        void setScrewAttackWallJump(bool v) { }
    };

    struct Edge : BigDNA
    {
        AT_DECL_DNA
        Value<atUint16> verts[2];
    };

    struct Triangle : BigDNA
    {
        AT_DECL_DNA
        Value<atUint16> edges[3];
    };

    Value<atUint32> unk1;
    Value<atUint32> length;
    Value<atUint32> magic;
    Value<atUint32> version;
    Value<atVec3f> aabb[2];
    Value<BspNodeType> rootNodeType;
    Value<atUint32> bspSize;
    Buffer<AT_DNA_COUNT(bspSize)> bspTree;
    Value<atUint32> materialCount;
    Vector<Material, AT_DNA_COUNT(materialCount)> materials;
    Value<atUint32> vertMatsCount;
    Vector<atUint8, AT_DNA_COUNT(vertMatsCount)> vertMats;
    Value<atUint32> edgeMatsCount;
    Vector<atUint8, AT_DNA_COUNT(edgeMatsCount)> edgeMats;
    Value<atUint32> triMatsCount;
    Vector<atUint8, AT_DNA_COUNT(triMatsCount)> triMats;
    Value<atUint32> edgeVertsCount;
    Vector<Edge, AT_DNA_COUNT(edgeVertsCount)> edgeVertConnections;
    Value<atUint32> triangleEdgesCount;
    Vector<Triangle, AT_DNA_COUNT(triangleEdgesCount / 3)> triangleEdgeConnections;
    Value<atUint32> vertCount;
    Vector<atVec3f, AT_DNA_COUNT(vertCount)> verts;

    /* Dummy MP2 member */
    void insertNoClimb(hecl::blender::PyOutStream&) const {}

    static void BlenderInit(hecl::blender::PyOutStream& os);
    void sendToBlender(hecl::blender::PyOutStream& os) const
    {
        DeafBabeSendToBlender(os, *this);
    }
};

}

#endif // _DNAMP1_DEAFBABE_HPP_
