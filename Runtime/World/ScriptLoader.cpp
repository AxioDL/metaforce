#include "Runtime/World/ScriptLoader.hpp"

#include "Runtime/Camera/CCinematicCamera.hpp"
#include "Runtime/Camera/CPathCamera.hpp"
#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/MP1/World/CActorContraption.hpp"
#include "Runtime/MP1/World/CAtomicAlpha.hpp"
#include "Runtime/MP1/World/CAtomicBeta.hpp"
#include "Runtime/MP1/World/CBabygoth.hpp"
#include "Runtime/MP1/World/CBeetle.hpp"
#include "Runtime/MP1/World/CBloodFlower.hpp"
#include "Runtime/MP1/World/CBurrower.hpp"
#include "Runtime/MP1/World/CChozoGhost.hpp"
#include "Runtime/MP1/World/CElitePirate.hpp"
#include "Runtime/MP1/World/CEnergyBall.hpp"
#include "Runtime/MP1/World/CEyeball.hpp"
#include "Runtime/MP1/World/CFireFlea.hpp"
#include "Runtime/MP1/World/CFlaahgra.hpp"
#include "Runtime/MP1/World/CFlaahgraTentacle.hpp"
#include "Runtime/MP1/World/CFlickerBat.hpp"
#include "Runtime/MP1/World/CFlyingPirate.hpp"
#include "Runtime/MP1/World/CIceSheegoth.hpp"
#include "Runtime/MP1/World/CJellyZap.hpp"
#include "Runtime/MP1/World/CMagdolite.hpp"
#include "Runtime/MP1/World/CMetaree.hpp"
#include "Runtime/MP1/World/CDrone.hpp"
#include "Runtime/MP1/World/CMetroid.hpp"
#include "Runtime/MP1/World/CMetroidBeta.hpp"
#include "Runtime/MP1/World/CMetroidPrimeEssence.hpp"
#include "Runtime/MP1/World/CMetroidPrimeRelay.hpp"
#include "Runtime/MP1/World/CNewIntroBoss.hpp"
#include "Runtime/MP1/World/COmegaPirate.hpp"
#include "Runtime/MP1/World/CParasite.hpp"
#include "Runtime/MP1/World/CPhazonHealingNodule.hpp"
#include "Runtime/MP1/World/CPhazonPool.hpp"
#include "Runtime/MP1/World/CPuddleSpore.hpp"
#include "Runtime/MP1/World/CPuddleToadGamma.hpp"
#include "Runtime/MP1/World/CPuffer.hpp"
#include "Runtime/MP1/World/CRidley.hpp"
#include "Runtime/MP1/World/CRipper.hpp"
#include "Runtime/MP1/World/CSeedling.hpp"
#include "Runtime/MP1/World/CSpacePirate.hpp"
#include "Runtime/MP1/World/CSpankWeed.hpp"
#include "Runtime/MP1/World/CThardus.hpp"
#include "Runtime/MP1/World/CThardusRockProjectile.hpp"
#include "Runtime/MP1/World/CTryclops.hpp"
#include "Runtime/MP1/World/CWarWasp.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CAmbientAI.hpp"
#include "Runtime/World/CAnimationParameters.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CFishCloud.hpp"
#include "Runtime/World/CFishCloudModifier.hpp"
#include "Runtime/World/CFluidUVMotion.hpp"
#include "Runtime/World/CGrappleParameters.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CRepulsor.hpp"
#include "Runtime/World/CScriptActor.hpp"
#include "Runtime/World/CScriptActorKeyframe.hpp"
#include "Runtime/World/CScriptActorRotate.hpp"
#include "Runtime/World/CScriptAiJumpPoint.hpp"
#include "Runtime/World/CScriptAreaAttributes.hpp"
#include "Runtime/World/CScriptBallTrigger.hpp"
#include "Runtime/World/CScriptBeam.hpp"
#include "Runtime/World/CScriptCameraBlurKeyframe.hpp"
#include "Runtime/World/CScriptCameraFilterKeyframe.hpp"
#include "Runtime/World/CScriptCameraHint.hpp"
#include "Runtime/World/CScriptCameraHintTrigger.hpp"
#include "Runtime/World/CScriptCameraPitchVolume.hpp"
#include "Runtime/World/CScriptCameraShaker.hpp"
#include "Runtime/World/CScriptCameraWaypoint.hpp"
#include "Runtime/World/CScriptColorModulate.hpp"
#include "Runtime/World/CScriptControllerAction.hpp"
#include "Runtime/World/CScriptCounter.hpp"
#include "Runtime/World/CScriptCoverPoint.hpp"
#include "Runtime/World/CScriptDamageableTrigger.hpp"
#include "Runtime/World/CScriptDebris.hpp"
#include "Runtime/World/CScriptDebugCameraWaypoint.hpp"
#include "Runtime/World/CScriptDistanceFog.hpp"
#include "Runtime/World/CScriptDock.hpp"
#include "Runtime/World/CScriptDockAreaChange.hpp"
#include "Runtime/World/CScriptDoor.hpp"
#include "Runtime/World/CScriptEffect.hpp"
#include "Runtime/World/CScriptEMPulse.hpp"
#include "Runtime/World/CScriptGenerator.hpp"
#include "Runtime/World/CScriptGrapplePoint.hpp"
#include "Runtime/World/CScriptGunTurret.hpp"
#include "Runtime/World/CScriptHUDMemo.hpp"
#include "Runtime/World/CScriptMazeNode.hpp"
#include "Runtime/World/CScriptMemoryRelay.hpp"
#include "Runtime/World/CScriptMidi.hpp"
#include "Runtime/World/CScriptPickup.hpp"
#include "Runtime/World/CScriptPickupGenerator.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptPlayerActor.hpp"
#include "Runtime/World/CScriptPlayerHint.hpp"
#include "Runtime/World/CScriptPlayerStateChange.hpp"
#include "Runtime/World/CScriptPointOfInterest.hpp"
#include "Runtime/World/CScriptRandomRelay.hpp"
#include "Runtime/World/CScriptRelay.hpp"
#include "Runtime/World/CScriptRipple.hpp"
#include "Runtime/World/CScriptRoomAcoustics.hpp"
#include "Runtime/World/CScriptShadowProjector.hpp"
#include "Runtime/World/CScriptSound.hpp"
#include "Runtime/World/CScriptSpawnPoint.hpp"
#include "Runtime/World/CScriptSpecialFunction.hpp"
#include "Runtime/World/CScriptSpiderBallAttractionSurface.hpp"
#include "Runtime/World/CScriptSpiderBallWaypoint.hpp"
#include "Runtime/World/CScriptSpindleCamera.hpp"
#include "Runtime/World/CScriptSteam.hpp"
#include "Runtime/World/CScriptStreamedMusic.hpp"
#include "Runtime/World/CScriptSwitch.hpp"
#include "Runtime/World/CScriptTargetingPoint.hpp"
#include "Runtime/World/CScriptTimer.hpp"
#include "Runtime/World/CScriptVisorFlare.hpp"
#include "Runtime/World/CScriptVisorGoo.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CScriptWorldTeleporter.hpp"
#include "Runtime/World/CSnakeWeedSwarm.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWallCrawlerSwarm.hpp"
#include "Runtime/World/CWorld.hpp"

namespace metaforce {
static logvisor::Module Log("metaforce::ScriptLoader");

constexpr SObjectTag MorphballDoorANCS = {FOURCC('ANCS'), 0x1F9DA858u};

constexpr int skElitePiratePropCount = 41;

static bool EnsurePropertyCount(int count, int expected, const char* structName) {
  if (count < expected) {
    Log.report(logvisor::Warning, FMT_STRING("Insufficient number of props ({}/{}) for {} entity"), count, expected,
               structName);
    return false;
  }
  return true;
}

struct SActorHead {
  std::string x0_name;
  zeus::CTransform x10_transform;
};

struct SScaledActorHead : SActorHead {
  zeus::CVector3f x40_scale;

  SScaledActorHead(SActorHead&& head) : SActorHead(std::move(head)) {}
};

static zeus::CTransform LoadEditorTransform(CInputStream& in) {
  zeus::CVector3f position = in.Get<zeus::CVector3f>();
  zeus::CVector3f orientation = in.Get<zeus::CVector3f>();
  return ScriptLoader::ConvertEditorEulerToTransform4f(orientation, position);
}

static zeus::CTransform LoadEditorTransformPivotOnly(CInputStream& in) {
  zeus::CVector3f position = in.Get<zeus::CVector3f>();
  zeus::CVector3f orientation = in.Get<zeus::CVector3f>();
  orientation.x() = 0.f;
  orientation.y() = 0.f;
  return ScriptLoader::ConvertEditorEulerToTransform4f(orientation, position);
}

static SActorHead LoadActorHead(CInputStream& in, CStateManager& stateMgr) {
  SActorHead ret;
  ret.x0_name = stateMgr.HashInstanceName(in);
  ret.x10_transform = LoadEditorTransform(in);
  return ret;
}

static SScaledActorHead LoadScaledActorHead(CInputStream& in, CStateManager& stateMgr) {
  SScaledActorHead ret = LoadActorHead(in, stateMgr);
  ret.x40_scale = in.Get<zeus::CVector3f>();
  return ret;
}

static zeus::CAABox GetCollisionBox(CStateManager& stateMgr, TAreaId id, const zeus::CVector3f& extent,
                                    const zeus::CVector3f& offset) {
  zeus::CAABox box(-extent * 0.5f + offset, extent * 0.5f + offset);
  const zeus::CTransform& rot = stateMgr.GetWorld()->GetGameAreas()[id]->GetTransform().getRotation();
  return box.getTransformedAABox(rot);
}

u32 ScriptLoader::LoadParameterFlags(CInputStream& in) {
  u32 count = in.ReadLong();
  u32 ret = 0;
  for (u32 i = 0; i < count; ++i)
    if (in.ReadBool())
      ret |= 1 << i;
  return ret;
}

CGrappleParameters ScriptLoader::LoadGrappleParameters(CInputStream& in) {
  float a = in.ReadFloat();
  float b = in.ReadFloat();
  float c = in.ReadFloat();
  float d = in.ReadFloat();
  float e = in.ReadFloat();
  float f = in.ReadFloat();
  float g = in.ReadFloat();
  float h = in.ReadFloat();
  float i = in.ReadFloat();
  float j = in.ReadFloat();
  float k = in.ReadFloat();
  bool l = in.ReadBool();
  return CGrappleParameters(a, b, c, d, e, f, g, h, i, j, k, l);
}

CActorParameters ScriptLoader::LoadActorParameters(CInputStream& in) {
  u32 propCount = in.ReadLong();
  if (propCount >= 5 && propCount <= 0xe) {
    CLightParameters lParms = ScriptLoader::LoadLightParameters(in);

    CScannableParameters sParams;
    if (propCount > 5)
      sParams = LoadScannableParameters(in);

    CAssetId xrayModel = in.Get<CAssetId>();
    CAssetId xraySkin = in.Get<CAssetId>();
    CAssetId infraModel = in.Get<CAssetId>();
    CAssetId infraSkin = in.Get<CAssetId>();

    bool globalTimeProvider = true;
    if (propCount > 7)
      globalTimeProvider = in.ReadBool();

    float fadeInTime = 1.f;
    if (propCount > 8)
      fadeInTime = in.ReadFloat();

    float fadeOutTime = 1.f;
    if (propCount > 9)
      fadeOutTime = in.ReadFloat();

    CVisorParameters vParms;
    if (propCount > 6)
      vParms = LoadVisorParameters(in);

    bool thermalHeat = false;
    if (propCount > 10)
      thermalHeat = in.ReadBool();

    bool renderUnsorted = false;
    if (propCount > 11)
      renderUnsorted = in.ReadBool();

    bool noSortThermal = false;
    if (propCount > 12)
      noSortThermal = in.ReadBool();

    float thermalMag = 1.f;
    if (propCount > 13)
      thermalMag = in.ReadFloat();

    std::pair<CAssetId, CAssetId> xray = {};
    if (g_ResFactory->GetResourceTypeById(xrayModel).IsValid())
      xray = {xrayModel, xraySkin};

    std::pair<CAssetId, CAssetId> infra = {};
    if (g_ResFactory->GetResourceTypeById(infraModel).IsValid())
      infra = {infraModel, infraSkin};

    return CActorParameters(lParms, sParams, xray, infra, vParms, globalTimeProvider, thermalHeat, renderUnsorted,
                            noSortThermal, fadeInTime, fadeOutTime, thermalMag);
  }
  return CActorParameters::None();
}

CVisorParameters ScriptLoader::LoadVisorParameters(CInputStream& in) {
  u32 propCount = in.ReadLong();
  if (propCount >= 1 && propCount <= 3) {
    bool b1 = in.ReadBool();
    bool scanPassthrough = false;
    u8 mask = 0xf;
    if (propCount > 2)
      scanPassthrough = in.ReadBool();
    if (propCount >= 2)
      mask = u8(in.ReadLong());
    return CVisorParameters(mask, b1, scanPassthrough);
  }
  return CVisorParameters();
}

CScannableParameters ScriptLoader::LoadScannableParameters(CInputStream& in) {
  u32 propCount = in.ReadLong();
  if (propCount == 1)
    return CScannableParameters(in.Get<CAssetId>());
  return CScannableParameters();
}

CLightParameters ScriptLoader::LoadLightParameters(CInputStream& in) {
  u32 propCount = in.ReadLong();
  if (propCount == 14) {
    bool a = in.ReadBool();
    float b = in.ReadFloat();
    CLightParameters::EShadowTesselation shadowTess = CLightParameters::EShadowTesselation(in.ReadLong());
    float d = in.ReadFloat();
    float e = in.ReadFloat();

    zeus::CColor noLightsAmbient = in.Get<zeus::CColor>();

    bool makeLights = in.ReadBool();
    CLightParameters::EWorldLightingOptions lightOpts = CLightParameters::EWorldLightingOptions(in.ReadLong());
    CLightParameters::ELightRecalculationOptions recalcOpts =
        CLightParameters::ELightRecalculationOptions(in.ReadLong());

    zeus::CVector3f actorPosBias = in.Get<zeus::CVector3f>();

    s32 maxDynamicLights = -1;
    s32 maxAreaLights = -1;
    if (propCount >= 12) {
      maxDynamicLights = in.ReadLong();
      maxAreaLights = in.ReadLong();
    }

    bool ambientChannelOverflow = false;
    if (propCount >= 13)
      ambientChannelOverflow = in.ReadBool();

    s32 layerIdx = 0;
    if (propCount >= 14)
      layerIdx = in.ReadLong();

    return CLightParameters(a, b, shadowTess, d, e, noLightsAmbient, makeLights, lightOpts, recalcOpts, actorPosBias,
                            maxDynamicLights, maxAreaLights, ambientChannelOverflow, layerIdx);
  }
  return CLightParameters::None();
}

CAnimationParameters ScriptLoader::LoadAnimationParameters(CInputStream& in) {
  CAssetId ancs = in.Get<CAssetId>();
  s32 charIdx = in.ReadLong();
  u32 defaultAnim = in.ReadLong();
  return CAnimationParameters(ancs, charIdx, defaultAnim);
}

CFluidUVMotion ScriptLoader::LoadFluidUVMotion(CInputStream& in) {
  auto motion = CFluidUVMotion::EFluidUVMotion(in.ReadLong());
  float a = in.ReadFloat();
  float b = in.ReadFloat();
  b = zeus::degToRad(b) - M_PIF;
  float c = in.ReadFloat();
  float d = in.ReadFloat();
  CFluidUVMotion::SFluidLayerMotion pattern1Layer(motion, a, b, c, d);

  motion = CFluidUVMotion::EFluidUVMotion(in.ReadLong());
  a = in.ReadFloat();
  b = in.ReadFloat();
  b = zeus::degToRad(b) - M_PIF;
  c = in.ReadFloat();
  d = in.ReadFloat();
  CFluidUVMotion::SFluidLayerMotion pattern2Layer(motion, a, b, c, d);

  motion = CFluidUVMotion::EFluidUVMotion(in.ReadLong());
  a = in.ReadFloat();
  b = in.ReadFloat();
  b = zeus::degToRad(b) - M_PIF;
  c = in.ReadFloat();
  d = in.ReadFloat();
  CFluidUVMotion::SFluidLayerMotion colorLayer(motion, a, b, c, d);

  a = in.ReadFloat();
  b = in.ReadFloat();

  b = zeus::degToRad(b) - M_PIF;

  return CFluidUVMotion(a, b, colorLayer, pattern1Layer, pattern2Layer);
}

zeus::CTransform ScriptLoader::ConvertEditorEulerToTransform4f(const zeus::CVector3f& orientation,
                                                               const zeus::CVector3f& position) {
  zeus::simd_floats f(orientation.mSimd);
  return zeus::CTransform::RotateZ(zeus::degToRad(f[2])) * zeus::CTransform::RotateY(zeus::degToRad(f[1])) *
             zeus::CTransform::RotateX(zeus::degToRad(f[0])) +
         position;
}

CEntity* ScriptLoader::LoadActor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 24, "Actor"))
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);

  zeus::CVector3f collisionExtent = in.Get<zeus::CVector3f>();

  zeus::CVector3f centroid = in.Get<zeus::CVector3f>();

  float mass = in.ReadFloat();
  float zMomentum = in.ReadFloat();

  CHealthInfo hInfo(in);

  CDamageVulnerability dVuln(in);

  CAssetId staticId = in.Get<CAssetId>();
  CAnimationParameters aParms = LoadAnimationParameters(in);

  CActorParameters actParms = LoadActorParameters(in);

  bool looping = in.ReadBool();
  bool immovable = in.ReadBool();
  bool solid = in.ReadBool();
  bool cameraPassthrough = in.ReadBool();
  bool active = in.ReadBool();
  u32 shaderIdx = in.ReadLong();
  float xrayAlpha = in.ReadFloat();
  bool noThermalHotZ = in.ReadBool();
  bool castsShadow = in.ReadBool();
  bool scaleAdvancementDelta = in.ReadBool();
  bool materialFlag54 = in.ReadBool();

  FourCC animType = g_ResFactory->GetResourceTypeById(aParms.GetACSFile());
  if (!g_ResFactory->GetResourceTypeById(staticId).IsValid() && !animType.IsValid())
    return nullptr;

  zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, centroid);

  CMaterialList list;
  if (immovable) // Bool 2
    list.Add(EMaterialTypes::Immovable);

  if (solid) // Bool 3
    list.Add(EMaterialTypes::Solid);

  if (cameraPassthrough) // Bool 4
    list.Add(EMaterialTypes::CameraPassthrough);

  CModelData data;
  if (animType == SBIG('ANCS')) {
    data = CModelData{
        CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.x40_scale, aParms.GetInitialAnimation(), false)};
  } else {
    data = CModelData{CStaticRes(staticId, head.x40_scale)};
  }

  if ((collisionExtent.x() < 0.f || collisionExtent.y() < 0.f || collisionExtent.z() < 0.f) || collisionExtent.isZero())
    aabb = data.GetBounds(head.x10_transform.getRotation());

  return new CScriptActor(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(data), aabb, mass,
                          zMomentum, list, hInfo, dVuln, actParms, looping, active, shaderIdx, xrayAlpha, noThermalHotZ,
                          castsShadow, scaleAdvancementDelta, materialFlag54);
}

CEntity* ScriptLoader::LoadWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 13, "Waypoint"))
    return nullptr;

  SActorHead head = LoadActorHead(in, mgr);

  bool active = in.ReadBool();
  float speed = in.ReadFloat();
  float pause = in.ReadFloat();
  u32 patternTranslate = in.ReadLong();
  u32 patternOrient = in.ReadLong();
  u32 patternFit = in.ReadLong();
  u32 behaviour = in.ReadLong();
  u32 behaviourOrient = in.ReadLong();
  u32 behaviourModifiers = in.ReadLong();
  u32 animation = in.ReadLong();

  return new CScriptWaypoint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, active, speed, pause,
                             patternTranslate, patternOrient, patternFit, behaviour, behaviourOrient,
                             behaviourModifiers, animation);
}

CEntity* ScriptLoader::LoadDoor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 13, "Door") || propCount > 14)
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);
  CAnimationParameters aParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);

  zeus::CVector3f orbitPos = in.Get<zeus::CVector3f>();
  zeus::CVector3f collisionExtent = in.Get<zeus::CVector3f>();
  zeus::CVector3f offset = in.Get<zeus::CVector3f>();

  bool active = in.ReadBool();
  bool open = in.ReadBool();
  bool projectilesCollide = in.ReadBool();
  float animationLength = in.ReadFloat();

  zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, offset);

  if (!g_ResFactory->GetResourceTypeById(aParms.GetACSFile()).IsValid())
    return nullptr;

  CModelData mData{CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.x40_scale, 0, false)};
  if (collisionExtent.isZero())
    aabb = mData.GetBounds(head.x10_transform.getRotation());

  bool isMorphballDoor = false;
  if (propCount == 13) {
    if (aParms.GetACSFile() == MorphballDoorANCS.id)
      isMorphballDoor = true;
  } else if (propCount == 14)
    isMorphballDoor = in.ReadBool();

  return new CScriptDoor(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(mData), actParms,
                         orbitPos, aabb, active, open, projectilesCollide, animationLength, isMorphballDoor);
}

CEntity* ScriptLoader::LoadTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 9, "Trigger"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);

  zeus::CVector3f position = in.Get<zeus::CVector3f>();

  zeus::CVector3f extent = in.Get<zeus::CVector3f>();

  CDamageInfo dInfo(in);

  zeus::CVector3f forceVec = in.Get<zeus::CVector3f>();

  ETriggerFlags flags = ETriggerFlags(in.ReadLong());
  bool active = in.ReadBool();
  bool b2 = in.ReadBool();
  bool b3 = in.ReadBool();

  zeus::CAABox box(-extent * 0.5f, extent * 0.5f);

  const zeus::CTransform& areaXf = mgr.GetWorld()->GetGameAreas()[info.GetAreaId()]->GetTransform();
  zeus::CVector3f orientedForce = areaXf.basis * forceVec;

  return new CScriptTrigger(mgr.AllocateUniqueId(), name, info, position, box, dInfo, orientedForce, flags, active, b2,
                            b3);
}

CEntity* ScriptLoader::LoadTimer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 6, "Timer"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);

  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  bool b1 = in.ReadBool();
  bool b2 = in.ReadBool();
  bool b3 = in.ReadBool();

  return new CScriptTimer(mgr.AllocateUniqueId(), name, info, f1, f2, b1, b2, b3);
}

CEntity* ScriptLoader::LoadCounter(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 5, "Counter"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);

  u32 initial = in.ReadLong();
  u32 max = in.ReadLong();
  bool autoReset = in.ReadBool();
  bool active = in.ReadBool();

  return new CScriptCounter(mgr.AllocateUniqueId(), name, info, initial, max, autoReset, active);
}

CEntity* ScriptLoader::LoadEffect(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 24, "Effect"))
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);

  CAssetId partId(in);
  CAssetId elscId(in);
  bool hotInThermal = in.ReadBool();
  bool noTimerUnlessAreaOccluded = in.ReadBool();
  bool rebuildSystemsOnActivate = in.ReadBool();
  bool active = in.ReadBool();

  if (!partId.IsValid() && !elscId.IsValid())
    return nullptr;

  if (!g_ResFactory->GetResourceTypeById(partId).IsValid() && !g_ResFactory->GetResourceTypeById(elscId).IsValid())
    return nullptr;

  bool useRateInverseCamDist = in.ReadBool();
  float rateInverseCamDist = in.ReadFloat();
  float rateInverseCamDistRate = in.ReadFloat();
  float duration = in.ReadFloat();
  float durationResetWhileVisible = in.ReadFloat();
  bool useRateCamDistRange = in.ReadBool();
  float rateCamDistRangeMin = in.ReadFloat();
  float rateCamDistRangeMax = in.ReadFloat();
  float rateCamDistRangeFarRate = in.ReadFloat();
  bool combatVisorVisible = in.ReadBool();
  bool thermalVisorVisible = in.ReadBool();
  bool xrayVisorVisible = in.ReadBool();
  bool dieWhenSystemsDone = in.ReadBool();

  CLightParameters lParms = LoadLightParameters(in);

  return new CScriptEffect(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, head.x40_scale, partId,
                           elscId, hotInThermal, noTimerUnlessAreaOccluded, rebuildSystemsOnActivate, active,
                           useRateInverseCamDist, rateInverseCamDist, rateInverseCamDistRate, duration,
                           durationResetWhileVisible, useRateCamDistRange, rateCamDistRangeMin, rateCamDistRangeMax,
                           rateCamDistRangeFarRate, combatVisorVisible, thermalVisorVisible, xrayVisorVisible, lParms,
                           dieWhenSystemsDone);
}

CEntity* ScriptLoader::LoadPlatform(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 19, "Platform"))
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);

  zeus::CVector3f extent = in.Get<zeus::CVector3f>();

  zeus::CVector3f centroid = in.Get<zeus::CVector3f>();

  CAssetId staticId = in.Get<CAssetId>();
  CAnimationParameters aParms = LoadAnimationParameters(in);

  CActorParameters actParms = LoadActorParameters(in);

  float speed = in.ReadFloat();
  bool active = in.ReadBool();
  CAssetId dclnId = in.Get<CAssetId>();

  CHealthInfo hInfo(in);

  CDamageVulnerability dInfo(in);

  bool detectCollision = in.ReadBool();
  float xrayAlpha = in.ReadFloat();
  bool rainSplashes = in.ReadBool();
  u32 maxRainSplashes = in.ReadLong();
  u32 rainGenRate = in.ReadLong();

  FourCC animType = g_ResFactory->GetResourceTypeById(aParms.GetACSFile());
  if (!g_ResFactory->GetResourceTypeById(staticId).IsValid() && !animType.IsValid())
    return nullptr;

  zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), extent, centroid);

  FourCC dclnType = g_ResFactory->GetResourceTypeById(dclnId);
  std::optional<TLockedToken<CCollidableOBBTreeGroupContainer>> dclnToken;
  if (dclnType.IsValid()) {
    dclnToken.emplace(g_SimplePool->GetObj({SBIG('DCLN'), dclnId}));
    dclnToken->GetObj();
  }

  CModelData data;
  if (animType == SBIG('ANCS')) {
    data = CModelData{
        CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.x40_scale, aParms.GetInitialAnimation(), true)};
  } else {
    data = CModelData{CStaticRes(staticId, head.x40_scale)};
  }

  if (extent.isZero())
    aabb = data.GetBounds(head.x10_transform.getRotation());

  return new CScriptPlatform(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(data), actParms,
                             aabb, speed, detectCollision, xrayAlpha, active, hInfo, dInfo, dclnToken, rainSplashes,
                             maxRainSplashes, rainGenRate);
}

CEntity* ScriptLoader::LoadSound(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 20, "Sound"))
    return nullptr;

  SActorHead head = LoadActorHead(in, mgr);
  s32 soundId = in.ReadInt32();
  bool active = in.ReadBool();
  float maxDist = in.ReadFloat();
  float distComp = in.ReadFloat();
  float startDelay = in.ReadFloat();
  u32 minVol = in.ReadLong();
  u32 vol = in.ReadLong();
  u32 prio = in.ReadLong();
  u32 pan = in.ReadLong();
  bool loop = in.ReadBool();
  bool nonEmitter = in.ReadBool();
  bool autoStart = in.ReadBool();
  bool occlusionTest = in.ReadBool();
  bool acoustics = in.ReadBool();
  bool worldSfx = in.ReadBool();
  bool allowDuplicates = in.ReadBool();
  s32 pitch = in.ReadInt32();

  if (soundId < 0) {
    Log.report(logvisor::Warning, FMT_STRING("Invalid sound ID specified in Sound {} ({}), dropping..."), head.x0_name,
               info.GetEditorId());
    return nullptr;
  }

  return new CScriptSound(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, u16(soundId), active, maxDist,
                          distComp, startDelay, minVol, vol, 0, prio, pan, 0, loop, nonEmitter, autoStart,
                          occlusionTest, acoustics, worldSfx, allowDuplicates, pitch);
}

CEntity* ScriptLoader::LoadGenerator(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 8, "Generator"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);

  u32 spawnCount = in.ReadLong();
  bool noReuseFollowers = in.ReadBool();
  bool noInheritXf = in.ReadBool();

  zeus::CVector3f offset = in.Get<zeus::CVector3f>();

  bool active = in.ReadBool();
  float minScale = in.ReadFloat();
  float maxScale = in.ReadFloat();

  return new CScriptGenerator(mgr.AllocateUniqueId(), name, info, spawnCount, noReuseFollowers, offset, noInheritXf,
                              active, minScale, maxScale);
}

CEntity* ScriptLoader::LoadDock(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 7, "Dock"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  zeus::CVector3f position = in.Get<zeus::CVector3f>();
  zeus::CVector3f scale = in.Get<zeus::CVector3f>();
  u32 dock = in.ReadLong();
  TAreaId area = in.ReadLong();
  bool loadConnected = in.ReadBool();
  return new CScriptDock(mgr.AllocateUniqueId(), name, info, position, scale, dock, area, active, 0, loadConnected);
}

CEntity* ScriptLoader::LoadCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 14, "Camera"))
    return nullptr;

  SActorHead head = LoadActorHead(in, mgr);

  bool active = in.ReadBool();
  float shotDuration = in.ReadFloat();
  bool lookAtPlayer = in.ReadBool();
  bool outOfPlayerEye = in.ReadBool();
  bool intoPlayerEye = in.ReadBool();
  bool drawPlayer = in.ReadBool();
  bool disableInput = in.ReadBool();
  bool b7 = in.ReadBool();
  bool finishCineSkip = in.ReadBool();
  float fov = in.ReadFloat();
  bool checkFailsafe = in.ReadBool();

  bool disableOutOfInto = false;
  if (propCount > 14)
    disableOutOfInto = in.ReadBool();

  s32 flags = s32(lookAtPlayer) | s32(outOfPlayerEye) << 1 | s32(intoPlayerEye) << 2 | s32(b7) << 3 |
              s32(finishCineSkip) << 4 | s32(disableInput) << 5 | s32(drawPlayer) << 6 | s32(checkFailsafe) << 7 |
              s32(disableOutOfInto) << 9;

  return new CCinematicCamera(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, active, shotDuration,
                              fov / CCameraManager::Aspect(), CCameraManager::NearPlane(), CCameraManager::FarPlane(),
                              CCameraManager::Aspect(), flags);
}

CEntity* ScriptLoader::LoadCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 6, "CameraWaypoint"))
    return nullptr;

  SActorHead head = LoadActorHead(in, mgr);

  bool active = in.ReadBool();
  float f1 = in.ReadFloat();
  u32 w1 = in.ReadLong();

  return new CScriptCameraWaypoint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, active, f1, w1);
}

CEntity* ScriptLoader::LoadNewIntroBoss(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 13, "NewIntroBoss"))
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);

  std::pair<bool, u32> pcount = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  CActorParameters actParms = LoadActorParameters(in);

  float minTurnAngle = in.ReadFloat();
  CAssetId projectile(in);

  CDamageInfo dInfo(in);

  CAssetId beamContactFxId(in);
  CAssetId beamPulseFxId(in);
  CAssetId beamTextureId(in);
  CAssetId beamGlowTextureId(in);

  const CAnimationParameters& aParms = pInfo.GetAnimationParameters();
  FourCC animType = g_ResFactory->GetResourceTypeById(aParms.GetACSFile());
  if (animType != SBIG('ANCS'))
    return nullptr;

  CAnimRes res(aParms.GetACSFile(), aParms.GetCharacter(), head.x40_scale, aParms.GetInitialAnimation(), true);

  return new MP1::CNewIntroBoss(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, CModelData{res}, pInfo,
                                actParms, minTurnAngle, projectile, dInfo, beamContactFxId, beamPulseFxId,
                                beamTextureId, beamGlowTextureId);
}

CEntity* ScriptLoader::LoadSpawnPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 35, "SpawnPoint"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);

  zeus::CVector3f position = in.Get<zeus::CVector3f>();

  zeus::CVector3f rotation = in.Get<zeus::CVector3f>();

  rstl::reserved_vector<u32, int(CPlayerState::EItemType::Max)> itemCounts;
  itemCounts.resize(size_t(CPlayerState::EItemType::Max), 0);
  for (int i = 0; i < propCount - 6; ++i)
    itemCounts[i] = in.ReadLong();

  bool defaultSpawn = in.ReadBool();
  bool active = in.ReadBool();
  bool morphed = false;
  if (propCount > 34)
    morphed = in.ReadBool();

  return new CScriptSpawnPoint(mgr.AllocateUniqueId(), name, info, ConvertEditorEulerToTransform4f(rotation, position),
                               itemCounts, defaultSpawn, active, morphed);
}

CEntity* ScriptLoader::LoadCameraHint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (propCount > 25) {
    Log.report(logvisor::Warning, FMT_STRING("Too many props ({} > 25) for CameraHint entity"), propCount);
    return nullptr;
  }

  SActorHead head = LoadActorHead(in, mgr);

  bool active = in.ReadBool();
  u32 prio = in.ReadLong();
  auto behaviour = CBallCamera::EBallCameraBehaviour(in.ReadLong());
  u32 overrideFlags = LoadParameterFlags(in);
  overrideFlags |= in.ReadBool() << 22;
  float minDist = in.ReadFloat();
  overrideFlags |= in.ReadBool() << 23;
  float maxDist = in.ReadFloat();
  overrideFlags |= in.ReadBool() << 24;
  float backwardsDist = in.ReadFloat();
  overrideFlags |= in.ReadBool() << 25;
  zeus::CVector3f lookAtOffset = in.Get<zeus::CVector3f>();
  overrideFlags |= in.ReadBool() << 26;
  zeus::CVector3f chaseLookAtOffset = in.Get<zeus::CVector3f>();
  zeus::CVector3f ballToCam = in.Get<zeus::CVector3f>();
  overrideFlags |= in.ReadBool() << 27;
  float fov = in.ReadFloat();
  overrideFlags |= in.ReadBool() << 28;
  float attitudeRange = zeus::degToRad(in.ReadFloat());
  overrideFlags |= in.ReadBool() << 29;
  float azimuthRange = zeus::degToRad(in.ReadFloat());
  overrideFlags |= in.ReadBool() << 30;
  float anglePerSecond = zeus::degToRad(in.ReadFloat());
  float clampVelRange = in.ReadFloat();
  float clampRotRange = zeus::degToRad(in.ReadFloat());
  overrideFlags |= in.ReadBool() << 31;
  float elevation = in.ReadFloat();
  float interpolateTime = in.ReadFloat();
  float clampVelTime = in.ReadFloat();
  float controlInterpDur = in.ReadFloat();

  return new CScriptCameraHint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, active, prio, behaviour,
                               overrideFlags, minDist, maxDist, backwardsDist, lookAtOffset, chaseLookAtOffset,
                               ballToCam, fov, attitudeRange, azimuthRange, anglePerSecond, clampVelRange,
                               clampRotRange, elevation, interpolateTime, clampVelTime, controlInterpDur);
}

CEntity* ScriptLoader::LoadPickup(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 18, "Pickup"))
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);
  zeus::CVector3f extent =  in.Get<zeus::CVector3f>();
  zeus::CVector3f offset =  in.Get<zeus::CVector3f>();
  CPlayerState::EItemType itemType = CPlayerState::EItemType(in.ReadLong());
  u32 capacity = in.ReadLong();
  u32 amount = in.ReadLong();
  float possibility = in.ReadFloat();
  float lifeTime = in.ReadFloat();
  float fadeInTime = in.ReadFloat();
  CAssetId staticModel = in.Get<CAssetId>();
  CAnimationParameters animParms = LoadAnimationParameters(in);
  CActorParameters actorParms = LoadActorParameters(in);
  bool active = in.ReadBool();
  float startDelay = in.ReadFloat();
  CAssetId pickupEffect(in);

  FourCC acsType = g_ResFactory->GetResourceTypeById(animParms.GetACSFile());
  if (g_ResFactory->GetResourceTypeById(staticModel) == 0 && acsType == 0)
    return nullptr;

  zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), extent, offset);

  CModelData data;

  if (acsType == SBIG('ANCS')) {
    data = CModelData{CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), head.x40_scale,
                               animParms.GetInitialAnimation(), true)};
  } else {
    data = CModelData{CStaticRes(staticModel, head.x40_scale)};
  }

  if (extent.isZero()) {
    aabb = data.GetBounds(head.x10_transform.getRotation());
  }

  return new CScriptPickup(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(data), actorParms,
                           aabb, itemType, amount, capacity, pickupEffect, possibility, lifeTime, fadeInTime,
                           startDelay, active);
}

CEntity* ScriptLoader::LoadMemoryRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 3, "MemoryRelay") || propCount > 4)
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool b1 = in.ReadBool();
  bool b2 = in.ReadBool();
  bool b3 = false;
  if (propCount > 3)
    b3 = in.ReadBool();

  return new CScriptMemoryRelay(mgr.AllocateUniqueId(), name, info, b1, b2, b3);
}

CEntity* ScriptLoader::LoadRandomRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 5, "RandomRelay"))
    return nullptr;
  std::string name = mgr.HashInstanceName(in);
  u32 sendSetSize = in.ReadLong();
  u32 sendSetVariance = in.ReadLong();
  bool percentSize = in.ReadBool();
  bool active = in.ReadBool();

  return new CScriptRandomRelay(mgr.AllocateUniqueId(), name, info, sendSetSize, sendSetVariance, percentSize, active);
}

CEntity* ScriptLoader::LoadRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 2, "Relay") || propCount > 3)
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  if (propCount >= 3)
    in.ReadLong();
  bool b1 = in.ReadBool();

  return new CScriptRelay(mgr.AllocateUniqueId(), name, info, b1);
}

CEntity* ScriptLoader::LoadBeetle(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 16, "Beetle"))
    return nullptr;
  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xfrm = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  std::pair<bool, u32> pcount = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters aParams = LoadActorParameters(in);
  CDamageInfo touchDamage(in);
  zeus::CVector3f tailAimReference =  in.Get<zeus::CVector3f>();
  float unused = in.ReadFloat();
  CDamageVulnerability tailVuln(in);
  CDamageVulnerability platingVuln(in);
  CAssetId tailModel = in.Get<CAssetId>();
  MP1::CBeetle::EEntranceType entranceType = MP1::CBeetle::EEntranceType(in.ReadLong());
  float initialAttackDelay = in.ReadFloat();
  float retreatTime = in.ReadFloat();

  FourCC animType = g_ResFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile());
  if (animType != SBIG('ANCS'))
    return nullptr;

  std::optional<CStaticRes> tailRes;
  if (flavor == CPatterned::EFlavorType::One)
    tailRes.emplace(CStaticRes(tailModel, scale));

  const CAnimationParameters& animParams = pInfo.GetAnimationParameters();
  CAnimRes animRes(animParams.GetACSFile(), animParams.GetCharacter(), scale, animParams.GetInitialAnimation(), true);

  return new MP1::CBeetle(mgr.AllocateUniqueId(), name, info, xfrm, CModelData{animRes}, pInfo, flavor, entranceType,
                          touchDamage, platingVuln, tailAimReference, initialAttackDelay, retreatTime, unused, tailVuln,
                          aParams, tailRes);
}

CEntity* ScriptLoader::LoadHUDMemo(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (propCount != 5 && !EnsurePropertyCount(propCount, 6, "HUDMemo")) {
    return nullptr;
  }

  const std::string name = mgr.HashInstanceName(in);
  const CHUDMemoParms hParms(in);
  auto displayType = CScriptHUDMemo::EDisplayType::MessageBox;
  if (propCount == 6) {
    displayType = CScriptHUDMemo::EDisplayType(in.ReadLong());
  }
  const CAssetId message = in.Get<CAssetId>();
  const bool active = in.ReadBool();

  return new CScriptHUDMemo(mgr.AllocateUniqueId(), name, info, hParms, displayType, message, active);
}

CEntity* ScriptLoader::LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& in, int propCount,
                                                const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 10, "CameraFilterKeyframe"))
    return nullptr;
  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  EFilterType ftype = EFilterType(in.ReadLong());
  EFilterShape shape = EFilterShape(in.ReadLong());
  u32 filterIdx = in.ReadLong();
  u32 unk = in.ReadLong();
  zeus::CColor color = in.Get<zeus::CColor>();
  float timeIn = in.ReadFloat();
  float timeOut = in.ReadFloat();
  CAssetId txtr(in);

  return new CScriptCameraFilterKeyframe(mgr.AllocateUniqueId(), name, info, ftype, shape, filterIdx, unk, color,
                                         timeIn, timeOut, txtr, active);
}

CEntity* ScriptLoader::LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 7, "CameraBlurKeyframe"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  EBlurType type = EBlurType(in.ReadLong());
  float amount = in.ReadFloat();
  u32 unk = in.ReadLong();
  float timeIn = in.ReadFloat();
  float timeOut = in.ReadFloat();

  return new CScriptCameraBlurKeyframe(mgr.AllocateUniqueId(), name, info, type, amount, unk, timeIn, timeOut, active);
}

u32 ClassifyVector(const zeus::CVector3f& dir) {
  zeus::CVector3f absDir(std::fabs(dir.x()), std::fabs(dir.y()), std::fabs(dir.z()));
  u32 max = (absDir.x() > absDir.y() ? 0 : 1);
  max = (absDir[max] > absDir.z() ? max : 2);

  bool positive = (absDir[max] == dir[max]);
  if (max == 0)
    return (positive ? 0x08 : 0x04);
  else if (max == 1)
    return (positive ? 0x01 : 0x02);
  else if (max == 2)
    return (positive ? 0x10 : 0x20);

  return 0;
}

u32 TransformDamagableTriggerFlags(CStateManager& mgr, TAreaId aId, u32 flags) {
  CGameArea* area = mgr.GetWorld()->GetGameAreas().at(u32(aId)).get();
  zeus::CTransform rotation = area->GetTransform().getRotation();

  u32 ret = 0;
  if (flags & 0x01)
    ret |= ClassifyVector(rotation * zeus::skForward);
  if (flags & 0x02)
    ret |= ClassifyVector(rotation * zeus::skBack);
  if (flags & 0x04)
    ret |= ClassifyVector(rotation * zeus::skLeft);
  if (flags & 0x08)
    ret |= ClassifyVector(rotation * zeus::skRight);
  if (flags & 0x10)
    ret |= ClassifyVector(rotation * zeus::skUp);
  if (flags & 0x20)
    ret |= ClassifyVector(rotation * zeus::skDown);
  return ret;
}

CEntity* ScriptLoader::LoadDamageableTrigger(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 12, "DamageableTrigger"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f position( in.Get<zeus::CVector3f>());
  zeus::CVector3f volume( in.Get<zeus::CVector3f>());

  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  u32 triggerFlags = in.ReadLong();
  triggerFlags = TransformDamagableTriggerFlags(mgr, info.GetAreaId(), triggerFlags);
  CAssetId patternTex1 = in.Get<CAssetId>();
  CAssetId patternTex2 = in.Get<CAssetId>();
  CAssetId colorTex = in.Get<CAssetId>();
  CScriptDamageableTrigger::ECanOrbit canOrbit = CScriptDamageableTrigger::ECanOrbit(in.ReadBool());
  bool active = in.ReadBool();
  CVisorParameters vParms = LoadVisorParameters(in);
  return new CScriptDamageableTrigger(mgr.AllocateUniqueId(), name, info, position, volume, hInfo, dVuln, triggerFlags,
                                      patternTex1, patternTex2, colorTex, canOrbit, active, vParms);
}

CEntity* ScriptLoader::LoadDebris(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 18, "Debris"))
    return nullptr;

  const SScaledActorHead head = LoadScaledActorHead(in, mgr);
  const float zImpulse = in.ReadFloat();
  const zeus::CVector3f velocity =  in.Get<zeus::CVector3f>();
  zeus::CColor endsColor = in.Get<zeus::CColor>();
  const float mass = in.ReadFloat();
  const float restitution = in.ReadFloat();
  const float duration = in.ReadFloat();
  const auto scaleType = CScriptDebris::EScaleType(in.ReadLong());
  const bool randomAngImpulse = in.ReadBool();
  const CAssetId model = in.Get<CAssetId>();
  const CActorParameters aParams = LoadActorParameters(in);
  const CAssetId particleId = in.Get<CAssetId>();
  const zeus::CVector3f particleScale =  in.Get<zeus::CVector3f>();
  const bool b1 = in.ReadBool();
  const bool active = in.ReadBool();

  if (!g_ResFactory->GetResourceTypeById(model).IsValid()) {
    return nullptr;
  }

  return new CScriptDebris(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform,
                           CModelData{CStaticRes(model, head.x40_scale)}, aParams, particleId, particleScale, zImpulse,
                           velocity, endsColor, mass, restitution, duration, scaleType, b1, randomAngImpulse, active);
}

CEntity* ScriptLoader::LoadCameraShaker(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 9, "CameraShaker"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CCameraShakeData shakeData = CCameraShakeData::LoadCameraShakeData(in);
  bool active = in.ReadBool();

  return new CScriptCameraShaker(mgr.AllocateUniqueId(), name, info, active, shakeData);
}

CEntity* ScriptLoader::LoadActorKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 7, "ActorKeyframe"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  s32 animId = in.ReadInt32();
  bool looping = in.ReadBool();
  float lifetime = in.ReadFloat();
  bool active = in.ReadBool();
  u32 fadeOut = in.ReadLong();
  float totalPlayback = in.ReadFloat();

  if (animId == -1)
    return nullptr;

  return new CScriptActorKeyframe(mgr.AllocateUniqueId(), name, info, animId, looping, lifetime, false, fadeOut, active,
                                  totalPlayback);
}

CEntity* ScriptLoader::LoadWater(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 63, "Water"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f position = in.Get<zeus::CVector3f>();
  zeus::CVector3f extent = in.Get<zeus::CVector3f>();
  CDamageInfo dInfo(in);
  zeus::CVector3f orientedForce = in.Get<zeus::CVector3f>();
  ETriggerFlags triggerFlags = ETriggerFlags(in.ReadLong()) | ETriggerFlags::DetectProjectiles1 |
                               ETriggerFlags::DetectProjectiles2 | ETriggerFlags::DetectProjectiles3 |
                               ETriggerFlags::DetectProjectiles4 | ETriggerFlags::DetectBombs |
                               ETriggerFlags::DetectPowerBombs | ETriggerFlags::DetectProjectiles5 |
                               ETriggerFlags::DetectProjectiles6 | ETriggerFlags::DetectProjectiles7;
  bool thermalCold = in.ReadBool();
  bool displaySurface = in.ReadBool();
  CAssetId patternMap1 = in.Get<CAssetId>();
  CAssetId patternMap2 = in.Get<CAssetId>();
  CAssetId colorMap = in.Get<CAssetId>();
  CAssetId bumpMap = in.Get<CAssetId>();
  CAssetId _envMap = in.Get<CAssetId>();
  CAssetId _envBumpMap = in.Get<CAssetId>();
  zeus::CVector3f _bumpLightDir  =in.Get<zeus::CVector3f>();

  zeus::CVector3f bumpLightDir = _bumpLightDir;
  if (!bumpLightDir.canBeNormalized())
    bumpLightDir.assign(0.f, 0.f, -1.f);

  float bumpScale = 1.f / in.ReadFloat();
  float morphInTime = in.ReadFloat();
  float morphOutTime = in.ReadFloat();
  bool active = in.ReadBool();
  auto fluidType = EFluidType(in.ReadLong());
  bool b4 = in.ReadBool();
  float alpha = in.ReadFloat();
  CFluidUVMotion uvMotion = LoadFluidUVMotion(in);

  float turbSpeed = in.ReadFloat();
  float turbDistance = in.ReadFloat();
  float turbFreqMax = in.ReadFloat();
  float turbFreqMin = in.ReadFloat();
  float turbPhaseMax = zeus::degToRad(in.ReadFloat());
  float turbPhaseMin = zeus::degToRad(in.ReadFloat());
  float turbAmplitudeMax = in.ReadFloat();
  float turbAmplitudeMin = in.ReadFloat();
  zeus::CColor splashColor = in.Get<zeus::CColor>();
  zeus::CColor insideFogColor = in.Get<zeus::CColor>();
  CAssetId splashParticle1 = in.Get<CAssetId>();
  CAssetId splashParticle2 = in.Get<CAssetId>();
  CAssetId splashParticle3 = in.Get<CAssetId>();
  CAssetId visorRunoffParticle = in.Get<CAssetId>();
  CAssetId unmorphVisorRunoffParticle = in.Get<CAssetId>();
  u32 visorRunoffSfx = in.ReadLong();
  u32 unmorphVisorRunoffSfx = in.ReadLong();
  u32 splashSfx1 = in.ReadLong();
  u32 splashSfx2 = in.ReadLong();
  u32 splashSfx3 = in.ReadLong();
  float tileSize = in.ReadFloat();
  u32 tileSubdivisions = in.ReadLong();
  float specularMin = in.ReadFloat();
  float specularMax = in.ReadFloat();
  float reflectionSize = in.ReadFloat();
  float rippleIntensity = in.ReadFloat();
  float reflectionBlend = in.ReadFloat();
  float fogBias = in.ReadFloat();
  float fogMagnitude = in.ReadFloat();
  float fogSpeed = in.ReadFloat();
  zeus::CColor fogColor = in.Get<zeus::CColor>();
  CAssetId lightmap = in.Get<CAssetId>();
  float unitsPerLightmapTexel = in.ReadFloat();
  float alphaInTime = in.ReadFloat();
  float alphaOutTime = in.ReadFloat();
  u32 w21 = in.ReadLong();
  u32 w22 = in.ReadLong();
  bool b5 = in.ReadBool();

  std::unique_ptr<u32[]> bitset;
  u32 bitVal0 = 0;
  u32 bitVal1 = 0;

  if (b5) {
    bitVal0 = in.ReadShort();
    bitVal1 = in.ReadShort();
    u32 len = ((bitVal0 * bitVal1) + 31) / 31;
    bitset.reset(new u32[len]);
    in.Get(reinterpret_cast<u8*>(bitset.get()), len * 4);
  }

  zeus::CAABox box(-extent * 0.5f, extent * 0.5f);

  CAssetId envBumpMap;
  if (!bumpMap.IsValid())
    envBumpMap = _envBumpMap;

  CAssetId envMap;
  if (!bumpMap.IsValid())
    envMap = _envMap;

  return new CScriptWater(mgr, mgr.AllocateUniqueId(), name, info, position, box, dInfo, orientedForce, triggerFlags,
                          thermalCold, displaySurface, patternMap1, patternMap2, colorMap, bumpMap, envMap, envBumpMap,
                          {}, bumpLightDir, bumpScale, morphInTime, morphOutTime, active, fluidType, b4, alpha,
                          uvMotion, turbSpeed, turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax, turbPhaseMin,
                          turbAmplitudeMax, turbAmplitudeMin, splashColor, insideFogColor, splashParticle1,
                          splashParticle2, splashParticle3, visorRunoffParticle, unmorphVisorRunoffParticle,
                          visorRunoffSfx, unmorphVisorRunoffSfx, splashSfx1, splashSfx2, splashSfx3, tileSize,
                          tileSubdivisions, specularMin, specularMax, reflectionSize, rippleIntensity, reflectionBlend,
                          fogBias, fogMagnitude, fogSpeed, fogColor, lightmap, unitsPerLightmapTexel, alphaInTime,
                          alphaOutTime, w21, w22, b5, bitVal0, bitVal1, std::move(bitset));
}

CEntity* ScriptLoader::LoadWarWasp(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 13, "WarWasp"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransformPivotOnly(in);
  zeus::CVector3f scale = in.Get<zeus::CVector3f>();

  std::pair<bool, u32> verifyPair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!verifyPair.first)
    return nullptr;

  CPatternedInfo pInfo(in, verifyPair.second);
  CActorParameters actorParms = LoadActorParameters(in);
  CPatterned::EColliderType collider = CPatterned::EColliderType(in.ReadBool());
  CDamageInfo damageInfo1(in);
  CAssetId projectileWeapon = in.Get<CAssetId>();
  CDamageInfo projectileDamage(in);
  CAssetId projectileVisorParticle = in.Get<CAssetId>();
  u32 projectileVisorSfx = in.ReadLong();

  const CAnimationParameters& aParms = pInfo.GetAnimationParameters();
  FourCC animType = g_ResFactory->GetResourceTypeById(aParms.GetACSFile());
  if (animType != SBIG('ANCS'))
    return nullptr;

  CAnimRes res(aParms.GetACSFile(), aParms.GetCharacter(), scale, aParms.GetInitialAnimation(), true);
  CModelData mData(res);
  return new MP1::CWarWasp(mgr.AllocateUniqueId(), name, info, xf, std::move(mData), pInfo, flavor, collider,
                           damageInfo1, actorParms, projectileWeapon, projectileDamage, projectileVisorParticle,
                           projectileVisorSfx);
}

CEntity* ScriptLoader::LoadSpacePirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 34, "SpacePirate"))
    return nullptr;
  SScaledActorHead head = LoadScaledActorHead(in, mgr);
  std::pair<bool, u32> verifyPair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!verifyPair.first)
    return nullptr;

  CPatternedInfo pInfo(in, verifyPair.second);
  CActorParameters aParams = LoadActorParameters(in);
  CAnimationParameters& animParms = pInfo.GetAnimationParameters();

  if (g_ResFactory->GetResourceTypeById(animParms.GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  if (animParms.GetCharacter() == 0) {
    Log.report(logvisor::Warning,
               FMT_STRING("SpacePirate <{}> has AnimationInformation property with invalid character selected"),
               head.x0_name);
    animParms.SetCharacter(2);
  }

  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), head.x40_scale,
                            animParms.GetInitialAnimation(), true));

  return new MP1::CSpacePirate(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(mData),
                               aParams, pInfo, in, propCount);
}

CEntity* ScriptLoader::LoadFlyingPirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 35, "FlyingPirate"))
    return nullptr;

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (g_ResFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), actHead.x40_scale,
                            animParms.GetInitialAnimation(), true));

  return new MP1::CFlyingPirate(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, std::move(mData),
                                actParms, pInfo, in, propCount);
}

CEntity* ScriptLoader::LoadElitePirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, skElitePiratePropCount, "ElitePirate"))
    return nullptr;

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  MP1::CElitePirateData elitePirateData(in, propCount);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            actHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CElitePirate(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, std::move(mData),
                               pInfo, actParms, elitePirateData);
}

CEntity* ScriptLoader::LoadMetroidBeta(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 23, "MetroidBeta"))
    return nullptr;

#if 0
  std::string name = mgr.HashInstanceName(in);
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  MP1::CMetroidBetaData metData(in);
  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), scale, animParms.GetInitialAnimation(), true));

  return new MP1::CMetroidBeta(mgr.AllocateUniqueId(), name, info, xf, std::move(mData), pInfo, actParms, metData);
#else
  return nullptr;
#endif
}

CEntity* ScriptLoader::LoadChozoGhost(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 31, "ChozoGhost"))
    return nullptr;

  SScaledActorHead actorHead = LoadScaledActorHead(in, mgr);

  auto pair = CPatternedInfo::HasCorrectParameterCount(in);

  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  CAssetId wpsc1(in);
  CDamageInfo dInfo1(in);
  CAssetId wpsc2(in);
  CDamageInfo dInfo2(in);
  MP1::CChozoGhost::CBehaveChance behaveChance1(in);
  MP1::CChozoGhost::CBehaveChance behaveChance2(in);
  MP1::CChozoGhost::CBehaveChance behaveChance3(in);
  s16 sId1 = CSfxManager::TranslateSFXID(in.ReadLong());
  float f5 = in.ReadFloat();
  s16 sId2 = CSfxManager::TranslateSFXID(in.ReadLong());
  s16 sId3 = CSfxManager::TranslateSFXID(in.ReadLong());
  u32 w1 = in.ReadLong();
  float f6 = in.ReadFloat();
  u32 w2 = in.ReadLong();
  float f7 = in.ReadFloat();
  CAssetId partId(in);
  s16 sId4 = CSfxManager::TranslateSFXID(in.ReadLong());
  float f8 = in.ReadFloat();
  float f9 = in.ReadFloat();
  u32 w3 = in.ReadLong();
  u32 w4 = in.ReadLong();

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (g_ResFactory->GetResourceTypeById(animParms.GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  CModelData mData(CAnimRes(animParms.GetACSFile(), 0, actorHead.x40_scale, animParms.GetInitialAnimation(), true));

  return new MP1::CChozoGhost(mgr.AllocateUniqueId(), actorHead.x0_name, info, actorHead.x10_transform,
                              std::move(mData), actParms, pInfo, f1, f2, f3, f4, wpsc1, dInfo1, wpsc2, dInfo2,
                              behaveChance1, behaveChance2, behaveChance3, sId1, f5, sId2, sId3, w1, f6, w2, f7, partId,
                              sId4, f8, f9, w3, w4);
}

CEntity* ScriptLoader::LoadCoverPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 9, "CoverPoint"))
    return nullptr;

  SActorHead head = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  u32 flags = in.ReadLong();
  bool crouch = in.ReadBool();
  float horizontalAngle = in.ReadFloat();
  float verticalAngle = in.ReadFloat();
  float coverTime = in.ReadFloat();

  return new CScriptCoverPoint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, active, flags, crouch,
                               horizontalAngle, verticalAngle, coverTime);
}

CEntity* ScriptLoader::LoadSpiderBallWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 5, "SpiderBallWaypoint"))
    return nullptr;

  SActorHead actHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  u32 w1 = in.ReadLong();

  return new CScriptSpiderBallWaypoint(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, active,
                                       w1);
}

CEntity* ScriptLoader::LoadBloodFlower(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 18, "BloodFlower"))
    return nullptr;

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  CAssetId partId1(in);
  CAssetId wpsc1(in);
  CAssetId wpsc2(in);
  CDamageInfo dInfo1(in);
  CDamageInfo dInfo2(in);
  CDamageInfo dInfo3(in);
  CAssetId partId2(in);
  CAssetId partId3(in);
  CAssetId partId4(in);
  float f1 = in.ReadFloat();
  CAssetId partId5(in);
  u32 soundId = in.ReadLong();

  if (g_ResFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            actHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CBloodFlower(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, std::move(mData),
                               pInfo, partId1, wpsc1, actParms, wpsc2, dInfo1, dInfo2, dInfo3, partId2, partId3,
                               partId4, f1, partId5, soundId);
}

CEntity* ScriptLoader::LoadFlickerBat(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 10, "FlickerBat"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  bool collider = in.ReadBool();
  bool excludePlayer = in.ReadBool();
  bool enableLineOfSight = in.ReadBool();

  if (g_ResFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));
  return new MP1::CFlickerBat(mgr.AllocateUniqueId(), name, flavor, info, xf, std::move(mData), pInfo,
                              CPatterned::EColliderType(collider), excludePlayer, actParms, enableLineOfSight);
}

CEntity* ScriptLoader::LoadPathCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (propCount > 15) {
    Log.report(logvisor::Warning, FMT_STRING("Too many props ({} > 15) for PathCamera entity"), propCount);
    return nullptr;
  }

  SActorHead aHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  u32 flags = LoadParameterFlags(in);
  float lengthExtent = in.ReadFloat();
  float filterMag = in.ReadFloat();
  float filterProportion = in.ReadFloat();
  CPathCamera::EInitialSplinePosition initPos = CPathCamera::EInitialSplinePosition(in.ReadLong());
  float minEaseDist = in.ReadFloat();
  float maxEaseDist = in.ReadFloat();

  return new CPathCamera(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, lengthExtent,
                         filterMag, filterProportion, minEaseDist, maxEaseDist, flags, initPos);
}

CEntity* ScriptLoader::LoadGrapplePoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 5, "GrapplePoint"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CTransform grappleXf = LoadEditorTransform(in);
  bool active = in.ReadBool();
  CGrappleParameters parameters = LoadGrappleParameters(in);
  return new CScriptGrapplePoint(mgr.AllocateUniqueId(), name, info, grappleXf, active, parameters);
}

CEntity* ScriptLoader::LoadPuddleSpore(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 16, "PuddleSpore"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();

  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  bool b1 = in.ReadBool();
  CAssetId w1(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  CAssetId w2(in);
  CDamageInfo dInfo(in);

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (g_ResFactory->GetResourceTypeById(animParms.GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), scale, animParms.GetInitialAnimation(), true));
  return new MP1::CPuddleSpore(mgr.AllocateUniqueId(), name, flavor, info, xf, std::move(mData), pInfo,
                               CPatterned::EColliderType(b1), w1, f1, f2, f3, f4, f5, actParms, w2, dInfo);
}

CEntity* ScriptLoader::LoadDebugCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                               const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "DebugCameraWaypoint"))
    return nullptr;

  SActorHead actHead = LoadActorHead(in, mgr);
  u32 w1 = in.ReadLong();
  return new CScriptDebugCameraWaypoint(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, w1);
}

CEntity* ScriptLoader::LoadSpiderBallAttractionSurface(CStateManager& mgr, CInputStream& in, int propCount,
                                                       const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 5, "SpiderBallAttractionSurface"))
    return nullptr;
  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  bool active = in.ReadBool();
  return new CScriptSpiderBallAttractionSurface(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform,
                                                aHead.x40_scale, active);
}

CEntity* ScriptLoader::LoadPuddleToadGamma(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 17, "PuddleToadGamma"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  float suckForceMultiplier = in.ReadFloat();
  float suckAngle = in.ReadFloat();
  float playerSuckRange = in.ReadFloat();
  zeus::CVector3f localShootDir =  in.Get<zeus::CVector3f>();
  float playerShootSpeed = in.ReadFloat();
  float shouldAttackWaitTime = in.ReadFloat();
  float spotPlayerWaitTime = in.ReadFloat();
  CDamageInfo playerShootDamage(in);
  CDamageInfo dInfo2(in);
  CAssetId collisionData(in);
  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), scale, animParms.GetInitialAnimation(), true));
  return new MP1::CPuddleToadGamma(mgr.AllocateUniqueId(), name, flavor, info, xf, std::move(mData), pInfo, actParms,
                                   suckForceMultiplier, suckAngle, playerSuckRange, localShootDir, playerShootSpeed,
                                   shouldAttackWaitTime, spotPlayerWaitTime, playerShootDamage, dInfo2, collisionData);
}

CEntity* ScriptLoader::LoadDistanceFog(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 8, "DistanceFog"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  u32 mode = in.ReadLong();
  zeus::CColor color = in.Get<zeus::CColor>();
  zeus::CVector2f range = in.Get<zeus::CVector2f>();
  float colorDelta = in.ReadFloat();
  zeus::CVector2f rangeDelta = in.Get<zeus::CVector2f>();
  bool expl = in.ReadBool();
  bool active = in.ReadBool();
  ERglFogMode realMode = ERglFogMode::None;

  if (mode == 0)
    realMode = ERglFogMode::None;
  else if (mode == 1)
    realMode = ERglFogMode::PerspLin;
  else if (mode == 2)
    realMode = ERglFogMode::PerspExp;
  else if (mode == 3)
    realMode = ERglFogMode::PerspExp2;
  else if (mode == 4)
    realMode = ERglFogMode::PerspRevExp;
  else if (mode == 5)
    realMode = ERglFogMode::PerspRevExp2;

  return new CScriptDistanceFog(mgr.AllocateUniqueId(), name, info, realMode, color, range, colorDelta, rangeDelta,
                                expl, active, 0.f, 0.f, 0.f, 0.f);
}

CEntity* ScriptLoader::LoadFireFlea(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 9, "FireFlea"))
    return nullptr;

  SScaledActorHead acthead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  in.ReadBool();
  in.ReadBool();
  float f1 = in.ReadFloat();

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            acthead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CFireFlea(mgr.AllocateUniqueId(), acthead.x0_name, info, acthead.x10_transform, std::move(mData),
                            actParms, pInfo, f1);
}

CEntity* ScriptLoader::LoadMetaree(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 12, "Metaree"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();

  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters aParms = LoadActorParameters(in);
  CDamageInfo dInfo(in);
  float f1 = in.ReadFloat();
  zeus::CVector3f vec =  in.Get<zeus::CVector3f>();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();

  if (g_ResFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  float f4 = in.ReadFloat();

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));
  return new MP1::CMetaree(mgr.AllocateUniqueId(), name, CPatterned::EFlavorType::Zero, info, xf, std::move(mData),
                           pInfo, dInfo, f1, vec, f2, EBodyType::Invalid, f3, f4, aParms);
}

CEntity* ScriptLoader::LoadDockAreaChange(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 3, "DockAreaChange"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  s32 w1 = in.ReadInt32();
  bool active = in.ReadBool();

  return new CScriptDockAreaChange(mgr.AllocateUniqueId(), name, info, w1, active);
}

CEntity* ScriptLoader::LoadActorRotate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 6, "ActorRotate"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f rotation =  in.Get<zeus::CVector3f>();
  float scale = in.ReadFloat();
  bool updateActors = in.ReadBool();
  bool updateOnCreation = in.ReadBool();
  bool active = in.ReadBool();

  return new CScriptActorRotate(mgr.AllocateUniqueId(), name, info, rotation, scale, updateActors, updateOnCreation,
                                active);
}

CEntity* ScriptLoader::LoadSpecialFunction(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 15, "SpecialFunction"))
    return nullptr;

  SActorHead head = LoadActorHead(in, mgr);
  CScriptSpecialFunction::ESpecialFunction specialFunction =
      CScriptSpecialFunction::ESpecialFunction(in.ReadLong());
  std::string str = in.Get<std::string>();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  u32 w2 = in.ReadLong();
  u32 w3 = in.ReadLong();
  CPlayerState::EItemType w4 = CPlayerState::EItemType(in.ReadLong());
  bool active1 = in.ReadBool();
  float f4 = in.ReadFloat();
  s16 w5 = in.ReadLong() & 0xFFFF;
  s16 w6 = in.ReadLong() & 0xFFFF;
  s16 w7 = in.ReadLong() & 0xFFFF;
  if (specialFunction == CScriptSpecialFunction::ESpecialFunction::FogVolume ||
      specialFunction == CScriptSpecialFunction::ESpecialFunction::RadialDamage)
    return nullptr;

  return new CScriptSpecialFunction(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, specialFunction,
                                    str, f1, f2, f3, f4, zeus::skZero3f, zeus::skBlack, active1, CDamageInfo(), w2, w3,
                                    w4, w5, w6, w7);
}

CEntity* ScriptLoader::LoadSpankWeed(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 11, "SpankWeed"))
    return nullptr;
  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  in.ReadBool();
  float maxDetectionRange = in.ReadFloat();
  float maxHearingRange = in.ReadFloat();
  float maxSightRange = in.ReadFloat();
  float hideTime = in.ReadFloat();

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (!animParms.GetACSFile().IsValid())
    return nullptr;

  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), aHead.x40_scale,
                            animParms.GetInitialAnimation(), true));
  return new MP1::CSpankWeed(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, std::move(mData),
                             actParms, pInfo, maxDetectionRange, maxHearingRange, maxSightRange, hideTime);
}

CEntity* ScriptLoader::LoadParasite(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 25, "Parasite"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();

  std::pair<bool, u32> pcount = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters aParms = LoadActorParameters(in);
  float maxTelegraphReactDist = in.ReadFloat();
  float advanceWpRadius = in.ReadFloat();
  float f3 = in.ReadFloat();
  float alignAngVel = in.ReadFloat();
  float f5 = in.ReadFloat();
  float stuckTimeThreshold = in.ReadFloat();
  float collisionCloseMargin = in.ReadFloat();
  float parasiteSearchRadius = in.ReadFloat();
  float parasiteSeparationDist = in.ReadFloat();
  float parasiteSeparationWeight = in.ReadFloat();
  float parasiteAlignmentWeight = in.ReadFloat();
  float parasiteCohesionWeight = in.ReadFloat();
  float destinationSeekWeight = in.ReadFloat();
  float forwardMoveWeight = in.ReadFloat();
  float playerSeparationDist = in.ReadFloat();
  float playerSeparationWeight = in.ReadFloat();
  float playerObstructionMinDist = in.ReadFloat();
  bool disableMove = in.ReadBool();

  if (g_ResFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) != SBIG('ANCS'))
    return nullptr;
  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), scale, animParms.GetInitialAnimation(), true));
  return new MP1::CParasite(mgr.AllocateUniqueId(), name, flavor, info, xf, std::move(mData), pInfo,
                            EBodyType::WallWalker, maxTelegraphReactDist, advanceWpRadius, f3, alignAngVel, f5,
                            stuckTimeThreshold, collisionCloseMargin, parasiteSearchRadius, parasiteSeparationDist,
                            parasiteSeparationWeight, parasiteAlignmentWeight, parasiteCohesionWeight,
                            destinationSeekWeight, forwardMoveWeight, playerSeparationDist, playerSeparationWeight,
                            playerObstructionMinDist, 0.f, disableMove, CWallWalker::EWalkerType::Parasite,
                            CDamageVulnerability::NormalVulnerabilty(), CDamageInfo(), -1, -1, -1, {}, {}, 0.f, aParms);
}

CEntity* ScriptLoader::LoadPlayerHint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 6, "PlayerHint"))
    return nullptr;

  SActorHead aHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  u32 overrideFlags = LoadParameterFlags(in);
  u32 priority = in.ReadLong();

  return new CScriptPlayerHint(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, priority,
                               overrideFlags);
}

CEntity* ScriptLoader::LoadRipper(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 8, "Ripper"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType type = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;
  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  CGrappleParameters grappleParms = LoadGrappleParameters(in);

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();

  if (!animParms.GetACSFile().IsValid())
    return nullptr;

  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), scale, animParms.GetInitialAnimation(), true));
  return new MP1::CRipper(mgr.AllocateUniqueId(), name, type, info, xf, std::move(mData), pInfo, actParms,
                          grappleParms);
}

CEntity* ScriptLoader::LoadPickupGenerator(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "PickupGenerator"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f pos =  in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  float f1 = in.ReadFloat();
  return new CScriptPickupGenerator(mgr.AllocateUniqueId(), name, info, pos, f1, active);
}

CEntity* ScriptLoader::LoadAIKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (CScriptActorKeyframe* kf = static_cast<CScriptActorKeyframe*>(LoadActorKeyframe(mgr, in, propCount, info))) {
    kf->SetIsPassive(true);
    return kf;
  }
  return nullptr;
}

CEntity* ScriptLoader::LoadPointOfInterest(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 6, "PointOfInterest"))
    return nullptr;

  SActorHead aHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  CScannableParameters sParms = LoadScannableParameters(in);
  float pointSize = in.ReadFloat();

  return new CScriptPointOfInterest(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, sParms,
                                    pointSize);
}

std::optional<CVisorFlare::CFlareDef> LoadFlareDef(CInputStream& in) {
  if (in.ReadLong() == 4) {
    CAssetId textureId(in);
    float f1 = in.ReadFloat();
    float f2 = in.ReadFloat();
    zeus::CColor color = in.Get<zeus::CColor>();
    if (textureId.IsValid()) {
      return {CVisorFlare::CFlareDef(g_SimplePool->GetObj({SBIG('TXTR'), textureId}), f1, f2, color)};
    }
  }

  return {};
}

CEntity* ScriptLoader::LoadDrone(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (propCount != 45 && EnsurePropertyCount(propCount, 45, "Drone"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  float f1 = in.ReadFloat();
  const auto [patternedValid, patternedPropCount] = CPatternedInfo::HasCorrectParameterCount(in);
  if (!patternedValid)
    return nullptr;
  CPatternedInfo pInfo(in, patternedPropCount);
  CActorParameters actParms = LoadActorParameters(in);
  CDamageInfo dInfo1(in);
  u32 w1 = in.ReadLong();
  CDamageInfo dInfo2(in);
  CAssetId aId1(in);
  in.ReadLong();
  CAssetId aId2(in);
  std::optional<CVisorFlare::CFlareDef> def1 = LoadFlareDef(in);
  std::optional<CVisorFlare::CFlareDef> def2 = LoadFlareDef(in);
  std::optional<CVisorFlare::CFlareDef> def3 = LoadFlareDef(in);
  std::optional<CVisorFlare::CFlareDef> def4 = LoadFlareDef(in);
  std::optional<CVisorFlare::CFlareDef> def5 = LoadFlareDef(in);
  std::vector<CVisorFlare::CFlareDef> flares(5);
  if (def1)
    flares.push_back(*def1);
  if (def2)
    flares.push_back(*def2);
  if (def3)
    flares.push_back(*def3);
  if (def4)
    flares.push_back(*def4);
  if (def4)
    flares.push_back(*def4);

  const auto& animParms = pInfo.GetAnimationParameters();
  if (g_ResFactory->GetResourceTypeById(animParms.GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();
  float f7 = in.ReadFloat();
  float f8 = in.ReadFloat();
  float f9 = in.ReadFloat();
  float f10 = in.ReadFloat();
  float f11 = in.ReadFloat();
  float f12 = in.ReadFloat();
  float f13 = in.ReadFloat();
  float f14 = in.ReadFloat();
  float f15 = in.ReadFloat();
  float f16 = in.ReadFloat();
  float f17 = in.ReadFloat();
  float f18 = in.ReadFloat();
  float f19 = in.ReadFloat();
  float f20 = in.ReadFloat();
  CAssetId crscId(in);
  float f21 = in.ReadFloat();
  float f22 = in.ReadFloat();
  float f23 = in.ReadFloat();
  float f24 = in.ReadFloat();
  s32 soundId = in.ReadLong();
  bool b1 = in.ReadBool();
  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), scale, animParms.GetInitialAnimation(), true));
  return new MP1::CDrone(mgr.AllocateUniqueId(), name, flavor, info, xf, f1, std::move(mData), pInfo, actParms,
                         CPatterned::EMovementType::Flyer, CPatterned::EColliderType::One, EBodyType::Pitchable, dInfo1,
                         aId1, dInfo2, aId2, flares, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,
                         f17, f18, f19, f20, crscId, f21, f22, f23, f24, soundId, b1);
}

CEntity* ScriptLoader::LoadMetroid(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, MP1::CMetroidData::GetNumProperties(), "Metroid"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  MP1::CMetroidData metData(in);
  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (!animParms.GetACSFile().IsValid() || flavor == CPatterned::EFlavorType::One)
    return nullptr;

  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), scale, animParms.GetInitialAnimation(), true));
  return new MP1::CMetroid(mgr.AllocateUniqueId(), name, flavor, info, xf, std::move(mData), pInfo, actParms, metData,
                           kInvalidUniqueId);
}

CEntity* ScriptLoader::LoadDebrisExtended(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 39, "DebrisExtended"))
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);

  float linConeAngle = in.ReadFloat();
  float linMinMag = in.ReadFloat();
  float linMaxMag = in.ReadFloat();
  float angMinMag = in.ReadFloat();
  float angMaxMag = in.ReadFloat();
  float minDuration = in.ReadFloat();
  float maxDuration = in.ReadFloat();
  float colorInT = in.ReadFloat();
  float colorOutT = in.ReadFloat();

  zeus::CColor color = in.Get<zeus::CColor>();
  zeus::CColor endsColor = in.Get<zeus::CColor>();

  float scaleOutT = in.ReadFloat();

  zeus::CVector3f endScale =  in.Get<zeus::CVector3f>();

  float restitution = in.ReadFloat();
  float downwardSpeed = in.ReadFloat();

  zeus::CVector3f localOffset =  in.Get<zeus::CVector3f>();

  CAssetId model = in.Get<CAssetId>();

  CActorParameters aParam = LoadActorParameters(in);

  CAssetId particle1 = in.Get<CAssetId>();
  zeus::CVector3f particle1Scale =  in.Get<zeus::CVector3f>();
  bool particle1GlobalTranslation = in.ReadBool();
  bool deferDeleteTillParticle1Done = in.ReadBool();
  CScriptDebris::EOrientationType particle1Or = CScriptDebris::EOrientationType(in.ReadLong());

  CAssetId particle2 = in.Get<CAssetId>();
  zeus::CVector3f particle2Scale =  in.Get<zeus::CVector3f>();
  bool particle2GlobalTranslation = in.ReadBool();
  bool deferDeleteTillParticle2Done = in.ReadBool();
  CScriptDebris::EOrientationType particle2Or = CScriptDebris::EOrientationType(in.ReadLong());

  CAssetId particle3 = in.Get<CAssetId>();
  /*zeus::CVector3f particle3Scale =*/(void) in.Get<zeus::CVector3f>(); /* Not actually used, go figure */
  CScriptDebris::EOrientationType particle3Or = CScriptDebris::EOrientationType(in.ReadLong());

  bool solid = in.ReadBool();
  bool dieOnProjectile = in.ReadBool();
  bool noBounce = in.ReadBool();
  bool active = in.ReadBool();

  CModelData modelData;
  if (g_ResFactory->GetResourceTypeById(model).IsValid())
    modelData = CModelData(CStaticRes(model, aHead.x40_scale));

  return new CScriptDebris(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, std::move(modelData),
                           aParam, linConeAngle, linMinMag, linMaxMag, angMinMag, angMaxMag, minDuration, maxDuration,
                           colorInT, colorOutT, color, endsColor, scaleOutT, aHead.x40_scale, endScale, restitution,
                           downwardSpeed, localOffset, particle1, particle1Scale, particle1GlobalTranslation,
                           deferDeleteTillParticle1Done, particle1Or, particle2, particle2Scale,
                           particle2GlobalTranslation, deferDeleteTillParticle2Done, particle2Or, particle3,
                           particle2Scale, particle3Or, solid, dieOnProjectile, noBounce, active);
}

CEntity* ScriptLoader::LoadSteam(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 11, "Steam"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);

  zeus::CVector3f v1 =  in.Get<zeus::CVector3f>();
  zeus::CVector3f v2 =  in.Get<zeus::CVector3f>();

  CDamageInfo dInfo(in);

  zeus::CVector3f v3 =  in.Get<zeus::CVector3f>();

  ETriggerFlags w1 = ETriggerFlags(in.ReadLong());
  bool b1 = in.ReadBool();
  u32 w2 = in.ReadLong();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  bool b2 = in.ReadBool();

  zeus::CAABox aabb(-v2 * 0.5f, v2 * 0.5f);

  return new CScriptSteam(mgr.AllocateUniqueId(), name, info, v1, aabb, dInfo, v3, w1, b1, w2, f1, f2, f3, f4, b2);
}

CEntity* ScriptLoader::LoadRipple(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "Ripple"))
    return nullptr;
  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f center =  in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  float mag = in.ReadFloat();
  return new CScriptRipple(mgr.AllocateUniqueId(), name, info, center, active, mag);
}

CEntity* ScriptLoader::LoadBallTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 9, "BallTrigger"))
    return nullptr;

  std::string name = in.Get<std::string>();
  zeus::CVector3f pos =  in.Get<zeus::CVector3f>();
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();

  bool b1 = in.ReadBool();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  zeus::CVector3f vec =  in.Get<zeus::CVector3f>();
  bool b2 = in.ReadBool();
  return new CScriptBallTrigger(mgr.AllocateUniqueId(), name, info, pos, scale, b1, f1, f2, f3, vec, b2);
}

CEntity* ScriptLoader::LoadTargetingPoint(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "TargetingPoint"))
    return nullptr;

  SActorHead aHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();

  return new CScriptTargetingPoint(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active);
}

CEntity* ScriptLoader::LoadEMPulse(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 12, "EMPulse"))
    return nullptr;

  SActorHead actorHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();
  float f7 = in.ReadFloat();
  CAssetId particleId(in);

  return new CScriptEMPulse(mgr.AllocateUniqueId(), actorHead.x0_name, info, actorHead.x10_transform, active, f1, f2,
                            f3, f4, f5, f6, f7, particleId);
}

CEntity* ScriptLoader::LoadIceSheegoth(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 35, "IceSheegoth"))
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);
  const auto [hasProperCount, patternedParmCount] = CPatternedInfo::HasCorrectParameterCount(in);
  if (!hasProperCount)
    return nullptr;
  CPatternedInfo pInfo(in, patternedParmCount);
  CActorParameters actParms = LoadActorParameters(in);
  MP1::CIceSheegothData sheegothData(in, propCount);

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (!animParms.GetACSFile().IsValid())
    return nullptr;

  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), head.x40_scale,
                            animParms.GetInitialAnimation(), true));
  return new MP1::CIceSheegoth(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(mData), pInfo,
                               actParms, sheegothData);
}

CEntity* ScriptLoader::LoadPlayerActor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 19, "PlayerActor"))
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  zeus::CVector3f extents =  in.Get<zeus::CVector3f>();
  zeus::CVector3f offset =  in.Get<zeus::CVector3f>();
  float mass = in.ReadFloat();
  float zMomentum = in.ReadFloat();
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  in.ReadLong();
  CAnimationParameters animParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  bool loop = in.ReadBool();
  bool snow = in.ReadBool();
  bool solid = in.ReadBool();
  bool active = in.ReadBool();
  u32 flags = LoadParameterFlags(in);
  CPlayerState::EBeamId beam = CPlayerState::EBeamId(in.ReadLong() - 1);

  FourCC fcc = g_ResFactory->GetResourceTypeById(animParms.GetACSFile());
  if (!fcc.IsValid() || fcc != SBIG('ANCS'))
    return nullptr;

  zeus::CAABox aabox = GetCollisionBox(mgr, info.GetAreaId(), extents, offset);
  CMaterialList list;
  if (snow)
    list.Add(EMaterialTypes::Snow);

  if (solid)
    list.Add(EMaterialTypes::Solid);

  if ((extents.x() < 0.f || extents.y() < 0.f || extents.z() < 0.f) || extents.isZero())
    aabox = zeus::CAABox(-.5f, 0.5f);

  return new CScriptPlayerActor(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform,
                                CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), aHead.x40_scale,
                                         animParms.GetInitialAnimation(), loop),
                                CModelData::CModelDataNull(), aabox, true, list, mass, zMomentum, hInfo, dVuln,
                                actParms, loop, active, flags, beam);
}

CEntity* ScriptLoader::LoadFlaahgra(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, MP1::CFlaahgraData::GetNumProperties(), "Flaahgra"))
    return nullptr;

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);

  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;
  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  MP1::CFlaahgraData flaahgraData(in);

  CAnimRes animRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                   actHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid()) {
    animRes = CAnimRes(flaahgraData.GetAnimationParameters().GetACSFile(),
                       flaahgraData.GetAnimationParameters().GetCharacter(), actHead.x40_scale,
                       flaahgraData.GetAnimationParameters().GetInitialAnimation(), true);
  }

  if (!animRes.GetId().IsValid())
    return nullptr;

  return new MP1::CFlaahgra(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, animRes, pInfo,
                            actParms, flaahgraData);
}

CEntity* ScriptLoader::LoadAreaAttributes(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 9, "AreaAttributes"))
    return nullptr;

  bool load = in.ReadLong() != 0;
  if (!load)
    return nullptr;

  bool showSkybox = in.ReadBool();
  EEnvFxType fxType = EEnvFxType(in.ReadLong());
  float envFxDensity = in.ReadFloat();
  float thermalHeat = in.ReadFloat();
  float xrayFogDistance = in.ReadFloat();
  float worldLightingLevel = in.ReadFloat();
  CAssetId skybox = in.Get<CAssetId>();
  EPhazonType phazonType = EPhazonType(in.ReadLong());

  return new CScriptAreaAttributes(mgr.AllocateUniqueId(), info, showSkybox, fxType, envFxDensity, thermalHeat,
                                   xrayFogDistance, worldLightingLevel, skybox, phazonType);
}

CEntity* ScriptLoader::LoadFishCloud(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 36, "FishCloud"))
    return nullptr;
  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  bool active = in.ReadBool();
  CAssetId model(in);
  CAnimationParameters animParms(in);
  u32 numBoids = u32(in.ReadFloat());
  float speed = in.ReadFloat();
  float separationRadius = in.ReadFloat();
  float cohesionMagnitude = in.ReadFloat();
  float alignmentWeight = in.ReadFloat();
  float separationMagnitude = in.ReadFloat();
  float weaponRepelMagnitude = in.ReadFloat();
  float playerRepelMagnitude = in.ReadFloat();
  float containmentMagnitude = in.ReadFloat();
  float scatterVel = in.ReadFloat();
  float maxScatterAngle = in.ReadFloat();
  float weaponRepelDampingSpeed = in.ReadFloat();
  float playerRepelDampingSpeed = in.ReadFloat();
  float containmentRadius = in.ReadFloat();
  u32 updateShift = in.ReadLong();

  if (!g_ResFactory->GetResourceTypeById(model).IsValid())
    return nullptr;

  zeus::CColor color = in.Get<zeus::CColor>();
  bool killable = in.ReadBool();
  float weaponKillRadius = in.ReadFloat();
  CAssetId part1 = in.Get<CAssetId>();
  u32 partCount1 = in.ReadLong();
  CAssetId part2 = in.Get<CAssetId>();
  u32 partCount2 = in.ReadLong();
  CAssetId part3 = in.Get<CAssetId>();
  u32 partCount3 = in.ReadLong();
  CAssetId part4 = in.Get<CAssetId>();
  u32 partCount4 = in.ReadLong();
  u32 deathSfx = in.ReadLong();
  bool repelFromThreats = in.ReadBool();
  bool hotInThermal = in.ReadBool();

  CModelData mData(CStaticRes(model, zeus::skOne3f));
  CAnimRes animRes(animParms.GetACSFile(), animParms.GetCharacter(), zeus::skOne3f, animParms.GetInitialAnimation(),
                   true);
  return new CFishCloud(mgr.AllocateUniqueId(), active, aHead.x0_name, info, aHead.x40_scale, aHead.x10_transform,
                        std::move(mData), animRes, numBoids, speed, separationRadius, cohesionMagnitude,
                        alignmentWeight, separationMagnitude, weaponRepelMagnitude, playerRepelMagnitude,
                        containmentMagnitude, scatterVel, maxScatterAngle, weaponRepelDampingSpeed,
                        playerRepelDampingSpeed, containmentRadius, updateShift, color, killable, weaponKillRadius,
                        part1, partCount1, part2, partCount2, part3, partCount3, part4, partCount4, deathSfx,
                        repelFromThreats, hotInThermal);
}

CEntity* ScriptLoader::LoadFishCloudModifier(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (propCount < 6 || !EnsurePropertyCount(propCount, 7, "FishCloudModifier"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f pos =  in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  bool repulsor = in.ReadBool();
  bool swirl = propCount > 6 ? in.ReadBool() : false;
  float radius = in.ReadFloat();
  float priority = in.ReadFloat();

  return new CFishCloudModifier(mgr.AllocateUniqueId(), active, name, info, pos, repulsor, swirl, radius, priority);
}

CEntity* ScriptLoader::LoadVisorFlare(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 14, "VisorFlare")) {
    return nullptr;
  }

  const std::string name = mgr.HashInstanceName(in);
  const zeus::CVector3f pos =  in.Get<zeus::CVector3f>();
  const bool b1 = in.ReadBool();
  const auto w1 = CVisorFlare::EBlendMode(in.ReadLong());
  const bool b2 = in.ReadBool();
  const float f1 = in.ReadFloat();
  const float f2 = in.ReadFloat();
  const float f3 = in.ReadFloat();
  const u32 w2 = in.ReadLong();

  std::vector<CVisorFlare::CFlareDef> flares;
  flares.reserve(5);
  for (size_t i = 0; i < flares.capacity(); ++i) {
    if (auto flare = CVisorFlare::LoadFlareDef(in)) {
      flares.push_back(*flare);
    }
  }

  return new CScriptVisorFlare(mgr.AllocateUniqueId(), name, info, b1, pos, w1, b2, f1, f2, f3, 2, w2,
                               std::move(flares));
}

CEntity* ScriptLoader::LoadWorldTeleporter(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (propCount < 4 || propCount > 26) {
    Log.report(logvisor::Warning, FMT_STRING("Incorrect number of props for WorldTeleporter"));
    return nullptr;
  }

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  CAssetId worldId = in.Get<CAssetId>();
  CAssetId areaId = in.Get<CAssetId>();

  if (propCount == 4)
    return new CScriptWorldTeleporter(mgr.AllocateUniqueId(), name, info, active, worldId, areaId);

  CAnimationParameters animParms = LoadAnimationParameters(in);
  zeus::CVector3f playerScale =  in.Get<zeus::CVector3f>();
  CAssetId platformModel = in.Get<CAssetId>();
  zeus::CVector3f platformScale =  in.Get<zeus::CVector3f>();
  CAssetId backgroundModel = in.Get<CAssetId>();
  zeus::CVector3f backgroundScale =  in.Get<zeus::CVector3f>();
  bool upElevator = in.ReadBool();

  s16 elevatorSound = (propCount < 12 ? s16(-1) : s16(in.ReadLong()));
  u8 volume = (propCount < 13 ? u8(127) : u8(in.ReadLong()));
  u8 panning = (propCount < 14 ? u8(64) : u8(in.ReadLong()));
  bool showText = (propCount < 15 ? false : in.ReadBool());
  CAssetId fontId = (propCount < 16 ? CAssetId() : in.Get<CAssetId>());
  CAssetId stringId = (propCount < 17 ? CAssetId() : in.Get<CAssetId>());
  bool fadeWhite = (propCount < 18 ? false : in.ReadBool());
  float charFadeInTime = (propCount < 19 ? 0.1f : in.ReadFloat());
  float charsPerSecond = (propCount < 20 ? 16.f : in.ReadFloat());
  float showDelay = (propCount < 21 ? 0.f : in.ReadFloat());
  std::string str1 = (propCount < 22 ? "" : in.Get<std::string>());
  std::string str2 = (propCount < 23 ? "" : in.Get<std::string>());
  /*float f1 =*/(propCount < 24 ? 0.f : in.ReadFloat());
  /*float f2 =*/(propCount < 25 ? 0.f : in.ReadFloat());
  /*float f3 =*/(propCount < 26 ? 0.f : in.ReadFloat());

  if (showText)
    return new CScriptWorldTeleporter(mgr.AllocateUniqueId(), name, info, active, worldId, areaId, elevatorSound,
                                      volume, panning, fontId, stringId, fadeWhite, charFadeInTime, charsPerSecond,
                                      showDelay);

  return new CScriptWorldTeleporter(mgr.AllocateUniqueId(), name, info, active, worldId, areaId, animParms.GetACSFile(),
                                    animParms.GetCharacter(), animParms.GetInitialAnimation(), playerScale,
                                    platformModel, platformScale, backgroundModel, backgroundScale, upElevator,
                                    elevatorSound, volume, panning);
}

CEntity* ScriptLoader::LoadVisorGoo(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 11, "VisorGoo"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f position = in.Get<zeus::CVector3f>();
  zeus::CTransform xf = zeus::CTransform::Translate(position);
  CAssetId particle(in);
  CAssetId electric(in);
  float minDist = in.ReadFloat();
  float maxDist = in.ReadFloat();
  float nearProb = in.ReadFloat();
  float farProb = in.ReadFloat();
  zeus::CColor color = in.Get<zeus::CColor>();
  s32 sfx = in.ReadInt32();
  bool forceShow = in.ReadBool();

  if (!particle.IsValid() && !electric.IsValid())
    return nullptr;
  return new CScriptVisorGoo(mgr.AllocateUniqueId(), name, info, xf, particle, electric, minDist, maxDist, nearProb,
                             farProb, color, sfx, forceShow, false);
}

CEntity* ScriptLoader::LoadJellyZap(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 20, "JellyZap")) {
    return nullptr;
  }

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first) {
    return nullptr;
  }

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  CDamageInfo dInfo(in);
  float attackRadius = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();
  float f7 = in.ReadFloat();
  float f8 = in.ReadFloat();
  float priority = in.ReadFloat();
  float repulseRadius = in.ReadFloat();
  float attractRadius = in.ReadFloat();
  float f12 = in.ReadFloat();
  bool b1 = in.ReadBool();

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (g_ResFactory->GetResourceTypeById(animParms.GetACSFile()) != SBIG('ANCS')) {
    return nullptr;
  }

  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), aHead.x40_scale,
                            animParms.GetInitialAnimation(), true));
  return new MP1::CJellyZap(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, std::move(mData), dInfo,
                            b1, attackRadius, f2, f3, f4, f5, f6, f7, f8, priority, repulseRadius, attractRadius, f12,
                            pInfo, actParms);
}

CEntity* ScriptLoader::LoadControllerAction(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "ControllerAction") || propCount > 6)
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  ControlMapper::ECommands w1 = ControlMapper::ECommands(in.ReadLong());
  bool b1 = false;
  u32 w2 = 0;
  if (propCount == 6) {
    b1 = in.ReadBool();
    w2 = in.ReadLong();
  }
  bool b2 = in.ReadBool();

  return new CScriptControllerAction(mgr.AllocateUniqueId(), name, info, active, w1, b1, w2, b2);
}

CEntity* ScriptLoader::LoadSwitch(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "Switch"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  bool b2 = in.ReadBool();
  bool b3 = in.ReadBool();

  return new CScriptSwitch(mgr.AllocateUniqueId(), name, info, active, b2, b3);
}

CEntity* ScriptLoader::LoadPlayerStateChange(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 7, "PlayerStateChange"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  s32 itemType = in.ReadLong();
  s32 itemCount = in.ReadInt32();
  s32 itemCapacity = in.ReadInt32();
  CScriptPlayerStateChange::EControl ctrl = CScriptPlayerStateChange::EControl(in.ReadLong());
  CScriptPlayerStateChange::EControlCommandOption ctrlCmdOpt =
      CScriptPlayerStateChange::EControlCommandOption(in.ReadLong());
  return new CScriptPlayerStateChange(mgr.AllocateUniqueId(), name, info, active, itemType, itemCount, itemCapacity,
                                      ctrl, ctrlCmdOpt);
}

CEntity* ScriptLoader::LoadThardus(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {

  if (!EnsurePropertyCount(propCount, 43, "Thardus")) {
    return nullptr;
  }

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  const auto [isValid, patternedCount] = CPatternedInfo::HasCorrectParameterCount(in);
  if (!isValid)
    return nullptr;

  CPatternedInfo pInfo(in, patternedCount);
  CActorParameters actParms = LoadActorParameters(in);
  in.ReadBool();
  in.ReadBool();
  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (!animParms.GetACSFile().IsValid())
    return nullptr;
  CStaticRes staticRes[2][7];
  staticRes[0][6] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[0][5] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[0][4] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[0][3] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[0][2] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[0][1] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[0][0] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[1][6] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[1][5] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[1][4] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[1][3] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[1][2] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[1][1] = CStaticRes(CAssetId(in), zeus::skOne3f);
  staticRes[1][0] = CStaticRes(CAssetId(in), zeus::skOne3f);
  CAssetId particle1(in);
  CAssetId particle2(in);
  CAssetId particle3(in);
  CAssetId stateMachine(in);
  CAssetId particle4(in);
  CAssetId particle5(in);
  CAssetId particle6(in);
  CAssetId particle7(in);
  CAssetId particle8(in);
  CAssetId particle9(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();
  CAssetId texture(in);
  int sfxID1 = in.ReadLong();
  CAssetId particle10 = (propCount < 44) ? CAssetId() : CAssetId(in);
  int sfxID2 = in.ReadLong();
  int sfxID3 = in.ReadLong();
  int sfxID4 = in.ReadLong();
  std::vector<CStaticRes> mData1(std::rbegin(staticRes[0]), std::rend(staticRes[0]));
  std::vector<CStaticRes> mData2(std::rbegin(staticRes[1]), std::rend(staticRes[1]));

  CModelData mData(CAnimRes(animParms.GetACSFile(), 0, actHead.x40_scale, animParms.GetInitialAnimation(), true));
  return new MP1::CThardus(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, std::move(mData),
                           actParms, pInfo, std::move(mData1), std::move(mData2), particle1, particle2, particle3, f1,
                           f2, f3, f4, f5, f6, stateMachine, particle4, particle5, particle6, particle7, particle8,
                           particle9, texture, sfxID1, particle10, sfxID2, sfxID3, sfxID4);
}

CEntity* ScriptLoader::LoadWallCrawlerSwarm(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 39, "WallCrawlerSwarm"))
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  bool active = in.ReadBool();
  CActorParameters aParams = LoadActorParameters(in);
  CWallCrawlerSwarm::EFlavor flavor = CWallCrawlerSwarm::EFlavor(in.ReadLong());
  u32 actor = in.ReadLong();
  u32 charIdx = in.ReadLong();
  u32 defaultAnim = in.ReadLong();
  u32 launchAnim = in.ReadLong();
  u32 attractAnim = in.ReadLong();
  u32 part1 = in.ReadLong();
  u32 part2 = in.ReadLong();
  u32 part3 = in.ReadLong();
  u32 part4 = in.ReadLong();
  CDamageInfo crabDamage(in);
  float crabDamageCooldown = in.ReadFloat();
  CDamageInfo scarabExplodeDamage(in);
  float boidRadius = in.ReadFloat();
  float touchRadius = in.ReadFloat();
  float playerTouchRadius = in.ReadFloat();
  float animPlaybackSpeed = in.ReadFloat();
  u32 numBoids = in.ReadLong();
  u32 maxCreatedBoids = in.ReadLong();
  float separationRadius = in.ReadFloat();
  float cohesionMagnitude = in.ReadFloat();
  float alignmentWeight = in.ReadFloat();
  float separationMagnitude = in.ReadFloat();
  float moveToWaypointWeight = in.ReadFloat();
  float attractionMagnitude = in.ReadFloat();
  float attractionRadius = in.ReadFloat();
  float boidGenRate = in.ReadFloat();
  u32 maxLaunches = in.ReadLong();
  float scarabBoxMargin = in.ReadFloat();
  float scarabScatterXYVelocity = in.ReadFloat();
  float scarabTimeToExplode = in.ReadFloat();
  CHealthInfo hInfo(in);
  CDamageVulnerability dVulns(in);
  u32 launchSfx = in.ReadLong();
  u32 scatterSfx = in.ReadLong();

  return new CWallCrawlerSwarm(
      mgr.AllocateUniqueId(), active, aHead.x0_name, info, aHead.x40_scale, aHead.x10_transform, flavor,
      CAnimRes(actor, charIdx, zeus::CVector3f(1.5f), defaultAnim, true), launchAnim, attractAnim, part1, part2, part3,
      part4, crabDamage, scarabExplodeDamage, crabDamageCooldown, boidRadius, touchRadius, playerTouchRadius, numBoids,
      maxCreatedBoids, animPlaybackSpeed, separationRadius, cohesionMagnitude, alignmentWeight, separationMagnitude,
      moveToWaypointWeight, attractionMagnitude, attractionRadius, boidGenRate, maxLaunches, scarabBoxMargin,
      scarabScatterXYVelocity, scarabTimeToExplode, hInfo, dVulns, launchSfx, scatterSfx, aParams);
}

CEntity* ScriptLoader::LoadAiJumpPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 5, "AiJumpPoint"))
    return nullptr;

  SActorHead aHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  float apex = in.ReadFloat();

  return new CScriptAiJumpPoint(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, apex);
}

CEntity* ScriptLoader::LoadFlaahgraTentacle(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {

  if (!EnsurePropertyCount(propCount, 6, "FlaahgraTentacle"))
    return nullptr;

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;
  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), actHead.x40_scale,
                            animParms.GetInitialAnimation(), true));

  return new MP1::CFlaahgraTentacle(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform,
                                    std::move(mData), pInfo, actParms);
}

CEntity* ScriptLoader::LoadRoomAcoustics(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 32, "RoomAcoustics"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool a = in.ReadBool();
  u32 b = in.ReadLong();
  bool c = in.ReadBool();
  bool d = in.ReadBool();
  float e = in.ReadFloat();
  float f = in.ReadFloat();
  float g = in.ReadFloat();
  float h = in.ReadFloat();
  float i = in.ReadFloat();
  float j = in.ReadFloat();
  bool k = in.ReadBool();
  float l = in.ReadFloat();
  float m = in.ReadFloat();
  float n = in.ReadFloat();
  bool o = in.ReadBool();
  bool p = in.ReadBool();
  float q = in.ReadFloat();
  float r = in.ReadFloat();
  float s = in.ReadFloat();
  float t = in.ReadFloat();
  float u = in.ReadFloat();
  bool v = in.ReadBool();
  u32 w = in.ReadLong();
  u32 x = in.ReadLong();
  u32 y = in.ReadLong();
  u32 z = in.ReadLong();
  u32 _a = in.ReadLong();
  u32 _b = in.ReadLong();
  u32 _c = in.ReadLong();
  u32 _d = in.ReadLong();
  u32 _e = in.ReadLong();

  return new CScriptRoomAcoustics(mgr.AllocateUniqueId(), name, info, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q,
                                  r, s, t, u, v, w, x, y, z, _a, _b, _c, _d, _e);
}

CEntity* ScriptLoader::LoadColorModulate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 12, "ColorModulate"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CColor colorA = in.Get<zeus::CColor>();
  zeus::CColor colorB = in.Get<zeus::CColor>();
  CScriptColorModulate::EBlendMode blendMode = CScriptColorModulate::EBlendMode(in.ReadLong());
  float timeA2B = in.ReadFloat();
  float timeB2A = in.ReadFloat();
  bool doReverse = in.ReadBool();
  bool resetTargetWhenDone = in.ReadBool();
  bool depthCompare = in.ReadBool();
  bool depthUpdate = in.ReadBool();
  bool depthBackwards = in.ReadBool();
  bool active = in.ReadBool();
  return new CScriptColorModulate(mgr.AllocateUniqueId(), name, info, colorA, colorB, blendMode, timeA2B, timeB2A,
                                  doReverse, resetTargetWhenDone, depthCompare, depthUpdate, depthBackwards, active);
}

CEntity* ScriptLoader::LoadThardusRockProjectile(CStateManager& mgr, CInputStream& in, int propCount,
                                                 const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 3, "ThardusRockProjectile"))
    return nullptr;

  SScaledActorHead actorHead = LoadScaledActorHead(in, mgr);
  auto [pInfoValid, pInfoCount] = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pInfoValid)
    return nullptr;

  CPatternedInfo pInfo(in, pInfoCount);
  CActorParameters actParms = LoadActorParameters(in);
  in.ReadBool();
  in.ReadBool();
  float f1 = in.ReadFloat();
  CAssetId modelId(in);
  CAssetId stateMachine(in);
  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  std::vector<CStaticRes> mDataVec;
  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), 0, actorHead.x40_scale,
                            pInfo.GetAnimationParameters().GetInitialAnimation(), true));
  mDataVec.reserve(3);
  mDataVec.emplace_back(modelId, zeus::skOne3f);
  return new MP1::CThardusRockProjectile(mgr.AllocateUniqueId(), actorHead.x0_name, info, actorHead.x10_transform,
                                         std::move(mData), actParms, pInfo, std::move(mDataVec), stateMachine, f1);
}

CEntity* ScriptLoader::LoadMidi(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 6, "Midi"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  u32 csng = in.ReadLong();
  float fadeIn = in.ReadFloat();
  float fadeOut = in.ReadFloat();
  u32 vol = in.ReadLong();
  return new CScriptMidi(mgr.AllocateUniqueId(), info, name, active, csng, fadeIn, fadeOut, vol);
}

CEntity* ScriptLoader::LoadStreamedAudio(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 9, "StreamedAudio"))
    return nullptr;

  const std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  std::string fileName = in.Get<std::string>();
  bool noStopOnDeactivate = in.ReadBool();
  float fadeIn = in.ReadFloat();
  float fadeOut = in.ReadFloat();
  u32 volume = in.ReadLong();
  u32 oneShot = in.ReadLong();
  bool music = in.ReadBool();

  return new CScriptStreamedMusic(mgr.AllocateUniqueId(), info, name, active, fileName, noStopOnDeactivate, fadeIn,
                                  fadeOut, volume, !oneShot, music);
}

CEntity* ScriptLoader::LoadRepulsor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "Repulsor"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f center = in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  float radius = in.ReadFloat();

  return new CRepulsor(mgr.AllocateUniqueId(), active, name, info, center, radius);
}

CEntity* ScriptLoader::LoadGunTurret(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, CScriptGunTurretData::GetMinProperties(), "GunTurret"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CScriptGunTurret::ETurretComponent component = CScriptGunTurret::ETurretComponent(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  zeus::CVector3f collisionExtent =  in.Get<zeus::CVector3f>();
  zeus::CVector3f collisionOffset =  in.Get<zeus::CVector3f>();
  CAnimationParameters animParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  CScriptGunTurretData turretData(in, propCount);

  if (!g_ResFactory->GetResourceTypeById(animParms.GetACSFile()).IsValid())
    return nullptr;

  CModelData mData(
      CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), scale, animParms.GetInitialAnimation(), true));
  zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, collisionOffset);

  if ((collisionExtent.x() < 0.f || collisionExtent.y() < 0.f || collisionExtent.z() < 0.f) || collisionExtent.isZero())
    aabb = mData.GetBounds(xf.getRotation());

  return new CScriptGunTurret(mgr.AllocateUniqueId(), name, component, info, xf, std::move(mData), aabb, hInfo, dVuln,
                              actParms, turretData);
}

CEntity* ScriptLoader::LoadFogVolume(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 7, "FogVolume"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f center =  in.Get<zeus::CVector3f>();
  zeus::CVector3f volume =  in.Get<zeus::CVector3f>();
  float flickerSpeed = in.ReadFloat();
  float f2 = in.ReadFloat();
  zeus::CColor fogColor = in.Get<zeus::CColor>();
  bool active = in.ReadBool();

  volume.x() = std::fabs(volume.x());
  volume.y() = std::fabs(volume.y());
  volume.z() = std::fabs(volume.z());

  return new CScriptSpecialFunction(mgr.AllocateUniqueId(), name, info, ConvertEditorEulerToTransform4f(center, {}),
                                    CScriptSpecialFunction::ESpecialFunction::FogVolume, "", flickerSpeed, f2, 0.f, 0.f,
                                    volume, fogColor, active, CDamageInfo(), -1, -1, CPlayerState::EItemType::Invalid,
                                    -1, -1, -1);
}

CEntity* ScriptLoader::LoadBabygoth(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 33, "Babygoth"))
    return nullptr;

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  MP1::CBabygothData babyData(in);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            actHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));
  return new MP1::CBabygoth(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, std::move(mData),
                            pInfo, actParms, babyData);
}

CEntity* ScriptLoader::LoadEyeball(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 21, "Eyeball"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  zeus::CTransform xf = LoadEditorTransform(in);
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();

  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  float attackDelay = in.ReadFloat();
  float attackStartTime = in.ReadFloat();
  CAssetId wpsc(in);
  CDamageInfo dInfo(in);
  CAssetId beamContactFxId(in);
  CAssetId beamPulseFxId(in);
  CAssetId beamTextureId(in);
  CAssetId beamGlowTextureId(in);
  u32 anim0 = in.ReadLong();
  u32 anim1 = in.ReadLong();
  u32 anim2 = in.ReadLong();
  u32 anim3 = in.ReadLong();
  u32 beamSfx = in.ReadLong();

  if (g_ResFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) != SBIG('ANCS'))
    return nullptr;

  bool attackDisabled = in.ReadBool();

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CEyeball(mgr.AllocateUniqueId(), name, flavor, info, xf, std::move(mData), pInfo, attackDelay,
                           attackStartTime, wpsc, dInfo, beamContactFxId, beamPulseFxId, beamTextureId,
                           beamGlowTextureId, anim0, anim1, anim2, anim3, beamSfx, attackDisabled, actParms);
}

CEntity* ScriptLoader::LoadRadialDamage(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 5, "RadialDamage"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f center =  in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  CDamageInfo dInfo(in);
  float radius = in.ReadFloat();
  zeus::CTransform xf = ConvertEditorEulerToTransform4f(zeus::skZero3f, center);

  return new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), name, info, xf, CScriptSpecialFunction::ESpecialFunction::RadialDamage, "", radius, 0.f,
      0.f, 0.f, zeus::skZero3f, zeus::skBlack, active, dInfo, -1, -1, CPlayerState::EItemType::Invalid, -1, -1, -1);
}

CEntity* ScriptLoader::LoadCameraPitchVolume(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 8, "CameraPitchVolume"))
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  bool active = in.ReadBool();
  zeus::CRelAngle upPitch = zeus::CRelAngle::FromDegrees(in.ReadFloat());
  zeus::CRelAngle downPitch = zeus::CRelAngle::FromDegrees(in.ReadFloat());
  float scale = in.ReadFloat();

  return new CScriptCameraPitchVolume(mgr.AllocateUniqueId(), active, aHead.x0_name, info, aHead.x40_scale,
                                      aHead.x10_transform, upPitch, downPitch, scale);
}

CEntity* ScriptLoader::LoadEnvFxDensityController(CStateManager& mgr, CInputStream& in, int propCount,
                                                  const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "EnvFxDensityController"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  float density = in.ReadFloat();
  u32 maxDensityDeltaSpeed = in.ReadLong();

  return new CScriptSpecialFunction(mgr.AllocateUniqueId(), name, info, zeus::CTransform(),
                                    CScriptSpecialFunction::ESpecialFunction::EnvFxDensityController, "", density,
                                    maxDensityDeltaSpeed, 0.f, 0.f, zeus::skZero3f, zeus::skBlack, active,
                                    CDamageInfo(), -1, -1, CPlayerState::EItemType::Invalid, -1, -1, -1);
}

CEntity* ScriptLoader::LoadMagdolite(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 22, "Magdolite"))
    return nullptr;

  SScaledActorHead actorHead = LoadScaledActorHead(in, mgr);

  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first) {
    return nullptr;
  }

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actorParameters = LoadActorParameters(in);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid()) {
    return nullptr;
  }

  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  CDamageInfo damageInfo1(in);
  CDamageInfo damageInfo2(in);
  CDamageVulnerability damageVulnerability1(in);
  CDamageVulnerability damageVulnerability2(in);
  CAssetId modelId(in);
  CAssetId skinId(in);
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();
  CFlameInfo flameInfo(in);
  float f7 = in.ReadFloat();
  float f8 = in.ReadFloat();
  float f9 = in.ReadFloat();

  CModelData modelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                                pInfo.GetAnimationParameters().GetCharacter(), actorHead.x40_scale,
                                pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CMagdolite(mgr.AllocateUniqueId(), actorHead.x0_name, info, actorHead.x10_transform,
                             std::move(modelData), pInfo, actorParameters, f1, f2, damageInfo1, damageInfo2,
                             damageVulnerability1, damageVulnerability2, modelId, skinId, f6, f3, f4, f5, flameInfo, f7,
                             f8, f9);
}

CEntity* ScriptLoader::LoadTeamAIMgr(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 8, "TeamAiMgr"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  CTeamAiData data(in, propCount);
  return new CTeamAiMgr(mgr.AllocateUniqueId(), name, info, data);
}

CEntity* ScriptLoader::LoadSnakeWeedSwarm(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 25, "SnakeWeedSwarm") || propCount > 29)
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f pos =  in.Get<zeus::CVector3f>();
  zeus::CVector3f scale =  in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  CAnimationParameters animParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  float spacing = in.ReadFloat();
  float height = in.ReadFloat();
  float f3 = in.ReadFloat();
  float weaponDamageRadius = in.ReadFloat();
  float maxPlayerDistance = in.ReadFloat();
  float loweredTime = in.ReadFloat();
  float loweredTimeVariation = in.ReadFloat();
  float maxZOffset = in.ReadFloat();
  float speed = in.ReadFloat();
  float speedVariation = in.ReadFloat();
  float f11 = in.ReadFloat();
  float scaleMin = in.ReadFloat();
  float scaleMax = in.ReadFloat();
  float distanceBelowGround = in.ReadFloat();
  CDamageInfo dInfo(in);
  float unused = in.ReadFloat();
  u32 sfxId1 = in.ReadLong();
  u32 sfxId2 = in.ReadLong();
  u32 sfxId3 = in.ReadLong();
  CAssetId particleGenDesc1 = (propCount < 29 ? CAssetId() : CAssetId(in));
  u32 w5 = (propCount < 29 ? 0 : in.ReadLong());
  CAssetId particleGenDesc2 = (propCount < 29 ? CAssetId() : CAssetId(in));
  float f16 = (propCount < 29 ? 0.f : in.ReadFloat());

  CAnimRes animRes(animParms.GetACSFile(), animParms.GetCharacter(), zeus::skOne3f, animParms.GetInitialAnimation(),
                   true);
  return new CSnakeWeedSwarm(mgr.AllocateUniqueId(), active, name, info, pos, scale, animRes, actParms, spacing, height,
                             f3, weaponDamageRadius, maxPlayerDistance, loweredTime, loweredTimeVariation, maxZOffset,
                             speed, speedVariation, f11, scaleMin, scaleMax, distanceBelowGround, dInfo, unused, sfxId1,
                             sfxId2, sfxId3, particleGenDesc1, w5, particleGenDesc2, f16);
}

CEntity* ScriptLoader::LoadActorContraption(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 15, "ActorContraption"))
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);
  zeus::CVector3f collisionExtent =  in.Get<zeus::CVector3f>();
  zeus::CVector3f collisionOrigin =  in.Get<zeus::CVector3f>();
  float mass = in.ReadFloat();
  float zMomentum = in.ReadFloat();
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  CAnimationParameters animParams(in);
  CActorParameters actParams = LoadActorParameters(in);
  CAssetId flameFxId = in.Get<CAssetId>();
  CDamageInfo dInfo(in);
  bool active = in.ReadBool();

  if (!g_ResFactory->GetResourceTypeById(animParams.GetACSFile()).IsValid())
    return nullptr;

  zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, collisionOrigin);
  CMaterialList list;
  list.Add(EMaterialTypes::Immovable);
  list.Add(EMaterialTypes::Solid);

  CModelData data(CAnimRes(animParams.GetACSFile(), animParams.GetCharacter(), head.x40_scale,
                           animParams.GetInitialAnimation(), true));

  if ((collisionExtent.x() < 0.f || collisionExtent.y() < 0.f || collisionExtent.z() < 0.f) || collisionExtent.isZero())
    aabb = data.GetBounds(head.x10_transform.getRotation());

  return new MP1::CActorContraption(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(data),
                                    aabb, list, mass, zMomentum, hInfo, dVuln, actParams, flameFxId, dInfo, active);
}

CEntity* ScriptLoader::LoadOculus(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 15, "Oculus"))
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;
  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();
  CDamageVulnerability dVuln(in);
  float f7 = in.ReadFloat();
  CDamageInfo dInfo(in);
  const CAnimationParameters animParms = pInfo.GetAnimationParameters();
  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), aHead.x40_scale,
                            animParms.GetInitialAnimation(), true));

  return new MP1::CParasite(
      mgr.AllocateUniqueId(), aHead.x0_name, CPatterned::EFlavorType::Zero, info, aHead.x10_transform, std::move(mData),
      pInfo, EBodyType::WallWalker, 0.f, f1, f2, f3, f4, 0.2f, 0.4f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, f7, 0.f, 0.f, f5, f6,
      false, CWallWalker::EWalkerType::Oculus, dVuln, dInfo, -1, -1, -1, CAssetId(), CAssetId(), 0.f, actParms);
}

CEntity* ScriptLoader::LoadGeemer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 16, "Geemer"))
    return nullptr;
  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);

  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == CAssetId())
    return nullptr;

  float advanceWpRadius = in.ReadFloat();
  float f2 = in.ReadFloat();
  float alignAngVel = in.ReadFloat();
  float f4 = in.ReadFloat();
  float playerObstructionMinDist = in.ReadFloat();
  float haltDelay = in.ReadFloat();
  float forwardMoveWeight = in.ReadFloat();
  u16 haltSfx = in.ReadLong() & 0xFFFF;
  u16 getUpSfx = in.ReadLong() & 0xFFFF;
  u16 crouchSfx = in.ReadLong() & 0xFFFF;

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            actHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CParasite(mgr.AllocateUniqueId(), actHead.x0_name, CPatterned::EFlavorType::Zero, info,
                            actHead.x10_transform, std::move(mData), pInfo, EBodyType::WallWalker, 0.f, advanceWpRadius,
                            f2, alignAngVel, f4, 0.2f, 0.4f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, forwardMoveWeight, 0.f, 0.f,
                            playerObstructionMinDist, haltDelay, false, CWallWalker::EWalkerType::Geemer,
                            CDamageVulnerability::NormalVulnerabilty(), CDamageInfo(), haltSfx, getUpSfx, crouchSfx, {},
                            {}, 0.f, actParms);
}

CEntity* ScriptLoader::LoadSpindleCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 24, "SpindleCamera"))
    return nullptr;

  SActorHead aHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  u32 flags = LoadParameterFlags(in);
  float hintToCamDistMin = in.ReadFloat();
  float hintToCamDistMax = in.ReadFloat();
  float hintToCamVOffMin = in.ReadFloat();
  float hintToCamVOffMax = in.ReadFloat();

  SSpindleProperty targetHintToCamDeltaAngleVel(in);
  targetHintToCamDeltaAngleVel.FixupAngles();
  SSpindleProperty deltaAngleScaleWithCamDist(in);
  SSpindleProperty hintToCamDist(in);
  SSpindleProperty distOffsetFromBallDist(in);
  SSpindleProperty hintBallToCamAzimuth(in);
  hintBallToCamAzimuth.FixupAngles();
  SSpindleProperty unused(in);
  unused.FixupAngles();
  SSpindleProperty maxHintBallToCamAzimuth(in);
  maxHintBallToCamAzimuth.FixupAngles();
  SSpindleProperty camLookRelAzimuth(in);
  camLookRelAzimuth.FixupAngles();
  SSpindleProperty lookPosZOffset(in);
  SSpindleProperty camPosZOffset(in);
  SSpindleProperty clampedAzimuthFromHintDir(in);
  clampedAzimuthFromHintDir.FixupAngles();
  SSpindleProperty dampingAzimuthSpeed(in);
  dampingAzimuthSpeed.FixupAngles();
  SSpindleProperty targetHintToCamDeltaAngleVelRange(in);
  targetHintToCamDeltaAngleVelRange.FixupAngles();
  SSpindleProperty deleteHintBallDist(in);
  SSpindleProperty recoverClampedAzimuthFromHintDir(in);
  recoverClampedAzimuthFromHintDir.FixupAngles();

  return new CScriptSpindleCamera(
      mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, flags, hintToCamDistMin,
      hintToCamDistMax, hintToCamVOffMin, hintToCamVOffMax, targetHintToCamDeltaAngleVel, deltaAngleScaleWithCamDist,
      hintToCamDist, distOffsetFromBallDist, hintBallToCamAzimuth, unused, maxHintBallToCamAzimuth, camLookRelAzimuth,
      lookPosZOffset, camPosZOffset, clampedAzimuthFromHintDir, dampingAzimuthSpeed, targetHintToCamDeltaAngleVelRange,
      deleteHintBallDist, recoverClampedAzimuthFromHintDir);
}

CEntity* ScriptLoader::LoadAtomicAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 14, "AtomicAlpha"))
    return nullptr;

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);

  CAssetId wpsc(in);
  CAssetId model(in);
  CDamageInfo dInfo(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  bool b1 = in.ReadBool();
  bool b2 = in.ReadBool();

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            actHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CAtomicAlpha(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, std::move(mData),
                               actParms, pInfo, wpsc, dInfo, f1, f2, f3, model, b1, b2);
}

CEntity* ScriptLoader::LoadCameraHintTrigger(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 7, "CameraHintTrigger"))
    return nullptr;

  SActorHead aHead = LoadActorHead(in, mgr);
  zeus::CVector3f scale = 0.5f *  in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  bool deactivateOnEnter = in.ReadBool();
  bool deactivateOnExit = in.ReadBool();

  zeus::CTransform xfRot = aHead.x10_transform.getRotation();
  if (xfRot == zeus::CTransform())
    return new CScriptTrigger(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform.origin,
                              zeus::CAABox(-scale, scale), CDamageInfo(), zeus::skZero3f, ETriggerFlags::DetectPlayer,
                              active, deactivateOnEnter, deactivateOnExit);

  return new CScriptCameraHintTrigger(mgr.AllocateUniqueId(), active, aHead.x0_name, info, scale, aHead.x10_transform,
                                      deactivateOnEnter, deactivateOnExit);
}

CEntity* ScriptLoader::LoadRumbleEffect(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 6, "RumbleEffect"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f position = in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  float f1 = in.ReadFloat();
  u32 w1 = in.ReadLong();
  u32 pFlags = LoadParameterFlags(in);

  return new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), name, info, ConvertEditorEulerToTransform4f(zeus::skZero3f, position),
      CScriptSpecialFunction::ESpecialFunction::RumbleEffect, "", f1, w1, pFlags, 0.f, zeus::skZero3f, zeus::skBlack,
      active, {}, {}, {}, CPlayerState::EItemType::Invalid, -1, -1, -1);
}

CEntity* ScriptLoader::LoadAmbientAI(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 16, "AmbientAI"))
    return nullptr;

  SScaledActorHead head = LoadScaledActorHead(in, mgr);
  zeus::CVector3f collisionExtent =  in.Get<zeus::CVector3f>();
  zeus::CVector3f collisionOffset =  in.Get<zeus::CVector3f>();
  float mass = in.ReadFloat();
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  CAnimationParameters animParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  float alertRange = in.ReadFloat();
  float impactRange = in.ReadFloat();
  s32 alertAnim = in.ReadInt32();
  s32 impactAnim = in.ReadInt32();
  bool active = in.ReadBool();

  if (!g_ResFactory->GetResourceTypeById(animParms.GetACSFile()).IsValid())
    return nullptr;

  zeus::CAABox aabox = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, collisionOffset);

  CMaterialList matList(EMaterialTypes::Immovable, EMaterialTypes::NonSolidDamageable);

  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), head.x40_scale,
                            animParms.GetInitialAnimation(), true));
  if ((collisionExtent.x() < 0.f || collisionExtent.y() < 0.f || collisionExtent.z() < 0.f) || collisionExtent.isZero())
    aabox = mData.GetBounds(head.x10_transform.getRotation());

  return new CAmbientAI(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(mData), aabox,
                        matList, mass, hInfo, dVuln, actParms, alertRange, impactRange, alertAnim, impactAnim, active);
}

CEntity* ScriptLoader::LoadAtomicBeta(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 21, "AtomicBeta"))
    return nullptr;
  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;
  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  CAssetId electricId(in);
  CAssetId weaponId(in);
  CDamageInfo dInfo(in);
  CAssetId particleId(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  CDamageVulnerability dVuln(in);
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();
  s16 sId1 = s16(in.ReadInt32() & 0xFFFF);
  s16 sId2 = s16(in.ReadInt32() & 0xFFFF);
  s16 sId3 = s16(in.ReadInt32() & 0xFFFF);
  float f7 = in.ReadFloat();
  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), aHead.x40_scale,
                            animParms.GetInitialAnimation(), true));
  return new MP1::CAtomicBeta(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, std::move(mData),
                              actParms, pInfo, electricId, weaponId, dInfo, particleId, f1, f2, f7, dVuln, f3, f4, f5,
                              sId1, sId2, sId3, f6);
}

CEntity* ScriptLoader::LoadIceZoomer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 16, "IceZoomer"))
    return nullptr;
  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (!animParms.GetACSFile().IsValid())
    return nullptr;

  float advanceWpRadius = in.ReadFloat();
  float f2 = in.ReadFloat();
  float alignAngleVel = in.ReadFloat();
  float f4 = in.ReadFloat();
  float playerObstructionMinDist = in.ReadFloat();
  float moveFowardWeight = in.ReadFloat();
  CAssetId modelRes(in.Get<CAssetId>());
  CAssetId skinRes(in.Get<CAssetId>());
  CDamageVulnerability dVuln(in);
  float iceZoomerJointHP = in.ReadFloat();

  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), actHead.x40_scale,
                            animParms.GetInitialAnimation(), true));
  return new MP1::CParasite(mgr.AllocateUniqueId(), actHead.x0_name, CPatterned::EFlavorType::Zero, info,
                            actHead.x10_transform, std::move(mData), pInfo, EBodyType::WallWalker, 0.f, advanceWpRadius,
                            f2, alignAngleVel, f4, 0.2f, 0.4f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, moveFowardWeight, 0.f, 0.f,
                            playerObstructionMinDist, 0.f, false, CWallWalker::EWalkerType::IceZoomer, dVuln,
                            CDamageInfo(), 0xFFFF, 0xFFFF, 0xFFFF, modelRes, skinRes, iceZoomerJointHP, actParms);
}

CEntity* ScriptLoader::LoadPuffer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 16, "Puffer"))
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  CActorParameters actorParameters = LoadActorParameters(in);
  float hoverSpeed = in.ReadFloat();
  CAssetId cloudEffect(in);
  CDamageInfo cloudDamage(in);
  CAssetId cloudSteam(in);
  float f2 = in.ReadFloat();
  bool b1 = in.ReadBool();
  bool b2 = in.ReadBool();
  bool b3 = in.ReadBool();
  CDamageInfo explosionDamage(in);
  s16 sfxId = in.ReadShort();

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            aHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));
  return new MP1::CPuffer(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, std::move(mData),
                          actorParameters, pInfo, hoverSpeed, cloudEffect, cloudDamage, cloudSteam, f2, b1, b2, b3,
                          explosionDamage, sfxId);
}

CEntity* ScriptLoader::LoadTryclops(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 10, "Tryclops"))
    return nullptr;

  SScaledActorHead actorHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);

  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  CActorParameters actorParameters = LoadActorParameters(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            actorHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CTryclops(mgr.AllocateUniqueId(), actorHead.x0_name, info, actorHead.x10_transform, std::move(mData),
                            pInfo, actorParameters, f1, f2, f3, f4);
}

CEntity* ScriptLoader::LoadRidley(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 47, "Ridley"))
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            aHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));
  return new MP1::CRidley(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, std::move(mData), pInfo,
                          actParms, in, propCount);
}

CEntity* ScriptLoader::LoadSeedling(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 14, "Seedling"))
    return nullptr;
  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);

  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  CAssetId needleId(in);
  CAssetId weaponId(in);
  CDamageInfo dInfo1(in);
  CDamageInfo dInfo2(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            aHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));
  return new MP1::CSeedling(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, std::move(mData), pInfo,
                            actParms, needleId, weaponId, dInfo1, dInfo2, f1, f2, f3, f4);
}

CEntity* ScriptLoader::LoadThermalHeatFader(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "ThermalHeatFader"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  float fadedLevel = in.ReadFloat();
  float initialLevel = in.ReadFloat();
  return new CScriptDistanceFog(mgr.AllocateUniqueId(), name, info, ERglFogMode::None, zeus::skBlack, zeus::CVector2f(),
                                0.f, zeus::CVector2f(), false, active, fadedLevel, initialLevel, 0.f, 0.f);
}

CEntity* ScriptLoader::LoadBurrower(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 13, "Burrower"))
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid())
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  CAssetId w1(in);
  CAssetId w2(in);
  CAssetId w3(in);
  CDamageInfo dInfo(in);

  CAssetId w4(in);
  u32 w5 = in.ReadLong();
  CAssetId w6(in);

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            aHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CBurrower(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, std::move(mData), pInfo,
                            actParms, w1, w2, w3, dInfo, w4, w5, w6);
}

CEntity* ScriptLoader::LoadBeam(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 7, "Beam"))
    return nullptr;

  SActorHead aHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  u32 weaponDescId = in.ReadLong();
  if (!g_ResFactory->GetResourceTypeById(weaponDescId).IsValid())
    return nullptr;

  CBeamInfo beamInfo(in);
  CDamageInfo dInfo(in);
  TToken<CWeaponDescription> weaponDesc = g_SimplePool->GetObj({SBIG('WPSC'), weaponDescId});

  return new CScriptBeam(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, weaponDesc, beamInfo,
                         dInfo);
}

CEntity* ScriptLoader::LoadWorldLightFader(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 4, "WorldLightFader"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();

  return new CScriptDistanceFog(mgr.AllocateUniqueId(), name, info, ERglFogMode::None, zeus::skBlack, zeus::skZero2f,
                                0.f, zeus::skZero2f, false, active, 0.f, 0.f, f1, f2);
}

CEntity* ScriptLoader::LoadMetroidPrimeEssence(CStateManager& mgr, CInputStream& in, int propCount,
                                               const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 11, "MetroidPrimeEssence")) {
    return nullptr;
  }

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  auto [valid, pInfoPropCount] = CPatternedInfo::HasCorrectParameterCount(in);
  if (!valid) {
    return nullptr;
  }
  CPatternedInfo pInfo{in, pInfoPropCount};
  CActorParameters actParms = LoadActorParameters(in);
  CAssetId particle1{in};
  CDamageInfo dInfo{in};
  CAssetId electric{in};
  u32 w3 = in.ReadLong();
  CAssetId particle2{in};

  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  CModelData mData{CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), aHead.x40_scale,
                            animParms.GetInitialAnimation(), true)};
  return new MP1::CMetroidPrimeEssence(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform,
                                       std::move(mData), pInfo, actParms, particle1, dInfo, aHead.x40_scale.y(),
                                       electric, w3, particle2);
};

CEntity* ScriptLoader::LoadMetroidPrimeStage1(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 22, "MetroidPrimeStage1"))
    return nullptr;
  u32 version = in.ReadLong();
  if (version != 3)
    return nullptr;

  SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
  bool active = in.ReadBool();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  u32 w1 = in.ReadLong();
  bool b1 = in.ReadBool();
  u32 w2 = in.ReadLong();
  CHealthInfo hInfo1(in);
  CHealthInfo hInfo2(in);
  u32 w3 = in.ReadLong();
  rstl::reserved_vector<MP1::CMetroidPrimeAttackWeights, 4> roomParms;
  for (int i = 0; i < 4; ++i)
    roomParms.emplace_back(in);
  u32 w4 = in.ReadLong();
  u32 w5 = in.ReadLong();
  MP1::SPrimeExoParameters primeParms(in);

  return new MP1::CMetroidPrimeRelay(mgr.AllocateUniqueId(), aHead.x0_name, info, active, aHead.x10_transform,
                                     aHead.x40_scale, std::move(primeParms), f1, f2, f3, w1, b1, w2, hInfo1, hInfo2, w3,
                                     w4, w5, std::move(roomParms));
}

CEntity* ScriptLoader::LoadMazeNode(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 10, "MazeNode"))
    return nullptr;

  SActorHead aHead = LoadActorHead(in, mgr);
  bool active = in.ReadBool();
  u32 col = in.ReadLong();
  u32 row = in.ReadLong();
  u32 side = in.ReadLong();
  zeus::CVector3f actorPos =  in.Get<zeus::CVector3f>();
  zeus::CVector3f triggerPos =  in.Get<zeus::CVector3f>();
  zeus::CVector3f effectPos =  in.Get<zeus::CVector3f>();

  return new CScriptMazeNode(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, col, row, side,
                             actorPos, triggerPos, effectPos);
}

CEntity* ScriptLoader::LoadOmegaPirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, skElitePiratePropCount + 1, "OmegaPirate")) {
    return nullptr;
  }

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first) {
    return nullptr;
  }

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);
  MP1::CElitePirateData elitePirateData(in, propCount);

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid()) {
    return nullptr;
  }

  const CAssetId skeletonModelId{in};
  const CAssetId skeletonSkinRulesId{in};
  const CAssetId skeletonLayoutInfoId{in};
  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            actHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));
  return new MP1::COmegaPirate(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform, std::move(mData),
                               pInfo, actParms, elitePirateData, skeletonModelId, skeletonSkinRulesId,
                               skeletonLayoutInfoId);
}

CEntity* ScriptLoader::LoadPhazonPool(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 18, "PhazonPool")) {
    return nullptr;
  }

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  bool active = in.ReadBool();
  CAssetId w1{in};
  CAssetId w2{in};
  CAssetId w3{in};
  CAssetId w4{in};
  u32 u1 = in.ReadLong();
  CDamageInfo dInfo{in};
  zeus::CVector3f orientedForce = in.Get<zeus::CVector3f>();
  ETriggerFlags triggerFlags = static_cast<ETriggerFlags>(in.ReadLong());
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  bool b2 = in.ReadBool();
  float f4 = in.ReadFloat();

  return new MP1::CPhazonPool(mgr.AllocateUniqueId(), actHead.x0_name, info,
                              zeus::CTransform::Translate(actHead.x10_transform.origin), actHead.x40_scale, active, w1,
                              w2, w3, w4, u1, dInfo, orientedForce, triggerFlags, b2, f1, f2, f3, f4);
}

CEntity* ScriptLoader::LoadPhazonHealingNodule(CStateManager& mgr, CInputStream& in, int propCount,
                                               const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 9, "PhazonHealingNodule")) {
    return nullptr;
  }

  SScaledActorHead actHead = LoadScaledActorHead(in, mgr);
  auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first) {
    return nullptr;
  }

  CPatternedInfo pInfo(in, pair.second);
  CActorParameters actParms = LoadActorParameters(in);

  in.ReadBool();
  CAssetId w1{in};
  std::string w2 = in.Get<std::string>();

  if (!pInfo.GetAnimationParameters().GetACSFile().IsValid()) {
    return nullptr;
  }

  CModelData mData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                            actHead.x40_scale, pInfo.GetAnimationParameters().GetInitialAnimation(), true));

  return new MP1::CPhazonHealingNodule(mgr.AllocateUniqueId(), actHead.x0_name, info, actHead.x10_transform,
                                       std::move(mData), actParms, pInfo, w1, std::move(w2));
}

CEntity* ScriptLoader::LoadNewCameraShaker(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 8, "NewCameraShaker"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f sfxPos =  in.Get<zeus::CVector3f>();
  bool active = in.ReadBool();
  u32 flags = LoadParameterFlags(in);
  float duration = in.ReadFloat();
  float sfxDist = in.ReadFloat();
  CCameraShakerComponent shakerX = CCameraShakerComponent::LoadNewCameraShakerComponent(in);
  CCameraShakerComponent shakerY = CCameraShakerComponent::LoadNewCameraShakerComponent(in);
  CCameraShakerComponent shakerZ = CCameraShakerComponent::LoadNewCameraShakerComponent(in);

  CCameraShakeData shakeData(duration, sfxDist, flags, sfxPos, shakerX, shakerY, shakerZ);

  return new CScriptCameraShaker(mgr.AllocateUniqueId(), name, info, active, shakeData);
}

CEntity* ScriptLoader::LoadShadowProjector(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 10, "ShadowProjector"))
    return nullptr;

  std::string name = mgr.HashInstanceName(in);
  zeus::CVector3f position( in.Get<zeus::CVector3f>());
  bool b1 = in.ReadBool();
  float f1 = in.ReadFloat();
  zeus::CVector3f vec2( in.Get<zeus::CVector3f>());
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  bool b2 = in.ReadBool();
  u32 w1 = in.ReadLong();
  return new CScriptShadowProjector(mgr.AllocateUniqueId(), name, info, zeus::CTransform::Translate(position), b1, vec2,
                                    b2, f1, f2, f3, f4, w1);
}

CEntity* ScriptLoader::LoadEnergyBall(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info) {
  if (!EnsurePropertyCount(propCount, 16, "EnergyBall"))
    return nullptr;

  SScaledActorHead actorHead = LoadScaledActorHead(in, mgr);
  const auto pair = CPatternedInfo::HasCorrectParameterCount(in);
  if (!pair.first)
    return nullptr;

  CPatternedInfo pInfo(in, pair.second);
  const CAnimationParameters& animParms = pInfo.GetAnimationParameters();
  if (!animParms.GetACSFile().IsValid())
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  u32 w1 = in.ReadLong();
  float f1 = in.ReadFloat();
  CDamageInfo dInfo1(in);
  float f2 = in.ReadFloat();
  CAssetId a1(in);
  s16 sfxId1 = CSfxManager::TranslateSFXID(in.ReadLong());
  CAssetId a2(in);
  CAssetId a3(in);
  s16 sfxId2 = CSfxManager::TranslateSFXID(in.ReadLong());
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  CAssetId a4(in);

  CDamageInfo dInfo2 = propCount >= 19 ? CDamageInfo(in) : CDamageInfo();
  float f5 = propCount >= 20 ? in.ReadFloat() : 3.0f;

  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), actorHead.x40_scale,
                            animParms.GetInitialAnimation(), true));
  return new MP1::CEnergyBall(mgr.AllocateUniqueId(), actorHead.x0_name, info, actorHead.x10_transform,
                              std::move(mData), actParms, pInfo, w1, f1, dInfo1, f2, a1, sfxId1, a2, a3, sfxId2, f3, f4,
                              a4, dInfo2, f5);
}
} // namespace metaforce
