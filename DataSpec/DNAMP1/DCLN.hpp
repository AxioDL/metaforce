#ifndef __DNAMP1_DCLN_HPP__
#define __DNAMP1_DCLN_HPP__

#include "../DNACommon/DeafBabe.hpp"
#include "../DNACommon/PAK.hpp"
#include "DNAMP1.hpp"
#include "DeafBabe.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct DCLN : BigDNA
{
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

        struct LeafData : BigDNA
        {
            DECL_DNA
            Value<atUint32> edgeIndexCount;
            Vector<atUint16, DNA_COUNT(edgeIndexCount)> edgeIndices;
            size_t getMemoryUsage() const { return (((edgeIndices.size() * 2) + 16) + 3) & ~3; }
        };

        struct Node : BigDNA
        {
            Delete _d;
            Value<atVec4f> xf[3];
            Value<atVec3f> origin;
            Value<bool> isLeaf;
            std::unique_ptr<LeafData> leafData;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;

            void read(athena::io::IStreamReader & __dna_reader)
            {
                xf[0] = __dna_reader.readVec4fBig();
                xf[1] = __dna_reader.readVec4fBig();
                xf[2] = __dna_reader.readVec4fBig();
                origin = __dna_reader.readVec3fBig();
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
                __dna_writer.writeVec3fBig(origin);
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
        };
        Node root;
        size_t getMemoryUsage()
        {
            return root.getMemoryUsage();
        }

        /* Dummy MP2 member */
        void insertNoClimb(hecl::BlenderConnection::PyOutStream&) const {}
    };


    Vector<Collision, DNA_COUNT(colCount)> collision;

    void sendToBlender(hecl::BlenderConnection& conn, const std::string& entryName)
    {
        /* Open Py Stream and read sections */
        hecl::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
        os.format("import bpy\n"
                  "import bmesh\n"
                  "from mathutils import Vector\n"
                  "\n"
                  "bpy.context.scene.name = '%s'\n"
                  "# Clear Scene\n"
                  "for ob in bpy.data.objects:\n"
                  "    if ob.type != 'CAMERA':\n"
                  "        bpy.context.scene.objects.unlink(ob)\n"
                  "        bpy.data.objects.remove(ob)\n",
                  entryName.c_str());

        DeafBabe::BlenderInit(os);
        atInt32 idx = 0;
        for (const Collision& col : collision)
            DeafBabeSendToBlender(os, col, true, idx++);
        os.centerView();
        os.close();
    }

    static bool Extract(const SpecBase& dataSpec,
                 PAKEntryReadStream& rs,
                 const hecl::ProjectPath& outPath,
                 PAKRouter<PAKBridge>& pakRouter,
                 const PAK::Entry& entry,
                 bool force,
                 hecl::BlenderToken& btok,
                 std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        DCLN dcln;
        dcln.read(rs);
        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        if (!conn.createBlend(outPath, hecl::BlenderConnection::BlendType::Mesh))
            return false;

        dcln.sendToBlender(conn, pakRouter.getBestEntryName(entry, false));
        return conn.saveBlend();
    }
};

}
}
 #endif // __DNAMP1_DCLN_HPP__
