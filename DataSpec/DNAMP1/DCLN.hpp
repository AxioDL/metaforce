#ifndef __DNAMP1_DCLN_HPP__
#define __DNAMP1_DCLN_HPP__

#include <athena/Types.hpp>
#include "../DNACommon/DeafBabe.hpp"
#include "../DNACommon/PAK.hpp"
#include "../DNACommon/OBBTreeBuilder.hpp"
#include "DNAMP1.hpp"
#include "DeafBabe.hpp"

#define DCLN_DUMP_OBB 0

namespace DataSpec::DNAMP1
{

struct DCLN : BigDNA
{
    using Mesh = hecl::blender::ColMesh;

    DECL_DNA
    Value<atUint32> colCount;
    struct Collision : BigDNA
    {
        using Material = DeafBabe::Material;
        using Edge = DeafBabe::Edge;
        using Triangle = DeafBabe::Triangle;

        DECL_DNA
        Value<atUint32> magic;
        Value<atUint32> version;
        Value<atUint32> memSize;
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

        struct Node : BigDNA
        {
            Delete _d;

            struct LeafData : BigDNA
            {
                DECL_DNA
                Value<atUint32> triangleIndexCount;
                Vector<atUint16, DNA_COUNT(triangleIndexCount)> triangleIndices;
                size_t getMemoryUsage() const { return (((triangleIndices.size() * 2) + 16) + 3) & ~3; }
            };

            Value<atVec4f> xf[3];
            Value<atVec3f> halfExtent;
            Value<bool> isLeaf;
            std::unique_ptr<LeafData> leafData;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;

            void read(athena::io::IStreamReader & __dna_reader)
            {
                xf[0] = __dna_reader.readVec4fBig();
                xf[1] = __dna_reader.readVec4fBig();
                xf[2] = __dna_reader.readVec4fBig();
                halfExtent = __dna_reader.readVec3fBig();
                isLeaf = __dna_reader.readBool();
                if (isLeaf)
                {
                    leafData.reset(new LeafData);
                    leafData->read(__dna_reader);
                }
                else
                {
                    left.reset(new Node);
                    left->read(__dna_reader);
                    right.reset(new Node);
                    right->read(__dna_reader);
                }
            }

            void write(athena::io::IStreamWriter & __dna_writer) const
            {
                __dna_writer.writeVec4fBig(xf[0]);
                __dna_writer.writeVec4fBig(xf[1]);
                __dna_writer.writeVec4fBig(xf[2]);
                __dna_writer.writeVec3fBig(halfExtent);
                __dna_writer.writeBool(isLeaf);
                if (isLeaf && leafData)
                    leafData->write(__dna_writer);
                else if (!isLeaf && left && right)
                {
                    left->write(__dna_writer);
                    right->write(__dna_writer);
                }
            }

            size_t binarySize(size_t __isz) const
            {
                __isz += 61;
                if (isLeaf && leafData)
                    __isz = leafData->binarySize(__isz);
                else if (!isLeaf && left && right)
                {
                    __isz = left->binarySize(__isz);
                    __isz = right->binarySize(__isz);
                }
                return __isz;
            }

            size_t getMemoryUsage() const
            {
                size_t ret = 80;
                if (isLeaf)
                    ret += leafData->getMemoryUsage();
                else
                {
                    ret += left->getMemoryUsage();
                    ret += right->getMemoryUsage();
                }

                return (ret + 3) & ~3;
            }

#if DCLN_DUMP_OBB
            sendToBlender(hecl::blender::PyOutStream& os) const;
#endif
        };
        Node root;
        size_t getMemoryUsage()
        {
            return root.getMemoryUsage();
        }

        /* Dummy MP2 member */
        void insertNoClimb(hecl::blender::PyOutStream&) const {}
    };


    Vector<Collision, DNA_COUNT(colCount)> collision;

    void sendToBlender(hecl::blender::Connection& conn, std::string_view entryName);

    static bool Extract(const SpecBase& dataSpec,
                 PAKEntryReadStream& rs,
                 const hecl::ProjectPath& outPath,
                 PAKRouter<PAKBridge>& pakRouter,
                 const PAK::Entry& entry,
                 bool force,
                 hecl::blender::Token& btok,
                 std::function<void(const hecl::SystemChar*)> fileChanged);

    static bool Cook(const hecl::ProjectPath& outPath,
                     const hecl::ProjectPath& inPath,
                     const std::vector<Mesh>& meshes,
                     hecl::blender::Connection* conn = nullptr);
};

}
 #endif // __DNAMP1_DCLN_HPP__
