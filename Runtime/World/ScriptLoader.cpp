#include "ScriptLoader.hpp"
#include "CStateManager.hpp"
#include "CGrappleParameters.hpp"
#include "CActorParameters.hpp"
#include "CVisorParameters.hpp"
#include "CScannableParameters.hpp"
#include "CLightParameters.hpp"
#include "CAnimationParameters.hpp"
#include "CFluidUVMotion.hpp"
#include "GameGlobalObjects.hpp"
#include "CWorld.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "Particle/CWeaponDescription.hpp"
#include "CDamageInfo.hpp"
#include "CScriptActor.hpp"
#include "CScriptWaypoint.hpp"
#include "CScriptDoor.hpp"
#include "CScriptTrigger.hpp"
#include "CScriptTimer.hpp"
#include "CScriptCounter.hpp"
#include "CScriptDock.hpp"
#include "CScriptActorKeyframe.hpp"
#include "CScriptWater.hpp"
#include "CScriptEffect.hpp"
#include "CScriptPlatform.hpp"
#include "CScriptSound.hpp"
#include "CScriptGenerator.hpp"
#include "CScriptGrapplePoint.hpp"
#include "CScriptPickupGenerator.hpp"
#include "CScriptPointOfInterest.hpp"
#include "CScriptPlayerActor.hpp"
#include "CScriptAreaAttributes.hpp"
#include "CScriptCameraWaypoint.hpp"
#include "CScriptCoverPoint.hpp"
#include "CScriptSpawnPoint.hpp"
#include "CScriptCameraHint.hpp"
#include "CScriptPickup.hpp"
#include "CScriptMemoryRelay.hpp"
#include "CScriptRandomRelay.hpp"
#include "CScriptRelay.hpp"
#include "CScriptHUDMemo.hpp"
#include "CScriptCameraFilterKeyframe.hpp"
#include "CScriptCameraBlurKeyframe.hpp"
#include "CScriptDamageableTrigger.hpp"
#include "CScriptDebris.hpp"
#include "CScriptSteam.hpp"
#include "CScriptDistanceFog.hpp"
#include "CScriptDockAreaChange.hpp"
#include "CScriptActorRotate.hpp"
#include "CScriptSpecialFunction.hpp"
#include "CScriptSwitch.hpp"
#include "CWallCrawlerSwarm.hpp"
#include "CScriptAiJumpPoint.hpp"
#include "CScriptColorModulate.hpp"
#include "CScriptCameraShaker.hpp"
#include "CRepulsor.hpp"
#include "CScriptCameraPitchVolume.hpp"
#include "CScriptCameraHintTrigger.hpp"
#include "CScriptVisorFlare.hpp"
#include "CScriptBeam.hpp"
#include "Camera/CCinematicCamera.hpp"
#include "MP1/World/CNewIntroBoss.hpp"
#include "MP1/World/CBeetle.hpp"
#include "MP1/World/CWarWasp.hpp"
#include "MP1/World/CSpacePirate.hpp"
#include "MP1/World/CActorContraption.hpp"
#include "CScriptShadowProjector.hpp"
#include "CScriptStreamedMusic.hpp"
#include "CScriptMidi.hpp"
#include "CScriptRoomAcoustics.hpp"
#include "CScriptControllerAction.hpp"
#include "CPatternedInfo.hpp"
#include "CSimplePool.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"
#include "Editor/ProjectResourceFactoryMP1.hpp"
#include "logvisor/logvisor.hpp"

namespace urde
{
static logvisor::Module Log("urde::ScriptLoader");

static SObjectTag MorphballDoorANCS = {};
static const SObjectTag& GetMorphballDoorACS()
{
    if (!MorphballDoorANCS)
    {
        MorphballDoorANCS.type = FOURCC('ANCS');
        MorphballDoorANCS.id = g_ResFactory->TranslateOriginalToNew(0x1F9DA858);
    }
    return MorphballDoorANCS;
}

static bool EnsurePropertyCount(int count, int expected, const char* structName)
{
    if (count < expected)
    {
        Log.report(logvisor::Warning, "Insufficient number of props (%d/%d) for %s entity", count, expected,
                   structName);
        return false;
    }
    return true;
}

struct SActorHead
{
    std::string x0_name;
    zeus::CTransform x10_transform;
};

struct SScaledActorHead : SActorHead
{
    zeus::CVector3f x40_scale;
    SScaledActorHead(SActorHead&& head) : SActorHead(std::move(head)) {}
};

static zeus::CTransform LoadEditorTransform(CInputStream& in)
{
    zeus::CVector3f position;
    position.readBig(in);
    zeus::CVector3f orientation;
    orientation.readBig(in);
    return ScriptLoader::ConvertEditorEulerToTransform4f(orientation, position);
}

static zeus::CTransform LoadEditorTransformPivotOnly(CInputStream& in)
{
    zeus::CVector3f position;
    position.readBig(in);
    zeus::CVector3f orientation;
    orientation.readBig(in);
    orientation.x = 0.f;
    orientation.y = 0.f;
    return ScriptLoader::ConvertEditorEulerToTransform4f(orientation, position);
}

static SActorHead LoadActorHead(CInputStream& in, CStateManager& stateMgr)
{
    SActorHead ret;
    ret.x0_name = stateMgr.HashInstanceName(in);
    ret.x10_transform = LoadEditorTransform(in);
    return ret;
}

static SScaledActorHead LoadScaledActorHead(CInputStream& in, CStateManager& stateMgr)
{
    SScaledActorHead ret = LoadActorHead(in, stateMgr);
    ret.x40_scale.readBig(in);
    return ret;
}

static zeus::CAABox GetCollisionBox(CStateManager& stateMgr, TAreaId id, const zeus::CVector3f& extent,
                                    const zeus::CVector3f& offset)
{
    zeus::CAABox box(-extent * 0.5f + offset, extent * 0.5f + offset);
    const zeus::CTransform& rot = stateMgr.WorldNC()->GetGameAreas()[id]->GetTransform().getRotation();
    return box.getTransformedAABox(rot);
}

u32 ScriptLoader::LoadParameterFlags(CInputStream& in)
{
    u32 count = in.readUint32Big();
    u32 ret = 0;
    for (u32 i = 0; i < count; ++i)
        if (in.readBool())
            ret |= 1 << i;
    return ret;
}

CGrappleParameters ScriptLoader::LoadGrappleParameters(CInputStream& in)
{
    float a = in.readFloatBig();
    float b = in.readFloatBig();
    float c = in.readFloatBig();
    float d = in.readFloatBig();
    float e = in.readFloatBig();
    float f = in.readFloatBig();
    float g = in.readFloatBig();
    float h = in.readFloatBig();
    float i = in.readFloatBig();
    float j = in.readFloatBig();
    float k = in.readFloatBig();
    bool l = in.readBool();
    return CGrappleParameters(a, b, c, d, e, f, g, h, i, j, k, l);
}

CActorParameters ScriptLoader::LoadActorParameters(CInputStream& in)
{
    u32 propCount = in.readUint32Big();
    if (propCount >= 5 && propCount <= 0xe)
    {
        CLightParameters lParms = ScriptLoader::LoadLightParameters(in);

        CScannableParameters sParams;
        if (propCount > 5)
            sParams = LoadScannableParameters(in);

        ResId xrayModel = in.readUint32Big();
        ResId xraySkin = in.readUint32Big();
        ResId infraModel = in.readUint32Big();
        ResId infraSkin = in.readUint32Big();

        bool b1 = true;
        if (propCount > 7)
            b1 = in.readBool();

        float f1 = 1.f;
        if (propCount > 8)
            f1 = in.readFloatBig();

        float f2 = 1.f;
        if (propCount > 9)
            f2 = in.readFloatBig();

        CVisorParameters vParms;
        if (propCount > 6)
            vParms = LoadVisorParameters(in);

        bool thermalHeat = false;
        if (propCount > 10)
            thermalHeat = in.readBool();

        bool b3 = false;
        if (propCount > 11)
            b3 = in.readBool();

        bool b4 = false;
        if (propCount > 12)
            b4 = in.readBool();

        float f3 = 1.f;
        if (propCount > 13)
            f3 = in.readFloatBig();

        std::pair<ResId, ResId> xray = {};
        if (g_ResFactory->GetResourceTypeById(xrayModel))
            xray = {xrayModel, xraySkin};

        std::pair<ResId, ResId> infra = {};
        if (g_ResFactory->GetResourceTypeById(infraModel))
            infra = {infraModel, infraSkin};

        return CActorParameters(lParms, sParams, xray, infra, vParms, b1, thermalHeat, b3, b4);
    }
    return CActorParameters::None();
}

CVisorParameters ScriptLoader::LoadVisorParameters(CInputStream& in)
{
    u32 propCount = in.readUint32Big();
    if (propCount >= 1 && propCount <= 3)
    {
        bool b1 = in.readBool();
        bool b2 = false;
        u8 mask = 0xf;
        if (propCount > 1)
            b2 = in.readBool();
        if (propCount > 2)
            mask = in.readUint32Big();
        return CVisorParameters(mask, b1, b2);
    }
    return CVisorParameters();
}

CScannableParameters ScriptLoader::LoadScannableParameters(CInputStream& in)
{
    u32 propCount = in.readUint32Big();
    if (propCount == 1)
        return CScannableParameters(in.readUint32Big());
    return CScannableParameters();
}

CLightParameters ScriptLoader::LoadLightParameters(CInputStream& in)
{
    u32 propCount = in.readUint32Big();
    if (propCount == 14)
    {
        bool a = in.readBool();
        float b = in.readFloatBig();
        CLightParameters::EShadowTesselation shadowTess = CLightParameters::EShadowTesselation(in.readUint32Big());
        float d = in.readFloatBig();
        float e = in.readFloatBig();

        zeus::CColor noLightsAmbient;
        noLightsAmbient.readRGBABig(in);

        bool makeLights = in.readBool();
        CLightParameters::EWorldLightingOptions lightOpts = CLightParameters::EWorldLightingOptions(in.readUint32Big());
        CLightParameters::ELightRecalculationOptions recalcOpts =
            CLightParameters::ELightRecalculationOptions(in.readUint32Big());

        zeus::CVector3f actorPosBias;
        actorPosBias.readBig(in);

        s32 maxDynamicLights = -1;
        s32 maxAreaLights = -1;
        if (propCount >= 12)
        {
            maxDynamicLights = in.readUint32Big();
            maxAreaLights = in.readUint32Big();
        }

        bool ambientChannelOverflow = false;
        if (propCount >= 13)
            ambientChannelOverflow = in.readBool();

        s32 layerIdx = 0;
        if (propCount >= 14)
            layerIdx = in.readUint32Big();

        return CLightParameters(a, b, shadowTess, d, e, noLightsAmbient, makeLights, lightOpts, recalcOpts, actorPosBias,
                                maxDynamicLights, maxAreaLights, ambientChannelOverflow, layerIdx);
    }
    return CLightParameters::None();
}

CAnimationParameters ScriptLoader::LoadAnimationParameters(CInputStream& in)
{
    ResId ancs = in.readUint32Big();
    s32 charIdx = in.readUint32Big();
    u32 defaultAnim = in.readUint32Big();
    return CAnimationParameters(ancs, charIdx, defaultAnim);
}

CFluidUVMotion ScriptLoader::LoadFluidUVMotion(CInputStream& in)
{
    /* NOTE: DO NOT RE-ORDER THIS FUNCTION
     * For some inexplicable reason Retro stores the layers in this order.
     * Changing it will change the behavior of CFluidUVMotion,
     * which is something we don't want.
     * - Phil
     * P.S: If you do change it, I'll hunt you down and put pink lipstick on your dog.
     */
    CFluidUVMotion::EFluidUVMotion motion = CFluidUVMotion::EFluidUVMotion(in.readUint32Big());
    float a = in.readFloatBig();
    float b = in.readFloatBig();
    b = zeus::degToRad(b) - M_PIF;
    float c = in.readFloatBig();
    float d = in.readFloatBig();
    CFluidUVMotion::SFluidLayerMotion layerMotion2(motion, a, b, c, d);

    motion = CFluidUVMotion::EFluidUVMotion(in.readUint32Big());
    a = in.readFloatBig();
    b = in.readFloatBig();
    b = zeus::degToRad(b) - M_PIF;
    c = in.readFloatBig();
    d = in.readFloatBig();
    CFluidUVMotion::SFluidLayerMotion layerMotion3(motion, a, b, c, d);

    motion = CFluidUVMotion::EFluidUVMotion(in.readUint32Big());
    a = in.readFloatBig();
    b = in.readFloatBig();
    b = zeus::degToRad(b) - M_PIF;
    c = in.readFloatBig();
    d = in.readFloatBig();
    CFluidUVMotion::SFluidLayerMotion layerMotion1(motion, a, b, c, d);

    a = in.readFloatBig();
    b = in.readFloatBig();

    b = zeus::degToRad(b) - M_PIF;

    return CFluidUVMotion(a, b, layerMotion1, layerMotion2, layerMotion3);
}

zeus::CTransform ScriptLoader::ConvertEditorEulerToTransform4f(const zeus::CVector3f& orientation,
                                                               const zeus::CVector3f& position)
{
    return zeus::CTransform::RotateZ(zeus::degToRad(orientation.z)) *
               zeus::CTransform::RotateY(zeus::degToRad(orientation.y)) *
               zeus::CTransform::RotateX(zeus::degToRad(orientation.x)) +
           position;
}

CEntity* ScriptLoader::LoadActor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 24, "Actor"))
        return nullptr;

    SScaledActorHead head = LoadScaledActorHead(in, mgr);

    zeus::CVector3f collisionExtent;
    collisionExtent.readBig(in);

    zeus::CVector3f centroid;
    centroid.readBig(in);

    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();

    CHealthInfo hInfo(in);

    CDamageVulnerability dInfo(in);

    ResId staticId = in.readUint32Big();
    CAnimationParameters aParms = LoadAnimationParameters(in);

    CActorParameters actParms = LoadActorParameters(in);

    bool b1 = in.readBool();
    bool snow = in.readBool();
    bool solid = in.readBool();
    bool cameraPassthrough = in.readBool();
    bool b5 = in.readBool();
    u32 w2 = in.readUint32Big();
    float f3 = in.readFloatBig();
    bool b6 = in.readBool();
    bool b7 = in.readBool();
    bool b8 = in.readBool();
    bool b9 = in.readBool();

    FourCC animType = g_ResFactory->GetResourceTypeById(aParms.GetACSFile());
    if (!g_ResFactory->GetResourceTypeById(staticId) && !animType)
        return nullptr;

    zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, centroid);

    CMaterialList list;
    if (snow) // Bool 2
        list.Add(EMaterialTypes::Snow);

    if (solid) // Bool 3
        list.Add(EMaterialTypes::Solid);

    if (cameraPassthrough) // Bool 4
        list.Add(EMaterialTypes::CameraPassthrough);

    CModelData data;
    if (animType == SBIG('ANCS'))
        data = CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.x40_scale, aParms.GetInitialAnimation(), true);
    else
        data = CStaticRes(staticId, head.x40_scale);

    if ((collisionExtent.x < 0.f || collisionExtent.y < 0.f || collisionExtent.z < 0.f)|| collisionExtent.isZero())
        aabb = data.GetBounds(head.x10_transform.getRotation());

    return new CScriptActor(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(data), aabb, f1,
                            f2, list, hInfo, dInfo, actParms, b1, b5, w2, f3, b6, b7, b8, b9);
}

CEntity* ScriptLoader::LoadWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 13, "Waypoint"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);

    bool active = in.readBool();
    float f1 = in.readFloatBig();
    float delay = in.readFloatBig();
    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();
    u32 w4 = in.readUint32Big();
    u32 w5 = in.readUint32Big();
    u32 w6 = in.readUint32Big();
    u32 w7 = in.readUint32Big();

    return new CScriptWaypoint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, active, f1, delay, w1,
                               w2, w3, w4, w5, w6, w7);
}

CEntity* ScriptLoader::LoadDoor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 13, "Door") || propCount > 14)
        return nullptr;

    SScaledActorHead head = LoadScaledActorHead(in, mgr);
    CAnimationParameters aParms = LoadAnimationParameters(in);
    CActorParameters actParms = LoadActorParameters(in);

    zeus::CVector3f v1;
    v1.readBig(in);
    zeus::CVector3f collisionExtent;
    collisionExtent.readBig(in);
    zeus::CVector3f offset;
    offset.readBig(in);

    bool active = in.readBool();
    bool open = in.readBool();
    bool b3 = in.readBool();
    float animationLength = in.readFloatBig();

    zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, offset);

    if (!g_ResFactory->GetResourceTypeById(aParms.GetACSFile()))
        return nullptr;

    CModelData mData =
        CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.x40_scale, true, aParms.GetInitialAnimation());
    if (collisionExtent.isZero())
        aabb = mData.GetBounds(head.x10_transform.getRotation());

    bool isMorphballDoor = false;
    if (propCount == 13)
    {
        if (aParms.GetACSFile() == GetMorphballDoorACS().id)
            isMorphballDoor = true;
    }
    else if (propCount == 14)
        isMorphballDoor = in.readBool();

    return new CScriptDoor(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(mData), actParms,
                           v1, aabb, active, open, b3, animationLength, isMorphballDoor);
}

CEntity* ScriptLoader::LoadTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 9, "Trigger"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);

    zeus::CVector3f position = zeus::CVector3f::ReadBig(in);

    zeus::CVector3f extent = zeus::CVector3f::ReadBig(in);

    CDamageInfo dInfo(in);

    zeus::CVector3f forceVec = zeus::CVector3f::ReadBig(in);

    ETriggerFlags flags = ETriggerFlags(in.readUint32Big());
    bool active = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();

    zeus::CAABox box(-extent * 0.5f, extent * 0.5f);

    const zeus::CTransform& areaXf = mgr.WorldNC()->GetGameAreas()[info.GetAreaId()]->GetTransform();
    zeus::CVector3f orientedForce = areaXf.basis * forceVec;

    return new CScriptTrigger(mgr.AllocateUniqueId(), name, info, position, box, dInfo, orientedForce, flags, active,
                              b2, b3);
}

CEntity* ScriptLoader::LoadTimer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 6, "Timer"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);

    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();

    return new CScriptTimer(mgr.AllocateUniqueId(), name, info, f1, f2, b1, b2, b3);
}

CEntity* ScriptLoader::LoadCounter(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 5, "Counter"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);

    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    bool b1 = in.readBool();
    bool b2 = in.readBool();

    return new CScriptCounter(mgr.AllocateUniqueId(), name, info, w1, w2, b1, b2);
}

CEntity* ScriptLoader::LoadEffect(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 24, "Effect"))
        return nullptr;

    SScaledActorHead head = LoadScaledActorHead(in, mgr);

    ResId partId = in.readUint32Big();
    ResId elscId = in.readUint32Big();
    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();
    bool b4 = in.readBool();

    if (partId == 0xffffffff && elscId == 0xffffffff)
        return nullptr;

    if (!g_ResFactory->GetResourceTypeById(partId) && !g_ResFactory->GetResourceTypeById(elscId))
        return nullptr;

    bool b5 = in.readBool();
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    float f4 = in.readFloatBig();
    bool b6 = in.readBool();
    float f5 = in.readFloatBig();
    float f6 = in.readFloatBig();
    float f7 = in.readFloatBig();
    bool b7 = in.readBool();
    bool b8 = in.readBool();
    bool b9 = in.readBool();
    bool b10 = in.readBool();

    CLightParameters lParms = LoadLightParameters(in);

    return new CScriptEffect(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, head.x40_scale, partId,
                             elscId, b1, b2, b3, b4, b5, f1, f2, f3, f4, b6, f5, f6, f7, b7, b8, b9, lParms, b10);
}

CEntity* ScriptLoader::LoadPlatform(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 19, "Platform"))
        return nullptr;

    SScaledActorHead head = LoadScaledActorHead(in, mgr);

    zeus::CVector3f extent;
    extent.readBig(in);

    zeus::CVector3f centroid;
    centroid.readBig(in);

    ResId staticId = in.readUint32Big();
    CAnimationParameters aParms = LoadAnimationParameters(in);

    CActorParameters actParms = LoadActorParameters(in);

    float f1 = in.readFloatBig();
    bool b1 = in.readBool();
    ResId dclnId = in.readUint32Big();

    CHealthInfo hInfo(in);

    CDamageVulnerability dInfo(in);

    bool b2 = in.readBool();
    float f2 = in.readFloatBig();
    bool b3 = in.readBool();
    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();

    FourCC animType = g_ResFactory->GetResourceTypeById(aParms.GetACSFile());
    if (!g_ResFactory->GetResourceTypeById(staticId) && !animType)
        return nullptr;

    zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), extent, centroid);

    FourCC dclnType = g_ResFactory->GetResourceTypeById(dclnId);
    rstl::optional_object<TLockedToken<CCollidableOBBTreeGroup>> dclnToken;
    if (dclnType)
    {
        dclnToken.emplace(g_SimplePool->GetObj({SBIG('DCLN'), dclnId}));
        dclnToken->GetObj();
    }

    CModelData data;
    if (animType == SBIG('ANCS'))
        data = CAnimRes(aParms.GetACSFile(), aParms.GetCharacter(), head.x40_scale, true, aParms.GetInitialAnimation());
    else
        data = CStaticRes(staticId, head.x40_scale);

    if (extent.isZero())
        aabb = data.GetBounds(head.x10_transform.getRotation());

    return new CScriptPlatform(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(data),
                               actParms, aabb, f1, b2, f2, b1, hInfo, dInfo, dclnToken, b3, w2, w3);
}

CEntity* ScriptLoader::LoadSound(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 20, "Sound"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);
    s32 soundId = in.readInt32Big();
    bool active = in.readBool();
    float maxDistance = in.readFloatBig();
    float minDistance = in.readFloatBig();
    float f3 = in.readFloatBig();
    u32 priority = in.readUint32Big();
    u32 maxVolume = in.readUint32Big();
    u32 minVolume = in.readUint32Big();
    u32 panning = in.readUint32Big();
    bool loop = in.readBool();
    bool persistent = in.readBool();
    bool triggered = in.readBool();
    bool b4 = in.readBool();
    bool roomAcoustic = in.readBool();
    bool b6 = in.readBool();
    bool b7 = in.readBool();
    u32 w5 = in.readUint32Big();

    if (soundId < 0)
        return nullptr;

    return new CScriptSound(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, soundId, active,
                            maxDistance, minDistance, f3, priority, maxVolume, 0, minVolume, panning, 0, loop,
                            persistent, triggered, b4, roomAcoustic, b6, b7, w5);
}

CEntity* ScriptLoader::LoadGenerator(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 8, "Generator"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);

    u32 spawnCount = in.readUint32Big();
    bool reuseFollowers = in.readBool();
    bool inheritXf = in.readBool();

    zeus::CVector3f offset = zeus::CVector3f::ReadBig(in);

    bool active = in.readBool();
    float minScale = in.readFloatBig();
    float maxScale = in.readFloatBig();

    return new CScriptGenerator(mgr.AllocateUniqueId(), name, info, spawnCount, reuseFollowers, offset, inheritXf,
                                active, minScale, maxScale);
}

CEntity* ScriptLoader::LoadDock(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 7, "Dock"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    bool active = in.readBool();
    zeus::CVector3f position;
    position.readBig(in);
    zeus::CVector3f scale;
    scale.readBig(in);
    u32 dock = in.readUint32Big();
    TAreaId area = in.readUint32Big();
    bool b1 = in.readBool();
    return new CScriptDock(mgr.AllocateUniqueId(), name, info, position, scale, dock, area, active, 0, b1);
}

CEntity* ScriptLoader::LoadCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 14, "Camera"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);

    bool b1 = in.readBool();
    float f1 = in.readFloatBig();
    bool b2 = in.readBool();
    bool b3 = in.readBool();
    bool b4 = in.readBool();
    bool b5 = in.readBool();
    bool b6 = in.readBool();
    bool b7 = in.readBool();
    bool b8 = in.readBool();
    float f2 = in.readFloatBig();
    bool b9 = in.readBool();

    bool b10 = false;
    if (propCount > 14)
        b10 = in.readBool();

    u32 flags = u32(b2) | u32(b3) << 1 | u32(b4) << 2 | u32(b5) << 3 | u32(b6) << 4 | u32(b7) << 5 | u32(b8) << 6 |
                u32(b9) << 8;

    return new CCinematicCamera(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, b1, f1,
                                f2 / CCameraManager::Aspect(), CCameraManager::NearPlane(), CCameraManager::FarPlane(),
                                CCameraManager::Aspect(), flags);
}

CEntity* ScriptLoader::LoadCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 6, "CameraWaypoint"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);

    bool b1 = in.readBool();
    float f1 = in.readFloatBig();
    u32 w1 = in.readUint32Big();

    return new CScriptCameraWaypoint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, b1, f1, w1);
}

CEntity* ScriptLoader::LoadNewIntroBoss(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 13, "NewIntroBoss"))
        return nullptr;

    SScaledActorHead head = LoadScaledActorHead(in, mgr);

    std::pair<bool, u32> pcount = CPatternedInfo::HasCorrectParameterCount(in);
    if (!pcount.first)
        return nullptr;

    CPatternedInfo pInfo(in, pcount.second);

    CActorParameters actParms = LoadActorParameters(in);

    float f1 = in.readFloatBig();
    u32 w1 = in.readUint32Big();

    CDamageInfo dInfo(in);

    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();
    u32 w4 = in.readUint32Big();
    u32 w5 = in.readUint32Big();

    const CAnimationParameters& aParms = pInfo.GetAnimationParameters();
    FourCC animType = g_ResFactory->GetResourceTypeById(aParms.GetACSFile());
    if (animType != SBIG('ANCS'))
        return nullptr;

    CAnimRes res(aParms.GetACSFile(), aParms.GetCharacter(), head.x40_scale, true, aParms.GetInitialAnimation());

    return new MP1::CNewIntroBoss(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, res, pInfo, actParms,
                                  f1, w1, dInfo, w2, w3, w4, w5);
}

CEntity* ScriptLoader::LoadSpawnPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 35, "SpawnPoint"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);

    zeus::CVector3f position;
    position.readBig(in);

    zeus::CVector3f rotation;
    rotation.readBig(in);

    std::vector<u32> itemCounts;
    itemCounts.reserve(propCount - 6);
    for (int i = 0; i < propCount - 6; ++i)
        itemCounts.push_back(in.readUint32Big());

    bool defaultSpawn = in.readBool();
    bool active = in.readBool();
    bool morphed = false;
    if (propCount > 34)
        morphed = in.readBool();

    return new CScriptSpawnPoint(mgr.AllocateUniqueId(), name, info,
                                 ConvertEditorEulerToTransform4f(rotation, position), itemCounts, defaultSpawn, active,
                                 morphed);
}

CEntity* ScriptLoader::LoadCameraHint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 25, "CamerHint"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);

    bool active = in.readBool();
    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    u32 pf = LoadParameterFlags(in);
    pf |= in.readBool() << 6;
    float f1 = in.readFloatBig();
    pf |= in.readBool() << 7;
    float f2 = in.readFloatBig();
    pf |= in.readBool() << 8;
    float f3 = in.readFloatBig();
    zeus::CVector3f vec1;
    vec1.readBig(in);
    pf |= in.readBool() << 9;
    zeus::CVector3f vec2;
    vec2.readBig(in);
    zeus::CVector3f vec3;
    vec3.readBig(in);
    pf |= in.readBool() << 10;
    float f4 = in.readFloatBig();
    pf |= in.readBool() << 11;
    float f5 = in.readFloatBig();
    pf |= in.readBool() << 12;
    float f6 = in.readFloatBig();
    pf |= in.readBool() << 13;
    float f7 = in.readFloatBig();
    float f8 = in.readFloatBig();
    float f9 = in.readFloatBig();
    pf |= in.readBool() << 14;
    float f10 = in.readFloatBig();
    float f11 = in.readFloatBig();
    float f12 = in.readFloatBig();
    float f13 = in.readFloatBig();

    return new CScriptCameraHint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, active, w1, w2, pf, f1,
                                 f2, f3, vec1, vec2, vec3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13);
}

CEntity* ScriptLoader::LoadPickup(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 18, "Pickup"))
        return nullptr;

    SScaledActorHead head = LoadScaledActorHead(in, mgr);
    zeus::CVector3f extent = zeus::CVector3f::ReadBig(in);
    zeus::CVector3f offset = zeus::CVector3f::ReadBig(in);
    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    ResId staticModel = in.readUint32Big();
    CAnimationParameters animParms = LoadAnimationParameters(in);
    CActorParameters actorParms = LoadActorParameters(in);
    bool active = in.readBool();
    float f4 = in.readFloatBig();
    u32 w5 = in.readUint32Big();

    FourCC acsType = g_ResFactory->GetResourceTypeById(animParms.GetACSFile());
    if (g_ResFactory->GetResourceTypeById(staticModel) == 0 && acsType == 0)
        return nullptr;

    zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), extent, offset);

    CModelData data;

    if (acsType == SBIG('ANCS'))
        data = CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), head.x40_scale,
                        animParms.GetInitialAnimation(), true);
    else
        data = CStaticRes(staticModel, head.x40_scale);

    if (extent.isZero())
        aabb = data.GetBounds(head.x10_transform.getRotation());

    return new CScriptPickup(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(data),
                             actorParms, aabb, w1, w3, w2, w5, f1, f2, f3, f4, active);
}

CEntity* ScriptLoader::LoadMemoryRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 3, "MemoryRelay") || propCount > 4)
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool b3 = false;
    if (propCount > 3)
        b3 = in.readBool();

    return new CScriptMemoryRelay(mgr.AllocateUniqueId(), name, info, b1, b2, b3);
}

CEntity* ScriptLoader::LoadRandomRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 5, "RandomRelay"))
        return nullptr;
    std::string name = mgr.HashInstanceName(in);
    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    bool b1 = in.readBool();
    bool b2 = in.readBool();

    return new CScriptRandomRelay(mgr.AllocateUniqueId(), name, info, w1, w2, b1, b2);
}

CEntity* ScriptLoader::LoadRelay(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 2, "Relay") || propCount > 3)
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    if (propCount >= 3)
        in.readUint32Big();
    bool b1 = in.readBool();

    return new CScriptRelay(mgr.AllocateUniqueId(), name, info, b1);
}

CEntity* ScriptLoader::LoadBeetle(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 16, "Beetle"))
        return nullptr;
    std::string name = mgr.HashInstanceName(in);
    CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.readUint32Big());
    zeus::CTransform xfrm = LoadEditorTransform(in);
    zeus::CVector3f scale = zeus::CVector3f::ReadBig(in);
    std::pair<bool, u32> pcount = CPatternedInfo::HasCorrectParameterCount(in);
    if (!pcount.first)
        return nullptr;

    CPatternedInfo pInfo(in, pcount.second);
    CActorParameters aParams = LoadActorParameters(in);
    CDamageInfo dInfo(in);
    zeus::CVector3f v1 = zeus::CVector3f::ReadBig(in);
    float f1 = in.readFloatBig();
    CDamageVulnerability dVuln1(in);
    CDamageVulnerability dVuln2(in);
    ResId abdomen = in.readUint32Big();
    MP1::CBeetle::EEntranceType entrance = MP1::CBeetle::EEntranceType(in.readUint32Big());
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();

    FourCC animType = g_ResFactory->GetResourceTypeById(pInfo.GetAnimationParameters().GetACSFile());
    if (animType != SBIG('ANCS'))
        return nullptr;

    rstl::optional_object<CStaticRes> abdomenRes;
    if (flavor == CPatterned::EFlavorType::One)
        abdomenRes.emplace(CStaticRes(abdomen, scale));

    const CAnimationParameters& animParams = pInfo.GetAnimationParameters();
    CAnimRes animRes(animParams.GetACSFile(), animParams.GetCharacter(), scale, animParams.GetInitialAnimation(), true);

    return new MP1::CBeetle(mgr.AllocateUniqueId(), name, info, xfrm, animRes, pInfo, flavor, entrance, dInfo, dVuln2,
                            v1, f2, f3, f1, dVuln1, aParams, abdomenRes);
}

CEntity* ScriptLoader::LoadHUDMemo(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (propCount != 5 && !EnsurePropertyCount(propCount, 6, "HUDMemo"))
        return 0;
    std::string name = mgr.HashInstanceName(in);
    CHUDMemoParms hParms(in);
    CScriptHUDMemo::EDisplayType displayType = CScriptHUDMemo::EDisplayType::MessageBox;
    if (propCount == 6)
        displayType = CScriptHUDMemo::EDisplayType(in.readUint32Big());
    ResId message = in.readUint32Big();
    bool active = in.readBool();

    return new CScriptHUDMemo(mgr.AllocateUniqueId(), name, info, hParms, displayType, message, active);
}

CEntity* ScriptLoader::LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& in, int propCount,
                                                const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 10, "CameraFilterKeyframe"))
        return nullptr;
    std::string name = mgr.HashInstanceName(in);
    bool active = in.readBool();
    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();
    u32 w4 = in.readUint32Big();
    zeus::CColor color;
    color.readRGBABig(in);
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    u32 w5 = in.readUint32Big();

    return new CScriptCameraFilterKeyframe(mgr.AllocateUniqueId(), name, info, w1, w2, w3, w4, color, f1, f2, w5,
                                           active);
}

CEntity* ScriptLoader::LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 7, "CameraBlurKeyframe"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    bool active = in.readBool();
    u32 w1 = in.readUint32Big();
    float f1 = in.readFloatBig();
    u32 w2 = in.readUint32Big();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();

    return new CScriptCameraBlurKeyframe(mgr.AllocateUniqueId(), name, info, w1, f1, w2, f2, f3, active);
}

u32 ClassifyVector(const zeus::CVector3f& dir)
{
    zeus::CVector3f absDir(std::fabs(dir.x), std::fabs(dir.y), std::fabs(dir.z));
    u32 max = (absDir.x > absDir.y ? 0 : 1);
    max = (absDir[max] > absDir.z ? max : 2);

    bool positive = (absDir[max] == dir[max]);
    if (max == 0)
        return (positive ? 0x08 : 0x04);
    else if (max == 1)
        return (positive ? 0x01 : 0x02);
    else if (max == 2)
        return (positive ? 0x10 : 0x20);

    return 0;
}

u32 TransformDamagableTriggerFlags(CStateManager& mgr, TAreaId aId, u32 flags)
{
    CGameArea* area = mgr.WorldNC()->GetGameAreas().at(u32(aId)).get();
    zeus::CTransform rotation = area->GetTransform().getRotation();

    u32 ret = 0;
    if (flags & 0x01)
        ret |= ClassifyVector(rotation * zeus::CVector3f::skForward);
    if (flags & 0x02)
        ret |= ClassifyVector(rotation * zeus::CVector3f::skBack);
    if (flags & 0x04)
        ret |= ClassifyVector(rotation * zeus::CVector3f::skLeft);
    if (flags & 0x08)
        ret |= ClassifyVector(rotation * zeus::CVector3f::skRight);
    if (flags & 0x10)
        ret |= ClassifyVector(rotation * zeus::CVector3f::skUp);
    if (flags & 0x20)
        ret |= ClassifyVector(rotation * zeus::CVector3f::skDown);
    return ret;
}

CEntity* ScriptLoader::LoadDamageableTrigger(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 12, "DamageableTrigger"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f position(zeus::CVector3f::ReadBig(in));
    zeus::CVector3f volume(zeus::CVector3f::ReadBig(in));

    CHealthInfo hInfo(in);
    CDamageVulnerability dVuln(in);
    u32 triggerFlags = in.readUint32Big();
    triggerFlags = TransformDamagableTriggerFlags(mgr, info.GetAreaId(), triggerFlags);
    ResId w1 = in.readUint32Big();
    ResId w2 = in.readUint32Big();
    ResId w3 = in.readUint32Big();
    CScriptDamageableTrigger::ECanOrbit canOrbit = CScriptDamageableTrigger::ECanOrbit(in.readBool());
    bool active = in.readBool();
    CVisorParameters vParms = LoadVisorParameters(in);
    return new CScriptDamageableTrigger(mgr.AllocateUniqueId(), name, info, position, volume, hInfo, dVuln,
                                        triggerFlags, w1, w2, w3, canOrbit, active, vParms);
}

CEntity* ScriptLoader::LoadDebris(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 18, "Debris"))
        return nullptr;

    SScaledActorHead head = LoadScaledActorHead(in, mgr);
    float f1 = in.readFloatBig();
    zeus::CVector3f v1 = zeus::CVector3f::ReadBig(in);
    zeus::CColor color;
    color.readRGBABig(in);
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    float f4 = in.readFloatBig();
    CScriptDebris::EScaleType scaleType = CScriptDebris::EScaleType(in.readUint32Big());
    bool b1 = in.readBool();
    ResId model = in.readUint32Big();
    CActorParameters aParams = LoadActorParameters(in);
    ResId w3 = in.readUint32Big();
    zeus::CVector3f v2 = zeus::CVector3f::ReadBig(in);
    bool b2 = in.readBool();
    bool b3 = in.readBool();

    if (!g_ResFactory->GetResourceTypeById(model))
        return nullptr;
    return new CScriptDebris(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform,
                             CStaticRes(model, head.x40_scale), aParams, w3, v2, f1, v1, color, f2, f3, f4, scaleType,
                             b2, b1, b3);
}

CEntity* ScriptLoader::LoadCameraShaker(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 9, "CameraShaker"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    CCameraShakeData shakeData = CCameraShakeData::LoadCameraShakeData(in);
    bool active = in.readBool();

    return new CScriptCameraShaker(mgr.AllocateUniqueId(), name, info, active, shakeData);
}

CEntity* ScriptLoader::LoadActorKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 7, "ActorKeyframe"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    s32 w1 = in.readInt32Big();
    bool b1 = in.readBool();
    float f1 = in.readFloatBig();
    bool active = in.readBool();
    u32 w2 = in.readUint32Big();
    float f2 = in.readFloatBig();

    if (w1 == -1)
        return nullptr;

    return new CScriptActorKeyframe(mgr.AllocateUniqueId(), name, info, w1, b1, f1, false, w2, active, f2);
}

CEntity* ScriptLoader::LoadWater(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 63, "Water"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f position;
    position.readBig(in);
    zeus::CVector3f extent;
    extent.readBig(in);
    CDamageInfo dInfo(in);
    zeus::CVector3f orientedForce;
    orientedForce.readBig(in);
    ETriggerFlags triggerFlags = ETriggerFlags(in.readUint32Big()) | ETriggerFlags::DetectProjectiles1 |
                                 ETriggerFlags::DetectProjectiles2 | ETriggerFlags::DetectProjectiles3 |
                                 ETriggerFlags::DetectProjectiles4 | ETriggerFlags::DetectBombs |
                                 ETriggerFlags::DetectPowerBombs | ETriggerFlags::DetectProjectiles5 |
                                 ETriggerFlags::DetectProjectiles6 | ETriggerFlags::DetectProjectiles7;
    bool b1 = in.readBool();
    bool displaySurface = in.readBool();
    ResId textureId1 = in.readUint32Big();
    ResId textureId2 = in.readUint32Big();
    ResId textureId3 = in.readUint32Big();
    ResId textureId4 = in.readUint32Big();
    ResId textureId5 = in.readUint32Big();
    ResId textureId6 = in.readUint32Big();
    zeus::CVector3f v2;
    v2.readBig(in);

    zeus::CVector3f otherV2 = v2;
    if (otherV2.canBeNormalized())
        otherV2.assign(0.f, 0.f, -1.f);

    float f1 = 1.f / in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    bool active = in.readBool();
    CFluidPlane::EFluidType fluidType = CFluidPlane::EFluidType(in.readUint32Big());
    bool b4 = in.readBool();
    float f4 = in.readFloatBig();
    CFluidUVMotion fluidMotion = LoadFluidUVMotion(in);

    float f5 = in.readFloatBig();
    float f6 = in.readFloatBig();
    float f7 = in.readFloatBig();
    float f8 = in.readFloatBig();
    float f9 = zeus::degToRad(in.readFloatBig());
    float f10 = zeus::degToRad(in.readFloatBig());
    float f11 = in.readFloatBig();
    float f12 = in.readFloatBig();
    zeus::CColor c1;
    c1.readRGBABig(in);
    zeus::CColor c2;
    c2.readRGBABig(in);
    ResId enterParticle = in.readUint32Big();
    ResId partId2 = in.readUint32Big();
    ResId partId3 = in.readUint32Big();
    ResId partId4 = in.readUint32Big();
    ResId partId5 = in.readUint32Big();
    u32 soundId1 = in.readUint32Big();
    u32 soundId2 = in.readUint32Big();
    u32 soundId3 = in.readUint32Big();
    u32 soundId4 = in.readUint32Big();
    u32 soundId5 = in.readUint32Big();
    float f13 = in.readFloatBig();
    u32 w19 = in.readUint32Big();
    float f14 = in.readFloatBig();
    float f15 = in.readFloatBig();
    float f16 = in.readFloatBig();
    float f17 = in.readFloatBig();
    float f18 = in.readFloatBig();
    float f19 = in.readFloatBig();
    float heatWaveHeight = in.readFloatBig();
    float heatWaveSpeed = in.readFloatBig();
    zeus::CColor heatWaveColor;
    heatWaveColor.readRGBABig(in);
    ResId lightmap = in.readUint32Big();
    float f22 = in.readFloatBig();
    float f23 = in.readFloatBig();
    float f24 = in.readFloatBig();
    u32 w21 = in.readUint32Big();
    u32 w22 = in.readUint32Big();
    bool b5 = in.readBool();

    u32* bitset = nullptr;
    u32 bitVal0 = 0;
    u32 bitVal1 = 0;

    if (b5)
    {
        bitVal0 = in.readUint16Big();
        bitVal1 = in.readUint16Big();
        u32 len = ((bitVal0 * bitVal1) + 31) / 31;
        bitset = new u32[len];
        in.readBytesToBuf(&bitset, len * 4);
    }

    zeus::CAABox box(-extent * 0.5f, extent * 0.5f);

    ResId realTextureId6 = -1;
    if (textureId4 == -1)
        realTextureId6 = textureId6;

    ResId realTextureId5 = -1;
    if (textureId4 == -1)
        realTextureId5 = textureId5;

    return new CScriptWater(
        mgr, mgr.AllocateUniqueId(), name, info, position, box, dInfo, orientedForce, triggerFlags, b1, displaySurface,
        textureId1, textureId2, textureId3, textureId4, realTextureId5, realTextureId6, -1, otherV2, f1, f2, f3, active,
        fluidType, b4, f4, fluidMotion, f5, f6, f7, f8, f9, f10, f11, f12, c1, c2, enterParticle, partId2, partId3,
        partId4, partId5, soundId1, soundId2, soundId3, soundId4, soundId5, f13, w19, f14, f15, f16, f17, f18, f19,
        heatWaveHeight, heatWaveSpeed, heatWaveColor, lightmap, f22, f23, f24, w21, w22, b5, bitVal0, bitVal1, bitset);
}

CEntity* ScriptLoader::LoadWarWasp(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 13, "WarWasp"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    CPatterned::EFlavorType flavor = CPatterned::EFlavorType(in.readUint32Big());
    zeus::CTransform xf = LoadEditorTransformPivotOnly(in);
    zeus::CVector3f scale;
    scale.readBig(in);

    std::pair<bool, u32> verifyPair = CPatternedInfo::HasCorrectParameterCount(in);
    if (!verifyPair.first)
        return nullptr;

    CPatternedInfo pInfo(in, verifyPair.second);
    CActorParameters actorParms = LoadActorParameters(in);
    CPatterned::EColliderType collider = CPatterned::EColliderType(in.readBool());
    CDamageInfo damageInfo1(in);
    ResId weaponDesc = in.readUint32Big();
    CDamageInfo damageInfo2(in);
    ResId particle = in.readUint32Big();
    u32 w1 = in.readUint32Big();

    const CAnimationParameters& aParms = pInfo.GetAnimationParameters();
    FourCC animType = g_ResFactory->GetResourceTypeById(aParms.GetACSFile());
    if (animType != SBIG('ANCS'))
        return nullptr;

    CAnimRes res(aParms.GetACSFile(), aParms.GetCharacter(), scale, true, aParms.GetInitialAnimation());
    CModelData mData(res);
    return new MP1::CWarWasp(mgr.AllocateUniqueId(), name, info, xf, std::move(mData), pInfo, flavor, collider,
                             damageInfo1, actorParms, weaponDesc, damageInfo2, particle, w1);
}

CEntity* ScriptLoader::LoadSpacePirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
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

    if (animParms.GetCharacter() == 0)
    {
        Log.report(logvisor::Warning,
                   "SpacePirate <%s> has AnimationInformation property with invalid character selected",
                   head.x0_name.c_str());
        animParms.SetCharacter(2);
    }

    CModelData mData(CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), head.x40_scale,
                              animParms.GetInitialAnimation(), true));

    return new MP1::CSpacePirate(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, std::move(mData),
                                 aParams, pInfo, in, propCount);
}

CEntity* ScriptLoader::LoadFlyingPirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadElitePirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadMetroidBeta(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadChozoGhost(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadCoverPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 9, "CoverPoint"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);
    bool active = in.readBool();
    u32 flags = in.readUint32Big();
    bool crouch = in.readBool();
    float horizontalAngle = in.readFloatBig();
    float verticalAngle = in.readFloatBig();
    float coverTime = in.readFloatBig();

    return new CScriptCoverPoint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, active, flags, crouch,
                                 horizontalAngle, verticalAngle, coverTime);
}

CEntity* ScriptLoader::LoadSpiderBallWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadBloodFlower(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadFlickerBat(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPathCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadGrapplePoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 5, "GrapplePoint"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CTransform grappleXf = LoadEditorTransform(in);
    bool active = in.readBool();
    CGrappleParameters parameters = LoadGrappleParameters(in);
    return new CScriptGrapplePoint(mgr.AllocateUniqueId(), name, info, grappleXf, active, parameters);
}

CEntity* ScriptLoader::LoadPuddleSpore(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadDebugCameraWaypoint(CStateManager& mgr, CInputStream& in, int propCount,
                                               const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadSpiderBallAttractionSurface(CStateManager& mgr, CInputStream& in, int propCount,
                                                       const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPuddleToadGamma(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadDistanceFog(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 8, "DistanceFog"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    u32 mode = in.readUint32Big();
    zeus::CColor color;
    color.readRGBABig(in);
    zeus::CVector2f range;
    range.readBig(in);
    float colorDelta = in.readFloatBig();
    zeus::CVector2f rangeDelta;
    rangeDelta.readBig(in);
    bool expl = in.readBool();
    bool active = in.readBool();
    ERglFogMode realMode;

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

CEntity* ScriptLoader::LoadFireFlea(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadMetareeAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadDockAreaChange(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 3, "DockAreaChange"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    s32 w1 = in.readInt32Big();
    bool active = in.readBool();

    return new CScriptDockAreaChange(mgr.AllocateUniqueId(), name, info, w1, active);
}

CEntity* ScriptLoader::LoadActorRotate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 6, "ActorRotate"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f rotation = zeus::CVector3f::ReadBig(in);
    float scale = in.readFloatBig();
    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool active = in.readBool();

    return new CScriptActorRotate(mgr.AllocateUniqueId(), name, info, rotation, scale, b1, b2, active);
}

CEntity* ScriptLoader::LoadSpecialFunction(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 15, "SpecialFunction"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);
    CScriptSpecialFunction::ESpecialFunction specialFunction =
        CScriptSpecialFunction::ESpecialFunction(in.readUint32Big());
    std::string str = in.readString();
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();
    u32 w4 = in.readUint32Big();
    bool active1 = in.readBool();
    float f4 = in.readFloatBig();
    s16 w5 = in.readUint32Big() & 0xFFFF;
    s16 w6 = in.readUint32Big() & 0xFFFF;
    s16 w7 = in.readUint32Big() & 0xFFFF;
    if (specialFunction == CScriptSpecialFunction::ESpecialFunction::FogVolume ||
        specialFunction == CScriptSpecialFunction::ESpecialFunction::RadialDamage)
        return nullptr;

    return new CScriptSpecialFunction(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform, specialFunction,
                                      str, f1, f2, f3, f4, zeus::CVector3f::skZero, zeus::CColor::skBlack, active1,
                                      CDamageInfo(), w2, w3, w4, w5, w6, w7);
}

CEntity* ScriptLoader::LoadSpankWeed(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadParasite(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPlayerHint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadRipper(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPickupGenerator(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 4, "PickupGenerator"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f pos = zeus::CVector3f::ReadBig(in);
    bool active = in.readBool();
    float f1 = in.readFloatBig();
    return new CScriptPickupGenerator(mgr.AllocateUniqueId(), name, info, pos, f1, active);
}

CEntity* ScriptLoader::LoadAIKeyframe(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPointOfInterest(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 6, "PointOfInterest"))
        return nullptr;

    SActorHead aHead = LoadActorHead(in, mgr);
    bool active = in.readBool();
    CScannableParameters sParms = LoadScannableParameters(in);
    float f1 = in.readFloatBig();

    return new CScriptPointOfInterest(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, sParms,
                                      f1);
}

CEntity* ScriptLoader::LoadDrone(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadMetroidAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadDebrisExtended(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 39, "DebrisExtended"))
        return nullptr;

    SScaledActorHead aHead = LoadScaledActorHead(in, mgr);

    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    float f4 = in.readFloatBig();
    float f5 = in.readFloatBig();
    float f6 = in.readFloatBig();
    float f7 = in.readFloatBig();
    float f8 = in.readFloatBig();
    float f9 = in.readFloatBig();

    zeus::CColor c1 = zeus::CColor::ReadRGBABig(in);
    zeus::CColor c2 = zeus::CColor::ReadRGBABig(in);

    float f10 = in.readFloatBig();

    zeus::CVector3f v1 = zeus::CVector3f::ReadBig(in);

    float f11 = in.readFloatBig();
    float f12 = in.readFloatBig();

    zeus::CVector3f v2 = zeus::CVector3f::ReadBig(in);

    ResId model = in.readUint32Big();

    CActorParameters aParam = LoadActorParameters(in);

    ResId particle1 = in.readUint32Big();
    zeus::CVector3f particle1Scale = zeus::CVector3f::ReadBig(in);
    bool particle1B1 = in.readBool();
    bool particle1B2 = in.readBool();
    CScriptDebris::EOrientationType particle1W = CScriptDebris::EOrientationType(in.readUint32Big());

    ResId particle2 = in.readUint32Big();
    zeus::CVector3f particle2Scale = zeus::CVector3f::ReadBig(in);
    bool particle2B1 = in.readBool();
    bool particle2B2 = in.readBool();
    CScriptDebris::EOrientationType particle2W = CScriptDebris::EOrientationType(in.readUint32Big());

    ResId particle3 = in.readUint32Big();
    zeus::CVector3f particle3Scale = zeus::CVector3f::ReadBig(in);
    CScriptDebris::EOrientationType particle3W = CScriptDebris::EOrientationType(in.readUint32Big());

    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();
    bool b4 = in.readBool();

    CModelData modelData;
    if (g_ResFactory->GetResourceTypeById(model))
        modelData = CModelData(CStaticRes(model, aHead.x40_scale));

    return new CScriptDebris(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform,
                             std::move(modelData), aParam, f1, f2, f3, f4, f5, f6, f7, f8, f9,
                             c1, c2, f10, aHead.x40_scale, v1, f11, f12, v2,
                             particle1, particle1Scale, particle1B1, particle1B2, particle1W,
                             particle2, particle2Scale, particle2B1, particle2B2, particle2W,
                             particle3, particle3Scale, particle3W, b1, b2, b3, b4);
}

CEntity* ScriptLoader::LoadSteam(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 11, "Steam"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);

    zeus::CVector3f v1 = zeus::CVector3f::ReadBig(in);
    zeus::CVector3f v2 = zeus::CVector3f::ReadBig(in);

    CDamageInfo dInfo(in);

    zeus::CVector3f v3 = zeus::CVector3f::ReadBig(in);

    ETriggerFlags w1 = ETriggerFlags(in.readUint32Big());
    bool b1 = in.readBool();
    u32 w2 = in.readUint32Big();
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    float f4 = in.readFloatBig();
    bool b2 = in.readBool();

    zeus::CAABox aabb(-v2 * 0.5f, v2 * 0.5f);

    return new CScriptSteam(mgr.AllocateUniqueId(), name, info, v1, aabb, dInfo,
                            v3, w1, b1, w2, f1, f2, f3, f4, b2);
}

CEntity* ScriptLoader::LoadRipple(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadBallTrigger(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadTargetingPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadEMPulse(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadIceSheegoth(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPlayerActor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 19, "PlayerActor"))
        return nullptr;

    SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
    zeus::CVector3f extents = zeus::CVector3f::ReadBig(in);
    zeus::CVector3f offset = zeus::CVector3f::ReadBig(in);
    float mass = in.readFloatBig();
    float zMomentum = in.readFloatBig();
    CHealthInfo hInfo(in);
    CDamageVulnerability dVuln(in);
    in.readUint32Big();
    CAnimationParameters animParms = LoadAnimationParameters(in);
    CActorParameters actParms = LoadActorParameters(in);
    bool loop = in.readBool();
    bool snow = in.readBool();
    bool solid = in.readBool();
    bool active = in.readBool();
    u32 flags = LoadParameterFlags(in);
    bool w1 = in.readUint32Big() - 1;

    FourCC fcc = g_ResFactory->GetResourceTypeById(animParms.GetACSFile());
    if (!fcc || fcc != SBIG('ANCS'))
        return nullptr;

    zeus::CAABox aabox = GetCollisionBox(mgr, info.GetAreaId(), extents, offset);
    CMaterialList list;
    if (snow)
        list.Add(EMaterialTypes::Snow);

    if (solid)
        list.Add(EMaterialTypes::Solid);

    if ((extents.x < 0.f || extents.y < 0.f || extents.z < 0.f) || extents.isZero())
        aabox = zeus::CAABox({-.5f}, {0.5f});

    return new CScriptPlayerActor(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform,
                                  CAnimRes(animParms.GetACSFile(), animParms.GetCharacter(), aHead.x40_scale,
                                           animParms.GetInitialAnimation(), loop), CModelData::CModelDataNull(),
                                  aabox, true, list, mass, zMomentum, hInfo, dVuln, actParms, loop, active, flags, w1);
}

CEntity* ScriptLoader::LoadFlaahgra(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadAreaAttributes(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 9, "AreaAttributes"))
        return nullptr;

    bool load = in.readUint32Big() != 0;
    if (!load)
        return nullptr;

    bool showSkybox = in.readBool();
    EEnvFxType fxType = EEnvFxType(in.readUint32Big());
    float envFxDensity = in.readFloatBig();
    float thermalHeat = in.readFloatBig();
    float xrayFogDistance = in.readFloatBig();
    float worldLightingLevel = in.readFloatBig();
    ResId skybox = in.readUint32Big();
    EPhazonType phazonType = EPhazonType(in.readUint32Big());

    return new CScriptAreaAttributes(mgr.AllocateUniqueId(), info, showSkybox, fxType, envFxDensity, thermalHeat,
                                     xrayFogDistance, worldLightingLevel, skybox, phazonType);
}

CEntity* ScriptLoader::LoadFishCloud(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadFishCloudModifier(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadVisorFlare(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 14, "VisorFlare"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f pos = zeus::CVector3f::ReadBig(in);
    bool b1 = in.readBool();
    CVisorFlare::EBlendMode w1 = CVisorFlare::EBlendMode(in.readUint32Big());
    bool b2 = in.readBool();
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    u32 w2 = in.readUint32Big();
    std::vector<CVisorFlare::CFlareDef> flares;
    flares.reserve(5);
    for (int i=0 ; i<5 ; ++i)
        if (auto flare = CVisorFlare::LoadFlareDef(in))
            flares.push_back(*flare);

    return new CScriptVisorFlare(mgr.AllocateUniqueId(), name, info, b1, pos, w1, b2, f1, f2, f3, 2, w2, flares);
}

CEntity* ScriptLoader::LoadWorldTeleporter(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadVisorGoo(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadJellyZap(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadControllerAction(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 4, "ControllerAction") || propCount > 6)
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    bool active = in.readBool();
    ControlMapper::ECommands w1 = ControlMapper::ECommands(in.readUint32Big());
    bool b1 = false;
    u32 w2 = 0;
    if (propCount == 6)
    {
        b1 = in.readBool();
        w2 = in.readUint32Big();
    }
    bool b2 = in.readBool();

    return new CScriptControllerAction(mgr.AllocateUniqueId(), name, info, active, w1, b1, w2, b2);
}

CEntity* ScriptLoader::LoadSwitch(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 4, "Switch"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    bool active = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();

    return new CScriptSwitch(mgr.AllocateUniqueId(), name, info, active, b2, b3);
}

CEntity* ScriptLoader::LoadPlayerStateChange(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadThardus(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadWallCrawlerSwarm(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 39, "WallCrawlerSwarm"))
        return nullptr;

    SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
    bool active = in.readBool();
    CActorParameters aParams = LoadActorParameters(in);
    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();
    u32 w4 = in.readUint32Big();
    u32 w5 = in.readUint32Big();
    u32 w6 = in.readUint32Big();
    u32 w7 = in.readUint32Big();
    u32 w8 = in.readUint32Big();
    u32 w9 = in.readUint32Big();
    u32 w10 = in.readUint32Big();
    CDamageInfo dInfo1(in);
    float f1 = in.readFloatBig();
    CDamageInfo dInfo2(in);
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    float f4 = in.readFloatBig();
    float f5 = in.readFloatBig();
    u32 w11 = in.readUint32Big();
    u32 w12 = in.readUint32Big();
    float f6 = in.readFloatBig();
    float f7 = in.readFloatBig();
    float f8 = in.readFloatBig();
    float f9 = in.readFloatBig();
    float f10 = in.readFloatBig();
    float f11 = in.readFloatBig();
    float f12 = in.readFloatBig();
    float f13 = in.readFloatBig();
    u32 w13 = in.readUint32Big();
    float f14 = in.readFloatBig();
    float f15 = in.readFloatBig();
    float f16 = in.readFloatBig();
    CHealthInfo hInfo(in);
    CDamageVulnerability dVulns(in);
    u32 w14 = in.readUint32Big();
    u32 w15 = in.readUint32Big();

    return new CWallCrawlerSwarm(mgr.AllocateUniqueId(), active, aHead.x0_name, info, aHead.x40_scale,
                                 aHead.x10_transform, w1, CAnimRes(w2, w3, {1.5f}, w4, true), w5, w6, w7, w8, w9, w10,
                                 dInfo1, dInfo2, f1, f2, f3, f4, w11, w12, f5, f6, f7, f8, f9, f10, f11, f12, f13, w13,
                                 f14, f15, f16, hInfo, dVulns, w14, w15, aParams);
}

CEntity* ScriptLoader::LoadAiJumpPoint(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 5, "AiJumpPoint"))
        return nullptr;

    SActorHead aHead = LoadActorHead(in, mgr);
    bool active = in.readBool();
    float f1 = in.readFloat();

    return new CScriptAiJumpPoint(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, f1);
}

CEntity* ScriptLoader::LoadFlaahgraTentacle(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadRoomAcoustics(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 32, "RoomAcoustics"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    bool a = in.readBool();
    u32 b = in.readUint32Big();
    bool c = in.readBool();
    bool d = in.readBool();
    float e = in.readFloatBig();
    float f = in.readFloatBig();
    float g = in.readFloatBig();
    float h = in.readFloatBig();
    float i = in.readFloatBig();
    float j = in.readFloatBig();
    bool k = in.readBool();
    float l = in.readFloatBig();
    float m = in.readFloatBig();
    float n = in.readFloatBig();
    bool o = in.readBool();
    bool p = in.readBool();
    float q = in.readFloatBig();
    float r = in.readFloatBig();
    float s = in.readFloatBig();
    float t = in.readFloatBig();
    float u = in.readFloatBig();
    bool v = in.readBool();
    u32 w = in.readUint32Big();
    u32 x = in.readUint32Big();
    u32 y = in.readUint32Big();
    u32 z = in.readUint32Big();
    u32 _a = in.readUint32Big();
    u32 _b = in.readUint32Big();
    u32 _c = in.readUint32Big();
    u32 _d = in.readUint32Big();
    u32 _e = in.readUint32Big();

    return new CScriptRoomAcoustics(mgr.AllocateUniqueId(), name, info, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p,
                                    q, r, s, t, u, v, w, x, y, z, _a, _b, _c, _d, _e);
}

CEntity* ScriptLoader::LoadColorModulate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 12, "ColorModulate"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CColor c1;
    c1.readRGBABig(in);
    zeus::CColor c2;
    c2.readRGBABig(in);
    CScriptColorModulate::EBlendMode bm = CScriptColorModulate::EBlendMode(in.readUint32Big());
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();
    bool b4 = in.readBool();
    bool b5 = in.readBool();
    bool active = in.readBool();
    return new CScriptColorModulate(mgr.AllocateUniqueId(), name, info, c1, c2, bm, f1, f2, b1, b2, b3, b4, b5, active);
}

CEntity* ScriptLoader::LoadThardusRockProjectile(CStateManager& mgr, CInputStream& in, int propCount,
                                                 const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadMidi(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 6, "Midi"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    bool active = in.readBool();
    u32 csng = in.readUint32Big();
    float fadeIn = in.readFloatBig();
    float fadeOut = in.readFloatBig();
    u32 vol = in.readUint32Big();
    return new CScriptMidi(mgr.AllocateUniqueId(), info, name, active, csng, fadeIn, fadeOut, vol);
}

CEntity* ScriptLoader::LoadStreamedAudio(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 9, "StreamedAudio"))
        return nullptr;

    const std::string name = mgr.HashInstanceName(in);
    bool active = in.readBool();
    std::string fileName = in.readString();
    bool noStopOnDeactivate = in.readBool();
    float fadeIn = in.readFloatBig();
    float fadeOut = in.readFloatBig();
    u32 volume = in.readUint32Big();
    u32 oneShot = in.readUint32Big();
    bool music = in.readBool();

    return new CScriptStreamedMusic(mgr.AllocateUniqueId(), info, name, active, fileName, noStopOnDeactivate, fadeIn,
                                    fadeOut, volume, !oneShot, music);
}

CEntity* ScriptLoader::LoadRepulsor(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 4, "Repulsor"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f center = in.readVec3fBig();
    bool active = in.readBool();
    float radius = in.readFloatBig();

    return new CRepulsor(mgr.AllocateUniqueId(), active, name, info, center, radius);
}

CEntity* ScriptLoader::LoadGunTurret(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadFogVolume(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadBabygoth(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadEyeball(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadRadialDamage(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 5, "RadialDamage"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f center = zeus::CVector3f::ReadBig(in);
    bool active = in.readBool();
    CDamageInfo dInfo(in);
    float radius = in.readFloatBig();
    zeus::CTransform xf = ConvertEditorEulerToTransform4f(zeus::CVector3f::skZero, center);

    return new CScriptSpecialFunction(
        mgr.AllocateUniqueId(), name, info, xf, CScriptSpecialFunction::ESpecialFunction::RadialDamage, "", radius, 0.f,
        0.f, 0.f, zeus::CVector3f::skZero, zeus::CColor::skBlack, active, dInfo, -1, -1, -1, -1, -1, -1);
}

CEntity* ScriptLoader::LoadCameraPitchVolume(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 8, "CameraPitchVolume"))
        return nullptr;

    SScaledActorHead aHead = LoadScaledActorHead(in, mgr);
    bool active = in.readBool();
    zeus::CRelAngle f1 = zeus::CRelAngle::FromDegrees(in.readFloatBig());
    zeus::CRelAngle f2 = zeus::CRelAngle::FromDegrees(in.readFloatBig());
    float f3 = in.readFloatBig();

    return new CScriptCameraPitchVolume(mgr.AllocateUniqueId(), active, aHead.x0_name, info, aHead.x40_scale,
                                        aHead.x10_transform, f1, f2, f3);
}

CEntity* ScriptLoader::LoadEnvFxDensityController(CStateManager& mgr, CInputStream& in, int propCount,
                                                  const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadMagdolite(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadTeamAIMgr(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadSnakeWeedSwarm(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::Load(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadActorContraption(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info)
{

    return nullptr;
}

CEntity* ScriptLoader::LoadOculus(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadGeemer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadSpindleCamera(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadAtomicAlpha(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadCameraHintTrigger(CStateManager& mgr, CInputStream& in, int propCount,
                                             const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 7, "CameraHintTrigger"))
        return nullptr;

    SActorHead aHead = LoadActorHead(in, mgr);
    zeus::CVector3f scale = 0.5f * zeus::CVector3f::ReadBig(in);
    bool active = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();

    zeus::CTransform xfRot = aHead.x10_transform.getRotation();
    if (xfRot == zeus::CTransform::Identity())
        return new CScriptTrigger(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform.origin,
                                  zeus::CAABox(-scale, scale), CDamageInfo(), zeus::CVector3f::skZero,
                                  ETriggerFlags::DetectPlayer, active, b2, b3);

    return new CScriptCameraHintTrigger(mgr.AllocateUniqueId(), active, aHead.x0_name, info, scale, aHead.x10_transform,
                                        b2, b3);
}

CEntity* ScriptLoader::LoadRumbleEffect(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadAmbientAI(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadAtomicBeta(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadIceZoomer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPuffer(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadTryclops(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadRidley(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadSeedling(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadThermalHeatFader(CStateManager& mgr, CInputStream& in, int propCount,
                                            const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 4, "ThermalHeatFader"))
        return nullptr;

    return LoadWorldLightFader(mgr, in, propCount, info);
}

CEntity* ScriptLoader::LoadBurrower(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadBeam(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 7, "Beam"))
        return nullptr;

    SActorHead aHead = LoadActorHead(in, mgr);
    bool active = in.readBool();
    u32 weaponDescId = in.readUint32Big();
    if (!g_ResFactory->GetResourceTypeById(weaponDescId))
        return nullptr;

    CBeamInfo beamInfo(in);
    CDamageInfo dInfo(in);
    TToken<CWeaponDescription> weaponDesc = g_SimplePool->GetObj({SBIG('WPSC'), weaponDescId});

    return new CScriptBeam(mgr.AllocateUniqueId(), aHead.x0_name, info, aHead.x10_transform, active, weaponDesc,
                           beamInfo, dInfo);
}

CEntity* ScriptLoader::LoadWorldLightFader(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 4, "WorldLightFader"))
        return nullptr;

    return nullptr;
}

CEntity* ScriptLoader::LoadMetroidPrimeStage2(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadMetroidPrimeStage1(CStateManager& mgr, CInputStream& in, int propCount,
                                              const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadMazeNode(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadOmegaPirate(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPhazonPool(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadPhazonHealingNodule(CStateManager& mgr, CInputStream& in, int propCount,
                                               const CEntityInfo& info)
{
    return nullptr;
}

CEntity* ScriptLoader::LoadNewCameraShaker(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 8, "NewCameraShaker"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f v1 = zeus::CVector3f::ReadBig(in);
    bool active = in.readBool();
    u32 flags = LoadParameterFlags(in);
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    CCameraShakerComponent shaker1 = CCameraShakerComponent::LoadNewCameraShakerComponent(in);
    CCameraShakerComponent shaker2 = CCameraShakerComponent::LoadNewCameraShakerComponent(in);
    CCameraShakerComponent shaker3 = CCameraShakerComponent::LoadNewCameraShakerComponent(in);

    CCameraShakeData shakeData(f1, f2, flags, v1, shaker1, shaker2, shaker3);

    return new CScriptCameraShaker(mgr.AllocateUniqueId(), name, info, active, shakeData);
}

CEntity* ScriptLoader::LoadShadowProjector(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 10, "ShadowProjector"))
        return nullptr;

    std::string name = mgr.HashInstanceName(in);
    zeus::CVector3f position(zeus::CVector3f::ReadBig(in));
    bool b1 = in.readBool();
    float f1 = in.readFloatBig();
    zeus::CVector3f vec2(zeus::CVector3f::ReadBig(in));
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    float f4 = in.readFloatBig();
    bool b2 = in.readBool();
    u32 w1 = in.readUint32Big();
    return new CScriptShadowProjector(mgr.AllocateUniqueId(), name, info, zeus::CTransform::Translate(position), b1,
                                      vec2, b2, f1, f2, f3, f4, w1);
}

CEntity* ScriptLoader::LoadEnergyBall(CStateManager& mgr, CInputStream& in, int propCount, const CEntityInfo& info)
{
    return nullptr;
}
}
