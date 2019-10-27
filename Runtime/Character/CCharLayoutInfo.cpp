#include "CCharLayoutInfo.hpp"
#include "CToken.hpp"

namespace urde {

zeus::CVector3f CCharLayoutInfo::GetFromParentUnrotated(const CSegId& id) const {
  const CCharLayoutNode::Bone& bone = x0_node->GetBoneMap()[id];
  if (!x0_node->GetBoneMap().HasElement(bone.x0_parentId))
    return bone.x4_origin;
  else {
    const CCharLayoutNode::Bone& pBone = x0_node->GetBoneMap()[bone.x0_parentId];
    return bone.x4_origin - pBone.x4_origin;
  }
}

zeus::CVector3f CCharLayoutInfo::GetFromRootUnrotated(const CSegId& id) const {
  const CCharLayoutNode::Bone& bone = x0_node->GetBoneMap()[id];
  return bone.x4_origin;
}

CSegId CCharLayoutInfo::GetSegIdFromString(std::string_view name) const {
  const auto it = x18_segIdMap.find(name);

  if (it == x18_segIdMap.cend()) {
    return {};
  }

  return it->second;
}

void CCharLayoutNode::Bone::read(CInputStream& in) {
  x0_parentId = CSegId(in);
  x4_origin.readBig(in);

  u32 chCount = in.readUint32Big();
  x10_children.reserve(chCount);
  for (u32 i = 0; i < chCount; ++i)
    x10_children.emplace_back(in);
}

CCharLayoutNode::CCharLayoutNode(CInputStream& in) : x0_boneMap(in.readUint32Big()) {
  u32 cap = x0_boneMap.GetCapacity();
  for (u32 i = 0; i < cap; ++i) {
    u32 thisId = in.readUint32Big();
    Bone& bone = x0_boneMap[thisId];
    bone.read(in);
  }
}

CCharLayoutInfo::CCharLayoutInfo(CInputStream& in) : x0_node(std::make_shared<CCharLayoutNode>(in)), x8_segIdList(in) {
  atUint32 mapCount = in.readUint32Big();
  for (atUint32 i = 0; i < mapCount; ++i) {
    std::string key = in.readString();
    x18_segIdMap.emplace(key, in);
  }
}

CFactoryFnReturn FCharLayoutInfo(const SObjectTag&, CInputStream& in, const CVParamTransfer&,
                                 CObjectReference* selfRef) {
  return TToken<CCharLayoutInfo>::GetIObjObjectFor(std::make_unique<CCharLayoutInfo>(in));
}

} // namespace urde
