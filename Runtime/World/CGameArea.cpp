#include "CGameArea.hpp"

namespace urde
{

void CGameArea::CAreaFog::SetCurrent() const
{
}

void CGameArea::CAreaFog::Update(float dt)
{
}

void CGameArea::CAreaFog::RollFogOut(float, float, const zeus::CColor& color)
{
}

void CGameArea::CAreaFog::FadeFog(ERglFogMode,
                                  const zeus::CColor& color, const zeus::CVector2f& vec1,
                                  float, const zeus::CVector2f& vec2)
{
}

void CGameArea::CAreaFog::SetFogExplicit(ERglFogMode, const zeus::CColor& color, const zeus::CVector2f& vec)
{
}

bool CGameArea::CAreaFog::IsFogDisabled() const
{
    return true;
}

void CGameArea::CAreaFog::DisableFog()
{
}

CDummyGameArea::CDummyGameArea(CInputStream& in, int idx, int mlvlVersion)
{
    x8_nameSTRG = in.readUint32Big();
    x14_transform.read34RowMajor(in);
    zeus::CAABox aabb;
    aabb.readBoundingBoxBig(in);
    xc_mrea = in.readUint32Big();
    if (mlvlVersion > 15)
        x10_areaId = in.readUint32Big();

    u32 attachAreaCount = in.readUint32Big();
    x44_attachedAreaIndices.reserve(attachAreaCount);
    for (u32 i=0 ; i<attachAreaCount ; ++i)
        x44_attachedAreaIndices.push_back(in.readUint16Big());

    u32 depCount = in.readUint32Big();
    for (u32 i=0 ; i<depCount ; ++i)
        in.readUint32Big();

    u32 dockCount = in.readUint32Big();
    x54_docks.reserve(dockCount);
    for (u32 i=0 ; i<dockCount ; ++i)
        x54_docks.emplace_back(in, x14_transform);
}

bool CDummyGameArea::IGetScriptingMemoryAlways() const
{
    return false;
}

TAreaId CDummyGameArea::IGetAreaId() const
{
    return 0;
}

ResId CDummyGameArea::IGetAreaAssetId() const
{
    return 0;
}

bool CDummyGameArea::IIsActive() const
{
    return false;
}

TAreaId CDummyGameArea::IGetAttachedAreaId(int) const
{
    return 0;
}

u32 CDummyGameArea::IGetNumAttachedAreas() const
{
    return 0;
}

ResId CDummyGameArea::IGetStringTableAssetId() const
{
    return 0;
}

static zeus::CTransform identityXf(zeus::CMatrix3f::skIdentityMatrix3f);
const zeus::CTransform& CDummyGameArea::IGetTM() const
{
    return identityXf;
}

static std::vector<SObjectTag> ReadDependencyList(CInputStream& in)
{
    std::vector<SObjectTag> ret;
    u32 count = in.readUint32Big();
    ret.reserve(count);
    for (u32 i=0 ; i<count ; ++i)
    {
        ret.emplace_back();
        ret.back().readMLVL(in);
    }
    return ret;
}

CGameArea::CGameArea(CInputStream& in, int idx, int mlvlVersion)
: x4_selfIdx(idx), xf0_25_active(true)
{
    x8_nameSTRG = in.readUint32Big();
    xc_transform.read34RowMajor(in);
    x3c_invTransform = xc_transform.inverse();
    x6c_aabb.readBoundingBoxBig(in);

    x84_mrea = in.readUint32Big();
    if (mlvlVersion > 15)
        x88_areaId = in.readUint32Big();
    else
        x88_areaId = -1;

    u32 attachedCount = in.readUint32Big();
    x8c_attachedAreaIndices.reserve(attachedCount);
    for (u32 i=0 ; i<attachedCount ; ++i)
        x8c_attachedAreaIndices.push_back(in.readUint16Big());

    x9c_deps1 = ::urde::ReadDependencyList(in);
    xac_deps2 = ::urde::ReadDependencyList(in);

}

bool CGameArea::IGetScriptingMemoryAlways() const
{
    return false;
}

TAreaId CGameArea::IGetAreaId() const
{
    return 0;
}

ResId CGameArea::IGetAreaAssetId() const
{
    return 0;
}

bool CGameArea::IIsActive() const
{
    return false;
}

TAreaId CGameArea::IGetAttachedAreaId(int) const
{
    return 0;
}

u32 CGameArea::IGetNumAttachedAreas() const
{
    return 0;
}

ResId CGameArea::IGetStringTableAssetId() const
{
    return 0;
}

const zeus::CTransform& CGameArea::IGetTM() const
{
    return identityXf;
}

bool CGameArea::DoesAreaNeedEnvFx() const
{
    return false;
}

bool CGameArea::DoesAreaNeedSkyNow() const
{
    return false;
}

void CGameArea::UpdateFog(float dt)
{
}

bool CGameArea::OtherAreaOcclusionChanged()
{
    return false;
}

void CGameArea::PingOcclusionState()
{
}

void CGameArea::PreRender()
{
}

void CGameArea::UpdateThermalVisor(float dt)
{
    if (x12c_postConstructed->x1120_thermalSpeed == 0.f)
        return;

    float influence = x12c_postConstructed->x111c_thermalCurrent;

    float delta = x12c_postConstructed->x1120_thermalSpeed * dt;
    if (std::fabs(x12c_postConstructed->x1124_thermalTarget -
                  x12c_postConstructed->x111c_thermalCurrent) < delta)
    {
        influence = x12c_postConstructed->x1124_thermalTarget;
        x12c_postConstructed->x1120_thermalSpeed = 0.f;
    }
    else if (x12c_postConstructed->x1124_thermalTarget < influence)
        influence -= delta;
    else
        influence += delta;

    x12c_postConstructed->x111c_thermalCurrent = influence;
}

void CGameArea::AliveUpdate(float dt)
{
    if (!x12c_postConstructed->x10dc_)
        x12c_postConstructed->x10e4_ += dt;
    else
        x12c_postConstructed->x10e4_ = 0.f;
    UpdateFog(dt);
    UpdateThermalVisor(dt);

}

void CGameArea::SetOcclusionState(EOcclusionState state)
{
}

void CGameArea::RemoveStaticGeometry()
{
}

void CGameArea::AddStaticGeometry()
{
}

void CGameArea::SetChain(CGameArea* other, int)
{
}

void CGameArea::StartStreamingMainArea()
{
}

u32 CGameArea::GetNumPartSizes() const
{
    return 0;
}

void CGameArea::AllocNewAreaData(int, int)
{
}

void CGameArea::Invalidate(CStateManager& mgr)
{
}

void CGameArea::CullDeadAreaRequests()
{
}

void CGameArea::StartStreamIn(CStateManager& mgr)
{
}

bool CGameArea::Validate(CStateManager& mgr)
{
    return false;
}

void CGameArea::PostConstructArea()
{
}

void CGameArea::FillInStaticGeometry()
{
}

void CGameArea::VerifyTokenList()
{
}

void CGameArea::ClearTokenList()
{
}

u32 CGameArea::GetPreConstructedSize() const
{
    return 0;
}

bool CGameArea::VerifyHeader() const
{
    return false;
}

}
