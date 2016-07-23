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

CDummyGameArea::CDummyGameArea(CInputStream& in, int mlvlVersion)
{
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

CGameArea::CGameArea(CInputStream& in, int mlvlVersion)
: x4_mlvlVersion(mlvlVersion), xf0_25_active(true)
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

void CGameArea::AliveUpdate(float dt)
{
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
