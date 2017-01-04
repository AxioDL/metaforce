#include "CScriptMazeNode.hpp"
#include "Character/CModelData.hpp"
#include "GameGlobalObjects.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CScriptMazeNode::CScriptMazeNode(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, bool active, s32 w1, s32 w2, s32 w3,
                                 const zeus::CVector3f& vec1, const zeus::CVector3f& vec2, const zeus::CVector3f& vec3)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_(w1)
, xec_(w1)
, xf0_(w2)
, x100_(vec1)
, x110_(vec2)
, x120_(vec3)
{
    x13c_24_ = true;
}

void CScriptMazeNode::LoadSeeds()
{
    const SObjectTag* tag = g_ResFactory->GetResourceIdByName("DUMB_MazeSeeds");
    u32 resSize = g_ResFactory->ResourceSize(*tag);
    std::unique_ptr<u8[]> buf = g_ResFactory->LoadResourceSync(*tag);
    CMemoryInStream in(buf.get(), resSize);
    for (u32 i = 0; i<300 ; ++i)
        sMazeSeeds[i] = in.readUint32Big();
}
}
