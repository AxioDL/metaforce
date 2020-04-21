#include "Runtime/World/CScriptMazeNode.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/World/CActorParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

std::array<u32, 300> CScriptMazeNode::sMazeSeeds{};

CScriptMazeNode::CScriptMazeNode(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, s32 w1, s32 w2, s32 w3,
                                 const zeus::CVector3f& vec1, const zeus::CVector3f& vec2, const zeus::CVector3f& vec3)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_(w1)
, xec_(w1)
, xf0_(w2)
, x100_(vec1)
, x110_(vec2)
, x120_(vec3) {}

void CScriptMazeNode::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptMazeNode::LoadMazeSeeds() {
  const SObjectTag* tag = g_ResFactory->GetResourceIdByName("DUMB_MazeSeeds");
  const u32 resSize = g_ResFactory->ResourceSize(*tag);
  const std::unique_ptr<u8[]> buf = g_ResFactory->LoadResourceSync(*tag);
  CMemoryInStream in(buf.get(), resSize);
  for (auto& seed : sMazeSeeds) {
    seed = in.readUint32Big();
  }
}
} // namespace urde
