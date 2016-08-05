#ifndef __URDE_CAREAOCTTREE_HPP__
#define __URDE_CAREAOCTTREE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"
#include "Collision/CCollisionEdge.hpp"

namespace urde
{
class CCollisionEdge;
class CMaterialFilter;
class CAreaOctTree
{
    friend class CBooRenderer;
public:
    struct SRayResult
    {
    };

    class TriListReference
    {
    public:
        TriListReference(const void*);
        TriListReference(const u16*);
        void GetAt(s32) const;
        void GetSize() const;
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

    public:
        Node(const void*, const zeus::CAABox&, const CAreaOctTree& owner, ETreeType);
#if 0
        void LineTestEx(const zeus::CLine&, const CMaterialFilter&, SRayResult&, float) const;
        void LineTestExInternal(const zeus::CLine&, const CMaterialFilter&, SRayResult&, float, float, float,
                                const zeus::CVector3f&) const;
#endif

        const CAreaOctTree& GetOwner() const;
        const zeus::CAABox& GetBoundingBox() const;
        u32 GetChildFlags() const;
        const Node* GetChild(s32) const;
        TriListReference GetTriangleArray() const;
        ETreeType GetChildType(s32) const;
        ETreeType GetTreeType(s32) const;
    };

    zeus::CAABox x0_aabb;
    Node::ETreeType x18_treeType;
    u8* x1c_buf;
    void* x20_treeBuf;
    u32 x24_matCount;
    u32* x28_materials;
    u8* x2c_vertMats;
    u8* x30_edgeMats;
    u8* x34_polyMats;
    u32 x38_edgeCount;
    std::vector<CCollisionEdge> x3c_edges;
    u32 x40_polyCount;
    u16* x44_polyEdges;
    u32 x48_vertCount;
    std::vector<zeus::CVector3f> x4c_verts;

public:
    CAreaOctTree(const zeus::CAABox& aabb, Node::ETreeType treeType, u8* buf, void* treeBuf,
                 u32 matCount, u32* materials, u8* vertMats, u8* edgeMats, u8* polyMats,
                 u32 edgeCount, CCollisionEdge* edges, u32 polyCount, u16* polyEdges,
                 u32 vertCount, zeus::CVector3f* verts);

    const Node* GetRootNode() const;
    void GetTreeMemory() const;
    zeus::CVector3f GetVert(s32) const;
    CCollisionEdge GetEdge(s32) const;
    void GetEdgeMaterial();
    void GetTriangleMaterial();
    u32 GetNumEdges() const;
    u32 GetNumVerts() const;
    void GetNumTriangles() const;
    void GetMasterListTriangle(u16);
    void GetTriangleVertexIndices(u16);
    void GetTriangleEdgeIndices(u16);

    static std::unique_ptr<CAreaOctTree> MakeFromMemory(void* buf, unsigned int size);

    void RecursiveMatchXray(std::vector<u32>& out, const zeus::CAABox& inner, const zeus::CAABox& outer)
    {
        if (outer.intersects(inner))
        {
            if (inner.inside(outer))
            {
            }
        }
    }

    void MatchXray(std::vector<u32>& out, const zeus::CAABox&)
    {
    }
};

}

#endif // __URDE_CAREAOCTTREE_HPP__
