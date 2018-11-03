#pragma once

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"
#include "Collision/CCollisionEdge.hpp"
#include "Collision/CCollisionSurface.hpp"
#include "zeus/CLine.hpp"

namespace urde
{
class CMaterialFilter;
class CAreaOctTree
{
    friend class CBooRenderer;
public:
    struct SRayResult
    {
        zeus::CPlane x0_plane;
        std::experimental::optional<CCollisionSurface> x10_surface;
        float x3c_t;
    };

    class TriListReference
    {
        const u16* m_ptr;
    public:
        TriListReference(const u16* ptr)
        : m_ptr(ptr) {}
        u16 GetAt(int idx) const { return m_ptr[idx+1]; }
        u16 GetSize() const { return m_ptr[0]; }
    };

    class Node
    {
    public:
        enum class ETreeType
        {
            Invalid,
            Branch,
            Leaf
        };

    private:
        zeus::CAABox x0_aabb;
        const u8* x18_ptr;
        const CAreaOctTree& x1c_owner;
        ETreeType x20_nodeType;

        bool LineTestInternal(const zeus::CLine&, const CMaterialFilter&, float, float, float,
                              const zeus::CVector3f&) const;
        void LineTestExInternal(const zeus::CLine&, const CMaterialFilter&, SRayResult&, float, float, float,
                                const zeus::CVector3f&) const;

    public:
        Node(const void* ptr, const zeus::CAABox& aabb,
             const CAreaOctTree& owner, ETreeType type)
        : x0_aabb(aabb), x18_ptr(reinterpret_cast<const u8*>(ptr)), x1c_owner(owner), x20_nodeType(type)
        {
        }

        bool LineTest(const zeus::CLine&, const CMaterialFilter&, float) const;
        void LineTestEx(const zeus::CLine&, const CMaterialFilter&, SRayResult&, float) const;

        const CAreaOctTree& GetOwner() const
        {
            return x1c_owner;
        }

        const zeus::CAABox& GetBoundingBox() const
        {
            return x0_aabb;
        }

        u16 GetChildFlags() const
        {
            return *reinterpret_cast<const u16*>(x18_ptr);
        }

        Node GetChild(int idx) const;

        TriListReference GetTriangleArray() const
        {
            return TriListReference(reinterpret_cast<const u16*>(x18_ptr + 24));
        }

        ETreeType GetChildType(int idx) const
        {
            u16 flags = *reinterpret_cast<const u16*>(x18_ptr);
            return ETreeType((flags >> (2 * idx)) & 0x3);
        }

        ETreeType GetTreeType() const { return x20_nodeType; }
    };

    zeus::CAABox x0_aabb;
    Node::ETreeType x18_treeType;
    const u8* x1c_buf;
    const u8* x20_treeBuf;
    u32 x24_matCount;
    const u32* x28_materials;
    const u8* x2c_vertMats;
    const u8* x30_edgeMats;
    const u8* x34_polyMats;
    u32 x38_edgeCount;
    const CCollisionEdge* x3c_edges;
    u32 x40_polyCount;
    const u16* x44_polyEdges;
    u32 x48_vertCount;
    const float* x4c_verts;

    void SwapTreeNode(u8* ptr, Node::ETreeType type);

public:
    CAreaOctTree(const zeus::CAABox& aabb, Node::ETreeType treeType, const u8* buf, const u8* treeBuf,
                 u32 matCount, const u32* materials, const u8* vertMats, const u8* edgeMats, const u8* polyMats,
                 u32 edgeCount, const CCollisionEdge* edges, u32 polyCount, const u16* polyEdges,
                 u32 vertCount, const float* verts);

    const zeus::CAABox& GetAABB() const { return x0_aabb; }
    Node GetRootNode() const { return Node(x20_treeBuf, x0_aabb, *this, x18_treeType); }
    const u8* GetTreeMemory() const { return x20_treeBuf; }
    zeus::CVector3f GetVert(int idx) const
    {
        const float* vert = &x4c_verts[idx * 3];
        return zeus::CVector3f(vert[0], vert[1], vert[2]);
    }
    const CCollisionEdge& GetEdge(int idx) const { return x3c_edges[idx]; }
    u32 GetVertMaterial(int idx) const { return x28_materials[x2c_vertMats[idx]]; }
    u32 GetEdgeMaterial(int idx) const { return x28_materials[x30_edgeMats[idx]]; }
    u32 GetTriangleMaterial(int idx) const { return x28_materials[x34_polyMats[idx]]; }
    u32 GetNumEdges() const { return x38_edgeCount; }
    u32 GetNumVerts() const { return x48_vertCount; }
    u32 GetNumTriangles() const { return x40_polyCount; }
    CCollisionSurface GetMasterListTriangle(u16 idx) const;
    void GetTriangleVertexIndices(u16 idx, u16 indicesOut[3]) const;
    const u16* GetTriangleEdgeIndices(u16 idx) const { return &x44_polyEdges[idx * 3]; }

    static std::unique_ptr<CAreaOctTree> MakeFromMemory(const u8* buf, unsigned int size);
};

}

