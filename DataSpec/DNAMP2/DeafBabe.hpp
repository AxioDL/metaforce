#pragma once

#include "../DNAMP1/DeafBabe.hpp"

namespace DataSpec::DNAMP2 {

struct DeafBabe : BigDNA {
  AT_DECL_DNA
  using BspNodeType = DataSpec::BspNodeType;

  struct Material : BigDNA {
    AT_DECL_DNA
    Value<atUint64> material = 0;
    bool unknown() const { return material & 1; }
    void setUnknown(bool v) {
      material &= ~1;
      material |= atUint64(v);
    }
    bool surfaceStone() const { return (material >> 1ull) & 1; }
    void setSurfaceStone(bool v) {
      material &= ~(1ull << 1ull);
      material |= (atUint64(v) << 1ull);
    }
    bool surfaceMetal() const { return (material >> 2) & 1; }
    void setSurfaceMetal(bool v) {
      material &= ~(1ull << 2ull);
      material |= (atUint64(v) << 2ull);
    }
    bool surfaceGrass() const { return (material >> 3) & 1; }
    void setSurfaceGrass(bool v) {
      material &= ~(1ull << 3ull);
      material |= (atUint64(v) << 3ull);
    }
    bool surfaceIce() const { return (material >> 4ull) & 1; }
    void setSurfaceIce(bool v) {
      material &= ~(1ull << 4ull);
      material |= (atUint64(v) << 4ull);
    }
    bool pillar() const { return (material >> 5ull) & 1; }
    void setPillar(bool v) {
      material &= ~(1ull << 5ull);
      material |= (atUint64(v) << 5ull);
    }
    bool surfaceMetalGrating() const { return (material >> 6ull) & 1; }
    void setSurfaceMetalGrating(bool v) {
      material &= ~(1ull << 6ull);
      material |= (atUint64(v) << 6ull);
    }
    bool surfacePhazon() const { return (material >> 7ull) & 1; }
    void setSurfacePhazon(bool v) {
      material &= ~(1ull << 7ull);
      material |= (atUint64(v) << 7ull);
    }
    bool surfaceDirt() const { return (material >> 8ull) & 1; }
    void setSurfaceDirt(bool v) {
      material &= ~(1ull << 8);
      material |= (atUint64(v) << 8ull);
    }
    bool surfaceSPMetal() const { return (material >> 9ull) & 1; }
    void setSurfaceSPMetal(bool v) {
      material &= ~(1ull << 9ull);
      material |= (atUint64(v) << 9ull);
    }
    bool surfaceGlass() const { return (material >> 10ull) & 1; }
    void setSurfaceGlass(bool v) {
      material &= ~(1ull << 10ull);
      material |= (atUint64(v) << 10ull);
    }
    bool surfaceSnow() const { return (material >> 11ull) & 1; }
    void setSurfaceSnow(bool v) {
      material &= ~(1ull << 11ull);
      material |= (atUint64(v) << 11ull);
    }
    bool surfaceFabric() const { return (material >> 12ull) & 1; }
    void setSurfaceFabric(bool v) {
      material &= ~(1ull << 12ull);
      material |= (atUint64(v) << 12ull);
    }
    bool halfPipe() const { return (material >> 13ull) & 1; }
    void setHalfPipe(bool v) {
      material &= ~(1ull << 13ull);
      material |= (atUint64(v) << 13ull);
    }
    bool unused3() const { return (material >> 14ull) & 1; }
    void setUnused3(bool v) {
      material &= ~(1ull << 14ull);
      material |= (atUint64(v) << 14ull);
    }
    bool unused4() const { return (material >> 15ull) & 1; }
    void setUnused4(bool v) {
      material &= ~(1ull << 15ull);
      material |= (atUint64(v) << 15ull);
    }
    bool surfaceShield() const { return (material >> 16ull) & 1; }
    void setSurfaceShield(bool v) {
      material &= ~(1ull << 16);
      material |= (atUint64(v) << 16ull);
    }
    bool surfaceSand() const { return (material >> 17ull) & 1; }
    void setSurfaceSand(bool v) {
      material &= ~(1ull << 17ull);
      material |= (atUint64(v) << 17ull);
    }
    bool surfaceMothOrSeedOrganics() const { return (material >> 18) & 1; }
    void setSurfaceMothOrSeedOrganics(bool v) {
      material &= ~(1ull << 18);
      material |= (atUint64(v) << 18ull);
    }
    bool surfaceWeb() const { return (material >> 19ull) & 1; }
    void setSurfaceWeb(bool v) {
      material &= ~(1ull << 19ull);
      material |= (atUint64(v) << 19ull);
    }
    bool projectilePassthrough() const { return (material >> 20ull) & 1; }
    void setProjectilePassthrough(bool v) {
      material &= ~(1ull << 20ull);
      material |= (atUint64(v) << 20ull);
    }
    bool cameraPassthrough() const { return (material >> 21ull) & 1; }
    void setCameraPassthrough(bool v) {
      material &= ~(1ull << 21ull);
      material |= (atUint64(v) << 21ull);
    }
    bool surfaceWood() const { return (material >> 22ull) & 1; }
    void setSurfaceWood(bool v) {
      material &= ~(1ull << 22ull);
      material |= (atUint64(v) << 22ull);
    }
    bool surfaceOrganic() const { return (material >> 23ull) & 1; }
    void setSurfaceOrganic(bool v) {
      material &= ~(1ull << 23ull);
      material |= (atUint64(v) << 23ull);
    }
    bool flipFace() const { return (material >> 24ull) & 1; }
    void setFlipFace(bool v) {
      material &= ~(1ull << 24ull);
      material |= (atUint64(v) << 24ull);
    }
    bool surfaceRubber() const { return (material >> 25) & 1; }
    void setSurfaceRubber(bool v) {
      material &= ~(1ull << 25ull);
      material |= (atUint64(v) << 25ull);
    }
    bool seeThrough() const { return (material >> 26ull) & 1; }
    void setSeeThrough(bool v) {
      material &= ~(1ull << 26ull);
      material |= (atUint64(v) << 26ull);
    }
    bool scanPassthrough() const { return (material >> 27ull) & 1; }
    void setScanPassthrough(bool v) {
      material &= ~(1ull << 27ull);
      material |= (atUint64(v) << 27ull);
    }
    bool aiPassthrough() const { return (material >> 28) & 1; }
    void setAiPassthrough(bool v) {
      material &= ~(1ull << 28ull);
      material |= (atUint64(v) << 28ull);
    }
    bool ceiling() const { return (material >> 29ull) & 1; }
    void setCeiling(bool v) {
      material &= ~(1ull << 29ull);
      material |= (atUint64(v) << 29ull);
    }
    bool wall() const { return (material >> 30ull) & 1; }
    void setWall(bool v) {
      material &= ~(1ull << 30ull);
      material |= (atUint64(v) << 30ull);
    }
    bool floor() const { return (material >> 31ull) & 1; }
    void setFloor(bool v) {
      material &= ~(1ull << 31ull);
      material |= (atUint64(v) << 31ull);
    }
    bool aiBlock() const { return (material >> 48ull) & 1; }
    void setAiBlock(bool v) {
      material &= ~(1ull << 48ull);
      material |= (atUint64(v) << 48ull);
    }
    bool jumpNotAllowed() const { return (material >> 58ull) & 1; }
    void setJumpNotAllowed(bool v) {
      material &= ~(1ull << 58ull);
      material |= (atUint64(v) << 58ull);
    }
    bool spiderBall() const { return (material >> 61ull) & 1; }
    void setSpiderBall(bool v) {
      material &= ~(1ull << 61ull);
      material |= (atUint64(v) << 61ull);
    }
    bool screwAttackWallJump() const { return (material >> 62ull) & 1; }
    void setScrewAttackWallJump(bool v) {
      material &= ~(1ull << 62ull);
      material |= (atUint64(v) << 62ull);
    }

    /* Dummies for MP1*/
    bool surfaceLava() const { return false; }
    void setSurfaceLava(bool v) {}
    bool surfaceMudSlow() const { return false; }
    void setSurfaceMudSlow(bool v) {}
    bool surfaceMud() const { return false; }
    void setSurfaceMud(bool v) {}
    bool surfaceStoneRock() const { return false; }
    void setSurfaceLavaStone(bool v) {}
    bool solid() const { return false; }
    void setSolid(bool v) {}
    bool noPlatformCollision() const { return false; }
    void setNoPlatformCollision(bool v) {}
    bool noEdgeCollision() const { return false; }
    void setNoEdgeCollision(bool v) {}
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
  Value<atUint32> noClimbEdgeCount;
  Vector<atInt16, AT_DNA_COUNT(noClimbEdgeCount)> noClimbEdges;
  Value<atUint32> vertCount;
  Vector<atVec3f, AT_DNA_COUNT(vertCount)> verts;

  static void BlenderInit(hecl::blender::PyOutStream& os);
  void insertNoClimb(hecl::blender::PyOutStream& os) const;
  void sendToBlender(hecl::blender::PyOutStream& os) const { DeafBabeSendToBlender(os, *this); }
};

} // namespace DataSpec::DNAMP2
