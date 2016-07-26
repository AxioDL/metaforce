#ifndef __URDE_CAREAOCTTREE_HPP__
#define __URDE_CAREAOCTTREE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CCollisionEdge;
class CMaterialFilter;
class CAreaOctTree
{
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

public:
    CAreaOctTree(const zeus::CAABox& box, Node::ETreeType treeType, u8*, void*, u32, u32*, u8*, u8*, u8*, u32,
                 CCollisionEdge*, u32, u16*, u32, zeus::CVector3f*);

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

    static void MakeFromMemory(void*, unsigned int, CAreaOctTree*, bool*);
};

}

#endif // __URDE_CAREAOCTTREE_HPP__
