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
    for (int i=0 ; i<attachAreaCount ; ++i)
        x44_attachedAreaIndices.push_back(in.readUint16Big());

    u32 depCount = in.readUint32Big();
    for (int i=0 ; i<depCount ; ++i)
        in.readUint32Big();

    u32 dockCount = in.readUint32Big();
    x54_docks.reserve(dockCount);
    for (int i=0 ; i<dockCount ; ++i)
        x54_docks.emplace_back(in, x14_transform);
}

bool CDummyGameArea::IGetScriptingMemoryAlways() const
{
}

TAreaId CDummyGameArea::IGetAreaId() const
{
}

ResId CDummyGameArea::IGetAreaAssetId() const
{
}

bool CDummyGameArea::IIsActive() const
{
}

TAreaId CDummyGameArea::IGetAttachedAreaId(int) const
{
}

u32 CDummyGameArea::IGetNumAttachedAreas() const
{
}

ResId CDummyGameArea::IGetStringTableAssetId() const
{
}

const zeus::CTransform& CDummyGameArea::IGetTM() const
{
}

static std::vector<SObjectTag> ReadDependencyList(CInputStream& in)
{
    std::vector<SObjectTag> ret;
    u32 count = in.readUint32Big();
    ret.reserve(count);
    for (int i=0 ; i<count ; ++i)
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
    for (int i=0 ; i<attachedCount ; ++i)
        x8c_attachedAreaIndices.push_back(in.readUint16Big());

    x9c_deps1 = ::urde::ReadDependencyList(in);
    xac_deps2 = ::urde::ReadDependencyList(in);

}

bool CGameArea::IGetScriptingMemoryAlways() const
{
}

TAreaId CGameArea::IGetAreaId() const
{
}

ResId CGameArea::IGetAreaAssetId() const
{
}

bool CGameArea::IIsActive() const
{
}

TAreaId CGameArea::IGetAttachedAreaId(int) const
{
}

u32 CGameArea::IGetNumAttachedAreas() const
{
}

ResId CGameArea::IGetStringTableAssetId() const
{
}

const zeus::CTransform& CGameArea::IGetTM() const
{
}

bool CGameArea::DoesAreaNeedEnvFx() const
{
}

bool CGameArea::DoesAreaNeedSkyNow() const
{
}

void CGameArea::UpdateFog(float dt)
{
}

bool CGameArea::OtherAreaOcclusionChanged()
{
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
}

bool CGameArea::VerifyHeader() const
{
}

}
