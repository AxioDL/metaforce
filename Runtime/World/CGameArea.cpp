#if _WIN32
#include <D3Dcommon.h>
#endif
#include "CGameArea.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"

namespace urde
{

CAreaRenderOctTree::CAreaRenderOctTree(std::unique_ptr<u8[]>&& buf)
: x0_buf(std::move(buf))
{
    athena::io::MemoryReader r(x0_buf.get() + 8, INT32_MAX);
    x8_bitmapCount = r.readUint32Big();
    xc_meshCount = r.readUint32Big();
    x10_nodeCount = r.readUint32Big();
    x14_bitmapWordCount = (xc_meshCount + 31) / 32;
    x18_aabb.readBoundingBoxBig(r);

    x30_bitmaps = reinterpret_cast<u32*>(x0_buf.get() + 64);
    u32 wc = x14_bitmapWordCount * x8_bitmapCount;
    for (int i=0 ; i<wc ; ++i)
        x30_bitmaps[i] = hecl::SBig(x30_bitmaps[i]);

    x34_indirectionTable = x30_bitmaps + wc;
    x38_entries = reinterpret_cast<u8*>(x34_indirectionTable) + x10_nodeCount;
    for (int i=0 ; i<x10_nodeCount ; ++i)
    {
        x34_indirectionTable[i] = hecl::SBig(x34_indirectionTable[i]);
        Node* n = reinterpret_cast<Node*>(x38_entries + x34_indirectionTable[i]);
        n->x0_bitmapIdx = hecl::SBig(n->x0_bitmapIdx);
        n->x2_flags = hecl::SBig(n->x2_flags);
        if (n->x2_flags)
        {
            u32 childCount = n->GetChildCount();
            for (u32 c=0 ; c<childCount ; ++c)
                n->x4_children[c] = hecl::SBig(n->x4_children[c]);
        }
    }
}

static const u32 ChildCounts[] = { 0, 2, 2, 4, 2, 4, 4, 8 };
u32 CAreaRenderOctTree::Node::GetChildCount() const
{
    return ChildCounts[x2_flags];
}

zeus::CAABox CAreaRenderOctTree::Node::GetNodeBounds(const zeus::CAABox& curAABB, int idx) const
{
    zeus::CVector3f center = curAABB.center();
    switch (x2_flags)
    {
    case 0:
    default:
        return curAABB;
    case 1:
        if (idx == 0)
            return {curAABB.min.x, curAABB.min.y, curAABB.min.z, center.x, curAABB.max.y, curAABB.max.z};
        else
            return {center.x, curAABB.min.y, curAABB.min.z, curAABB.max.x, curAABB.max.y, curAABB.max.z};
    case 2:
        if (idx == 0)
            return {curAABB.min.x, curAABB.min.y, curAABB.min.z, curAABB.max.x, center.y, curAABB.max.z};
        else
            return {curAABB.min.x, center.y, curAABB.min.z, curAABB.max.x, curAABB.max.y, curAABB.max.z};
    case 3:
    {
        switch (idx)
        {
        case 0:
        default:
            return {curAABB.min.x, curAABB.min.y, curAABB.min.z, center.x, center.y, curAABB.max.z};
        case 1:
            return {center.x, curAABB.min.y, curAABB.min.z, curAABB.max.x, center.y, curAABB.max.z};
        case 2:
            return {curAABB.min.x, center.y, curAABB.min.z, center.x, curAABB.max.y, curAABB.max.z};
        case 3:
            return {center.x, center.y, curAABB.min.z, curAABB.max.x, curAABB.max.y, curAABB.max.z};
        }
    }
    case 4:
        if (idx == 0)
            return {curAABB.min.x, curAABB.min.y, curAABB.min.z, curAABB.max.x, curAABB.max.y, center.z};
        else
            return {curAABB.min.x, curAABB.min.y, center.z, curAABB.max.x, curAABB.max.y, curAABB.max.z};
    case 5:
    {
        switch (idx)
        {
        case 0:
        default:
            return {curAABB.min.x, curAABB.min.y, curAABB.min.z, center.x, curAABB.max.y, center.z};
        case 1:
            return {center.x, curAABB.min.y, curAABB.min.z, curAABB.max.x, curAABB.max.y, center.z};
        case 2:
            return {curAABB.min.x, curAABB.min.y, center.z, center.x, curAABB.max.y, curAABB.max.z};
        case 3:
            return {center.x, curAABB.min.y, center.z, curAABB.max.x, curAABB.max.y, curAABB.max.z};
        }
    }
    case 6:
    {
        switch (idx)
        {
        case 0:
        default:
            return {curAABB.min.x, curAABB.min.y, curAABB.min.z, curAABB.max.x, center.y, center.z};
        case 1:
            return {curAABB.min.x, center.y, curAABB.min.z, curAABB.max.x, curAABB.max.y, center.z};
        case 2:
            return {curAABB.min.x, curAABB.min.y, center.z, curAABB.max.x, center.y, curAABB.max.z};
        case 3:
            return {curAABB.min.x, center.y, center.z, curAABB.max.x, curAABB.max.y, curAABB.max.z};
        }
    }
    case 7:
    {
        switch (idx)
        {
        case 0:
        default:
            return {curAABB.min.x, curAABB.min.y, curAABB.min.z, center.x, center.y, center.z};
        case 1:
            return {center.x, curAABB.min.y, curAABB.min.z, curAABB.max.x, center.y, center.z};
        case 2:
            return {curAABB.min.x, center.y, curAABB.min.z, center.x, curAABB.max.y, center.z};
        case 3:
            return {center.x, center.y, curAABB.min.z, curAABB.max.x, curAABB.max.y, center.z};
        case 4:
            return {curAABB.min.x, curAABB.min.y, center.z, center.x, center.y, curAABB.max.z};
        case 5:
            return {center.x, curAABB.min.y, center.z, curAABB.max.x, center.y, curAABB.max.z};
        case 6:
            return {curAABB.min.x, center.y, center.z, center.x, curAABB.max.y, curAABB.max.z};
        case 7:
            return {center.x, center.y, center.z, curAABB.max.x, curAABB.max.y, curAABB.max.z};
        }
    }
    }
}

void CAreaRenderOctTree::Node::RecursiveBuildOverlaps(u32* bmpOut,
                                                      const CAreaRenderOctTree& parent,
                                                      const zeus::CAABox& curAABB,
                                                      const zeus::CAABox& testAABB) const
{
    if (testAABB.intersects(curAABB))
    {
        if (curAABB.inside(testAABB))
        {
            const u32* bmp = &parent.x30_bitmaps[x0_bitmapIdx * parent.x14_bitmapWordCount];
            for (u32 c=0 ; c<parent.x14_bitmapWordCount ; ++c)
                bmpOut[c] |= bmp[c];
        }
        else
        {
            u32 childCount = GetChildCount();
            for (u32 c=0 ; c<childCount ; ++c)
            {
                zeus::CAABox childAABB = GetNodeBounds(curAABB, c);
                reinterpret_cast<Node*>(parent.x38_entries[parent.x34_indirectionTable[x4_children[c]]])->
                    RecursiveBuildOverlaps(bmpOut, parent, childAABB, testAABB);
            }
        }
    }
}

void CAreaRenderOctTree::FindOverlappingModels(std::vector<u32>& out, const zeus::CAABox& testAABB) const
{
    out.resize(x14_bitmapWordCount);
    reinterpret_cast<Node*>(x38_entries[x34_indirectionTable[0]])->
        RecursiveBuildOverlaps(out.data(), *this, x18_aabb, testAABB);
}

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

    ::urde::ReadDependencyList(in);
    ::urde::ReadDependencyList(in);

    if (mlvlVersion > 13)
    {
        u32 depCount = in.readUint32Big();
        for (u32 i=0 ; i<depCount ; ++i)
            in.readUint32Big();
    }

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

    zeus::CAABox aabb = x6c_aabb.getTransformedAABox(xc_transform);
    x6c_aabb = aabb;

    if (mlvlVersion > 13)
    {
        u32 depCount = in.readUint32Big();
        xbc_layerDepOffsets.reserve(depCount);
        for (u32 i=0 ; i<depCount ; ++i)
            xbc_layerDepOffsets.push_back(in.readUint32Big());
    }

    u32 dockCount = in.readUint32Big();
    xcc_docks.reserve(dockCount);
    for (u32 i=0 ; i<dockCount ; ++i)
        xcc_docks.emplace_back(in, xc_transform);

    ClearTokenList();

    for (CToken& tok : xdc_tokens)
        xec_totalResourcesSize += g_ResFactory->ResourceSize(*tok.GetObjectTag());

    xec_totalResourcesSize += g_ResFactory->ResourceSize(SObjectTag{FOURCC('MREA'), x84_mrea});
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
    if (!xf0_24_postConstructed || !x12c_postConstructed || !x12c_postConstructed->x10dc_)
        return;
    x12c_postConstructed->x10e0_ = 0;
    x12c_postConstructed->x10dc_ = 0;
    g_Renderer->RemoveStaticGeometry(&x12c_postConstructed->x4c_insts);
}

void CGameArea::AddStaticGeometry()
{
    if (x12c_postConstructed->x10dc_ != 1)
    {
        x12c_postConstructed->x10e0_ = 0;
        x12c_postConstructed->x10dc_ = 1;
        if (!x12c_postConstructed->x1108_25_)
            FillInStaticGeometry();
        g_Renderer->AddStaticGeometry(&x12c_postConstructed->x4c_insts,
                                      x12c_postConstructed->xc_octTree ?
                                          &*x12c_postConstructed->xc_octTree : nullptr,
                                      x4_selfIdx);
    }
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
    MREAHeader header = VerifyHeader();

    auto secIt = x110_mreaSecBufs.begin() + 2;

    /* Models */
    if (header.modelCount)
    {
        for (int i=0 ; i<header.modelCount ; ++i)
        {
            u32 surfCount = hecl::SBig(*reinterpret_cast<u32*>((secIt+6)->first.get()));
            secIt += 7 + surfCount;
        }
    }

    /* Render octree */
    if (header.version == 15 && header.arotSecIdx != -1)
    {
        x12c_postConstructed->xc_octTree.emplace(std::move(secIt->first));
        ++secIt;
    }

    /* Scriptable layer section */
    x12c_postConstructed->x10c8_sclyBuf = std::move(secIt->first);
    x12c_postConstructed->x10d0_sclySize = secIt->second;
    ++secIt;

    /* Collision section */
    std::unique_ptr<CAreaOctTree> collision = CAreaOctTree::MakeFromMemory(secIt->first.get(), secIt->second);
    if (collision)
    {
        x12c_postConstructed->x0_collision = std::move(collision);
        x12c_postConstructed->x8_collisionSize = secIt->second;
    }
    ++secIt;

    /* Unknown section */
    ++secIt;

    /* Lights section */
    if (header.version > 6)
    {
        athena::io::MemoryReader r(secIt->first.get(), secIt->second);
        u32 magic = r.readUint32Big();
        if (magic == 0xBABEDEAD)
        {
            u32 aCount = r.readUint32Big();
            x12c_postConstructed->x60_lightsA.reserve(aCount);
            x12c_postConstructed->x70_gfxLightsA.reserve(aCount);
            for (u32 i=0 ; i<aCount ; ++i)
            {
                x12c_postConstructed->x60_lightsA.emplace_back(r);
                x12c_postConstructed->x70_gfxLightsA.push_back(
                    x12c_postConstructed->x60_lightsA.back().GetAsCGraphicsLight());
            }

            u32 bCount = r.readUint32Big();
            x12c_postConstructed->x80_lightsB.reserve(bCount);
            x12c_postConstructed->x90_gfxLightsB.reserve(bCount);
            for (u32 i=0 ; i<bCount ; ++i)
            {
                x12c_postConstructed->x80_lightsB.emplace_back(r);
                x12c_postConstructed->x90_gfxLightsB.push_back(
                    x12c_postConstructed->x80_lightsB.back().GetAsCGraphicsLight());
            }
        }

        ++secIt;
    }

    /* PVS section */
    if (header.version > 7)
    {
        athena::io::MemoryReader r(secIt->first.get(), secIt->second);
        u32 magic = r.readUint32Big();
        if (magic == 'VISI')
        {
            x12c_postConstructed->x10a8_pvsVersion = r.readUint32Big();
            if (x12c_postConstructed->x10a8_pvsVersion == 2)
            {
                x12c_postConstructed->x1108_29_ = r.readBool();
                x12c_postConstructed->x1108_30_ = r.readBool();
                x12c_postConstructed->xa0_pvs.reset(new CPVSAreaSet::CPVSAreaHolder(r));
            }
        }

        ++secIt;
    }

    /* Pathfinding section */
    if (header.version > 9)
    {
        athena::io::MemoryReader r(secIt->first.get(), secIt->second);
        ResId pathId = r.readUint32Big();
        x12c_postConstructed->x10ac_path = g_SimplePool->GetObj(SObjectTag{FOURCC('PATH'), pathId});
        ++secIt;
    }

    x12c_postConstructed->x10c0_areaObjs.reset(new CObjectList(EGameObjectList::Invalid));
    x12c_postConstructed->x10c0_areaObjs->m_areaIdx = x4_selfIdx;

    x12c_postConstructed->x10c4_areaFog.reset(new CAreaFog());

    xf0_24_postConstructed = true;

    /* Resolve layer pointers */
    if (x12c_postConstructed->x10c8_sclyBuf)
    {
        athena::io::MemoryReader r(x12c_postConstructed->x10c8_sclyBuf.get(), x12c_postConstructed->x10d0_sclySize);
        u32 magic = r.readUint32Big();
        if (magic == 'SCLY')
        {
            r.readUint32Big();
            u32 layerCount = r.readUint32Big();
            x12c_postConstructed->x110c_layerPtrs.resize(layerCount);
            for (u32 l=0 ; l<layerCount ; ++l)
                x12c_postConstructed->x110c_layerPtrs[l].second = r.readUint32Big();
            u8* ptr = x12c_postConstructed->x10c8_sclyBuf.get() + r.position();
            for (u32 l=0 ; l<layerCount ; ++l)
            {
                x12c_postConstructed->x110c_layerPtrs[l].first = ptr;
                ptr += x12c_postConstructed->x110c_layerPtrs[l].second;
            }
        }
    }
}

void CGameArea::FillInStaticGeometry()
{
}

void CGameArea::VerifyTokenList(CStateManager& stateMgr)
{
    if (xdc_tokens.empty())
        return;
    ClearTokenList();

    if (xac_deps2.empty())
        return;

    u32 lastOff = 0;
    int lidx = 0;
    for (u32 off : xbc_layerDepOffsets)
    {
        if (stateMgr.IsLayerActive(x4_selfIdx, lidx))
        {
            auto it = xac_deps2.begin() + lastOff;
            auto end = xac_deps2.begin() + off;
            for (; it != end ; ++it)
            {
                xdc_tokens.push_back(g_SimplePool->GetObj(*it));
                xdc_tokens.back().Lock();
            }
        }
        lastOff = off;
        ++lidx;
    }
}

void CGameArea::ClearTokenList()
{
    if (xdc_tokens.empty())
        xdc_tokens.reserve(xac_deps2.size());
    else
        xdc_tokens.clear();

    xf0_26_tokensReady = 0;
}

u32 CGameArea::GetPreConstructedSize() const
{
    return 0;
}

CGameArea::MREAHeader CGameArea::VerifyHeader() const
{
    if (x110_mreaSecBufs.empty())
        return {};
    if (*reinterpret_cast<u32*>(x110_mreaSecBufs[0].first.get()) != SBIG(0xDEADBEEF))
        return {};

    MREAHeader header;
    athena::io::MemoryReader r(x110_mreaSecBufs[0].first.get() + 4, INT32_MAX);
    u32 version = r.readUint32Big();
    header.version = (version >= 12 && version <= 15) ? version : 0;
    if (!header.version)
        return {};

    header.xf.read34RowMajor(r);
    header.modelCount = r.readUint32Big();
    header.secCount = r.readUint32Big();
    header.geomSecIdx = r.readUint32Big();
    header.sclySecIdx = r.readUint32Big();
    header.collisionSecIdx = r.readUint32Big();
    header.unkSecIdx = r.readUint32Big();
    header.lightSecIdx = r.readUint32Big();
    header.visiSecIdx = r.readUint32Big();
    header.pathSecIdx = r.readUint32Big();
    header.arotSecIdx = r.readUint32Big();

    header.secSizes.reserve(header.secCount);
    for (int i=0 ; i<header.secCount ; ++i)
        header.secSizes.push_back(r.readUint32Big());

    return header;
}

}
