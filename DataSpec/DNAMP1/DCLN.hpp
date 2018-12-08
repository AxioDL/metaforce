#pragma once

#include "athena/Types.hpp"
#include "DataSpec/DNACommon/DeafBabe.hpp"
#include "DataSpec/DNACommon/PAK.hpp"
#include "DataSpec/DNACommon/OBBTreeBuilder.hpp"
#include "DNAMP1.hpp"
#include "DeafBabe.hpp"

#define DCLN_DUMP_OBB 0

namespace DataSpec::DNAMP1 {

struct DCLN : BigDNA {
  using Mesh = hecl::blender::ColMesh;

  AT_DECL_DNA
  Value<atUint32> colCount;
  struct Collision : BigDNA {
    using Material = DeafBabe::Material;
    using Edge = DeafBabe::Edge;
    using Triangle = DeafBabe::Triangle;

    AT_DECL_DNA
    Value<atUint32> magic;
    Value<atUint32> version;
    Value<atUint32> memSize;
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

    struct Node : BigDNA {
      AT_DECL_EXPLICIT_DNA

      struct LeafData : BigDNA {
        AT_DECL_DNA
        Value<atUint32> triangleIndexCount;
        Vector<atUint16, AT_DNA_COUNT(triangleIndexCount)> triangleIndices;
        size_t getMemoryUsage() const { return (((triangleIndices.size() * 2) + 16) + 3) & ~3; }
      };

      Value<atVec4f> xf[3];
      Value<atVec3f> halfExtent;
      Value<bool> isLeaf;
      std::unique_ptr<LeafData> leafData;
      std::unique_ptr<Node> left;
      std::unique_ptr<Node> right;

      size_t getMemoryUsage() const {
        size_t ret = 80;
        if (isLeaf)
          ret += leafData->getMemoryUsage();
        else {
          ret += left->getMemoryUsage();
          ret += right->getMemoryUsage();
        }

        return (ret + 3) & ~3;
      }

#if DCLN_DUMP_OBB
      void sendToBlender(hecl::blender::PyOutStream& os) const;
#endif
    };
    Node root;
    size_t getMemoryUsage() { return root.getMemoryUsage(); }

    /* Dummy MP2 member */
    void insertNoClimb(hecl::blender::PyOutStream&) const {}
  };

  Vector<Collision, AT_DNA_COUNT(colCount)> collision;

  void sendToBlender(hecl::blender::Connection& conn, std::string_view entryName);

  static bool Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                      PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                      std::function<void(const hecl::SystemChar*)> fileChanged);

  static bool Cook(const hecl::ProjectPath& outPath, const std::vector<Mesh>& meshes);
};

} // namespace DataSpec::DNAMP1
