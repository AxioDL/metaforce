#include "Runtime/Character/CAnimTreeSingleChild.hpp"

namespace urde {

CAnimTreeSingleChild::CAnimTreeSingleChild(const std::weak_ptr<CAnimTreeNode>& node, std::string_view name)
: CAnimTreeNode(name), x14_child(node.lock()) {}

SAdvancementResults CAnimTreeSingleChild::VAdvanceView(const CCharAnimTime& dt) { return x14_child->VAdvanceView(dt); }

CCharAnimTime CAnimTreeSingleChild::VGetTimeRemaining() const { return x14_child->VGetTimeRemaining(); }

bool CAnimTreeSingleChild::VHasOffset(const CSegId& seg) const { return x14_child->VHasOffset(seg); }

zeus::CVector3f CAnimTreeSingleChild::VGetOffset(const CSegId& seg) const { return x14_child->VGetOffset(seg); }

zeus::CQuaternion CAnimTreeSingleChild::VGetRotation(const CSegId& seg) const { return x14_child->VGetRotation(seg); }

u32 CAnimTreeSingleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator,
                                          u32 unk) const {
  return x14_child->GetBoolPOIList(time, listOut, capacity, iterator, unk);
}

u32 CAnimTreeSingleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity,
                                           u32 iterator, u32 unk) const {
  return x14_child->GetInt32POIList(time, listOut, capacity, iterator, unk);
}

u32 CAnimTreeSingleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity,
                                              u32 iterator, u32 unk) const {
  return x14_child->GetParticlePOIList(time, listOut, capacity, iterator, unk);
}

u32 CAnimTreeSingleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity,
                                           u32 iterator, u32 unk) const {
  return x14_child->GetSoundPOIList(time, listOut, capacity, iterator, unk);
}

bool CAnimTreeSingleChild::VGetBoolPOIState(const char* name) const { return x14_child->VGetBoolPOIState(name); }

s32 CAnimTreeSingleChild::VGetInt32POIState(const char* name) const { return x14_child->VGetInt32POIState(name); }

CParticleData::EParentedMode CAnimTreeSingleChild::VGetParticlePOIState(const char* name) const {
  return x14_child->VGetParticlePOIState(name);
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
  x14_child->VGetSegStatementSet(list, setOut);
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                               const CCharAnimTime& time) const {
  x14_child->VGetSegStatementSet(list, setOut, time);
}

void CAnimTreeSingleChild::VSetPhase(float phase) { x14_child->VSetPhase(phase); }

SAdvancementResults CAnimTreeSingleChild::VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const {
  return x14_child->VGetAdvancementResults(a, b);
}

u32 CAnimTreeSingleChild::Depth() const { return x14_child->Depth() + 1; }

u32 CAnimTreeSingleChild::VGetNumChildren() const { return x14_child->VGetNumChildren() + 1; }

} // namespace urde
