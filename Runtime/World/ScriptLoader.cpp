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
#include "CDamageInfo.hpp"
#include "CScriptActor.hpp"
#include "CScriptWaypoint.hpp"
#include "CScriptDoor.hpp"
#include "CScriptTrigger.hpp"
#include "CScriptTimer.hpp"
#include "CScriptCounter.hpp"
#include "CScriptDock.hpp"
#include "CScriptWater.hpp"
#include "CScriptEffect.hpp"
#include "CScriptPlatform.hpp"
#include "CScriptSound.hpp"
#include "CScriptGenerator.hpp"
#include "CScriptGrapplePoint.hpp"
#include "CSimplePool.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"
#include "Editor/ProjectResourceFactoryMP1.hpp"
#include "logvisor/logvisor.hpp"

namespace urde
{
static logvisor::Module Log("urde::ScriptLoader");

static SObjectTag MorphballDoorANCS = {};
static const SObjectTag& GetMorphballDoorANCS()
{
    if (!MorphballDoorANCS)
        MorphballDoorANCS = static_cast<ProjectResourceFactoryBase*>(g_ResFactory)->
            TagFromPath(_S("MP1/Shared/ANCS_1F9DA858.blend"));
    return MorphballDoorANCS;
}

static bool EnsurePropertyCount(int count, int expected, const char* structName)
{
    if (count < expected)
    {
        Log.report(logvisor::Warning, "Insufficient number of props (%d/%d) for %s entity",
                   count, expected, structName);
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
    ret.x0_name = *stateMgr.HashInstanceName(in);
    ret.x10_transform = LoadEditorTransform(in);
    return ret;
}

static SScaledActorHead LoadScaledActorHead(CInputStream& in, CStateManager& stateMgr)
{
    SScaledActorHead ret = LoadActorHead(in, stateMgr);
    ret.x40_scale.readBig(in);
    return ret;
}

static zeus::CAABox GetCollisionBox(CStateManager& stateMgr, TAreaId id,
                                    const zeus::CVector3f& extent, const zeus::CVector3f& offset)
{
    zeus::CAABox box(-extent * 0.5f + offset, extent * 0.5f + offset);
    const zeus::CTransform& rot = stateMgr.GetWorld()->GetGameAreas()[id]->GetTransform().getRotation();
    return box.getTransformedAABox(rot);
}

u32 ScriptLoader::LoadParameterFlags(CInputStream& in)
{
    u32 count = in.readUint32Big();
    u32 ret = 0;
    for (u32 i=0 ; i<count ; ++i)
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

        bool b2 = false;
        if (propCount > 10)
            b2 = in.readBool();

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

        return CActorParameters(lParms, sParams, xray, infra, vParms, b1, b2, b3, b4);
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
        u32 c = in.readUint32Big();
        float d = in.readFloatBig();
        float e = in.readFloatBig();

        zeus::CColor col;
        col.readRGBABig(in);

        bool f = in.readBool();
        u32 g = in.readUint32Big();
        u32 h = in.readUint32Big();

        zeus::CVector3f vec;
        vec.readBig(in);

        s32 w1 = -1;
        s32 w2 = -1;
        if (propCount >= 12)
        {
            w1 = in.readUint32Big();
            w2 = in.readUint32Big();
        }

        bool b1 = false;
        if (propCount >= 13)
            b1 = in.readBool();

        s32 w3 = 0;
        if (propCount >= 14)
            w3 = in.readUint32Big();

        return CLightParameters(a, b, c, d, e, col, f, g, h, vec, w1, w2, b1, w3);
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
    CFluidUVMotion::EFluidUVMotion motion = CFluidUVMotion::EFluidUVMotion(in.readUint32Big());
    float a = in.readFloatBig();
    float b = in.readFloatBig();
    b = zeus::degToRad(b) - M_PIF;
    float c = in.readFloatBig();
    float d = in.readFloatBig();
    CFluidUVMotion::SFluidLayerMotion motionLayer2(motion, a, b, c, d);

    motion = CFluidUVMotion::EFluidUVMotion(in.readUint32Big());
    a = in.readFloatBig();
    b = in.readFloatBig();
    b = zeus::degToRad(b) - M_PIF;
    c = in.readFloatBig();
    d = in.readFloatBig();
    CFluidUVMotion::SFluidLayerMotion motionLayer3(motion, a, b, c, d);

    motion = CFluidUVMotion::EFluidUVMotion(in.readUint32Big());
    a = in.readFloatBig();
    b = in.readFloatBig();
    b = zeus::degToRad(b) - M_PIF;
    c = in.readFloatBig();
    d = in.readFloatBig();
    CFluidUVMotion::SFluidLayerMotion motionLayer1(motion, a, b, c, d);

    a = in.readFloatBig();
    b = in.readFloatBig();

    b = zeus::degToRad(b) - M_PIF;

    return CFluidUVMotion(a, b, motionLayer1, motionLayer2, motionLayer3);
}

zeus::CTransform ScriptLoader::ConvertEditorEulerToTransform4f(const zeus::CVector3f& orientation,
                                                               const zeus::CVector3f& position)
{
    return zeus::CTransform::RotateZ(zeus::degToRad(orientation.z)) *
           zeus::CTransform::RotateY(zeus::degToRad(orientation.y)) *
           zeus::CTransform::RotateX(zeus::degToRad(orientation.x)) + position;
}

CEntity* ScriptLoader::LoadActor(CStateManager& mgr, CInputStream& in,
                                 int propCount, const CEntityInfo& info)
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
    bool b2 = in.readBool();
    bool b3 = in.readBool();
    bool b4 = in.readBool();
    bool b5 = in.readBool();
    u32 w2 = in.readUint32Big();
    float f3 = in.readFloatBig();
    bool b6 = in.readBool();
    bool b7 = in.readBool();
    bool b8 = in.readBool();
    bool b9 = in.readBool();

    FourCC animType = g_ResFactory->GetResourceTypeById(aParms.x0_ancs);
    if (!g_ResFactory->GetResourceTypeById(staticId) && !animType)
        return nullptr;

    zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, centroid);

    CMaterialList list;
    if (b2)
        list.x0_ = 0x80000000000;

    if (b3)
        list.x0_ |= 8;

    if (b4)
        list.x0_ |= 32;

    bool generateExtent = false;
    if (collisionExtent.x < 0.f || collisionExtent.y < 0.f || collisionExtent.z < 0.f)
        generateExtent = true;

    CModelData data;
    if (animType == SBIG('ANCS'))
    {
        CAnimRes aRes;
        aRes.x0_ancsId = aParms.x0_ancs;
        aRes.x4_charIdx = aParms.x4_charIdx;
        aRes.x8_scale = head.x40_scale;
        aRes.x14_ = true;
        aRes.x1c_defaultAnim = aParms.x8_defaultAnim;
        data = aRes;
    }
    else
    {
        CStaticRes sRes;
        sRes.x0_cmdlId = staticId;
        sRes.x4_scale = head.x40_scale;
        data = sRes;
    }

    if (generateExtent || collisionExtent.isZero())
        aabb = data.GetBounds(head.x10_transform.getRotation());

    return new CScriptActor(mgr.AllocateUniqueId(), head.x0_name, info,
                            head.x10_transform, data, aabb, f1, f2, list, hInfo, dInfo,
                            actParms, b1, b5, w2, f3, b6, b7, b8, b9);
}

CEntity* ScriptLoader::LoadWaypoint(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 13, "Waypoint"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);

    bool b1 = in.readBool();
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();
    u32 w4 = in.readUint32Big();
    u32 w5 = in.readUint32Big();
    u32 w6 = in.readUint32Big();
    u32 w7 = in.readUint32Big();

    return new CScriptWaypoint(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform,
                               b1, f1, f2, w1, w2, w3, w4, w5, w6, w7);
}

CEntity* ScriptLoader::LoadDoor(CStateManager& mgr, CInputStream& in,
                                int propCount, const CEntityInfo& info)
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

    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();
    float f1 = in.readFloatBig();

    zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), collisionExtent, offset);

    if (!g_ResFactory->GetResourceTypeById(aParms.x0_ancs))
        return nullptr;

    CAnimRes aRes;
    aRes.x0_ancsId = aParms.x0_ancs;
    aRes.x4_charIdx = aParms.x4_charIdx;
    aRes.x1c_defaultAnim = aParms.x8_defaultAnim;
    aRes.x8_scale = head.x40_scale;

    CModelData mData = aRes;
    if (collisionExtent.isZero())
        aabb = mData.GetBounds(head.x10_transform.getRotation());

    bool isMorphballDoor = false;
    if (propCount == 13)
    {
        if (aParms.x0_ancs == GetMorphballDoorANCS().id)
            isMorphballDoor = true;
    }
    else if (propCount == 14)
        isMorphballDoor = in.readBool();

    return new CScriptDoor(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform,
                           mData, actParms, v1, aabb, b1, b2, b3, f1, isMorphballDoor);
}

CEntity* ScriptLoader::LoadTrigger(CStateManager& mgr, CInputStream& in,
                                   int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 9, "Trigger"))
        return nullptr;

    const std::string* name = mgr.HashInstanceName(in);

    zeus::CVector3f position;
    position.readBig(in);

    zeus::CVector3f extent;
    extent.readBig(in);

    CDamageInfo dInfo(in);

    zeus::CVector3f forceVec;
    forceVec.readBig(in);

    u32 w1 = in.readUint32Big();
    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();

    zeus::CAABox box(-extent * 0.5f, extent * 0.5f);

    const zeus::CTransform& areaXf = mgr.GetWorld()->GetGameAreas()[info.GetAreaId()]->GetTransform();
    zeus::CVector3f orientedForce = areaXf.m_basis * forceVec;

    return new CScriptTrigger(mgr.AllocateUniqueId(), *name, info, position, box, dInfo,
                              orientedForce, w1, b1, b2, b3);
}

CEntity* ScriptLoader::LoadTimer(CStateManager& mgr, CInputStream& in,
                                 int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 6, "Timer"))
        return nullptr;

    const std::string* name = mgr.HashInstanceName(in);

    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    bool b1 = in.readBool();
    bool b2 = in.readBool();
    bool b3 = in.readBool();

    return new CScriptTimer(mgr.AllocateUniqueId(), *name, info, f1, f2, b1, b2, b3);
}

CEntity* ScriptLoader::LoadCounter(CStateManager& mgr, CInputStream& in,
                                   int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 5, "Counter"))
        return nullptr;

    const std::string* name = mgr.HashInstanceName(in);

    u32 w1 = in.readUint32Big();
    u32 w2 = in.readUint32Big();
    bool b1 = in.readBool();
    bool b2 = in.readBool();

    return new CScriptCounter(mgr.AllocateUniqueId(), *name, info, w1, w2, b1, b2);
}

CEntity* ScriptLoader::LoadEffect(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
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

    if (!g_ResFactory->GetResourceTypeById(partId) &&
        !g_ResFactory->GetResourceTypeById(elscId))
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

    return new CScriptEffect(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform,
                             head.x40_scale, partId, elscId, b1, b2, b3, b4, b5, f1, f2, f3, f4,
                             b6, f5, f6, f7, b7, b8, b9, lParms, b10);
}

CEntity* ScriptLoader::LoadPlatform(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
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

    FourCC animType = g_ResFactory->GetResourceTypeById(aParms.x0_ancs);
    if (!g_ResFactory->GetResourceTypeById(staticId) && !animType)
        return nullptr;

    zeus::CAABox aabb = GetCollisionBox(mgr, info.GetAreaId(), extent, centroid);

    FourCC dclnType = g_ResFactory->GetResourceTypeById(dclnId);
    TLockedToken<CCollidableOBBTreeGroup> dclnToken;
    if (dclnType)
    {
         dclnToken = g_SimplePool->GetObj({SBIG('DCLN'), dclnId});
         dclnToken.GetObj();
    }

    CModelData data;
    if (animType == SBIG('ANCS'))
    {
        CAnimRes aRes;
        aRes.x0_ancsId = aParms.x0_ancs;
        aRes.x4_charIdx = aParms.x4_charIdx;
        aRes.x8_scale = head.x40_scale;
        aRes.x14_ = true;
        aRes.x1c_defaultAnim = aParms.x8_defaultAnim;
        data = aRes;
    }
    else
    {
        CStaticRes sRes;
        sRes.x0_cmdlId = staticId;
        sRes.x4_scale = head.x40_scale;
        data = sRes;
    }

    if (extent.isZero())
        aabb = data.GetBounds(head.x10_transform.getRotation());

    return new CScriptPlatform(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform,
                               data, actParms, aabb, f1, b2, f2, b1, hInfo, dInfo, dclnToken, b3, w2, w3);
}

CEntity* ScriptLoader::LoadSound(CStateManager& mgr, CInputStream& in,
                                 int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 20, "Sound"))
        return nullptr;

    SActorHead head = LoadActorHead(in, mgr);

    s32 soundId = in.readInt32Big();
    bool b1 = in.readBool();
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();
    float f3 = in.readFloatBig();
    u32 w2 = in.readUint32Big();
    u32 w3 = in.readUint32Big();
    u32 w4 = in.readUint32Big();
    u32 w5 = in.readUint32Big();
    bool b2 = in.readBool();
    bool b3 = in.readBool();
    bool b4 = in.readBool();
    bool b5 = in.readBool();
    bool b6 = in.readBool();
    bool b7 = in.readBool();
    bool b8 = in.readBool();
    u32 w6 = in.readUint32Big();

    if (soundId < 0)
        return nullptr;

    return new CScriptSound(mgr.AllocateUniqueId(), head.x0_name, info, head.x10_transform,
                            soundId, b1, f1, f2, f3, w2, w3, w4, w5, w6, 0, b2, b3, b4, b5, b6, b7, b8, w6);
}

CEntity* ScriptLoader::LoadGenerator(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 8, "Generator"))
        return nullptr;

    const std::string* name = mgr.HashInstanceName(in);

    u32 w1 = in.readUint32Big();
    bool b1 = in.readBool();
    bool b2 = in.readBool();

    zeus::CVector3f v1;
    v1.readBig(in);

    bool b3 = in.readBool();
    float f1 = in.readFloatBig();
    float f2 = in.readFloatBig();

    return new CScriptGenerator(mgr.AllocateUniqueId(), *name, info, w1, b1, v1, b2, b3, f1, f2);
}

CEntity* ScriptLoader::LoadDock(CStateManager& mgr, CInputStream& in,
                                int propCount, const CEntityInfo& info)
{    
    if (!EnsurePropertyCount(propCount, 7, "Dock"))
        return nullptr;

    std::string name = *mgr.HashInstanceName(in);
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

CEntity* ScriptLoader::LoadCamera(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraWaypoint(CStateManager& mgr, CInputStream& in,
                                          int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadNewIntroBoss(CStateManager& mgr, CInputStream& in,
                                        int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpawnPoint(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraHint(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPickup(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMemoryRelay(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRandomRelay(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRelay(CStateManager& mgr, CInputStream& in,
                                 int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBeetle(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadHUDMemo(CStateManager& mgr, CInputStream& in,
                                   int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraFilterKeyframe(CStateManager& mgr, CInputStream& in,
                                                int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraBlurKeyframe(CStateManager& mgr, CInputStream& in,
                                              int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDamageableTrigger(CStateManager& mgr, CInputStream& in,
                                             int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDebris(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraShaker(CStateManager& mgr, CInputStream& in,
                                        int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadActorKeyframe(CStateManager& mgr, CInputStream& in,
                                         int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWater(CStateManager& mgr, CInputStream& in,
                                 int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 63, "Water"))
        return nullptr;

    std::string name = *mgr.HashInstanceName(in);
    zeus::CVector3f position;
    position.readBig(in);
    zeus::CVector3f extent;
    extent.readBig(in);
    CDamageInfo dInfo(in);
    zeus::CVector3f orientedForce;
    orientedForce.readBig(in);
    u32 triggerFlags = in.readUint32Big() | 2044;
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
    u32  bitVal0 = 0;
    u32  bitVal1 = 0;

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

    return new CScriptWater(mgr, mgr.AllocateUniqueId(), name, info, position, box, dInfo, orientedForce, triggerFlags, b1, displaySurface,
                            textureId1, textureId2, textureId3, textureId4, realTextureId5, realTextureId6, -1, otherV2, f1, f2,
                            f3, active, fluidType, b4, f4, fluidMotion, f5, f6, f7, f8, f9, f10, f11, f12, c1, c2, enterParticle,
                            partId2, partId3, partId4, partId5, soundId1, soundId2, soundId3, soundId4, soundId5,
                            f13, w19, f14, f15, f16, f17, f18, f19, heatWaveHeight, heatWaveSpeed, heatWaveColor, lightmap, f22, f23, f24,
                            w21, w22, b5, bitVal0, bitVal0, bitset);
}

CEntity* ScriptLoader::LoadWarwasp(CStateManager& mgr, CInputStream& in,
                                   int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpacePirate(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFlyingPirate(CStateManager& mgr, CInputStream& in,
                                        int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadElitePirate(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetroidBeta(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadChozoGhost(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCoverPoint(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpiderBallWaypoint(CStateManager& mgr, CInputStream& in,
                                              int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBloodFlower(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFlickerBat(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPathCamera(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadGrapplePoint(CStateManager& mgr, CInputStream& in,
                                        int propCount, const CEntityInfo& info)
{
    if (!EnsurePropertyCount(propCount, 5, "GrapplePoint"))
        return nullptr;

    std::string name = *mgr.HashInstanceName(in);
    zeus::CTransform grappleXf = LoadEditorTransform(in);
    bool active = in.readBool();
    CGrappleParameters parameters = LoadGrappleParameters(in);
    return new CScriptGrapplePoint(mgr.AllocateUniqueId(), name, info, grappleXf, active, parameters);
}

CEntity* ScriptLoader::LoadPuddleSpore(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDebugCameraWaypoint(CStateManager& mgr, CInputStream& in,
                                               int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpiderBallAttractionSurface(CStateManager& mgr, CInputStream& in,
                                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPuddleToadGamma(CStateManager& mgr, CInputStream& in,
                                           int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDistanceFog(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFireFlea(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetareeAlpha(CStateManager& mgr, CInputStream& in,
                                        int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDockAreaChange(CStateManager& mgr, CInputStream& in,
                                          int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadActorRotate(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpecialFunction(CStateManager& mgr, CInputStream& in,
                                           int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpankWeed(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadParasite(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPlayerHint(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRipper(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPickupGenerator(CStateManager& mgr, CInputStream& in,
                                           int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAIKeyframe(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPointOfInterest(CStateManager& mgr, CInputStream& in,
                                           int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDrone(CStateManager& mgr, CInputStream& in,
                                 int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetroidAlpha(CStateManager& mgr, CInputStream& in,
                                        int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadDebrisExtended(CStateManager& mgr, CInputStream& in,
                                          int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSteam(CStateManager& mgr, CInputStream& in,
                                 int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRipple(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBallTrigger(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadTargetingPoint(CStateManager& mgr, CInputStream& in,
                                          int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadEMPulse(CStateManager& mgr, CInputStream& in,
                                   int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadIceSheegoth(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPlayerActor(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFlaahgra(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAreaAttributes(CStateManager& mgr, CInputStream& in,
                                          int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFishCloud(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFishCloudModifier(CStateManager& mgr, CInputStream& in,
                                             int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadVisorFlare(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWorldTeleporter(CStateManager& mgr, CInputStream& in,
                                           int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadVisorGoo(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadJellyZap(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadControllerAction(CStateManager& mgr, CInputStream& in,
                                            int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSwitch(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPlayerStateChange(CStateManager& mgr, CInputStream& in,
                                             int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadThardus(CStateManager& mgr, CInputStream& in,
                                   int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWallCrawlerSwarm(CStateManager& mgr, CInputStream& in,
                                            int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAIJumpPoint(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFlaahgraTentacle(CStateManager& mgr, CInputStream& in,
                                            int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRoomAcoustics(CStateManager& mgr, CInputStream& in,
                                         int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadColorModulate(CStateManager& mgr, CInputStream& in,
                                         int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadThardusRockProjectile(CStateManager& mgr, CInputStream& in,
                                                 int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMidi(CStateManager& mgr, CInputStream& in,
                                int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadStreamedAudio(CStateManager& mgr, CInputStream& in,
                                         int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRepulsor(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadGunTurret(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadFogVolume(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBabygoth(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadEyeball(CStateManager& mgr, CInputStream& in,
                                   int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRadialDamage(CStateManager& mgr, CInputStream& in,
                                        int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraPitchVolume(CStateManager& mgr, CInputStream& in,
                                             int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadEnvFxDensityController(CStateManager& mgr, CInputStream& in,
                                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMagdolite(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadTeamAIMgr(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSnakeWeedSwarm(CStateManager& mgr, CInputStream& in,
                                          int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::Load(CStateManager& mgr, CInputStream& in,
                            int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadActorContraption(CStateManager& mgr, CInputStream& in,
                                            int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadOculus(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadGeemer(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSpindleCamera(CStateManager& mgr, CInputStream& in,
                                         int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAtomicAlpha(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadCameraHintTrigger(CStateManager& mgr, CInputStream& in,
                                             int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRumbleEffect(CStateManager& mgr, CInputStream& in,
                                        int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAmbientAI(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadAtomicBeta(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadIceZoomer(CStateManager& mgr, CInputStream& in,
                                     int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPuffer(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadTryclops(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadRidley(CStateManager& mgr, CInputStream& in,
                                  int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadSeedling(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadThermalHeatFader(CStateManager& mgr, CInputStream& in,
                                            int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadBurrower(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadScriptBeam(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadWorldLightFader(CStateManager& mgr, CInputStream& in,
                                           int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetroidPrimeStage2(CStateManager& mgr, CInputStream& in,
                                              int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMetroidPrimeStage1(CStateManager& mgr, CInputStream& in,
                                              int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadMazeNode(CStateManager& mgr, CInputStream& in,
                                    int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadOmegaPirate(CStateManager& mgr, CInputStream& in,
                                       int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPhazonPool(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadPhazonHealingNodule(CStateManager& mgr, CInputStream& in,
                                               int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadNewCameraShaker(CStateManager& mgr, CInputStream& in,
                                           int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadShadowProjector(CStateManager& mgr, CInputStream& in,
                                           int propCount, const CEntityInfo& info)
{
}

CEntity* ScriptLoader::LoadEnergyBall(CStateManager& mgr, CInputStream& in,
                                      int propCount, const CEntityInfo& info)
{
}

}
