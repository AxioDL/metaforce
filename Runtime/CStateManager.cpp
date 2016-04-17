#include "CStateManager.hpp"
#include "Camera/CCameraShakeData.hpp"
#include "CSortedLists.hpp"
#include "CWeaponMgr.hpp"
#include "CFluidPlaneManager.hpp"
#include "World/CEnvFxManager.hpp"
#include "World/CActorModelParticles.hpp"
#include "World/CTeamAiTypes.hpp"
#include "Input/CRumbleManager.hpp"

namespace urde
{

CStateManager::CStateManager(const std::weak_ptr<CScriptMailbox>&,
                             const std::weak_ptr<CMapWorldInfo>&,
                             const std::weak_ptr<CPlayerState>&,
                             const std::weak_ptr<CWorldTransManager>&)
{
}

void CStateManager::RenderLast(TUniqueId)
{
}

void CStateManager::AddDrawableActor(const CActor& actor, const zeus::CVector3f& vec,
                                     const zeus::CAABox& aabb) const
{
}

void CStateManager::SpecialSkipCinematic()
{
}

void CStateManager::GetVisAreaId() const
{
}

void CStateManager::GetWeaponIdCount(TUniqueId, EWeaponType)
{
}

void CStateManager::RemoveWeaponId(TUniqueId, EWeaponType)
{
}

void CStateManager::AddWeaponId(TUniqueId, EWeaponType)
{
}

void CStateManager::UpdateEscapeSequenceTimer(float)
{
}

float CStateManager::GetEscapeSequenceTimer() const
{
}

void CStateManager::ResetEscapeSequenceTimer(float)
{
}

void CStateManager::SetupParticleHook(const CActor& actor) const
{
}

void CStateManager::MurderScriptInstanceNames()
{
}

void CStateManager::HashInstanceName(CInputStream& in)
{
}

void CStateManager::SetActorAreaId(CActor& actor, TAreaId)
{
}

void CStateManager::TouchSky() const
{
}

void CStateManager::DrawSpaceWarp(const zeus::CVector3f&, float) const
{
}

void CStateManager::DrawReflection(const zeus::CVector3f&)
{
}

void CStateManager::CacheReflection()
{
}

bool CStateManager::CanCreateProjectile(TUniqueId, EWeaponType, int) const
{
}

const CGameLightList* CStateManager::GetDynamicLightList() const
{
}

void CStateManager::BuildDynamicLightListForWorld(std::vector<CLight>& listOut) const
{
}

void CStateManager::DrawDebugStuff() const
{
}

void CStateManager::RenderCamerasAndAreaLights() const
{
}

void CStateManager::DrawE3DeathEffect() const
{
}

void CStateManager::DrawAdditionalFilters() const
{
}

void CStateManager::DrawWorld() const
{
}

void CStateManager::SetupFogForArea(const CGameArea& area) const
{
}

void CStateManager::PreRender()
{
}

void CStateManager::GetVisSetForArea(TAreaId, TAreaId) const
{
}

void CStateManager::RecursiveDrawTree(TUniqueId) const
{
}

void CStateManager::SendScriptMsg(TUniqueId uid, TEditorId eid,
                                  EScriptObjectMessage msg, EScriptObjectState state)
{
}

void CStateManager::FreeScriptObjects(TAreaId)
{
}

void CStateManager::GetBuildForScript(TEditorId) const
{
}

void CStateManager::GetEditorIdForUniqueId() const
{
}

TUniqueId CStateManager::GetIdForScript(TEditorId) const
{
}

void CStateManager::GetIdListForScript(TEditorId) const
{
}

void CStateManager::LoadScriptObjects(TAreaId, CInputStream& in, EScriptPersistence)
{
}

void CStateManager::LoadScriptObject(TAreaId, EScriptObjectType, u32,
                                     CInputStream& in, EScriptPersistence)
{
}

void CStateManager::InformListeners(const zeus::CVector3f&, EListenNoiseType)
{
}

void CStateManager::ApplyKnockBack(CActor& actor, const CDamageInfo& info,
                                   const CDamageVulnerability&, const zeus::CVector3f&, float)
{
}

void CStateManager::ApplyDamageToWorld(TUniqueId, const CActor&, const zeus::CVector3f&,
                                       const CDamageInfo& info, const CMaterialFilter&)
{
}

void CStateManager::ProcessRadiusDamage(const CActor&, CActor&, const zeus::CVector3f&,
                                        const CDamageInfo& info, const CMaterialFilter&)
{
}

void CStateManager::ApplyRadiusDamage(const CActor&, const zeus::CVector3f&, CActor&,
                                      const CDamageInfo& info)
{
}

void CStateManager::ApplyLocalDamage(const zeus::CVector3f&, const zeus::CVector3f&, CActor&, float,
                                     const CWeaponMode&)
{
}

void CStateManager::ApplyDamage(TUniqueId, TUniqueId, TUniqueId, const CDamageInfo& info,
                                const CMaterialFilter&)
{
}

void CStateManager::UpdateAreaSounds()
{
}

void CStateManager::FrameEnd()
{
}

void CStateManager::ProcessPlayerInput()
{
}

void CStateManager::ProcessInput(const CFinalInput& input)
{
}

void CStateManager::Update(float dt)
{
}

void CStateManager::UpdateGameState()
{
}

void CStateManager::FrameBegin()
{
}

void CStateManager::InitializeState(u32, TAreaId, u32)
{
}

void CStateManager::CreateStandardGameObjects()
{
}

CObjectList* CStateManager::ObjectListById(EGameObjectList type)
{
    std::unique_ptr<CObjectList>* lists = &x80c_allObjs;
    return lists[int(type)].get();
}

const CObjectList* CStateManager::GetObjectListById(EGameObjectList type) const
{
    const std::unique_ptr<CObjectList>* lists = &x80c_allObjs;
    return lists[int(type)].get();
}

void CStateManager::RemoveObject(TUniqueId)
{
}

void CStateManager::UpdateRoomAcoustics(TAreaId)
{
}

void CStateManager::SetCurrentAreaId(TAreaId)
{
}

void CStateManager::ClearGraveyard()
{
}

void CStateManager::DeleteObjectRequest(TUniqueId)
{
}

CEntity* CStateManager::ObjectById(TUniqueId uid)
{
    return x80c_allObjs->GetObjectById(uid);
}
const CEntity* CStateManager::GetObjectById(TUniqueId uid) const
{
    return x80c_allObjs->GetObjectById(uid);
}

void CStateManager::AreaUnloaded(TAreaId)
{
}

void CStateManager::PrepareAreaUnload(TAreaId)
{
}

void CStateManager::AreaLoaded(TAreaId)
{
}

void CStateManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                                  const zeus::CVector3f&, const zeus::CVector3f&, float,
                                  const CMaterialFilter&, const CActor*) const
{
}

void CStateManager::BuildColliderList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                                      const CActor&, const zeus::CAABox&) const
{
}

void CStateManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut,
                                  const zeus::CAABox&, const CMaterialFilter&, const CActor*) const
{
}

void CStateManager::UpdateActorInSortedLists(CActor&)
{
}

void CStateManager::UpdateSortedLists()
{
}

zeus::CAABox CStateManager::CalculateObjectBounds(const CActor&)
{
}

void CStateManager::AddObject(CEntity&, EScriptPersistence)
{
}

void CStateManager::AddObject(CEntity*, EScriptPersistence)
{
}

bool CStateManager::RayStaticIntersection(const zeus::CVector3f&, const zeus::CVector3f&, float,
                                          const CMaterialFilter&) const
{
}

bool CStateManager::RayWorldIntersection(TUniqueId, const zeus::CVector3f&, const zeus::CVector3f&,
                                         float, const CMaterialFilter&,
                                         const rstl::reserved_vector<TUniqueId, 1024>& list) const
{
}

void CStateManager::UpdateObjectInLists(CEntity&)
{
}

TUniqueId CStateManager::AllocateUniqueId()
{
}

}
