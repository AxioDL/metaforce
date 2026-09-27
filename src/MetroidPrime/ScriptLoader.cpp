#include "MetroidPrime/ScriptLoader.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAnimationParameters.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CFluidUVMotion.hpp"
#include "MetroidPrime/CGrappleParameters.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CAmbientAI.hpp"
#include "MetroidPrime/Enemies/CAtomicAlpha.hpp"
#include "MetroidPrime/Enemies/CAtomicBeta.hpp"
#include "MetroidPrime/Enemies/CBabygoth.hpp"
#include "MetroidPrime/Enemies/CBeetle.hpp"
#include "MetroidPrime/Enemies/CBloodFlower.hpp"
#include "MetroidPrime/Enemies/CBurrower.hpp"
#include "MetroidPrime/Enemies/CChozoGhost.hpp"
#include "MetroidPrime/Enemies/CDrone.hpp"
#include "MetroidPrime/Enemies/CElitePirate.hpp"
#include "MetroidPrime/Enemies/CEnergyBall.hpp"
#include "MetroidPrime/Enemies/CEyeBall.hpp"
#include "MetroidPrime/Enemies/CFireFlea.hpp"
#include "MetroidPrime/Enemies/CFlaahgra.hpp"
#include "MetroidPrime/Enemies/CFlaahgraTentacle.hpp"
#include "MetroidPrime/Enemies/CFlickerBat.hpp"
#include "MetroidPrime/Enemies/CFlyingPirate.hpp"
#include "MetroidPrime/Enemies/CIceSheegoth.hpp"
#include "MetroidPrime/Enemies/CJellyZap.hpp"
#include "MetroidPrime/Enemies/CMagdolite.hpp"
#include "MetroidPrime/Enemies/CMetaree.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/CMetroidBeta.hpp"
#include "MetroidPrime/Enemies/CMetroidPrime.hpp"
#include "MetroidPrime/Enemies/CMetroidPrimeRelay.hpp"
#include "MetroidPrime/Enemies/CMetroidPrimeStage2.hpp"
#include "MetroidPrime/Enemies/CNewIntroBoss.hpp"
#include "MetroidPrime/Enemies/COmegaPirate.hpp"
#include "MetroidPrime/Enemies/CParasite.hpp"
#include "MetroidPrime/Enemies/CPhazonHealingNodule.hpp"
#include "MetroidPrime/Enemies/CPuddleSpore.hpp"
#include "MetroidPrime/Enemies/CPuddleToadGamma.hpp"
#include "MetroidPrime/Enemies/CPuffer.hpp"
#include "MetroidPrime/Enemies/CRidley.hpp"
#include "MetroidPrime/Enemies/CRipper.hpp"
#include "MetroidPrime/Enemies/CScriptContraption.hpp"
#include "MetroidPrime/Enemies/CScriptPhazonPool.hpp"
#include "MetroidPrime/Enemies/CSeedling.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Enemies/CSpankWeed.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Enemies/CThardus.hpp"
#include "MetroidPrime/Enemies/CThardusRockProjectile.hpp"
#include "MetroidPrime/Enemies/CTryclops.hpp"
#include "MetroidPrime/Enemies/CWallCrawlerSwarm.hpp"
#include "MetroidPrime/Enemies/CWarWasp.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

#include "MetroidPrime/ScriptObjects/CFishCloud.hpp"
#include "MetroidPrime/ScriptObjects/CFishCloudModifier.hpp"
#include "MetroidPrime/ScriptObjects/CRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActorRotate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaAttributes.hpp"
#include "MetroidPrime/ScriptObjects/CScriptBallTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptBeam.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraBlurKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraFilterKeyframe.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraHintTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraPitchVolume.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"
#include "MetroidPrime/ScriptObjects/CScriptControllerAction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCounter.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebugCameraWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDistanceFog.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDock.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDockAreaChange.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDoor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEMPulse.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGrapplePoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptGunTurret.hpp"
#include "MetroidPrime/ScriptObjects/CScriptHUDMemo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMazeNode.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMemoryRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMidi.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPickupGenerator.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerStateChange.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPointOfInterest.hpp"
#include "MetroidPrime/ScriptObjects/CScriptProjectedShadow.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRandomRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRelay.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRipple.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRoomAcoustics.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpawnPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpiderBallWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpindleCamera.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSteam.hpp"
#include "MetroidPrime/ScriptObjects/CScriptStreamedMusic.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSwitch.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptVisorFlare.hpp"
#include "MetroidPrime/ScriptObjects/CScriptVisorGoo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"
#include "MetroidPrime/ScriptObjects/CSnakeWeedSwarm.hpp"

#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCinematicCamera.hpp"
#include "MetroidPrime/Cameras/CPathCamera.hpp"

#include "MetroidPrime/CFluidPlaneManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

static CAABox GetCollisionBox(CStateManager& stateMgr, TAreaId id, const CVector3f& extent,
                              const CVector3f& offset) {
  const CAABox box(-extent.GetX() / 2.f + offset.GetX(), -extent.GetY() / 2.f + offset.GetY(),
                   -extent.GetZ() / 2.f + offset.GetZ(), extent.GetX() / 2.f + offset.GetX(),
                   extent.GetY() / 2.f + offset.GetY(), extent.GetZ() / 2.f + offset.GetZ());
  return box.GetTransformedAABox(stateMgr.GetWorld()->GetAreaAlways(id).GetTM().GetRotation());
}

static CTransform4f ConvertEditorEulerToTransform4f(const CVector3f& orientation,
                                                    const CVector3f& position) {
  CQuaternion quat = CQuaternion::ZRotation(CRelAngle::FromDegrees(orientation.GetZ())) *
                     CQuaternion::YRotation(CRelAngle::FromDegrees(orientation.GetY())) *
                     CQuaternion::XRotation(CRelAngle::FromDegrees(orientation.GetX()));
  CMatrix3f mat = quat.BuildTransform();
  const CVector3f& r0 = mat.GetRow(kDX);
  const CVector3f& r1 = mat.GetRow(kDY);
  const CVector3f& r2 = mat.GetRow(kDZ);
  return CTransform4f::FromColumns(CVector3f(r0[kDX], r1[kDX], r2[kDX]), CVector3f(r0[kDY], r1[kDY], r2[kDY]),
                      CVector3f(r0[kDZ], r1[kDZ], r2[kDZ]), position);
}

static CTransform4f LoadEditorTransform(CInputStream& in) {
  CVector3f position(in);
  CVector3f orientation(in);
  return ConvertEditorEulerToTransform4f(orientation, position);
}

static CTransform4f LoadEditorTransformPivotOnly(CInputStream& in) {
  CVector3f position(in);
  CVector3f orientation(in);
  orientation.SetY(0.0f);
  orientation.SetX(0.0f);
  return ConvertEditorEulerToTransform4f(orientation, position);
}

struct SActorHead {
  rstl::string mName;
  CTransform4f mTransform;

  SActorHead(CInputStream& in, CStateManager& stateMgr);
};

SActorHead::SActorHead(CInputStream& in, CStateManager& stateMgr)
: mName(stateMgr.HashInstanceName(in)), mTransform(LoadEditorTransform(in)) {}

struct SScaledActorHead {
  SActorHead mActorHead;
  CVector3f mScale;

  SScaledActorHead(CInputStream& in, CStateManager& stateMgr);
};

SScaledActorHead::SScaledActorHead(CInputStream& in, CStateManager& stateMgr)
: mActorHead(in, stateMgr), mScale(in) {}

CAnimationParameters LoadAnimationParameters(CInputStream& in) {
  CAssetId ancs = in.Get< CAssetId >();
  int charIdx = in.Get< int >();
  uint defaultAnim = in.Get< int >();
  return CAnimationParameters(ancs, charIdx, defaultAnim);
}

static CLightParameters LoadLightParameters(CInputStream& in) {
  int propCount = in.ReadLong();
  if (propCount != 14) {
    return CLightParameters::None();
  }

  bool castShadow = in.Get< bool >();
  float shadowScale = in.Get< float >();
  int shadowTess = in.Get< int >();
  float shadowAlpha = in.Get< float >();
  float maxShadowHeight = in.Get< float >();

  CColor noLightsAmbient(in);

  bool makeLights = in.Get< bool >();
  int lightOpts = in.Get< int >();
  int recalcOpts = in.Get< int >();

  CVector3f actorPosBias(in);

  int maxDynamicLights = propCount >= 12 ? in.Get< int >() : -1;
  int maxAreaLights = propCount >= 12 ? in.Get< int >() : -1;
  bool ambientChannelOverflow = propCount >= 13 ? in.ReadBool() : false;
  int layerIdx = propCount >= 14 ? in.Get< int >() : 0;

  return CLightParameters(castShadow, shadowScale,
                          CLightParameters::EShadowTessellation(shadowTess), shadowAlpha,
                          maxShadowHeight, noLightsAmbient, makeLights,
                          CLightParameters::EWorldLightingOptions(lightOpts),
                          CLightParameters::ELightRecalculationOptions(recalcOpts), actorPosBias,
                          maxDynamicLights, maxAreaLights, ambientChannelOverflow, layerIdx);
}

static CScannableParameters LoadScannableParameters(CInputStream& in) {
  int propCount = in.ReadLong();
  if (propCount != 1) {
    return CScannableParameters(kInvalidAssetId);
  } else {
    return CScannableParameters(in.Get< CAssetId >());
  }
}

static CVisorParameters LoadVisorParameters(CInputStream& in) {
  int propCount = in.ReadLong();
  bool valid = false;
  if (propCount >= 1 && propCount <= 3)
    valid = true;
  if (!valid)
    return CVisorParameters(0xf, false, false);

  bool b1 = in.ReadBool();
  bool scanPassthrough = false;
  if (propCount > 2)
    scanPassthrough = in.ReadBool();
  uint mask;
  if (propCount >= 2) {
    mask = in.ReadLong();
  } else {
    mask = 0xf;
  }
  return CVisorParameters(mask, b1, scanPassthrough);
}

CActorParameters LoadActorParameters(CInputStream& in) {
  int propCount = in.ReadLong();
  bool valid = false;
  if (propCount >= 5 && propCount <= 0xe)
    valid = true;
  if (!valid)
    return CActorParameters::None();

  CLightParameters lParms = LoadLightParameters(in);

  CScannableParameters sParms =
      propCount > 5 ? LoadScannableParameters(in) : CScannableParameters(kInvalidAssetId);

  CAssetId xrayModel = in.Get< CAssetId >();
  CAssetId xraySkin = in.Get< CAssetId >();
  CAssetId infraModel = in.Get< CAssetId >();
  CAssetId infraSkin = in.Get< CAssetId >();

  bool globalTimeProvider = propCount > 7 ? in.Get< bool >() : true;
  float fadeInTime = propCount > 8 ? in.Get< float >() : 1.f;
  float fadeOutTime = propCount > 9 ? in.Get< float >() : 1.f;
  CVisorParameters vParms =
      propCount > 6 ? LoadVisorParameters(in) : CVisorParameters(0xf, false, false);

  bool thermalHeat = propCount > 10 ? in.Get< bool >() : false;
  bool renderUnsorted = propCount > 11 ? in.Get< bool >() : false;
  bool noSortThermal = propCount > 12 ? in.Get< bool >() : false;
  float thermalMag = propCount > 13 ? in.Get< float >() : 1.f;

  uint xrayType = gpResourceFactory->GetResourceTypeById(xrayModel);
  uint infraType = gpResourceFactory->GetResourceTypeById(infraModel);

  int flags = 0;
  if (xrayType)
    flags |= 1;
  if (infraType)
    flags |= 2;

  rstl::pair< CAssetId, CAssetId > xray =
      (flags & 1) ? rstl::pair< CAssetId, CAssetId >(xrayModel, xraySkin)
                  : rstl::pair< CAssetId, CAssetId >(CAssetId(), CAssetId());

  rstl::pair< CAssetId, CAssetId > infra =
      (flags & 2) ? rstl::pair< CAssetId, CAssetId >(infraModel, infraSkin)
                  : rstl::pair< CAssetId, CAssetId >(CAssetId(), CAssetId());

  return CActorParameters(lParms, sParms, xray, infra, vParms, globalTimeProvider, thermalHeat,
                          renderUnsorted, noSortThermal, fadeInTime, fadeOutTime, thermalMag);
}

static CGrappleParameters LoadGrappleParameters(CInputStream& in) {
  in.ReadLong();
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

uint LoadParameterFlags(CInputStream& in) {
  int count = in.ReadLong();
  uint ret = 0;
  for (int i = 0; i < count; ++i)
    if (in.ReadBool())
      ret |= 1 << i;
  return ret;
}

CEntity* ScriptLoader::LoadTrigger(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount != 9)
    return nullptr;

  rstl::string name(mgr.HashInstanceName(in));
  CVector3f position(in);
  CVector3f extent(in);
  CDamageInfo dInfo(in);
  CVector3f forceVec(in);
  ETriggerFlags flags = ETriggerFlags(in.ReadLong());
  bool active = in.Get< bool >();
  bool b2 = in.Get< bool >();
  const bool b3 = in.Get< bool >();

  float halfX = extent.GetX() * 0.5f;
  float halfY = extent.GetY() * 0.5f;
  float halfZ = extent.GetZ() * 0.5f;
  CAABox box(CVector3f(-halfX, -halfY, -halfZ), CVector3f(halfX, halfY, halfZ));

  CVector3f orientedForce =
      mgr.GetWorld()->GetAreaAlways(info.GetAreaId()).GetTM().Rotate(forceVec);

  return rs_new CScriptTrigger(mgr.AllocateUniqueId(), name, info, position, box, dInfo,
                               orientedForce, flags, active, b2, b3);
}

CEntity* ScriptLoader::LoadCameraHintTrigger(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (propCount != 7)
    return nullptr;

  SActorHead aHead(in, mgr);
  CVector3f scale = CVector3f(in) * 0.5f;
  bool active = in.Get< bool >();
  bool deactivateOnEnter = in.Get< bool >();
  bool deactivateOnExit = in.ReadBool();

  if (aHead.mTransform.GetRotation() == CTransform4f::Identity()) {
    CAABox box(CVector3f(-(scale.GetX()), -(scale.GetY()), -(scale.GetZ())),
               CVector3f((scale.GetX()), (scale.GetY()), (scale.GetZ())));
    return rs_new CScriptTrigger(mgr.AllocateUniqueId(), aHead.mName, info,
                                 aHead.mTransform.GetTranslation(), box, CDamageInfo(),
                                 CVector3f::Zero(), kTFL_DetectPlayer, active, deactivateOnEnter,
                                 deactivateOnExit);
  } else {
    return rs_new CScriptCameraHintTrigger(mgr.AllocateUniqueId(), active, aHead.mName, info,
                                           scale, aHead.mTransform, deactivateOnEnter,
                                           deactivateOnExit);
  }
}

static int ClassifyVector(const CVector3f& dir) {
  CVector3f absDir(fabsf(dir.GetX()), fabsf(dir.GetY()), fabsf(dir.GetZ()));
  int indices[3] = {0, 1, 2};

  if (absDir[indices[0]] < absDir[indices[1]]) {
    int tmp = indices[0];
    indices[0] = indices[1];
    indices[1] = tmp;
  }
  if (absDir[indices[1]] < absDir[indices[2]]) {
    int tmp = indices[1];
    indices[1] = indices[2];
    indices[2] = tmp;
  }
  if (absDir[indices[0]] < absDir[indices[1]]) {
    int tmp = indices[0];
    indices[0] = indices[1];
    indices[1] = tmp;
  }

  int max = indices[0];
  bool negative = (absDir[max] != dir[max]);

  switch (max) {
  case 0:
    return negative ? 4 : 8;
  case 1:
    return (negative != 0) + 1;
  case 2:
    return negative ? 0x20 : 0x10;
  default:
    return 0;
  }
}

static int TransformDamageableTriggerFlags(CStateManager& mgr, TAreaId aId, uint flags) {
  const CGameArea& area = mgr.GetWorld()->GetAreaAlways(aId);
  CTransform4f rotation = area.GetTM().GetRotation();

  u32 ret = 0;
  if (flags & 0x01)
    ret |= ClassifyVector(rotation * CVector3f::Forward());
  if (flags & 0x02)
    ret |= ClassifyVector(rotation * CVector3f::Back());
  if (flags & 0x04)
    ret |= ClassifyVector(rotation * CVector3f::Left());
  if (flags & 0x08)
    ret |= ClassifyVector(rotation * CVector3f::Right());
  if (flags & 0x10)
    ret |= ClassifyVector(rotation * CVector3f::Up());
  if (flags & 0x20)
    ret |= ClassifyVector(rotation * CVector3f::Down());
  return ret;
}

CEntity* ScriptLoader::LoadDamageableTrigger(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (propCount != 12)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f position(in);
  CVector3f volume(in);

  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  int triggerFlags = TransformDamageableTriggerFlags(mgr, info.GetAreaId(), in.Get< int >());
  CAssetId patternTex1 = in.Get< CAssetId >();
  CAssetId patternTex2 = in.Get< CAssetId >();
  CAssetId colorTex = in.Get< CAssetId >();
  CScriptDamageableTrigger::ECanOrbit canOrbit = in.Get< bool >()
                                                     ? CScriptDamageableTrigger::kCO_Orbit
                                                     : CScriptDamageableTrigger::kCO_NoOrbit;
  bool active = in.Get< bool >();
  CVisorParameters vParms = LoadVisorParameters(in);
  return rs_new CScriptDamageableTrigger(mgr.AllocateUniqueId(), name, info, position, volume,
                                         hInfo, dVuln, triggerFlags, patternTex1, patternTex2,
                                         colorTex, canOrbit, active, vParms);
}

static CFluidUVMotion LoadFluidUVMotion(CInputStream& in) {
  CFluidUVMotion::EFluidMotion motion = CFluidUVMotion::EFluidMotion(in.ReadLong());
  float a = in.ReadFloat();
  float b = (in.ReadFloat() * M_PIF) / 180.f - M_PIF;
  float c = in.ReadFloat();
  float d = in.ReadFloat();
  CFluidUVMotion::SFluidLayerMotion pattern1Layer(motion, a, b, c, d);

  motion = CFluidUVMotion::EFluidMotion(in.ReadLong());
  a = in.ReadFloat();
  b = (in.ReadFloat() * M_PIF) / 180.f - M_PIF;
  c = in.ReadFloat();
  d = in.ReadFloat();
  CFluidUVMotion::SFluidLayerMotion pattern2Layer(motion, a, b, c, d);

  motion = CFluidUVMotion::EFluidMotion(in.ReadLong());
  a = in.ReadFloat();
  b = in.ReadFloat();
  c = in.ReadFloat();
  d = in.ReadFloat();
  CFluidUVMotion::SFluidLayerMotion colorLayer(motion, a, (b * M_PIF) / 180.f - M_PIF, c, d);

  a = in.ReadFloat();
  b = in.ReadFloat();
  return CFluidUVMotion(a, (b * M_PIF) / 180.f - M_PIF, colorLayer, pattern1Layer, pattern2Layer);
}

CEntity* ScriptLoader::LoadWater(CStateManager& mgr, CInputStream& in, int propCount,
                                 const CEntityInfo& info) {
  if (propCount != 63)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f position(in);
  CVector3f extent(in);
  CDamageInfo dInfo(in);
  CVector3f orientedForce(in);
  uint triggerFlags = in.ReadLong() | 0x7fc;
  bool thermalCold = in.ReadBool();
  bool displaySurface = in.ReadBool();
  CAssetId patternMap1 = in.Get< CAssetId >();
  CAssetId patternMap2 = in.Get< CAssetId >();
  CAssetId colorMap = in.Get< CAssetId >();
  CAssetId bumpMap = in.Get< CAssetId >();
  CAssetId envMap = in.Get< CAssetId >();
  CAssetId envBumpMap = in.Get< CAssetId >();
  CVector3f bumpLightDir = in.Get< CVector3f >();
  if (!bumpLightDir.CanBeNormalized()) {
    bumpLightDir = CVector3f(0.f, 0.f, -1.f);
  }

  float bumpScale = 1.f / in.Get< float >();
  float morphInTime = in.Get< float >();
  float morphOutTime = in.Get< float >();
  bool active = in.ReadBool();
  CFluidPlane::EFluidType fluidType = CFluidPlane::EFluidType(in.ReadLong());
  bool b4 = in.ReadBool();
  float alpha = in.Get< float >();
  CFluidUVMotion uvMotion = LoadFluidUVMotion(in);

  float turbSpeed = in.Get< float >();
  float turbDistance = in.Get< float >();
  float turbFreqMax = in.Get< float >();
  float turbFreqMin = in.Get< float >();
  float turbPhaseMax = (M_2PIF * in.Get< float >()) / 360.f;
  float turbPhaseMin = (M_2PIF * in.Get< float >()) / 360.f;
  float turbAmplitudeMax = in.Get< float >();
  float turbAmplitudeMin = in.Get< float >();
  CColor splashColor(in);
  CColor insideFogColor = in.Get< CColor >();
  CAssetId splashParticle1 = in.Get< CAssetId >();
  CAssetId splashParticle2 = in.Get< CAssetId >();
  CAssetId splashParticle3 = in.Get< CAssetId >();
  CAssetId visorRunoffParticle = in.Get< CAssetId >();
  CAssetId unmorphVisorRunoffParticle = in.Get< CAssetId >();
  int visorRunoffSfx = in.ReadLong();
  int unmorphVisorRunoffSfx = in.ReadLong();
  int splashSfx1 = in.ReadLong();
  int splashSfx2 = in.ReadLong();
  int splashSfx3 = in.ReadLong();
  float tileSize = in.Get< float >();
  uint tileSubdivisions = in.ReadLong();
  float specularMin = in.Get< float >();
  float specularMax = in.Get< float >();
  float reflectionSize = in.Get< float >();
  float rippleIntensity = in.Get< float >();
  float reflectionBlend = in.Get< float >();
  float fogBias = in.Get< float >();
  float fogMagnitude = in.Get< float >();
  float fogSpeed = in.Get< float >();
  CColor fogColor = in.Get< CColor >();
  CAssetId lightmap = in.Get< CAssetId >();
  float unitsPerLightmapTexel = in.Get< float >();
  float alphaInTime = in.Get< float >();
  float alphaOutTime = in.Get< float >();
  uint w21 = in.ReadLong();
  uint w22 = in.ReadLong();
  bool b5 = in.ReadBool();

  int bitVal0 = 0;
  int bitVal1 = 0;
  uint* bitset = NULL;

  if (b5) {
    bitVal0 = (short)in.ReadShort();
    bitVal1 = (short)in.ReadShort();
    int len = (bitVal0 * bitVal1 + 31) / 32;
    bitset = rs_new uint[len];
    in.ReadBytes(bitset, len * 4);
  }

  float hx = extent.GetX() * 0.5f;
  float hy = extent.GetY() * 0.5f;
  float hz = extent.GetZ() * 0.5f;
  CAABox box = CAABox(CVector3f(-hx, -hy, -hz), CVector3f(hx, hy, hz));
  return rs_new CScriptWater(
      mgr, mgr.AllocateUniqueId(), name, info, position, box, dInfo, orientedForce, triggerFlags,
      thermalCold, displaySurface, patternMap1, patternMap2, colorMap, bumpMap,
      bumpMap == kInvalidAssetId ? envMap : kInvalidAssetId,
      bumpMap == kInvalidAssetId ? envBumpMap : kInvalidAssetId, kInvalidAssetId, bumpLightDir,
      bumpScale, morphInTime, morphOutTime, active, fluidType, b4, alpha, uvMotion, turbSpeed,
      turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax, turbPhaseMin, turbAmplitudeMax,
      turbAmplitudeMin, splashColor, insideFogColor, splashParticle1, splashParticle2,
      splashParticle3, visorRunoffParticle, unmorphVisorRunoffParticle, visorRunoffSfx,
      unmorphVisorRunoffSfx, splashSfx1, splashSfx2, splashSfx3, tileSize, tileSubdivisions,
      specularMin, specularMax, reflectionSize, rippleIntensity, reflectionBlend, fogBias,
      fogMagnitude, fogSpeed, fogColor, lightmap, unitsPerLightmapTexel, alphaInTime, alphaOutTime,
      w21, w22, b5, bitVal0, bitVal1, bitset);
}

CEntity* ScriptLoader::LoadSteam(CStateManager& mgr, CInputStream& in, int propCount,
                                 const CEntityInfo& info) {
  if (propCount != 11)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f position(in);
  CVector3f extent(in);
  CDamageInfo dInfo(in);
  CVector3f orientedForce(in);
  uint triggerFlags = in.ReadLong();
  bool active = in.ReadBool();
  CAssetId texture = in.ReadLong();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  bool b2 = in.ReadBool();

  float hx = extent.GetX() * 0.5f;
  float hy = extent.GetY() * 0.5f;
  float hz = extent.GetZ() * 0.5f;
  CAABox aabb = CAABox(CVector3f(-hx, -hy, -hz), CVector3f(hx, hy, hz));
  return rs_new CScriptSteam(mgr.AllocateUniqueId(), name, info, position, aabb, dInfo,
                             orientedForce, triggerFlags, active, texture, f1, f2, f3, f4, b2);
}

CEntity* ScriptLoader::LoadRipple(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f center(in);
  bool active = in.Get< bool >();
  float mag = in.Get< float >();
  return rs_new CScriptRipple(mgr.AllocateUniqueId(), name, info, center, active, mag);
}

CEntity* ScriptLoader::LoadSpawnPoint(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount < 35)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f position(in);
  CVector3f rotation(in);

  rstl::reserved_vector< int, int(CPlayerState::kIT_Max) > itemCounts;
  for (int i = 0; i < propCount - 6; ++i)
    itemCounts.push_back(in.ReadLong());

  for (int i = itemCounts.size(); i < itemCounts.capacity(); ++i) {
    itemCounts.push_back(0);
  }

  bool defaultSpawn = in.Get< bool >();
  bool active = in.Get< bool >();
  bool morphed = false;
  if (propCount >= 35)
    morphed = in.Get< bool >();

  return rs_new CScriptSpawnPoint(
      mgr.AllocateUniqueId(), name, info, ConvertEditorEulerToTransform4f(rotation, position),
      rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >(itemCounts), defaultSpawn, active,
      morphed);
}

CEntity* ScriptLoader::LoadJumpPoint(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  return nullptr;
}

CEntity* ScriptLoader::LoadDock(CStateManager& mgr, CInputStream& in, int propCount,
                                const CEntityInfo& info) {
  if (propCount != 7)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  CVector3f position(in);
  CVector3f scale(in);
  int dock = in.Get< int >();
  int area = in.Get< int >();
  bool loadConnected = in.Get< bool >();
  return rs_new CScriptDock(mgr.AllocateUniqueId(), name, info, position, scale, dock, area, active,
                            0, loadConnected);
}

CEntity* ScriptLoader::LoadCameraHint(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount > 25)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  int priority = in.Get< int >();
  CBallCamera::EBallCameraBehaviour behaviour = CBallCamera::EBallCameraBehaviour(in.ReadLong());
  uint overrideFlags = LoadParameterFlags(in);
  if (in.ReadBool())
    overrideFlags |= 0x400000;
  float minDist = in.Get< float >();
  if (in.ReadBool())
    overrideFlags |= 0x800000;
  float maxDist = in.Get< float >();
  if (in.ReadBool())
    overrideFlags |= 0x1000000;
  float backwardsDist = in.Get< float >();
  if (in.ReadBool())
    overrideFlags |= 0x2000000;
  CVector3f lookAtOffset(in);
  if (in.ReadBool())
    overrideFlags |= 0x4000000;
  CVector3f chaseLookAtOffset(in);
  CVector3f ballToCam = CVector3f::Zero();
  ballToCam = CVector3f(in);
  if (in.ReadBool())
    overrideFlags |= 0x8000000;
  float fov = in.Get< float >();
  if (in.ReadBool())
    overrideFlags |= 0x10000000;
  CRelAngle attitudeRange = CRelAngle::FromDegrees(in.ReadFloat());
  if (in.ReadBool())
    overrideFlags |= 0x20000000;
  CRelAngle azimuthRange = CRelAngle::FromDegrees(in.ReadFloat());
  if (in.ReadBool())
    overrideFlags |= 0x40000000;
  CRelAngle anglePerSecond = CRelAngle::FromDegrees(in.ReadFloat());
  float clampVelRange = in.Get< float >();
  CRelAngle clampRotRange = CRelAngle::FromDegrees(in.ReadFloat());
  if (in.ReadBool())
    overrideFlags |= 0x80000000;
  float elevation = in.Get< float >();
  // TODO: the following 2 were swapped in Metaforce, so downstream symbols may be wrong
  float clampVelTime = in.Get< float >();
  float interpolateTime = in.Get< float >();
  float controlInterpDur = in.Get< float >();

  return rs_new CScriptCameraHint(
      mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform, active, priority, behaviour,
      overrideFlags, minDist, maxDist, backwardsDist, lookAtOffset, chaseLookAtOffset, ballToCam,
      fov, attitudeRange.AsRadians(), azimuthRange.AsRadians(), anglePerSecond.AsRadians(),
      clampVelRange, clampRotRange.AsRadians(), elevation, interpolateTime, clampVelTime,
      controlInterpDur);
}

CEntity* ScriptLoader::LoadPlayerHint(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount < 6)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  uint overrideFlags = LoadParameterFlags(in);
  int priority = in.Get< int >();
  return rs_new CScriptPlayerHint(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                                  active, priority, overrideFlags);
}

CEntity* ScriptLoader::LoadPointOfInterest(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (propCount != 6)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  CScannableParameters sParms = LoadScannableParameters(in);
  float pointSize = in.ReadFloat();
  return rs_new CScriptPointOfInterest(mgr.AllocateUniqueId(), aHead.mName, info,
                                       aHead.mTransform, active, sParms, pointSize);
}

CEntity* ScriptLoader::LoadActor(CStateManager& mgr, CInputStream& in, int propCount,
                                 const CEntityInfo& info) {
  if (propCount < 24)
    return nullptr;

  SScaledActorHead head(in, mgr);
  CVector3f collisionExtent(in);
  CVector3f centroid(in);

  float mass = in.ReadFloat();
  float zMomentum = in.ReadFloat();

  CHealthInfo hInfo(in);

  CDamageVulnerability dVuln(in);

  CAssetId staticId = in.Get< CAssetId >();
  CAnimationParameters aParms = LoadAnimationParameters(in);

  CActorParameters actParms = LoadActorParameters(in);

  bool looping = in.Get< bool >();
  bool immovable = in.Get< bool >();
  bool solid = in.Get< bool >();
  bool cameraPassthrough = in.Get< bool >();
  bool active = in.Get< bool >();
  uint shaderIdx = in.Get< uint >();
  float xrayAlpha = in.ReadFloat();
  bool noThermalHotZ = in.Get< bool >();
  bool castsShadow = in.Get< bool >();
  bool scaleAdvancementDelta = in.Get< bool >();
  bool materialFlag54 = in.Get< bool >();

  FourCC staticType = gpResourceFactory->GetResourceTypeById(staticId);
  FourCC animType = gpResourceFactory->GetResourceTypeById(aParms.GetACSFile());
  if (staticType == 0 && animType == 0)
    return nullptr;

  const CTransform4f& xf = head.mActorHead.mTransform;

  CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, centroid);

  CMaterialList list;
  if (immovable) // Bool 2
    list.Add(kMT_Immovable);

  if (solid) // Bool 3
    list.Add(kMT_Solid);

  if (cameraPassthrough) // Bool 4
    list.Add(kMT_CameraPassthrough);

  bool negativeCollisionExtent;
  if (collisionExtent.GetX() < 0.f)
    negativeCollisionExtent = true;
  else if (collisionExtent.GetY() < 0.f)
    negativeCollisionExtent = true;
  else if (collisionExtent.GetZ() < 0.f)
    negativeCollisionExtent = true;
  else
    negativeCollisionExtent = false;

  CModelData data(CModelData::CModelDataNull());
  if (animType == 'ANCS') {
    data = CModelData(CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.mScale,
                               aParms.GetInitialAnimation(), true));
  } else {
    data = CModelData(CStaticRes(staticId, head.mScale));
  }

  if (collisionExtent == CVector3f::Zero() || negativeCollisionExtent)
    aabb = data.GetBounds(xf.GetRotation());

  return rs_new CScriptActor(mgr.AllocateUniqueId(), head.mActorHead.mName, info, xf, data,
                             aabb, list, mass, zMomentum, hInfo, dVuln, actParms, looping, active,
                             shaderIdx, xrayAlpha, noThermalHotZ, castsShadow,
                             scaleAdvancementDelta, materialFlag54);
}

CEntity* ScriptLoader::LoadPickup(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != 18)
    return nullptr;

  SScaledActorHead head(in, mgr);
  CVector3f extent(in);
  CVector3f offset(in);
  CPlayerState::EItemType itemType = CPlayerState::EItemType(in.Get< int >());
  int capacity = in.Get< int >();
  int amount = in.Get< int >();
  float possibility = in.Get< float >();
  float lifeTime = in.Get< float >();
  float fadeInTime = in.Get< float >();
  CAssetId staticModel = in.Get< CAssetId >();
  CAnimationParameters aParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  bool active = in.Get< bool >();
  float startDelay = in.Get< float >();
  CAssetId pickupEffect = in.Get< CAssetId >();

  FourCC staticModelType = gpResourceFactory->GetResourceTypeById(staticModel);
  FourCC animType = gpResourceFactory->GetResourceTypeById(aParms.GetACSFile());
  if (staticModelType == 0 && animType == 0)
    return nullptr;

  const CTransform4f& xf = head.mActorHead.mTransform;
  CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), extent, offset);

  bool negativeExtent;
  if (extent.GetX() < 0.f)
    negativeExtent = true;
  else if (extent.GetY() < 0.f)
    negativeExtent = true;
  else if (extent.GetZ() < 0.f)
    negativeExtent = true;
  else
    negativeExtent = false;

  CModelData data(CModelData::CModelDataNull());
  if (animType == 'ANCS') {
    data = CModelData(CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.mScale,
                               aParms.GetInitialAnimation(), true));
  } else {
    data = CModelData(CStaticRes(staticModel, head.mScale));
  }

  if (extent == CVector3f::Zero() || negativeExtent)
    aabb = data.GetBounds(xf.GetRotation());

  return rs_new CScriptPickup(mgr.AllocateUniqueId(), head.mActorHead.mName, info, xf, data,
                              actParms, aabb, itemType, amount, capacity, pickupEffect, possibility,
                              lifeTime, fadeInTime, startDelay, active);
}

CEntity* ScriptLoader::LoadEnemy(CStateManager& mgr, CInputStream& in, int propCount,
                                 const CEntityInfo& info) {
  return nullptr;
}

CEntity* ScriptLoader::LoadBeetle(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != 16)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.Get< int >());
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Beetle");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CDamageInfo touchDamage(in);
  CVector3f tailAimReference = in.Get< CVector3f >();
  float unused = in.ReadFloat();
  CDamageVulnerability tailVuln(in);
  CDamageVulnerability platingVuln(in);
  CAssetId tailModel = in.Get< CAssetId >();
  CBeetle::EEntranceType entranceType = CBeetle::EEntranceType(in.ReadLong());
  float initialAttackDelay = in.ReadFloat();
  float retreatTime = in.ReadFloat();

  FourCC animType =
      gpResourceFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  rstl::optional_object< CStaticRes > tailRes;
  if (flavor == CPatterned::kFT_One) {
    if (gpResourceFactory->GetResourceTypeById(tailModel) == 0)
      return nullptr;
    tailRes = CStaticRes(tailModel, scale);
  }

  return rs_new CBeetle(
      mgr.AllocateUniqueId(), name, info, xf,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, flavor, entranceType, touchDamage, platingVuln, tailAimReference, initialAttackDelay,
      retreatTime, unused, tailVuln, actParms, tailRes);
}

CEntity* ScriptLoader::LoadWarWasp(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount != 13)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.Get< int >());
  CTransform4f xf = LoadEditorTransformPivotOnly(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Warwasp");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  bool collider = in.ReadBool();
  CDamageInfo damageInfo(in);
  CAssetId projectileWeapon = in.ReadLong();
  CDamageInfo projectileDamage(in);
  CAssetId projectileVisorParticle = in.ReadLong();
  uint projectileVisorSfx = in.ReadLong();

  const CAnimationParameters& anim = pInfo.GetAnimationParameters();
  FourCC animType = gpResourceFactory->GetResourceTypeById(anim.GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  return rs_new CWarWasp(
      mgr.AllocateUniqueId(), name, info, xf,
      CModelData(CAnimRes(anim.GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, flavor, collider ? CPatterned::kCT_One : CPatterned::kCT_Zero, damageInfo, actParms,
      projectileWeapon, projectileDamage, projectileVisorParticle, projectileVisorSfx);
}

CEntity* ScriptLoader::LoadWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 13)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  float speed = in.ReadFloat();
  float pause = in.ReadFloat();
  int patternTranslate = in.Get< int >();
  int patternOrient = in.Get< int >();
  int patternFit = in.Get< int >();
  int behaviour = in.Get< int >();
  int behaviourOrient = in.Get< int >();
  int behaviourModifiers = in.Get< int >();
  uint animation = in.Get< int >();

  return rs_new CScriptWaypoint(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                                active, speed, pause, patternTranslate, patternOrient, patternFit,
                                behaviour, behaviourOrient, behaviourModifiers, animation);
}

CEntity* ScriptLoader::LoadDoor(CStateManager& mgr, CInputStream& in, int propCount,
                                const CEntityInfo& info) {
  bool valid = false;
  if (propCount >= 13 && propCount <= 14)
    valid = true;
  if (!valid)
    return nullptr;

  SScaledActorHead head(in, mgr);
  CAnimationParameters aParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  CVector3f orbitPos(in);
  CVector3f collisionExtent(in);
  CVector3f offset(in);
  bool active = in.ReadBool();
  bool open = in.ReadBool();
  bool projectilesCollide = in.ReadBool();
  float animationLength = in.ReadFloat();
  const CTransform4f& xf = head.mActorHead.mTransform;

  CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, offset);

  if (gpResourceFactory->GetResourceTypeById(aParms.GetACSFile()) == 0)
    return nullptr;

  CModelData mData(
      CAnimRes(aParms.GetACSFile(), CAnimRes::kDefaultCharIdx, head.mScale, 0, false));

  if (collisionExtent == CVector3f::Zero())
    aabb = mData.GetBounds(xf.GetRotation());

  bool isMorphballDoor = false;
  if (propCount == 13) {
    if (mData.GetAnimationData()->GetSelfId() == 0x1f9da858)
      isMorphballDoor = true;
  }
  if (propCount == 14)
    isMorphballDoor = in.ReadBool();

  return rs_new CScriptDoor(mgr.AllocateUniqueId(), head.mActorHead.mName, info, xf, mData,
                            actParms, orbitPos, aabb, active, open, projectilesCollide,
                            animationLength, isMorphballDoor);
}

CEntity* ScriptLoader::LoadEffect(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != 24)
    return nullptr;

  SScaledActorHead head(in, mgr);
  CAssetId partId = in.Get< CAssetId >();
  CAssetId elscId = in.Get< CAssetId >();
  bool hotInThermal = in.ReadBool();
  bool noTimerUnlessAreaOccluded = in.ReadBool();
  bool rebuildSystemsOnActivate = in.ReadBool();
  bool active = in.ReadBool();

  if (partId == kInvalidAssetId && elscId == kInvalidAssetId)
    return nullptr;

  FourCC partType = gpResourceFactory->GetResourceTypeById(partId);
  FourCC elscType = gpResourceFactory->GetResourceTypeById(elscId);
  if (partType == 0 && elscType == 0)
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

  return rs_new CScriptEffect(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      head.mScale, partId, elscId, hotInThermal, noTimerUnlessAreaOccluded,
      rebuildSystemsOnActivate, active, useRateInverseCamDist, rateInverseCamDist,
      rateInverseCamDistRate, duration, durationResetWhileVisible, useRateCamDistRange,
      rateCamDistRangeMin, rateCamDistRangeMax, rateCamDistRangeFarRate, combatVisorVisible,
      thermalVisorVisible, xrayVisorVisible, lParms, dieWhenSystemsDone);
}

CEntity* ScriptLoader::LoadTimer(CStateManager& mgr, CInputStream& in, int propCount,
                                 const CEntityInfo& info) {
  if (propCount != 6)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  float startTime = in.Get< float >();
  float maxRandDelay = in.Get< float >();
  bool loop = in.Get< bool >();
  bool autoStart = in.Get< bool >();
  bool active = in.Get< bool >();
  return rs_new CScriptTimer(mgr.AllocateUniqueId(), name, info, startTime, maxRandDelay, loop,
                             autoStart, active);
}

CEntity* ScriptLoader::LoadCounter(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount != 5)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  int initial = in.Get< int >();
  int max = in.Get< int >();
  bool autoReset = in.Get< bool >();
  bool active = in.Get< bool >();
  return rs_new CScriptCounter(mgr.AllocateUniqueId(), name, info, initial, max, autoReset, active);
}

CEntity* ScriptLoader::LoadGenerator(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount != 8)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  int spawnCount = in.Get< int >();
  bool noReuseFollowers = in.ReadBool();
  bool noInheritXf = in.ReadBool();
  CVector3f offset(in);
  bool active = in.Get< bool >();
  float minScale = in.ReadFloat();
  float maxScale = in.ReadFloat();
  return rs_new CScriptGenerator(mgr.AllocateUniqueId(), name, info, spawnCount, noReuseFollowers,
                                 offset, noInheritXf, active, minScale, maxScale);
}

CEntity* ScriptLoader::LoadSound(CStateManager& mgr, CInputStream& in, int propCount,
                                 const CEntityInfo& info) {
  if (propCount != 20)
    return nullptr;

  SActorHead head(in, mgr);
  int soundId = in.Get< int >();
  bool active = in.Get< bool >();
  float maxDist = in.ReadFloat();
  float distComp = in.ReadFloat();
  float startDelay = in.ReadFloat();
  uint minVol = in.Get< uint >();
  uint vol = in.Get< uint >();
  uint prio = in.Get< uint >();
  uint pan = in.Get< uint >();
  bool looped = in.Get< bool >();
  bool nonEmitter = in.Get< bool >();
  bool autoStart = in.Get< bool >();
  bool occlusionTest = in.Get< bool >();
  bool acoustics = in.Get< bool >();
  bool worldSfx = in.Get< bool >();
  bool allowDuplicates = in.Get< bool >();
  int pitch = in.Get< int >();

  if (soundId < 0)
    return nullptr;

  return rs_new CScriptSound(mgr.AllocateUniqueId(), head.mName, info, head.mTransform,
                             static_cast< ushort >(soundId), active, maxDist, distComp, startDelay,
                             minVol, vol, 0, prio, pan, 0, looped, nonEmitter, autoStart,
                             occlusionTest, acoustics, worldSfx, allowDuplicates, pitch);
}

CEntity* ScriptLoader::LoadPlatform(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 19)
    return nullptr;

  SScaledActorHead head(in, mgr);
  CVector3f extent(in);
  CVector3f centroid(in);
  CAssetId staticId = in.Get< uint >();
  CAnimationParameters aParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  float speed = in.ReadFloat();
  bool active = in.ReadBool();
  CAssetId dclnId = in.Get< uint >();
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  bool detectCollision = in.ReadBool();
  float xrayAlpha = in.ReadFloat();
  bool rainSplashes = in.ReadBool();
  uint maxRainSplashes = in.Get< uint >();
  uint rainGenRate = in.Get< uint >();

  FourCC staticType = gpResourceFactory->GetResourceTypeById(staticId);
  FourCC animType = gpResourceFactory->GetResourceTypeById(aParms.GetACSFile());
  if (staticType == 0 && animType == 0)
    return nullptr;

  const CTransform4f& xf = head.mActorHead.mTransform;
  CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), extent, centroid);

  FourCC dclnType = gpResourceFactory->GetResourceTypeById(dclnId);
  rstl::optional_object< TLockedToken< CCollidableOBBTreeGroupContainer > > dclnToken;
  if (dclnType != 0) {
    dclnToken = TLockedToken< CCollidableOBBTreeGroupContainer >(
        gpSimplePool->GetObj(SObjectTag('DCLN', dclnId)));
  }

  CModelData data(CModelData::CModelDataNull());
  if (animType == 'ANCS') {
    data = CModelData(CAnimRes(aParms.GetACSFile(), CAnimRes::kDefaultCharIdx, head.mScale,
                               aParms.GetInitialAnimation(), true));
  } else {
    data = CModelData(CStaticRes(staticId, head.mScale));
  }

  if (extent == CVector3f::Zero())
    aabb = data.GetBounds(xf.GetRotation());

  return rs_new CScriptPlatform(mgr.AllocateUniqueId(), head.mActorHead.mName, info, xf, data,
                                actParms, aabb, speed, detectCollision, xrayAlpha, active, hInfo,
                                dVuln, dclnToken, rainSplashes, maxRainSplashes, rainGenRate);
}

CEntity* ScriptLoader::LoadCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (propCount != 6)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  float hfov = in.ReadFloat();
  uint w1 = in.Get< uint >();
  return rs_new CScriptCameraWaypoint(mgr.AllocateUniqueId(), aHead.mName, info,
                                      aHead.mTransform, active, hfov, w1);
}

CEntity* ScriptLoader::LoadCamera(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  bool valid = false;
  if (propCount >= 14 && propCount <= 15)
    valid = true;
  if (!valid)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  float shotDuration = in.ReadFloat();
  bool lookAtPlayer = in.Get< bool >();
  bool outOfPlayerEye = in.Get< bool >();
  bool intoPlayerEye = in.Get< bool >();
  bool drawPlayer = in.Get< bool >();
  bool disableInput = in.Get< bool >();
  bool b7 = in.Get< bool >();
  bool finishCineSkip = in.Get< bool >();
  float fov = in.ReadFloat();
  bool checkFailsafe = in.Get< bool >();
  bool disableOutOfInto;
  if (propCount > 14)
    disableOutOfInto = in.Get< bool >();
  else
    disableOutOfInto = false;
  float nearPlane = CCameraManager::GetDefaultFirstPersonNearClipDistance();
  float farPlane = CCameraManager::GetDefaultFirstPersonFarClipDistance();
  float aspect = CCameraManager::GetDefaultAspectRatio();
  uint flags = (lookAtPlayer ? 1u : 0u) | (outOfPlayerEye ? 2u : 0u) | (intoPlayerEye ? 4u : 0u) |
               (b7 ? 8u : 0u) | (finishCineSkip ? 0x10u : 0u) | (disableInput ? 0x20u : 0u) |
               (drawPlayer ? 0x40u : 0u) | (checkFailsafe ? 0x80u : 0u) |
               (disableOutOfInto ? 0x200u : 0u);
  return rs_new CCinematicCamera(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                                 active, shotDuration, fov / aspect, nearPlane, farPlane, aspect,
                                 flags);
}

CEntity* ScriptLoader::LoadPathCamera(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount > 15)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  uint flags = LoadParameterFlags(in);
  float lengthExtent = in.ReadFloat();
  float filterMag = in.ReadFloat();
  float filterProportion = in.ReadFloat();
  CPathCamera::EInitialSplinePosition initPos = CPathCamera::EInitialSplinePosition(in.ReadLong());
  float minEaseDist = in.ReadFloat();
  float maxEaseDist = in.ReadFloat();
  return rs_new CPathCamera(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                            active, lengthExtent, filterMag, filterProportion, minEaseDist,
                            maxEaseDist, flags, initPos);
}

CEntity* ScriptLoader::LoadSpindleCamera(CStateManager& mgr, CInputStream& in, int propCount,
                                         const CEntityInfo& info) {
  if (propCount != 24)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  uint flags = LoadParameterFlags(in);
  float hintToCamDistMin = in.ReadFloat();
  float hintToCamDistMax = in.ReadFloat();
  float hintToCamVOffMin = in.ReadFloat();
  float hintToCamVOffMax = in.ReadFloat();

  CSpindleCameraInterpolant seg1 = LoadSpindleSegment(in);
  seg1.ConvertToRadians();
  CSpindleCameraInterpolant seg2 = LoadSpindleSegment(in);
  CSpindleCameraInterpolant seg3 = LoadSpindleSegment(in);
  CSpindleCameraInterpolant seg4 = LoadSpindleSegment(in);
  CSpindleCameraInterpolant seg5 = LoadSpindleSegment(in);
  seg5.ConvertToRadians();
  CSpindleCameraInterpolant seg6 = LoadSpindleSegment(in);
  seg6.ConvertToRadians();
  CSpindleCameraInterpolant seg7 = LoadSpindleSegment(in);
  seg7.ConvertToRadians();
  CSpindleCameraInterpolant seg8 = LoadSpindleSegment(in);
  seg8.ConvertToRadians();
  CSpindleCameraInterpolant seg9 = LoadSpindleSegment(in);
  CSpindleCameraInterpolant seg10 = LoadSpindleSegment(in);
  CSpindleCameraInterpolant seg11 = LoadSpindleSegment(in);
  seg11.ConvertToRadians();
  CSpindleCameraInterpolant seg12 = LoadSpindleSegment(in);
  seg12.ConvertToRadians();
  CSpindleCameraInterpolant seg13 = LoadSpindleSegment(in);
  seg13.ConvertToRadians();
  CSpindleCameraInterpolant seg14 = LoadSpindleSegment(in);
  CSpindleCameraInterpolant seg15 = LoadSpindleSegment(in);
  seg15.ConvertToRadians();

  return rs_new CScriptSpindleCamera(
      mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform, active, flags,
      hintToCamDistMin, hintToCamDistMax, hintToCamVOffMin, hintToCamVOffMax, seg1, seg2, seg3,
      seg4, seg5, seg6, seg7, seg8, seg9, seg10, seg11, seg12, seg13, seg14, seg15);
}

CEntity* ScriptLoader::LoadRelay(CStateManager& mgr, CInputStream& in, int propCount,
                                 const CEntityInfo& info) {
  bool valid = false;
  if (propCount >= 3 && propCount <= 4)
    valid = true;
  if (!valid)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool b1 = in.Get< bool >();
  bool b2 = in.Get< bool >();
  bool b3 = false;
  if (propCount > 3)
    b3 = in.Get< bool >();
  return rs_new CScriptMemoryRelay(mgr.AllocateUniqueId(), name, info, b1, b2, b3);
}

CEntity* ScriptLoader::LoadRandomRelay(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 5)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  int sendSetSize = in.Get< int >();
  int sendSetVariance = in.Get< int >();
  bool percentSize = in.ReadBool();
  bool active = in.ReadBool();
  return rs_new CScriptRandomRelay(mgr.AllocateUniqueId(), name, info, sendSetSize, sendSetVariance,
                                   percentSize, active);
}

CEntity* ScriptLoader::LoadFaultyRelay(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  bool valid = propCount >= 2 && propCount <= 3;
  if (!valid)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  if (propCount >= 3)
    in.ReadLong();
  bool active = in.ReadBool();
  return rs_new CScriptRelay(mgr.AllocateUniqueId(), name, info, active);
}

CEntity* ScriptLoader::LoadHUDMemo(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  bool isPropCount5 = false;
  if (propCount == 5)
    isPropCount5 = true;
  else if (propCount != 6)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CHUDMemoParms hParms(in);
  CScriptHUDMemo::EDisplayType displayType;
  if (isPropCount5)
    displayType = CScriptHUDMemo::kDT_MessageBox;
  else
    displayType = CScriptHUDMemo::EDisplayType(in.ReadLong());
  CAssetId message = in.Get< CAssetId >();
  bool active = in.Get< bool >();
  return rs_new CScriptHUDMemo(mgr.AllocateUniqueId(), name, info, hParms, displayType, message,
                               active);
}

CEntity* ScriptLoader::LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& in, int propCount,
                                                const CEntityInfo& info) {
  if (propCount != 10)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.ReadBool();
  int ftype = in.Get< int >();
  int shape = in.Get< int >();
  int filterIdx = in.Get< int >();
  int unk = in.Get< int >();
  CColor color(in);
  float timeIn = in.ReadFloat();
  float timeOut = in.ReadFloat();
  CAssetId txtr = in.ReadLong();
  return rs_new CScriptCameraFilterKeyframe(
      mgr.AllocateUniqueId(), name, info, CCameraFilterPass::EFilterType(ftype),
      CCameraFilterPass::EFilterShape(shape), CStateManager::ECameraFilterStage(filterIdx), unk,
      color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha(), timeIn, timeOut, txtr,
      active);
}

CEntity* ScriptLoader::LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info) {
  if (propCount != 7)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  int type = in.Get< int >();
  float amount = in.ReadFloat();
  int unk = in.Get< int >();
  float timeIn = in.ReadFloat();
  float timeOut = in.ReadFloat();
  return rs_new CScriptCameraBlurKeyframe(mgr.AllocateUniqueId(), name, info,
                                          CCameraBlurPass::EBlurType(type), amount, unk, timeIn,
                                          timeOut, active);
}

CEntity* ScriptLoader::LoadDebris(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != 0x12)
    return nullptr;

  SScaledActorHead head(in, mgr);

  float zImpulse = in.ReadFloat();
  CVector3f velocity(in);
  CColor endsColor(in);
  float mass = in.ReadFloat();
  float restitution = in.ReadFloat();
  float duration = in.ReadFloat();
  CScriptDebris::EScaleType scaleType = CScriptDebris::EScaleType(in.Get< int >());
  bool randomAngImpulse = in.Get< bool >();
  CAssetId model = in.Get< CAssetId >();
  CActorParameters aParams = LoadActorParameters(in);
  CAssetId particleId = in.Get< CAssetId >();
  CVector3f particleScale(in);
  bool unused = in.Get< bool >();
  bool active = in.Get< bool >();

  if (gpResourceFactory->GetResourceTypeById(model) == 0)
    return nullptr;

  return rs_new CScriptDebris(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CStaticRes(model, head.mScale)), aParams, particleId, particleScale, zImpulse,
      velocity, endsColor, mass, restitution, duration, scaleType, unused,
      randomAngImpulse, active);
}

CEntity* ScriptLoader::LoadDebrisExtended(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (propCount != 0x27)
    return nullptr;

  SScaledActorHead head(in, mgr);

  float linConeAngle = in.ReadFloat();
  float linMinMag = in.ReadFloat();
  float linMaxMag = in.ReadFloat();
  float angMinMag = in.ReadFloat();
  float angMaxMag = in.ReadFloat();
  float minDuration = in.ReadFloat();
  float maxDuration = in.ReadFloat();
  float colorInT = in.ReadFloat();
  float colorOutT = in.ReadFloat();
  CColor color(in);
  CColor endsColor(in);
  float scaleOutStartT = in.ReadFloat();
  CVector3f endScale(in);
  float restitution = in.ReadFloat();
  float downwardSpeed = in.ReadFloat();
  CVector3f localOffset(in);

  CAssetId model = in.Get< CAssetId >();
  CActorParameters aParams = LoadActorParameters(in);

  CAssetId particle0 = in.Get< CAssetId >();
  CVector3f particle0Scale(in);
  bool particle1GlobalTranslation = in.Get< bool >();
  bool deferDeleteTillParticle1Done = in.Get< bool >();
  CScriptDebris::EOrientationType particleOr0 = CScriptDebris::EOrientationType(in.Get< int >());
  CAssetId particle1 = in.Get< CAssetId >();
  CVector3f particle1Scale(in);
  bool particle2GlobalTranslation = in.Get< bool >();
  bool deferDeleteTillParticle2Done = in.Get< bool >();
  CScriptDebris::EOrientationType particleOr1 = CScriptDebris::EOrientationType(in.Get< int >());
  CAssetId particle2 = in.Get< CAssetId >();
  CVector3f particle2Scale(in);
  CScriptDebris::EOrientationType particleOr2 = CScriptDebris::EOrientationType(in.Get< int >());
  bool solid = in.Get< bool >();
  bool dieOnProjectile = in.Get< bool >();
  bool noBounce = in.Get< bool >();
  bool active = in.Get< bool >();

  uint resType = gpResourceFactory->GetResourceTypeById(model);

  // Retail uses the second particle scale for the third generator as well.
  return rs_new CScriptDebris(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      resType != 0 ? CModelData(CStaticRes(model, head.mScale)) : CModelData::CModelDataNull(),
      aParams, linConeAngle, linMinMag, linMaxMag, angMinMag, angMaxMag, minDuration, maxDuration,
      colorInT, colorOutT, color, endsColor, scaleOutStartT, head.mScale, endScale, restitution,
      downwardSpeed, localOffset, particle0, particle0Scale, particle1GlobalTranslation,
      deferDeleteTillParticle1Done, particleOr0, particle1, particle1Scale,
      particle2GlobalTranslation, deferDeleteTillParticle2Done, particleOr1, particle2,
      particle1Scale, particleOr2, solid, dieOnProjectile, noBounce, active);
}

CEntity* ScriptLoader::LoadCameraShaker(CStateManager& mgr, CInputStream& in, int propCount,
                                        const CEntityInfo& info) {
  if (propCount != 9)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CCameraShakeData shakeData = LoadCameraShakeData(in);
  bool active = in.Get< bool >();
  return rs_new CScriptCameraShaker(mgr.AllocateUniqueId(), name, info, active, shakeData);
}

CEntity* ScriptLoader::LoadNewCameraShaker(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (propCount != 8)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f sfxPos(in);
  bool active = in.Get< bool >();
  int flags = LoadParameterFlags(in);
  float duration = in.ReadFloat();
  float sfxDist = in.ReadFloat();
  CCameraShakerComponent shakerX = LoadNewCameraShakerComponent(in);
  CCameraShakerComponent shakerY = LoadNewCameraShakerComponent(in);
  CCameraShakerComponent shakerZ = LoadNewCameraShakerComponent(in);

  CCameraShakeData shakeData(duration, sfxDist, flags, sfxPos, shakerX, shakerY, shakerZ);

  return rs_new CScriptCameraShaker(mgr.AllocateUniqueId(), name, info, active, shakeData);
}

CEntity* ScriptLoader::LoadActorKeyframe(CStateManager& mgr, CInputStream& in, int propCount,
                                         const CEntityInfo& info) {
  if (propCount != 7)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  int animId = in.Get< int >();
  bool looping = in.Get< bool >();
  float lifetime = in.ReadFloat();
  bool active = in.Get< bool >();
  int fadeOut = in.Get< int >();
  float totalPlayback = in.ReadFloat();

  if (animId == -1) {
    rstl::string msg =
        rstl::string_l("ERROR: Actor/Ai keyframe \'") + name +
        rstl::string_l("\' not loaded because it has no valid animation selected.\n");
    return nullptr;
  }

  return rs_new CScriptActorKeyframe(mgr.AllocateUniqueId(), name, info, animId, looping, lifetime,
                                     false, fadeOut, active, totalPlayback);
}

CEntity* ScriptLoader::LoadAIKeyframe(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  CScriptActorKeyframe* kf =
      static_cast< CScriptActorKeyframe* >(LoadActorKeyframe(mgr, in, propCount, info));
  if (kf != nullptr)
    kf->SetIsPassive(true);
  return kf;
}

CEntity* ScriptLoader::LoadColorModulate(CStateManager& mgr, CInputStream& in, int propCount,
                                         const CEntityInfo& info) {
  if (propCount != 12)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CColor colorA(in);
  CColor colorB(in);
  int blendMode = in.Get< int >();
  float timeA2B = in.ReadFloat();
  float timeB2A = in.ReadFloat();
  bool doReverse = in.ReadBool();
  bool resetTargetWhenDone = in.Get< bool >();
  bool depthCompare = in.Get< bool >();
  bool depthUpdate = in.Get< bool >();
  bool depthBackwards = in.Get< bool >();
  bool active = in.Get< bool >();
  return rs_new CScriptColorModulate(mgr.AllocateUniqueId(), name, info, colorA, colorB,
                                     CScriptColorModulate::EBlendMode(blendMode), timeA2B, timeB2A,
                                     doReverse, resetTargetWhenDone, depthCompare, depthUpdate,
                                     depthBackwards, active);
}

CEntity* ScriptLoader::LoadActorRotate(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 6)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f rotation(in);
  float scale = in.Get< float >();
  bool updateActors = in.Get< bool >();
  bool updateOnCreation = in.Get< bool >();
  bool active = in.Get< bool >();
  return rs_new CScriptActorRotate(mgr.AllocateUniqueId(), name, info, rotation, scale,
                                   updateActors, updateOnCreation, active);
}

CEntity* ScriptLoader::LoadSpecialFunction(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (propCount != 15)
    return nullptr;

  SActorHead head(in, mgr);
  const uint functionValue = in.Get< uint >();
  rstl::string str(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  int w2 = in.ReadLong();
  int w3 = in.ReadLong();
  CPlayerState::EItemType w4 = CPlayerState::EItemType(in.ReadLong());
  const bool active = in.ReadBool();
  float f4 = in.ReadFloat();
  int w5 = in.ReadLong() & 0xFFFF;
  int w6 = in.ReadLong() & 0xFFFF;
  int w7 = in.ReadLong() & 0xFFFF;

  const CScriptSpecialFunction::ESpecialFunction specialFunction =
      static_cast< CScriptSpecialFunction::ESpecialFunction >(functionValue);

  if (specialFunction == CScriptSpecialFunction::kSF_FogVolume)
    return nullptr;
  if (specialFunction == CScriptSpecialFunction::kSF_RadialDamage)
    return nullptr;

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), head.mName, info, head.mTransform, specialFunction, str, f1, f2,
      f3, f4, CVector3f::Zero(), CColor::Black(), active, CDamageInfo(), w2, w3, w4, w5, w6, w7);
}

CEntity* ScriptLoader::LoadFogVolume(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount != 7)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f center(in);
  CVector3f volume(in);
  float flickerSpeed = in.ReadFloat();
  float f2 = in.ReadFloat();
  CColor fogColor(in);
  bool active = in.ReadBool();

  volume[kDX] = CMath::AbsF(volume[kDX]) * 0.5f;
  volume[kDY] = CMath::AbsF(volume[kDY]) * 0.5f;
  volume[kDZ] = CMath::AbsF(volume[kDZ]) * 0.5f;

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), name, info,
      ConvertEditorEulerToTransform4f(CVector3f::Zero(), center),
      CScriptSpecialFunction::kSF_FogVolume, rstl::string_l(""), flickerSpeed, f2, 0.f, 0.f, volume,
      fogColor, active, CDamageInfo(), -1, -1, CPlayerState::kIT_Invalid,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId);
}

CEntity* ScriptLoader::LoadMapStation(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  return nullptr;
}

CEntity* ScriptLoader::LoadSpacePirate(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount < static_cast< int >(CSpacePirate::GetNumProperties()))
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Space Pirate");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (gpResourceFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) !=
      'ANCS') {
    return nullptr;
  }

  if (static_cast< int >(pInfo.GetAnimationParameters().GetCharacter()) == 0) {
    rstl::string msg = rstl::string_l("Space pirate <") + head.mActorHead.mName +
                       rstl::string_l("> has AnimationInformation property with invalid character "
                                      "selected.\n");
    pInfo.GetAnimationParameters().SetCharacter(2);
  }

  return rs_new CSpacePirate(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, in, propCount);
}

CEntity* ScriptLoader::LoadFlyingPirate(CStateManager& mgr, CInputStream& in, int propCount,
                                        const CEntityInfo& info) {
  if (propCount < static_cast< int >(CFlyingPirate::GetNumProperties()))
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "FlyingPirate");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (gpResourceFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) !=
      'ANCS') {
    return nullptr;
  }

  return rs_new CFlyingPirate(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, in, propCount);
}

CEntity* ScriptLoader::LoadCoverPoint(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount != 9)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  uint flags = in.Get< uint >();
  bool crouch = in.Get< bool >();
  float horizontalAngle = in.ReadFloat();
  float verticalAngle = in.ReadFloat();
  float coverTime = in.ReadFloat();
  return rs_new CScriptCoverPoint(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                                  active, flags, crouch, horizontalAngle, verticalAngle, coverTime);
}

CEntity* ScriptLoader::LoadAiJumpPoint(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 5)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  float apex = in.ReadFloat();
  return rs_new CScriptAiJumpPoint(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                                   active, apex);
}

CEntity* ScriptLoader::LoadSpiderBallWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info) {
  if (propCount != 5)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.ReadBool();
  uint w1 = in.Get< uint >();
  return rs_new CScriptSpiderBallWaypoint(mgr.AllocateUniqueId(), aHead.mName, info,
                                          aHead.mTransform, active, w1);
}

CEntity* ScriptLoader::LoadBloodFlower(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 18)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Blood Flower");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CAssetId partId1 = in.Get< CAssetId >();
  CAssetId wpsc1 = in.Get< CAssetId >();
  CAssetId wpsc2 = in.Get< CAssetId >();
  CDamageInfo dInfo1(in);
  CDamageInfo dInfo2(in);
  CDamageInfo dInfo3(in);
  CAssetId partId2 = in.Get< CAssetId >();
  CAssetId partId3 = in.Get< CAssetId >();
  CAssetId partId4 = in.Get< CAssetId >();
  float f1 = in.ReadFloat();
  CAssetId partId5 = in.Get< CAssetId >();
  uint soundId = in.ReadLong();

  const CAnimationParameters& anim = pInfo.GetAnimationParameters();
  FourCC animType = gpResourceFactory->GetResourceTypeById(anim.GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  return rs_new CBloodFlower(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(anim.GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                          head.mScale, pInfo.GetAnimationParameters().GetInitialAnimation(),
                          true)),
      pInfo, partId1, wpsc1, actParms, wpsc2, dInfo1, dInfo2, dInfo3, partId2, partId3, partId4, f1,
      partId5, soundId);
}

CEntity* ScriptLoader::LoadGrapplePoint(CStateManager& mgr, CInputStream& in, int propCount,
                                        const CEntityInfo& info) {
  if (propCount != 5)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f position(in);
  CVector3f orientation(in);
  bool active = in.Get< bool >();
  CGrappleParameters params = LoadGrappleParameters(in);
  if (params.GetLockSwingTurn()) {
    orientation.SetX(0.f);
    orientation.SetY(0.f);
  }
  return rs_new CScriptGrapplePoint(mgr.AllocateUniqueId(), name, info,
                                    ConvertEditorEulerToTransform4f(orientation, position), active,
                                    params);
}

CEntity* ScriptLoader::LoadFlickerBat(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount != 10)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  int flavor = in.Get< int >();
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "FlickerBat");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  bool collider = in.ReadBool();
  bool excludePlayer = in.ReadBool();
  bool enableLineOfSight = in.ReadBool();

  const CAnimationParameters& anim = pInfo.GetAnimationParameters();
  FourCC animType = gpResourceFactory->GetResourceTypeById(anim.GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  return rs_new CFlickerBat(
      mgr.AllocateUniqueId(), name, CPatterned::EFlavorType(flavor), info, xf,
      CModelData(CAnimRes(anim.GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, collider ? CPatterned::kCT_One : CPatterned::kCT_Zero, excludePlayer, actParms,
      enableLineOfSight);
}

CEntity* ScriptLoader::LoadChozoGhost(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount != 31)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Chozo Ghost");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  float hearingRadius = in.ReadFloat();
  float fadeOutDelay = in.ReadFloat();
  float attackDelay = in.ReadFloat();
  float freezeTime = in.ReadFloat();
  CAssetId wpsc1 = in.Get< uint >();
  CDamageInfo dInfo1(in);
  CAssetId wpsc2 = in.Get< uint >();
  CDamageInfo dInfo2(in);
  CChozoGhost::CBehaveChance behaveChance1(in);
  CChozoGhost::CBehaveChance behaveChance2(in);
  CChozoGhost::CBehaveChance behaveChance3(in);
  ushort soundImpact = CSfxManager::TranslateSFXID(in.Get< uint >());
  float f5 = in.ReadFloat();
  ushort sfxFadeIn = CSfxManager::TranslateSFXID(in.Get< uint >());
  ushort sfxFadeOut = CSfxManager::TranslateSFXID(in.Get< uint >());
  uint w1 = in.Get< uint >();
  float f6 = in.ReadFloat();
  uint w2 = in.Get< uint >();
  float hurlRecoverTime = in.ReadFloat();
  CAssetId projectileVisor = in.Get< uint >();
  ushort soundProjectileVisor = CSfxManager::TranslateSFXID(in.Get< uint >());
  float f8 = in.ReadFloat();
  float f9 = in.ReadFloat();
  uint nearChance = in.Get< uint >();
  uint midChance = in.Get< uint >();

  const CAnimationParameters& anim = pInfo.GetAnimationParameters();
  FourCC animType = gpResourceFactory->GetResourceTypeById(anim.GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  return rs_new CChozoGhost(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(anim.GetACSFile(), CAnimRes::kDefaultCharIdx, head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, hearingRadius, fadeOutDelay, attackDelay, freezeTime, wpsc1, dInfo1, wpsc2,
      dInfo2, behaveChance3, behaveChance2, behaveChance1, soundImpact, f5, sfxFadeIn, sfxFadeOut,
      w1, f6, w2, hurlRecoverTime, projectileVisor, soundProjectileVisor, f8, f9, nearChance,
      midChance);
}

CEntity* ScriptLoader::LoadPuddleSpore(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 16)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  const uint flavor = in.Get< uint >();
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Puddle Spore");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  bool b1 = in.ReadBool();
  CAssetId glowFx = in.ReadLong();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  CAssetId weapon = in.ReadLong();
  CDamageInfo dInfo(in);

  FourCC animType =
      gpResourceFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  return rs_new CPuddleSpore(
      mgr.AllocateUniqueId(), name, static_cast< CPatterned::EFlavorType >(flavor), info, xf,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, b1 ? CPatterned::kCT_One : CPatterned::kCT_Zero, glowFx, f1, f2, f3, f4, f5, actParms,
      weapon, dInfo);
}

CEntity* ScriptLoader::LoadSpiderBallAttractionSurface(CStateManager& mgr, CInputStream& in,
                                                       int propCount, const CEntityInfo& info) {
  if (propCount != 5)
    return nullptr;

  SScaledActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  return rs_new CScriptSpiderBallAttractionSurface(
      mgr.AllocateUniqueId(), aHead.mActorHead.mName, info, aHead.mActorHead.mTransform,
      aHead.mScale, active);
}

CEntity* ScriptLoader::LoadDebugCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                               const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  SActorHead aHead(in, mgr);
  uint w1 = in.Get< uint >();
  return rs_new CScriptDebugCameraWaypoint(mgr.AllocateUniqueId(), aHead.mName, info,
                                           aHead.mTransform, w1);
}

CEntity* ScriptLoader::LoadPuddleToadGamma(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (propCount != 17)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  const uint flavor = in.Get< uint >();
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount =
      CPatternedInfo::HasCorrectParameterCount(in, "Gamma Puddle Toad");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  float suckForceMultiplier = in.ReadFloat();
  float suckAngle = in.ReadFloat();
  float playerSuckRange = in.ReadFloat();
  CVector3f localShootDir(in);
  float playerShootSpeed = in.ReadFloat();
  float shouldAttackWaitTime = in.ReadFloat();
  float spotPlayerWaitTime = in.ReadFloat();
  CDamageInfo playerShootDamage(in);
  CDamageInfo dInfo2(in);
  uint dcln = in.ReadLong();

  return rs_new CPuddleToadGamma(
      mgr.AllocateUniqueId(), name, static_cast< CPatterned::EFlavorType >(flavor), info, xf,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, suckForceMultiplier, suckAngle, playerSuckRange, localShootDir,
      playerShootSpeed, shouldAttackWaitTime, spotPlayerWaitTime, playerShootDamage, dInfo2, dcln);
}

CEntity* ScriptLoader::LoadFireFlea(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 9)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "FireFlea");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  in.ReadBool();
  in.ReadBool();
  float f1 = in.ReadFloat();

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CFireFlea(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, f1);
}

CEntity* ScriptLoader::LoadSpankWeed(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount != 11)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "SpankWeed");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  in.ReadBool();
  float maxDetectionRange = in.ReadFloat();
  float maxHearingRange = in.ReadFloat();
  float maxSightRange = in.ReadFloat();
  float hideTime = in.ReadFloat();

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CSpankWeed(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, maxDetectionRange, maxHearingRange, maxSightRange, hideTime);
}

CEntity* ScriptLoader::LoadDistanceFog(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 8)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  uint mode = in.Get< uint >();
  CColor color(in);
  CVector2f range(in);
  float colorDelta = in.Get< float >();
  CVector2f rangeDelta(in);
  bool expl = in.Get< bool >();
  bool active = in.Get< bool >();

  ERglFogMode fogMode;
  switch (mode) {
  case 0:
    fogMode = kRFM_None;
    break;
  case 1:
    fogMode = kRFM_PerspLin;
    break;
  case 2:
    fogMode = kRFM_PerspExp;
    break;
  case 3:
    fogMode = kRFM_PerspExp2;
    break;
  case 4:
    fogMode = kRFM_PerspRevExp;
    break;
  case 5:
    fogMode = kRFM_PerspRevExp2;
    break;
  default:
    return nullptr;
  }

  return rs_new CScriptDistanceFog(mgr.AllocateUniqueId(), name, info, fogMode, color, range,
                                   colorDelta, rangeDelta, expl, active, 0.f, 0.f, 0.f, 0.f);
}

CEntity* ScriptLoader::LoadAreaAttributes(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (propCount != 9)
    return nullptr;

  int load = in.ReadLong();
  if (load != 1)
    return nullptr;

  bool showSkybox = in.ReadBool();
  EEnvFxType fxType = EEnvFxType(in.ReadLong());
  float envFxDensity = in.ReadFloat();
  float thermalHeat = in.ReadFloat();
  float xrayFogDistance = in.ReadFloat();
  float worldLightingLevel = in.ReadFloat();
  CAssetId skybox = in.Get< CAssetId >();
  EPhazonType phazonType = EPhazonType(in.Get< int >());

  return rs_new CScriptAreaAttributes(mgr.AllocateUniqueId(), info, showSkybox, fxType,
                                      envFxDensity, thermalHeat, xrayFogDistance,
                                      worldLightingLevel, skybox, phazonType);
}

CEntity* ScriptLoader::LoadDockAreaChange(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (propCount != 3)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  int dock = in.Get< int >();
  bool active = in.Get< bool >();
  return rs_new CScriptDockAreaChange(mgr.AllocateUniqueId(), name, info, dock, active);
}

CEntity* ScriptLoader::LoadMetaree(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount != 12)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Metaree");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters aParms = LoadActorParameters(in);
  CDamageInfo dInfo(in);
  float f1 = in.ReadFloat();
  CVector3f vec(in);
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();

  FourCC animType =
      gpResourceFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  float f4 = in.ReadFloat();

  return rs_new CMetaree(
      mgr.AllocateUniqueId(), name, CPatterned::kFT_Zero, info, xf,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, dInfo, f1, vec, f2, kBT_Invalid, f3, f4, aParms);
}

CEntity* ScriptLoader::LoadNewIntroBoss(CStateManager& mgr, CInputStream& in, int propCount,
                                        const CEntityInfo& info) {
  if (propCount != 13)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "NewIntroBoss");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  float minTurnAngle = in.ReadFloat();
  CAssetId projectile = in.ReadLong();
  CDamageInfo dInfo(in);
  CAssetId beamContactFxId = in.ReadLong();
  CAssetId beamPulseFxId = in.ReadLong();
  CAssetId beamTextureId = in.ReadLong();
  CAssetId beamGlowTextureId = in.ReadLong();

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CNewIntroBoss(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, minTurnAngle, projectile, dInfo, beamContactFxId, beamPulseFxId,
      beamTextureId, beamGlowTextureId);
}

CEntity* ScriptLoader::LoadFlaahgra(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount < CFlaahgraData::GetNumProperties())
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Flaahgra");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CFlaahgraData flaahgraData(in, propCount);

  const CVector3f& scale = head.mScale;
  CAnimRes animRes(pInfo.GetAnimationParameters().GetACSFile(),
                   pInfo.GetAnimationParameters().GetCharacter(), scale,
                   pInfo.GetAnimationParameters().GetInitialAnimation(), true);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId) {
    const CAnimationParameters& anim = flaahgraData.GetAnimationParameters();
    animRes =
        CAnimRes(anim.GetACSFile(), anim.GetCharacter(), scale, anim.GetInitialAnimation(), true);
  }

  if (animRes.GetId() == kInvalidAssetId)
    return nullptr;

  return rs_new CFlaahgra(mgr.AllocateUniqueId(), head.mActorHead.mName, info,
                          head.mActorHead.mTransform, animRes, pInfo, actParms, flaahgraData);
}

CEntity* ScriptLoader::LoadParasite(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 25)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  const uint flavor = in.Get< uint >();
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Parasite");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

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

  const CAnimationParameters& anim = pInfo.GetAnimationParameters();
  if (gpResourceFactory->GetResourceTypeById(anim.GetACSFile()) != 'ANCS') {
    return nullptr;
  }

  return rs_new CParasite(
      mgr.AllocateUniqueId(), name, static_cast< CPatterned::EFlavorType >(flavor), info, xf,
      CModelData(CAnimRes(anim.GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, kBT_WallWalker, maxTelegraphReactDist, advanceWpRadius, f3, alignAngVel, f5,
      stuckTimeThreshold, collisionCloseMargin, parasiteSearchRadius, parasiteSeparationDist,
      parasiteSeparationWeight, parasiteAlignmentWeight, parasiteCohesionWeight,
      destinationSeekWeight, forwardMoveWeight, playerSeparationDist, playerSeparationWeight,
      playerObstructionMinDist, 0.f, disableMove, CWallWalker::kWT_Parasite,
      CDamageVulnerability::NormalVulnerability(), CDamageInfo(CWeaponMode::Power(), 0.f, 0.f, 0.f),
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId, kInvalidAssetId, kInvalidAssetId, 0.f, actParms);
}

CEntity* ScriptLoader::LoadRipper(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != static_cast< int >(CRipper::GetNumProperties()))
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType type = CPatterned::EFlavorType(in.Get< int >());
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Ripper");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CGrappleParameters grappleParms = LoadGrappleParameters(in);

  return rs_new CRipper(
      mgr.AllocateUniqueId(), name, type, info, xf,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, grappleParms);
}

CEntity* ScriptLoader::LoadPickupGenerator(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f pos(in);
  bool active = in.Get< bool >();
  float frequency = in.Get< float >();
  return rs_new CScriptPickupGenerator(mgr.AllocateUniqueId(), name, info, pos, frequency, active);
}

CEntity* ScriptLoader::LoadMetroid(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount < static_cast< int >(CMetroidData::GetNumProperties()))
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.Get< int >());
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Metroid");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CMetroidData metData(in, propCount);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  if (flavor == CPatterned::kFT_One)
    return nullptr;

  return rs_new CMetroid(
      mgr.AllocateUniqueId(), name, flavor, info, xf,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, metData, kInvalidUniqueId);
}

CEntity* ScriptLoader::LoadMetroidBeta(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount < CMetroidBetaData::GetNumProperties())
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Metroid");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CMetroidBetaData metData(in, propCount);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CMetroidBeta(
      mgr.AllocateUniqueId(), name, info, xf,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, metData);
}

CEntity* ScriptLoader::LoadBallTrigger(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 9)
    return nullptr;

  rstl::string name(in);
  CVector3f pos(in);
  CVector3f scale(in);
  bool b1 = in.Get< bool >();
  float f1 = in.Get< float >();
  float f2 = in.Get< float >();
  float f3 = in.Get< float >();
  CVector3f vec = in.Get< CVector3f >();
  bool b2 = in.Get< bool >();
  return rs_new CScriptBallTrigger(mgr.AllocateUniqueId(), name, info, pos, scale, b1, f1, f2, f3,
                                   vec, b2);
}

CEntity* ScriptLoader::LoadTargetingPoint(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  return rs_new CScriptTargetingPoint(mgr.AllocateUniqueId(), aHead.mName, info,
                                      aHead.mTransform, active);
}

CEntity* ScriptLoader::LoadEMPulse(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount != 12)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  float f1 = in.Get< float >();
  float f2 = in.Get< float >();
  float f3 = in.Get< float >();
  float f4 = in.Get< float >();
  float f5 = in.Get< float >();
  float f6 = in.Get< float >();
  float f7 = in.Get< float >();
  CAssetId particleId = in.Get< CAssetId >();

  return rs_new CScriptEMPulse(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                               active, f1, f2, f3, f4, f5, f6, f7, particleId);
}

CEntity* ScriptLoader::LoadIceSheegoth(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount < CIceSheegothData::GetNumProperties())
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Ice Sheegoth");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CIceSheegothData sheegothData(in, propCount);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CIceSheegoth(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, sheegothData);
}

CEntity* ScriptLoader::LoadPlayerActor(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 19)
    return nullptr;

  SScaledActorHead head(in, mgr);
  CVector3f extents(in);
  CVector3f offset(in);
  float mass = in.ReadFloat();
  float zMomentum = in.ReadFloat();
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  in.ReadLong();
  CAnimationParameters aParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  bool loop = in.Get< bool >();
  bool immovable = in.Get< bool >();
  bool solid = in.Get< bool >();
  bool active = in.Get< bool >();
  uint flags = LoadParameterFlags(in);
  CPlayerState::EBeamId beam = CPlayerState::EBeamId(in.ReadLong() - 1);

  FourCC animType = gpResourceFactory->GetResourceTypeById(aParms.GetACSFile());
  if (animType == 0)
    return nullptr;

  CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), extents, offset);

  CMaterialList list;
  if (immovable)
    list.Add(kMT_Immovable);
  if (solid)
    list.Add(kMT_Solid);

  bool hasNegativeExtent;
  if (extents.GetX() < 0.f)
    hasNegativeExtent = true;
  else if (extents.GetY() < 0.f)
    hasNegativeExtent = true;
  else if (extents.GetZ() < 0.f)
    hasNegativeExtent = true;
  else
    hasNegativeExtent = false;

  if (animType == 'ANCS') {
    CAnimRes animRes(aParms.GetACSFile(), 4, head.mScale, aParms.GetInitialAnimation(), loop);

    const bool useDefaultBox = extents == CVector3f::Zero() || hasNegativeExtent;
    if (useDefaultBox)
      aabb = CAABox(CVector3f(-0.5f, -0.5f, -0.5f), CVector3f(0.5f, 0.5f, 0.5f));

    return rs_new CScriptPlayerActor(mgr.AllocateUniqueId(), head.mActorHead.mName, info,
                                     head.mActorHead.mTransform, animRes,
                                     CModelData::CModelDataNull(), aabb, true, list, mass,
                                     zMomentum, hInfo, dVuln, actParms, loop, active, flags, beam);
  }

  return nullptr;
}

CEntity* ScriptLoader::LoadRoomAcoustics(CStateManager& mgr, CInputStream& in, int propCount,
                                         const CEntityInfo& info) {
  if (propCount != 32)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  uint volScale = in.Get< uint >();
  bool revHi = in.Get< bool >();
  bool revHiDis = in.Get< bool >();
  float revHiColoration = in.Get< float >();
  float revHiMix = in.Get< float >();
  float revHiTime = in.Get< float >();
  float revHiDamping = in.Get< float >();
  float revHiPreDelay = in.Get< float >();
  float revHiCrosstalk = in.Get< float >();
  bool chorus = in.Get< bool >();
  float baseDelay = in.Get< float >();
  float variation = in.Get< float >();
  float period = in.Get< float >();
  bool revStd = in.Get< bool >();
  bool revStdDis = in.Get< bool >();
  float revStdColoration = in.Get< float >();
  float revStdMix = in.Get< float >();
  float revStdTime = in.Get< float >();
  float revStdDamping = in.Get< float >();
  float revStdPreDelay = in.Get< float >();
  bool delay = in.Get< bool >();
  uint delayL = in.Get< uint >();
  uint delayR = in.Get< uint >();
  uint delayS = in.Get< uint >();
  uint feedbackL = in.Get< uint >();
  uint feedbackR = in.Get< uint >();
  uint feedbackS = in.Get< uint >();
  uint outputL = in.Get< uint >();
  uint outputR = in.Get< uint >();
  uint outputS = in.Get< uint >();

  return rs_new CScriptRoomAcoustics(
      mgr.AllocateUniqueId(), name, info, active, volScale, revHi, revHiDis, revHiColoration,
      revHiMix, revHiTime, revHiDamping, revHiPreDelay, revHiCrosstalk, chorus, baseDelay,
      variation, period, revStd, revStdDis, revStdColoration, revStdMix, revStdTime, revStdDamping,
      revStdPreDelay, delay, delayL, delayR, delayS, feedbackL, feedbackR, feedbackS, outputL,
      outputR, outputS);
}

CEntity* ScriptLoader::LoadFishCloud(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount != 36)
    return nullptr;

  SScaledActorHead head(in, mgr);
  bool active = in.Get< bool >();
  CAssetId model = in.Get< CAssetId >();
  CAssetId ancsFile = in.Get< CAssetId >();
  int charIdx = in.Get< int >();
  int defaultAnim = in.Get< int >();
  int numBoids = CCast::ToInt32(in.ReadFloat());
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
  int updateShift = in.ReadLong();

  if (gpResourceFactory->GetResourceTypeById(model) == 0) {
    return nullptr;
  }

  CColor color(in);
  bool killable = in.ReadBool();
  float weaponKillRadius = in.ReadFloat();
  CAssetId part1 = in.ReadLong();
  int partCount1 = in.ReadLong();
  CAssetId part2 = in.ReadLong();
  int partCount2 = in.ReadLong();
  CAssetId part3 = in.ReadLong();
  int partCount3 = in.ReadLong();
  CAssetId part4 = in.ReadLong();
  int partCount4 = in.ReadLong();
  int deathSfx = in.ReadLong();
  bool repelFromThreats = in.ReadBool();
  bool hotInThermal = in.ReadBool();

  return rs_new CFishCloud(
      mgr.AllocateUniqueId(), active, head.mActorHead.mName, info, head.mScale,
      head.mActorHead.mTransform, CModelData(CStaticRes(model, CVector3f(1.f, 1.f, 1.f))),
      CAnimRes(ancsFile, charIdx, CVector3f(1.f, 1.f, 1.f), defaultAnim, true), numBoids, speed,
      separationRadius, cohesionMagnitude, alignmentWeight, separationMagnitude,
      weaponRepelMagnitude, playerRepelMagnitude, containmentMagnitude, scatterVel, maxScatterAngle,
      weaponRepelDampingSpeed, playerRepelDampingSpeed, containmentRadius, updateShift, color,
      killable, weaponKillRadius, part1, partCount1, part2, partCount2, part3, partCount3, part4,
      partCount4, deathSfx, repelFromThreats, hotInThermal);
}

CEntity* ScriptLoader::LoadFishCloudModifier(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (!(propCount >= 6 && propCount <= 7 ? true : false))
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f pos(in);
  bool active = in.Get< bool >();
  bool isRepulsor = in.Get< bool >();
  bool swirl = propCount > 6 ? in.Get< bool >() : false;
  float radius = in.ReadFloat();
  float priority = in.ReadFloat();

  return rs_new CFishCloudModifier(mgr.AllocateUniqueId(), active, name, info, pos, isRepulsor,
                                   swirl, radius, priority);
}

static rstl::optional_object< CVisorFlare::CFlareDef > LoadFlareDef(CInputStream& in) {
  int propCount = in.ReadLong();
  if (propCount == 4) {
    CAssetId texId = in.ReadLong();
    float pos = in.ReadFloat();
    float scale = in.ReadFloat();
    CColor color(in);
    if (texId != kInvalidAssetId) {
      TToken< CTexture > tex = gpSimplePool->GetObj(SObjectTag('TXTR', texId));
      tex.Lock();
      return CVisorFlare::CFlareDef(tex, pos, scale, color.GetColor_u32());
    }
  }
  return rstl::optional_object_null();
}

CEntity* ScriptLoader::LoadDrone(CStateManager& mgr, CInputStream& in, int propCount,
                                 const CEntityInfo& info) {
  if (propCount != 45)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.ReadLong());
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);
  float f1 = in.ReadFloat();

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Drone");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CDamageInfo dInfo1(in);
  in.ReadLong();
  CDamageInfo dInfo2(in);
  CAssetId aId1 = in.ReadLong();
  in.ReadLong();
  CAssetId aId2 = in.ReadLong();

  rstl::optional_object< CVisorFlare::CFlareDef > def1 = LoadFlareDef(in);
  rstl::optional_object< CVisorFlare::CFlareDef > def2 = LoadFlareDef(in);
  rstl::optional_object< CVisorFlare::CFlareDef > def3 = LoadFlareDef(in);
  rstl::optional_object< CVisorFlare::CFlareDef > def4 = LoadFlareDef(in);
  rstl::optional_object< CVisorFlare::CFlareDef > def5 = LoadFlareDef(in);

  rstl::vector< CVisorFlare::CFlareDef > flares;
  flares.reserve(5);
  if (def1)
    flares.push_back(*def1);
  if (def2)
    flares.push_back(*def2);
  if (def3)
    flares.push_back(*def3);
  if (def4)
    flares.push_back(*def4);
  if (def5)
    flares.push_back(*def5);

  if (gpResourceFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile()) != 'ANCS')
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
  CAssetId crscId = in.ReadLong();
  float f21 = in.ReadFloat();
  float f22 = in.ReadFloat();
  float f24 = in.ReadFloat();
  float f23 = in.ReadFloat();
  int soundId = in.ReadLong();
  bool b1 = in.ReadBool();

  return rs_new CDrone(
      mgr.AllocateUniqueId(), name, flavor, info, xf, f1,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, CPatterned::kMT_Flyer, CPatterned::kCT_One, kBT_Pitchable, dInfo1, aId1,
      dInfo2, aId2, flares, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17,
      f18, f19, f20, crscId, f21, f22, f23, f24, soundId, b1);
}

CEntity* ScriptLoader::LoadVisorFlare(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount != 14)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f pos(in);
  bool active = in.Get< bool >();
  CVisorFlare::EBlendMode blendMode = CVisorFlare::EBlendMode(in.Get< uint >());
  bool b1 = in.Get< bool >();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  uint w1 = in.Get< uint >();

  rstl::optional_object< CVisorFlare::CFlareDef > def1 = LoadFlareDef(in);
  rstl::optional_object< CVisorFlare::CFlareDef > def2 = LoadFlareDef(in);
  rstl::optional_object< CVisorFlare::CFlareDef > def3 = LoadFlareDef(in);
  rstl::optional_object< CVisorFlare::CFlareDef > def4 = LoadFlareDef(in);
  rstl::optional_object< CVisorFlare::CFlareDef > def5 = LoadFlareDef(in);

  rstl::vector< CVisorFlare::CFlareDef > flares;
  flares.reserve(5);
  if (def1)
    flares.push_back(*def1);
  if (def2)
    flares.push_back(*def2);
  if (def3)
    flares.push_back(*def3);
  if (def4)
    flares.push_back(*def4);
  if (def5)
    flares.push_back(*def5);

  return rs_new CScriptVisorFlare(mgr.AllocateUniqueId(), name, info, active, pos, blendMode, b1,
                                  f1, f2, f3, 2, w1, flares);
}

CEntity* ScriptLoader::LoadWorldTeleporter(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (!(propCount >= 4 && propCount <= 21 ? true : false))
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  CAssetId worldId = in.Get< CAssetId >();
  CAssetId areaId = in.Get< CAssetId >();

  if (propCount == 4)
    return rs_new CScriptWorldTeleporter(mgr.AllocateUniqueId(), name, info, active, worldId,
                                         areaId);

  CAnimationParameters animParms = LoadAnimationParameters(in);
  CVector3f playerScale(in);
  CAssetId platformModel = in.Get< CAssetId >();
  CVector3f platformScale(in);
  CAssetId backgroundModel = in.Get< CAssetId >();
  CVector3f backgroundScale(in);
  bool upElevator = in.ReadBool();

  ushort elevatorSound =
      propCount >= 12 ? static_cast< ushort >(in.ReadLong()) : CSfxManager::kInternalInvalidSfxId;
  uchar volume = propCount >= 13 ? static_cast< uchar >(in.ReadLong()) : uchar(127);
  uchar panning = propCount >= 14 ? static_cast< uchar >(in.ReadLong()) : uchar(64);
  bool showText = propCount >= 15 ? in.ReadBool() : false;
  CAssetId fontId = propCount >= 16 ? static_cast< CAssetId >(in.ReadLong()) : kInvalidAssetId;
  CAssetId stringId = propCount >= 17 ? static_cast< CAssetId >(in.ReadLong()) : kInvalidAssetId;
  bool fadeWhite = propCount >= 18 ? in.ReadBool() : false;

  float charFadeInTime;
  if (propCount >= 19) {
    charFadeInTime = in.Get< float >();
  } else {
    charFadeInTime = 0.1f;
  }

  float charsPerSecond;
  if (propCount >= 20) {
    charsPerSecond = in.Get< float >();
  } else {
    charsPerSecond = 16.f;
  }

  float showDelay;
  if (propCount >= 21) {
    showDelay = in.Get< float >();
  } else {
    showDelay = 0.f;
  }

  if (showText)
    return rs_new CScriptWorldTeleporter(mgr.AllocateUniqueId(), name, info, active, worldId,
                                         areaId, elevatorSound, volume, panning, fontId, stringId,
                                         fadeWhite, charFadeInTime, charsPerSecond, showDelay);

  return rs_new CScriptWorldTeleporter(
      mgr.AllocateUniqueId(), name, info, active, worldId, areaId, animParms.GetACSFile(),
      animParms.GetInitialAnimation(), animParms.GetCharacter(), playerScale, platformModel,
      platformScale, backgroundModel, backgroundScale, upElevator, elevatorSound, volume, panning);
}

CEntity* ScriptLoader::LoadControllerAction(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (!(propCount >= 4 && propCount <= 6 ? true : false))
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  int w1 = in.Get< int >();
  bool b1 = propCount >= 6 ? in.Get< bool >() : false;
  uint w2 = propCount >= 6 ? in.Get< uint >() : 0;
  bool b2 = in.Get< bool >();

  return rs_new CScriptControllerAction(mgr.AllocateUniqueId(), name, info, active,
                                        static_cast< CScriptControllerAction::ECommands >(w1), b1,
                                        w2, b2);
}

CEntity* ScriptLoader::LoadSwitch(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  bool b2 = in.Get< bool >();
  bool b3 = in.Get< bool >();
  return rs_new CScriptSwitch(mgr.AllocateUniqueId(), name, info, active, b2, b3);
}

CEntity* ScriptLoader::LoadPlayerStateChange(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (propCount != 7)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  int itemType = in.Get< int >();
  int itemCount = in.Get< int >();
  int itemCapacity = in.Get< int >();
  int ctrl = in.Get< int >();
  int ctrlCmdOpt = in.Get< int >();
  return rs_new CScriptPlayerStateChange(
      mgr.AllocateUniqueId(), name, info, active, itemType, itemCount, itemCapacity,
      CScriptPlayerStateChange::EControl(ctrl),
      CScriptPlayerStateChange::EControlCommandOption(ctrlCmdOpt));
}

CEntity* ScriptLoader::LoadSaveStation(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  return nullptr;
}

CEntity* ScriptLoader::LoadVisorGoo(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 11)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CTransform4f xf = CTransform4f::Translate(CVector3f(in));
  CAssetId particle = in.Get< CAssetId >();
  CAssetId electric = in.Get< CAssetId >();
  float minDist = in.Get< float >();
  float maxDist = in.Get< float >();
  float nearProb = in.Get< float >();
  float farProb = in.Get< float >();
  CColor color(in);
  int sfx = in.Get< int >();
  bool forceShow = in.Get< bool >();

  if (particle == kInvalidAssetId && electric == kInvalidAssetId)
    return nullptr;
  return rs_new CScriptVisorGoo(mgr.AllocateUniqueId(), name, info, xf, particle, electric, minDist,
                                maxDist, nearProb, farProb, color, sfx, forceShow, false);
}

CEntity* ScriptLoader::LoadJellyZap(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 20)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "JellyZap");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CDamageInfo dInfo(in);
  float attackRadius = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float attackDelay = in.ReadFloat();
  float f6 = in.ReadFloat();
  float f7 = in.ReadFloat();
  float f8 = in.ReadFloat();
  float priority = in.ReadFloat();
  float repulseRadius = in.ReadFloat();
  float attractRadius = in.ReadFloat();
  float f12 = in.ReadFloat();
  bool b1 = in.Get< bool >();

  const CAnimationParameters& anim = pInfo.GetAnimationParameters();
  FourCC animType = gpResourceFactory->GetResourceTypeById(anim.GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  return rs_new CJellyZap(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(anim.GetACSFile(), pInfo.GetAnimationParameters().GetCharacter(),
                          head.mScale, pInfo.GetAnimationParameters().GetInitialAnimation(),
                          true)),
      dInfo, b1, attackRadius, f2, f3, f4, attackDelay, f6, f7, f8, priority, repulseRadius,
      attractRadius, f12, pInfo, actParms);
}

CEntity* ScriptLoader::LoadThardus(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount < 0x2b)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Thardus");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  in.ReadBool();
  in.ReadBool();

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CAssetId phazonModel1 = in.ReadLong();
  CAssetId phazonModel2 = in.ReadLong();
  CAssetId phazonModel3 = in.ReadLong();
  CAssetId phazonModel4 = in.ReadLong();
  CAssetId phazonModel5 = in.ReadLong();
  CAssetId phazonModel6 = in.ReadLong();
  CAssetId phazonModel7 = in.ReadLong();
  CAssetId rockModel1 = in.ReadLong();
  CAssetId rockModel2 = in.ReadLong();
  CAssetId rockModel3 = in.ReadLong();
  CAssetId rockModel4 = in.ReadLong();
  CAssetId rockModel5 = in.ReadLong();
  CAssetId rockModel6 = in.ReadLong();
  CAssetId rockModel7 = in.ReadLong();

  CAssetId particle1 = in.ReadLong();
  CAssetId particle2 = in.ReadLong();
  CAssetId particle3 = in.ReadLong();
  CAssetId stateMachine = in.ReadLong();
  CAssetId particle4 = in.ReadLong();
  CAssetId particle5 = in.ReadLong();
  CAssetId particle6 = in.ReadLong();
  CAssetId particle7 = in.ReadLong();
  CAssetId particle8 = in.ReadLong();
  CAssetId particle9 = in.ReadLong();

  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();

  CAssetId texture = in.ReadLong();
  uint sfxID1 = in.ReadLong();
  CAssetId particle10 = (propCount > 0x2b) ? in.Get< CAssetId >() : kInvalidAssetId;
  uint sfxID2 = in.ReadLong();
  uint sfxID3 = in.ReadLong();
  uint sfxID4 = in.ReadLong();

  rstl::vector< CModelData > phazonModels;
  rstl::vector< CModelData > rockModels;
  rockModels.reserve(7);
  phazonModels.reserve(7);

  rockModels.push_back(CModelData(CStaticRes(rockModel1, CVector3f(1.f, 1.f, 1.f))));
  rockModels.push_back(CModelData(CStaticRes(rockModel2, CVector3f(1.f, 1.f, 1.f))));
  rockModels.push_back(CModelData(CStaticRes(rockModel3, CVector3f(1.f, 1.f, 1.f))));
  rockModels.push_back(CModelData(CStaticRes(rockModel4, CVector3f(1.f, 1.f, 1.f))));
  rockModels.push_back(CModelData(CStaticRes(rockModel5, CVector3f(1.f, 1.f, 1.f))));
  rockModels.push_back(CModelData(CStaticRes(rockModel6, CVector3f(1.f, 1.f, 1.f))));
  rockModels.push_back(CModelData(CStaticRes(rockModel7, CVector3f(1.f, 1.f, 1.f))));

  phazonModels.push_back(CModelData(CStaticRes(phazonModel1, CVector3f(1.f, 1.f, 1.f))));
  phazonModels.push_back(CModelData(CStaticRes(phazonModel2, CVector3f(1.f, 1.f, 1.f))));
  phazonModels.push_back(CModelData(CStaticRes(phazonModel3, CVector3f(1.f, 1.f, 1.f))));
  phazonModels.push_back(CModelData(CStaticRes(phazonModel4, CVector3f(1.f, 1.f, 1.f))));
  phazonModels.push_back(CModelData(CStaticRes(phazonModel5, CVector3f(1.f, 1.f, 1.f))));
  phazonModels.push_back(CModelData(CStaticRes(phazonModel6, CVector3f(1.f, 1.f, 1.f))));
  phazonModels.push_back(CModelData(CStaticRes(phazonModel7, CVector3f(1.f, 1.f, 1.f))));

  return rs_new CThardus(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), CAnimRes::kDefaultCharIdx,
                          head.mScale, pInfo.GetAnimationParameters().GetInitialAnimation(),
                          true)),
      actParms, pInfo, phazonModels, rockModels, particle1, particle2, particle3, f1, f2, f3, f4,
      f5, f6, stateMachine, particle4, particle5, particle6, particle7, particle8, particle9,
      texture, sfxID1, particle10, sfxID2, sfxID3, sfxID4);
}

CEntity* ScriptLoader::LoadWallCrawlerSwarm(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (propCount != 0x27)
    return nullptr;

  SScaledActorHead head(in, mgr);
  bool active = in.ReadBool();
  CActorParameters actParms = LoadActorParameters(in);
  uint flavor = in.Get< int >();
  uint actor = in.ReadLong();
  uint charIdx = in.ReadLong();
  uint defaultAnim = in.ReadLong();
  uint launchAnim = in.ReadLong();
  uint attractAnim = in.ReadLong();
  uint part1 = in.ReadLong();
  uint part2 = in.ReadLong();
  uint part3 = in.ReadLong();
  uint part4 = in.ReadLong();
  CDamageInfo crabDamage(in);
  float crabDamageCooldown = in.ReadFloat();
  CDamageInfo scarabExplodeDamage(in);
  float boidRadius = in.ReadFloat();
  float touchRadius = in.ReadFloat();
  float playerTouchRadius = in.ReadFloat();
  float animPlaybackSpeed = in.ReadFloat();
  int numBoids = in.ReadLong();
  int maxCreatedBoids = in.ReadLong();
  float separationRadius = in.ReadFloat();
  float cohesionMagnitude = in.ReadFloat();
  float alignmentWeight = in.ReadFloat();
  float separationMagnitude = in.ReadFloat();
  float moveToWaypointWeight = in.ReadFloat();
  float attractionMagnitude = in.ReadFloat();
  float attractionRadius = in.ReadFloat();
  float boidGenRate = in.ReadFloat();
  int maxLaunches = in.ReadLong();
  float scarabBoxMargin = in.ReadFloat();
  float scarabScatterXYVelocity = in.ReadFloat();
  float scarabTimeToExplode = in.ReadFloat();
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  int launchSfx = in.ReadLong();
  int scatterSfx = in.ReadLong();

  return rs_new CWallCrawlerSwarm(
      mgr.AllocateUniqueId(), active, head.mActorHead.mName, info, head.mScale,
      head.mActorHead.mTransform, flavor,
      CAnimRes(actor, charIdx, CVector3f(1.5f, 1.5f, 1.5f), defaultAnim, true), launchAnim,
      attractAnim, part1, part2, part3, part4, crabDamage, scarabExplodeDamage, crabDamageCooldown,
      boidRadius, touchRadius, playerTouchRadius, numBoids, maxCreatedBoids, animPlaybackSpeed,
      separationRadius, cohesionMagnitude, alignmentWeight, separationMagnitude,
      moveToWaypointWeight, attractionMagnitude, attractionRadius, boidGenRate, maxLaunches,
      scarabBoxMargin, scarabScatterXYVelocity, scarabTimeToExplode, hInfo, dVuln, launchSfx,
      scatterSfx, actParms);
}

CEntity* ScriptLoader::LoadFlaahgraTentacle(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (propCount != 6)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount =
      CPatternedInfo::HasCorrectParameterCount(in, "Flaahgra Tenticle");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CFlaahgraTentacle(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms);
}

CEntity* ScriptLoader::LoadThardusRockProjectile(CStateManager& mgr, CInputStream& in,
                                                 int propCount, const CEntityInfo& info) {
  if (propCount != 11)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount =
      CPatternedInfo::HasCorrectParameterCount(in, "Thardus Rock Projectile");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  in.ReadBool();
  in.ReadBool();
  float f1 = in.ReadFloat();
  CAssetId modelId = in.ReadLong();
  CAssetId stateMachine = in.ReadLong();

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  rstl::vector< CModelData > mDataVec;
  mDataVec.reserve(3);
  mDataVec.push_back(CModelData(CStaticRes(modelId, CVector3f(1.f, 1.f, 1.f))));

  return rs_new CThardusRockProjectile(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(), CAnimRes::kDefaultCharIdx,
                          head.mScale, pInfo.GetAnimationParameters().GetInitialAnimation(),
                          true)),
      actParms, pInfo, mDataVec, stateMachine, f1);
}

CEntity* ScriptLoader::LoadMidi(CStateManager& mgr, CInputStream& in, int propCount,
                                const CEntityInfo& info) {
  if (propCount != 6)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  uint csng = in.Get< uint >();
  float fadeIn = in.Get< float >();
  float fadeOut = in.Get< float >();
  uint vol = in.Get< uint >();
  return rs_new CScriptMidi(mgr.AllocateUniqueId(), info, name, active, csng, fadeIn, fadeOut, vol);
}

CEntity* ScriptLoader::LoadStreamedMusic(CStateManager& mgr, CInputStream& in, int propCount,
                                         const CEntityInfo& info) {
  if (propCount != 9)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  rstl::string fileName(in);
  bool noStopOnDeactivate = in.Get< bool >();
  float fadeIn = in.Get< float >();
  float fadeOut = in.Get< float >();
  uint volume = in.Get< uint >();
  bool loop = in.Get< uint >() == 0;
  bool music = in.Get< bool >();

  return rs_new CScriptStreamedMusic(mgr.AllocateUniqueId(), info, name, active, fileName,
                                     noStopOnDeactivate, fadeIn, fadeOut, volume, loop, music);
}

CEntity* ScriptLoader::LoadRepulsor(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f center(in);
  bool active = in.Get< bool >();
  float radius = in.Get< float >();
  return rs_new CRepulsor(mgr.AllocateUniqueId(), active, name, info, center, radius);
}

CEntity* ScriptLoader::LoadGunTurret(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount < CScriptGunTurretData::GetMinProperties())
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  int comp = in.ReadLong();
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);
  CVector3f collisionExtent(in);
  CVector3f collisionOffset(in);
  CAnimationParameters aParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  CScriptGunTurretData turretData(in, propCount);

  FourCC animType = gpResourceFactory->GetResourceTypeById(aParms.GetACSFile());
  if (animType == 0 || animType != 'ANCS')
    return nullptr;

  CModelData mData(CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), scale,
                            aParms.GetInitialAnimation(), true));
  const CTransform4f& orientedXf = xf;
  CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, collisionOffset);

  bool negativeExtent;
  if (collisionExtent.GetX() < 0.f)
    negativeExtent = true;
  else if (collisionExtent.GetY() < 0.f)
    negativeExtent = true;
  else if (collisionExtent.GetZ() < 0.f)
    negativeExtent = true;
  else
    negativeExtent = false;

  if (collisionExtent == CVector3f::Zero() || negativeExtent)
    aabb = mData.GetBounds(orientedXf.GetRotation());

  return rs_new CScriptGunTurret(mgr.AllocateUniqueId(), name,
                                 CScriptGunTurret::ETurretComponent(comp), info, orientedXf, mData,
                                 aabb, hInfo, dVuln, actParms, turretData);
}

CEntity* ScriptLoader::LoadBabygoth(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount < CBabygothData::GetNumProperties())
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Babygoth");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CBabygothData babyData(in, propCount);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CBabygoth(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, babyData);
}

CEntity* ScriptLoader::LoadEyeBall(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount != 21)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  int flavor = in.Get< int >();
  CTransform4f xf = LoadEditorTransform(in);
  CVector3f scale(in);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Eyeball");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  float attackDelay = in.ReadFloat();
  float attackStartTime = in.ReadFloat();
  CAssetId wpscId = in.ReadLong();
  CDamageInfo dInfo(in);
  CAssetId beamContactFxId = in.ReadLong();
  CAssetId beamPulseFxId = in.ReadLong();
  CAssetId beamTextureId = in.ReadLong();
  CAssetId beamGlowTextureId = in.ReadLong();
  uint anim0 = in.ReadLong();
  uint anim1 = in.ReadLong();
  uint anim2 = in.ReadLong();
  uint anim3 = in.ReadLong();
  uint beamSfx = in.ReadLong();

  FourCC animType =
      gpResourceFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile());
  if (animType != 'ANCS')
    return nullptr;

  bool attackDisabled = in.ReadBool();

  return rs_new CEyeBall(
      mgr.AllocateUniqueId(), name, CPatterned::EFlavorType(flavor), info, xf,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), scale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, attackDelay, attackStartTime, wpscId, dInfo, beamContactFxId, beamPulseFxId,
      beamTextureId, beamGlowTextureId, anim0, anim1, anim2, anim3, beamSfx, attackDisabled,
      actParms);
}

CEntity* ScriptLoader::LoadRadialDamage(CStateManager& mgr, CInputStream& in, int propCount,
                                        const CEntityInfo& info) {
  if (propCount != 5)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f center(in);
  bool active = in.Get< bool >();
  CDamageInfo dInfo = in.Get< CDamageInfo >();
  float radius = in.Get< float >();

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), name, info,
      ConvertEditorEulerToTransform4f(CVector3f::Zero(), center),
      CScriptSpecialFunction::kSF_RadialDamage, rstl::string_l(""), radius, 0.f, 0.f, 0.f,
      CVector3f::Zero(), CColor::Black(), active, dInfo, -1, -1, CPlayerState::kIT_Invalid,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId);
}

CEntity* ScriptLoader::LoadScriptCameraPitchVolume(CStateManager& mgr, CInputStream& in,
                                                   int propCount, const CEntityInfo& info) {
  if (propCount != 8)
    return nullptr;

  SScaledActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  float upPitch = in.Get< float >();
  float downPitch = in.Get< float >();
  float scale = in.Get< float >();

  return rs_new CScriptCameraPitchVolume(mgr.AllocateUniqueId(), active, aHead.mActorHead.mName,
                                         info, aHead.mScale, aHead.mActorHead.mTransform,
                                         CRelAngle::FromDegrees(upPitch),
                                         CRelAngle::FromDegrees(downPitch), scale);
}

CEntity* ScriptLoader::LoadElitePirate(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount < CElitePirateData::GetMinProperties())
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Elite Pirate");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CElitePirateData epData(in, propCount);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CElitePirate(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, epData);
}

CEntity* ScriptLoader::LoadEnvFxDensityController(CStateManager& mgr, CInputStream& in,
                                                  int propCount, const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  const bool active = in.Get< bool >();
  float density = in.Get< float >();
  int maxDensityDeltaSpeed = in.Get< int >();

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), name, info, CTransform4f::Identity(),
      CScriptSpecialFunction::kSF_EnvFxDensityController, rstl::string_l(""), density,
      float(maxDensityDeltaSpeed), 0.f, 0.f, CVector3f::Zero(), CColor::Black(), active,
      CDamageInfo(), -1, -1, CPlayerState::kIT_Invalid, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId);
}

CFlameInfo::CFlameInfo(CInputStream& in)
: mPropertyCount(in.ReadLong())
, mAttributes(in.ReadLong())
, mFlameFxId(in.ReadLong())
, mLength(in.ReadLong())
, x10_(in.ReadFloat())
, x18_(in.ReadFloat())
, x1c_(in.ReadFloat()) {}

CEntity* ScriptLoader::LoadMagdolite(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount != 22)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Magdolite");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  float headTrackAngle = in.ReadFloat();
  float losAngle = in.ReadFloat();
  CDamageInfo flameThrowerDamage(in);
  CDamageInfo headContactDamage(in);
  CDamageVulnerability headVuln(in);
  CDamageVulnerability bodyVuln(in);
  CAssetId headlessModel = in.ReadLong();
  CAssetId headlessSkin = in.ReadLong();
  float initialDelay = in.ReadFloat();
  float minDelay = in.ReadFloat();
  float maxDelay = in.ReadFloat();
  float minHp = in.ReadFloat();
  CFlameInfo flameInfo(in);
  float f7 = in.ReadFloat();
  float f8 = in.ReadFloat();
  float f9 = in.ReadFloat();

  return rs_new CMagdolite(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, headTrackAngle, losAngle, flameThrowerDamage, headContactDamage, headVuln,
      bodyVuln, headlessModel, headlessSkin, minHp, initialDelay, minDelay, maxDelay, flameInfo, f7,
      f8, f9);
}

CEntity* ScriptLoader::LoadTeamAiMgr(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount < CTeamAiMgr::CUnknown::GetNumProperties())
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CTeamAiMgr::CUnknown data(in, propCount);

  return rs_new CTeamAiMgr(mgr.AllocateUniqueId(), name, info, data);
}

CEntity* ScriptLoader::LoadSnakeWeedSwarm(CStateManager& mgr, CInputStream& in, int propCount,
                                          const CEntityInfo& info) {
  bool valid = false;
  if (propCount >= 25 && propCount <= 29)
    valid = true;
  if (!valid)
    return nullptr;

  rstl::string name(mgr.HashInstanceName(in));
  CVector3f pos(in);
  CVector3f scale(in);
  bool active = in.Get< bool >();
  CAssetId ancsId = in.Get< uint >();
  int charIdx = in.Get< uint >();
  int defaultAnim = in.Get< uint >();
  CActorParameters actParms = LoadActorParameters(in);

  float f1 = in.Get< float >();
  float f2 = in.Get< float >();
  float f3 = in.Get< float >();
  float f4 = in.Get< float >();
  float f5 = in.Get< float >();
  float f6 = in.Get< float >();
  float f7 = in.Get< float >();
  float f8 = in.Get< float >();
  float f9 = in.Get< float >();
  float f10 = in.Get< float >();
  float f11 = in.Get< float >();
  float f12 = in.Get< float >();
  float f13 = in.Get< float >();
  float f14 = in.Get< float >();

  CDamageInfo dInfo(in);
  float f15 = in.Get< float >();
  uint sfxId1 = in.Get< uint >();
  uint sfxId2 = in.Get< uint >();
  uint sfxId3 = in.Get< uint >();

  uint w4 = propCount >= 29 ? in.Get< uint >() : kInvalidAssetId;
  uint w5 = propCount >= 29 ? in.Get< uint >() : 0;
  uint w6 = propCount >= 29 ? in.Get< uint >() : kInvalidAssetId;
  float f16 = propCount >= 29 ? in.Get< float >() : 0.f;

  return rs_new CSnakeWeedSwarm(
      mgr.AllocateUniqueId(), active, name, info, pos, scale,
      CAnimRes(ancsId, charIdx, CVector3f(1.f, 1.f, 1.f), defaultAnim, true), actParms, f1, f2, f3,
      f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, dInfo, f15, sfxId1, sfxId2, sfxId3, w4, w5,
      w6, f16);
}

CEntity* ScriptLoader::LoadActorContraption(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (propCount != 15)
    return nullptr;

  SScaledActorHead head(in, mgr);
  CVector3f collisionExtent(in);
  CVector3f centroid(in);

  float mass = in.ReadFloat();
  float zMomentum = in.ReadFloat();

  CHealthInfo hInfo(in);

  CDamageVulnerability dVuln(in);

  CAnimationParameters aParms = LoadAnimationParameters(in);

  CActorParameters actParms = LoadActorParameters(in);

  CAssetId flameFxId = in.ReadLong();
  CDamageInfo dInfo(in);
  bool active = in.ReadBool();

  if (gpResourceFactory->GetResourceTypeById(aParms.GetACSFile()) == 0)
    return nullptr;

  const CTransform4f& xf = head.mActorHead.mTransform;

  CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, centroid);

  CMaterialList list(kMT_Immovable, kMT_Solid);

  bool negativeExtent;
  if (collisionExtent.GetX() < 0.f)
    negativeExtent = true;
  else if (collisionExtent.GetY() < 0.f)
    negativeExtent = true;
  else if (collisionExtent.GetZ() < 0.f)
    negativeExtent = true;
  else
    negativeExtent = false;

  CModelData data(CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.mScale,
                           aParms.GetInitialAnimation(), true));

  if (collisionExtent == CVector3f::Zero() || negativeExtent)
    aabb = data.GetBounds(xf.GetRotation());

  return rs_new CScriptContraption(mgr.AllocateUniqueId(), head.mActorHead.mName, info, xf,
                                   data, aabb, list, mass, zMomentum, hInfo, dVuln, actParms,
                                   flameFxId, dInfo, active);
}

CEntity* ScriptLoader::LoadOcculus(CStateManager& mgr, CInputStream& in, int propCount,
                                   const CEntityInfo& info) {
  if (propCount != 15)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Oculus");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  float advanceWpRadius = in.ReadFloat();
  float f2 = in.ReadFloat();
  float alignAngVel = in.ReadFloat();
  float f4 = in.ReadFloat();
  float playerObstructionMinDist = in.ReadFloat();
  float haltDelay = in.ReadFloat();
  CDamageVulnerability dVuln(in);
  float forwardMoveWeight = in.ReadFloat();
  CDamageInfo dInfo(in);

  return rs_new CParasite(
      mgr.AllocateUniqueId(), head.mActorHead.mName, CPatterned::kFT_Zero, info,
      head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, kBT_WallWalker, 0.f, advanceWpRadius, f2, alignAngVel, f4, 0.2f, 0.4f, 0.f, 0.f, 0.f,
      0.f, 0.f, 1.f, forwardMoveWeight, 0.f, 0.f, playerObstructionMinDist, haltDelay, false,
      CWallWalker::kWT_Oculus, dVuln, dInfo, static_cast< uint >(CSfxManager::kInternalInvalidSfxId),
      static_cast< uint >(CSfxManager::kInternalInvalidSfxId),
      static_cast< uint >(CSfxManager::kInternalInvalidSfxId), kInvalidAssetId, kInvalidAssetId, 0.f,
      actParms);
}

CEntity* ScriptLoader::LoadGeemer(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != 16)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Geemer");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  float advanceWpRadius = in.ReadFloat();
  float f2 = in.ReadFloat();
  float alignAngVel = in.ReadFloat();
  float f4 = in.ReadFloat();
  float playerObstructionMinDist = in.ReadFloat();
  float haltDelay = in.ReadFloat();
  float forwardMoveWeight = in.ReadFloat();
  ushort haltSfx = in.ReadLong();
  ushort getUpSfx = in.ReadLong();
  ushort crouchSfx = in.ReadLong();

  return rs_new CParasite(
      mgr.AllocateUniqueId(), head.mActorHead.mName, CPatterned::kFT_Zero, info,
      head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, kBT_WallWalker, 0.f, advanceWpRadius, f2, alignAngVel, f4, 0.2f, 0.4f, 0.f, 0.f, 0.f,
      0.f, 0.f, 1.f, forwardMoveWeight, 0.f, 0.f, playerObstructionMinDist, haltDelay, false,
      CWallWalker::kWT_Geemer, CDamageVulnerability::NormalVulnerability(),
      CDamageInfo(CWeaponMode::Power(), 0.f, 0.f, 0.f), haltSfx, getUpSfx, crouchSfx,
      kInvalidAssetId, kInvalidAssetId, 0.f, actParms);
}

CEntity* ScriptLoader::LoadAtomicAlpha(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount != 14)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Atomic Alpha");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  CAssetId bombWeapon = in.ReadLong();
  CAssetId bombModel = in.ReadLong();
  CDamageInfo bombDamage(in);
  float bombDropDelay = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  bool invisible = in.ReadBool();
  bool b2 = in.ReadBool();

  return rs_new CAtomicAlpha(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, bombWeapon, bombDamage, bombDropDelay, f2, f3, bombModel, invisible, b2);
}

CEntity* ScriptLoader::LoadRumbleEffect(CStateManager& mgr, CInputStream& in, int propCount,
                                        const CEntityInfo& info) {
  if (propCount != 6)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f position(in);
  bool active = in.Get< bool >();
  float f1 = in.Get< float >();
  int w1 = in.Get< int >();
  uint pFlags = LoadParameterFlags(in);

  return rs_new CScriptSpecialFunction(
      mgr.AllocateUniqueId(), name, info,
      ConvertEditorEulerToTransform4f(CVector3f::Zero(), position),
      CScriptSpecialFunction::kSF_RumbleEffect, rstl::string_l(""), f1, float(w1), pFlags, 0.f,
      CVector3f::Zero(), CColor::Black(), active, CDamageInfo(), -1, -1, CPlayerState::kIT_Invalid,
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId);
}

CEntity* ScriptLoader::LoadAmbientAI(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount != 16)
    return nullptr;

  SScaledActorHead head(in, mgr);
  CVector3f collisionExtent(in);
  CVector3f collisionOffset(in);
  float mass = in.ReadFloat();
  CHealthInfo hInfo(in);
  CDamageVulnerability dVuln(in);
  CAnimationParameters animParms = LoadAnimationParameters(in);
  CActorParameters actParms = LoadActorParameters(in);
  float alertRange = in.ReadFloat();
  float impactRange = in.ReadFloat();
  int alertAnim = in.ReadLong();
  int impactAnim = in.ReadLong();
  bool active = in.ReadBool();

  if (gpResourceFactory->GetResourceTypeById(animParms.GetACSFile()) == 0)
    return nullptr;

  const CTransform4f& xf = head.mActorHead.mTransform;
  CAABox aabox = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, collisionOffset);
  CMaterialList matList(kMT_Immovable, kMT_NonSolidDamageable);
  bool negativeExtent;
  if (collisionExtent.GetX() < 0.f)
    negativeExtent = true;
  else if (collisionExtent.GetY() < 0.f)
    negativeExtent = true;
  else if (collisionExtent.GetZ() < 0.f)
    negativeExtent = true;
  else
    negativeExtent = false;

  CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), head.mScale,
                            animParms.GetInitialAnimation(), true));

  if (collisionExtent == CVector3f::Zero() || negativeExtent)
    aabox = mData.GetBounds(xf.GetRotation());

  return rs_new CAmbientAI(mgr.AllocateUniqueId(), head.mActorHead.mName, info, xf, mData,
                           aabox, matList, mass, hInfo, dVuln, actParms, alertRange, impactRange,
                           alertAnim, impactAnim, active);
}

CEntity* ScriptLoader::LoadAtomicBeta(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount != 21)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Atomic Beta");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  CAssetId electricId = in.ReadLong();
  CAssetId weaponId = in.ReadLong();
  CDamageInfo dInfo(in);
  CAssetId particleId = in.ReadLong();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  CDamageVulnerability dVuln(in);
  float f4 = in.ReadFloat();
  float f5 = in.ReadFloat();
  float f6 = in.ReadFloat();
  int sId1 = in.ReadLong() & 0xFFFF;
  int sId2 = in.ReadLong() & 0xFFFF;
  int sId3 = in.ReadLong() & 0xFFFF;
  float f7 = in.ReadFloat();

  return rs_new CAtomicBeta(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, electricId, weaponId, dInfo, particleId, f1, f2, f7, dVuln, f3, f4, f5, sId1,
      sId2, sId3, f6);
}

CEntity* ScriptLoader::LoadIceZoomer(CStateManager& mgr, CInputStream& in, int propCount,
                                     const CEntityInfo& info) {
  if (propCount != 16)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "IceZoomer");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  float advanceWpRadius = in.ReadFloat();
  float f2 = in.ReadFloat();
  float alignAngVel = in.ReadFloat();
  float f4 = in.ReadFloat();
  float playerObstructionMinDist = in.ReadFloat();
  float forwardMoveWeight = in.ReadFloat();
  CAssetId modelRes = in.ReadLong();
  CAssetId skinRes = in.ReadLong();
  CDamageVulnerability dVuln(in);
  float iceZoomerJointHP = in.ReadFloat();

  return rs_new CParasite(
      mgr.AllocateUniqueId(), head.mActorHead.mName, CPatterned::kFT_Zero, info,
      head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, kBT_WallWalker, 0.f, advanceWpRadius, f2, alignAngVel, f4, 0.2f, 0.4f, 0.f, 0.f, 0.f,
      0.f, 0.f, 1.f, forwardMoveWeight, 0.f, 0.f, playerObstructionMinDist, 0.f, false,
      CWallWalker::kWT_IceZoomer, dVuln, CDamageInfo(CWeaponMode::Power(), 0.f, 0.f, 0.f),
      CSfxManager::kInternalInvalidSfxId, CSfxManager::kInternalInvalidSfxId,
      CSfxManager::kInternalInvalidSfxId, modelRes, skinRes, iceZoomerJointHP, actParms);
}

CEntity* ScriptLoader::LoadRidley(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount < CRidley::GetNumProperties())
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Ridley");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CRidley(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, in, propCount);
}

CEntity* ScriptLoader::LoadPuffer(CStateManager& mgr, CInputStream& in, int propCount,
                                  const CEntityInfo& info) {
  if (propCount != 16)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Puffer");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  float hoverSpeed = in.ReadFloat();
  CAssetId cloudEffect = in.ReadLong();
  CDamageInfo cloudDamage(in);
  CAssetId cloudSteam = in.ReadLong();
  float f2 = in.ReadFloat();
  bool b1 = in.ReadBool();
  bool b2 = in.ReadBool();
  bool b3 = in.ReadBool();
  CDamageInfo explosionDamage(in);
  ushort sfxId = in.ReadLong();

  return rs_new CPuffer(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, hoverSpeed, cloudEffect, cloudDamage, cloudSteam, f2, b1, b2, b3,
      explosionDamage, sfxId);
}

CEntity* ScriptLoader::LoadTryclops(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 10)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Tryclops");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();

  return rs_new CTryclops(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, f1, f2, f3, f4);
}

CEntity* ScriptLoader::LoadSeedling(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 14)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Seedling");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  CAssetId needleId = in.ReadLong();
  CAssetId weaponId = in.ReadLong();
  CDamageInfo dInfo1 = CDamageInfo(in);
  CDamageInfo dInfo2 = CDamageInfo(in);
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  float f4 = in.ReadFloat();

  return rs_new CSeedling(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, needleId, weaponId, dInfo1, dInfo2, f1, f2, f3, f4);
}

CEntity* ScriptLoader::LoadThermalHeatFader(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  float fadedLevel = in.Get< float >();
  float initialLevel = in.Get< float >();

  return rs_new CScriptDistanceFog(mgr.AllocateUniqueId(), name, info, kRFM_None, CColor::Black(),
                                   CVector2f(0.f, 0.f), 0.f, CVector2f(0.f, 0.f), false, active,
                                   fadedLevel, initialLevel, 0.f, 0.f);
}

CEntity* ScriptLoader::LoadBurrower(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 13)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Burrower");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  CAssetId jumpParticle = in.ReadLong();
  CAssetId trailParticle = in.ReadLong();
  CAssetId projectile = in.ReadLong();
  CDamageInfo dInfo = CDamageInfo(in);
  CAssetId visorParticle = in.ReadLong();
  uint visorSfx = in.ReadLong();
  CAssetId deathExplosionParticle = in.ReadLong();

  return rs_new CBurrower(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, jumpParticle, trailParticle, projectile, dInfo, visorParticle, visorSfx,
      deathExplosionParticle);
}

CEntity* ScriptLoader::LoadMetroidPrimeRelay(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info) {
  if (propCount != 22)
    return nullptr;

  int version = in.ReadLong();
  if (version != 3)
    return nullptr;

  SScaledActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  int w1 = in.ReadLong();
  bool b1 = in.ReadBool();
  int w2 = in.ReadLong();
  CHealthInfo hInfo1(in);
  CHealthInfo hInfo2(in);
  int w3 = in.ReadLong();

  CMetroidPrime::CMetroidPrimeAttackWeights roomParm1(in);
  CMetroidPrime::CMetroidPrimeAttackWeights roomParm2(in);
  CMetroidPrime::CMetroidPrimeAttackWeights roomParm3(in);
  CMetroidPrime::CMetroidPrimeAttackWeights roomParm4(in);

  rstl::reserved_vector< CMetroidPrime::CMetroidPrimeAttackWeights, 4 > roomParms;
  roomParms.push_back(roomParm1);
  roomParms.push_back(roomParm2);
  roomParms.push_back(roomParm3);
  roomParms.push_back(roomParm4);

  int w4 = in.ReadLong();
  int w5 = in.ReadLong();

  CMetroidPrimeData parms(in);

  if (parms.GetPatternedInfo().GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CMetroidPrimeRelay(mgr.AllocateUniqueId(), aHead.mActorHead.mName, info, active,
                                   aHead.mActorHead.mTransform, aHead.mScale, parms, f1, f2,
                                   f3, w1, b1, w2, hInfo1, hInfo2, w3, w4, w5, roomParms);
}

CBeamInfo::CBeamInfo(CInputStream& in)
: x0_(in.ReadLong())
, mBeamAttributes(in.ReadLong())
, mContactFxId(in.ReadLong())
, mPulseFxId(in.ReadLong())
, mTextureId(in.ReadLong())
, mGlowTextureId(in.ReadLong())
, mLength(static_cast< int >(in.ReadFloat()))
, mRadius(in.ReadFloat())
, mExpansionSpeed(in.ReadFloat())
, mLifeTime(in.ReadFloat())
, mPulseSpeed(in.ReadFloat())
, mShutdownTime(in.ReadFloat())
, mContactFxScale(in.ReadFloat())
, mPulseFxScale(in.ReadFloat())
, mTravelSpeed(in.ReadFloat())
, mInnerColor(in)
, mOuterColor(in) {}

CEntity* ScriptLoader::LoadScriptBeam(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount != 7)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  uint weaponDescId = in.ReadLong();

  if (gpResourceFactory->GetResourceTypeById(weaponDescId) == 0)
    return nullptr;

  CBeamInfo beamInfo(in);
  CDamageInfo dInfo(in);

  return rs_new CScriptBeam(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                            active, gpSimplePool->GetObj(SObjectTag('WPSC', weaponDescId)),
                            beamInfo, dInfo);
}

CEntity* ScriptLoader::LoadWorldLightFader(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (propCount != 4)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  bool active = in.Get< bool >();
  float f1 = in.Get< float >();
  float f2 = in.Get< float >();

  return rs_new CScriptDistanceFog(mgr.AllocateUniqueId(), name, info, kRFM_None, CColor::Black(),
                                   CVector2f(0.f, 0.f), 0.f, CVector2f(0.f, 0.f), false, active,
                                   0.f, 0.f, f1, f2);
}

CEntity* ScriptLoader::LoadMetroidPrimeStage2(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info) {
  if (propCount != 11)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Burrower");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  CAssetId particle1 = in.ReadLong();
  CDamageInfo dInfo = CDamageInfo(in);
  CAssetId electric = in.ReadLong();
  uint sfxId = in.ReadLong();
  CAssetId particle2 = in.ReadLong();

  return rs_new CMetroidPrimeStage2(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, particle1, dInfo, electric, sfxId, particle2);
}

CEntity* ScriptLoader::LoadMazeNode(CStateManager& mgr, CInputStream& in, int propCount,
                                    const CEntityInfo& info) {
  if (propCount != 10)
    return nullptr;

  SActorHead aHead(in, mgr);
  bool active = in.Get< bool >();
  int col = in.Get< int >();
  int row = in.Get< int >();
  int side = in.Get< int >();
  CVector3f actorPos(in);
  CVector3f triggerPos(in);
  CVector3f effectPos(in);

  return rs_new CScriptMazeNode(mgr.AllocateUniqueId(), aHead.mName, info, aHead.mTransform,
                                active, col, row, side, actorPos, triggerPos, effectPos);
}

CEntity* ScriptLoader::LoadOmegaPirate(CStateManager& mgr, CInputStream& in, int propCount,
                                       const CEntityInfo& info) {
  if (propCount < CElitePirateData::GetMinProperties() + 1)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Omega Pirate");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);
  CElitePirateData epData(in, propCount);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  int skeletonModelId = in.ReadLong();
  int skinRulesId = in.ReadLong();
  int layoutInfoId = in.ReadLong();

  return rs_new COmegaPirate(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      pInfo, actParms, epData, skeletonModelId, skinRulesId, layoutInfoId);
}

CEntity* ScriptLoader::LoadPhazonPool(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount != 18)
    return nullptr;

  SScaledActorHead head(in, mgr);
  bool active = in.Get< bool >();
  CAssetId w1 = in.ReadLong();
  CAssetId w2 = in.ReadLong();
  CAssetId w3 = in.ReadLong();
  CAssetId w4 = in.ReadLong();
  uint p11 = in.ReadLong();
  CDamageInfo dInfo = CDamageInfo(in);
  CVector3f orientedForce = CVector3f(in);
  ETriggerFlags triggerFlags = ETriggerFlags(in.ReadLong());
  float f1 = in.ReadFloat();
  float f2 = in.ReadFloat();
  float f3 = in.ReadFloat();
  bool b2 = in.ReadBool();
  float f4 = in.ReadFloat();
  const CVector3f& scale = head.mScale;

  return rs_new CScriptPhazonPool(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info,
      CTransform4f::Translate(head.mActorHead.mTransform.GetTranslation()), scale, active, w1,
      w2, w3, w4, p11, dInfo, orientedForce, triggerFlags, b2, f1, f2, f3, f4);
}

CEntity* ScriptLoader::LoadPhazonHealingNodule(CStateManager& mgr, CInputStream& in, int propCount,
                                               const CEntityInfo& info) {
  if (propCount != 9)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount =
      CPatternedInfo::HasCorrectParameterCount(in, "PhazonHealingNodule");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);
  CActorParameters actParms = LoadActorParameters(in);

  in.ReadBool();
  CAssetId particleDescId = in.ReadLong();
  rstl::string actorLctr(in);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  return rs_new CPhazonHealingNodule(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, particleDescId, actorLctr);
}

CEntity* ScriptLoader::LoadShadowProjector(CStateManager& mgr, CInputStream& in, int propCount,
                                           const CEntityInfo& info) {
  if (propCount != 10)
    return nullptr;

  rstl::string name = mgr.HashInstanceName(in);
  CVector3f position(in);
  bool active = in.ReadBool();
  float scale = in.ReadFloat();
  CVector3f offset(in);
  float zOffsetAdjust = in.ReadFloat();
  float opacity = in.ReadFloat();
  float opacityRecip = in.ReadFloat();
  bool persistent = in.ReadBool();
  int textureSize = in.ReadLong();

  return rs_new CScriptShadowProjector(
      mgr.AllocateUniqueId(), name, info, CTransform4f::Translate(position), active, offset,
      persistent, scale, zOffsetAdjust, opacity, opacityRecip, textureSize);
}

CEntity* ScriptLoader::LoadEnergyBall(CStateManager& mgr, CInputStream& in, int propCount,
                                      const CEntityInfo& info) {
  if (propCount < 16)
    return nullptr;

  SScaledActorHead head(in, mgr);

  rstl::pair< bool, uint > pcount = CPatternedInfo::HasCorrectParameterCount(in, "Atomic Alpha");
  if (!pcount.first)
    return nullptr;

  CPatternedInfo pInfo(in, pcount.second);

  if (pInfo.GetAnimationParameters().GetACSFile() == kInvalidAssetId)
    return nullptr;

  CActorParameters actParms = LoadActorParameters(in);
  int w1 = in.Get< int >();
  float f1 = in.Get< float >();
  CDamageInfo dInfo1(in);
  float f2 = in.Get< float >();
  uint a1 = in.Get< int >();
  ushort sfxId1 = CSfxManager::TranslateSFXID(in.Get< int >());
  uint a2 = in.Get< int >();
  uint a3 = in.Get< int >();
  ushort sfxId2 = CSfxManager::TranslateSFXID(in.Get< int >());
  float f3 = in.Get< float >();
  float f4 = in.Get< float >();
  uint a4 = in.Get< int >();

  CDamageInfo dInfo2 = propCount >= 19 ? CDamageInfo(in) : CDamageInfo();
  float f5 = propCount >= 20 ? in.Get< float >() : 3.0f;

  return rs_new CEnergyBall(
      mgr.AllocateUniqueId(), head.mActorHead.mName, info, head.mActorHead.mTransform,
      CModelData(CAnimRes(pInfo.GetAnimationParameters().GetACSFile(),
                          pInfo.GetAnimationParameters().GetCharacter(), head.mScale,
                          pInfo.GetAnimationParameters().GetInitialAnimation(), true)),
      actParms, pInfo, w1, f1, dInfo1, f2, a1, sfxId1, a2, a3, sfxId2, f3, f4, a4, dInfo2, f5);
}
