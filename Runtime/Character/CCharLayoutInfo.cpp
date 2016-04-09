#include "CCharLayoutInfo.hpp"
#include "CToken.hpp"

namespace urde
{

void CCharLayoutNode::Bone::read(CInputStream& in)
{
    x0_parentId = CSegId(in);
    x4_origin.readBig(in);

    u32 chCount = in.readUint32Big();
    x10_children.reserve(chCount);
    for (u32 i=0 ; i<chCount ; ++i)
        x10_children.emplace_back(in);
}

CCharLayoutNode::CCharLayoutNode(CInputStream& in)
{
    x0_boneCount = in.readUint32Big();
    for (u32 i=0 ; i<x0_boneCount ; ++i)
    {
        u32 thisId = in.readUint32Big();
        if (thisId >= 100)
        {
            Bone dummy;
            dummy.read(in);
        }
        else
            x108_bones[i].read(in);
    }
}

CCharLayoutInfo::CCharLayoutInfo(CInputStream& in)
: x0_node(std::make_shared<CCharLayoutNode>(in)),
  x8_segIdList(in)
{
    atUint32 mapCount = in.readUint32Big();
    for (int i=0 ; i<mapCount ; ++i)
    {
        std::string key = in.readString();
        x18_segIdMap.emplace(key, in);
    }
}

CFactoryFnReturn FCharLayoutInfo(const SObjectTag&, CInputStream& in, const CVParamTransfer&)
{
    return TToken<CCharLayoutInfo>::GetIObjObjectFor(std::make_unique<CCharLayoutInfo>(in));
}

}
