#include "COBBTree.hpp"


namespace urde
{
/* This is exactly what retro did >.< */
u32 verify_deaf_babe(CInputStream& in)
{
    return in.readUint32Big();
}

/* This is exactly what retro did >.< */
u32 verify_version(CInputStream& in)
{
    return in.readUint32Big();
}

COBBTree::COBBTree(const COBBTree::SIndexData&, const COBBTree::CNode*)
{
}

COBBTree::COBBTree(CInputStream& in)
    : x0_magic(verify_deaf_babe(in)),
      x4_version(verify_version(in)),
      x8_memsize(in.readUint32()),
      x18_indexData(in)
{
}

COBBTree::SIndexData::SIndexData(CInputStream& in)
{
    u32 count = in.readUint32Big();
    x0_.reserve(count);
    for (u32 i = 0 ; i < count ; i++)
        x0_.push_back(in.readUint32Big());

    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x10_.push_back(in.readUByte());
    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x20_.push_back(in.readUByte());
    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x30_.push_back(in.readUByte());

    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x40_.push_back(in);

    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x50_.push_back(in.readUint16Big());

    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x60_.push_back(zeus::CVector3f::ReadBig(in));
}

COBBTree::CNode::CNode(CInputStream& in)
{
    x0_obb = zeus::COBBox::ReadBig(in);
}

}
