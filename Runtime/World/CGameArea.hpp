#ifndef __URDE_CGAMEAREA_HPP__
#define __URDE_CGAMEAREA_HPP__

#include "zeus/CVector2f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CAABox.hpp"
#include "CToken.hpp"
#include "RetroTypes.hpp"
#include "IGameArea.hpp"
#include "Collision/CAreaOctTree.hpp"
#include "hecl/ClientProcess.hpp"
#include "Graphics/CMetroidModelInstance.hpp"
#include "CObjectList.hpp"
#include "CWorldLight.hpp"
#include "Graphics/CPVSAreaSet.hpp"
#include "Graphics/CGraphics.hpp"
#include "CPathFindArea.hpp"
#include "Editor/ProjectResourceFactoryBase.hpp"

namespace urde
{
class CStateManager;
class CScriptAreaAttributes;
class CDummyGameArea : public IGameArea
{
    friend class CDummyWorld;

    int x4_mlvlVersion;
    ResId x8_nameSTRG;
    ResId xc_mrea;
    ResId x10_areaId;
    zeus::CTransform x14_transform;
    std::vector<u16> x44_attachedAreaIndices;
    std::vector<Dock> x54_docks;

public:
    CDummyGameArea(CInputStream& in, int idx, int mlvlVersion);

    bool IGetScriptingMemoryAlways() const;
    TAreaId IGetAreaId() const;
    ResId IGetAreaAssetId() const;
    bool IIsActive() const;
    TAreaId IGetAttachedAreaId(int) const;
    u32 IGetNumAttachedAreas() const;
    ResId IGetStringTableAssetId() const;
    const zeus::CTransform& IGetTM() const;
};

struct CAreaRenderOctTree
{
    struct Node
    {
        u16 x0_bitmapIdx;
        u16 x2_flags;
        u16 x4_children[];

        u32 GetChildCount() const;
        zeus::CAABox GetNodeBounds(const zeus::CAABox& curAABB, int idx) const;

        void RecursiveBuildOverlaps(u32* out, const CAreaRenderOctTree& parent, const zeus::CAABox& curAABB,
                                    const zeus::CAABox& testAABB) const;
    };

    const u8* x0_buf;
    u32 x8_bitmapCount;
    u32 xc_meshCount;
    u32 x10_nodeCount;
    u32 x14_bitmapWordCount;
    zeus::CAABox x18_aabb;
    const u32* x30_bitmaps;
    const u32* x34_indirectionTable;
    const u8* x38_entries;

    CAreaRenderOctTree(const u8* buf);

    void FindOverlappingModels(std::vector<u32>& out, const zeus::CAABox& testAABB) const;
    void FindOverlappingModels(u32* out, const zeus::CAABox& testAABB) const;
};

class CGameArea : public IGameArea
{
    friend class CWorld;
    friend class CStateManager;

    int x4_selfIdx;
    ResId x8_nameSTRG;
    zeus::CTransform xc_transform;
    zeus::CTransform x3c_invTransform;
    zeus::CAABox x6c_aabb;
    ResId x84_mrea;
    u32 x88_areaId;
    std::vector<u16> x8c_attachedAreaIndices;
    std::vector<SObjectTag> x9c_deps1;
    std::vector<SObjectTag> xac_deps2;

    std::vector<u32> xbc_layerDepOffsets;
    std::vector<Dock> xcc_docks;
    std::vector<CToken> xdc_tokens;

    u32 xec_totalResourcesSize = 0;

    union
    {
        struct
        {
            bool xf0_24_postConstructed : 1;
            bool xf0_25_active : 1;
            bool xf0_26_tokensReady : 1;
            bool xf0_27_paused : 1;
            bool xf0_28_validated : 1;
        };
        u8 _dummy = 0;
    };

    enum class EPhase
    {
        LoadHeader,
        LoadSecSizes,
        ReserveSections,
        LoadDataSections,
        WaitForFinish
    } xf4_phase = EPhase::LoadHeader;

    std::list<std::shared_ptr<ProjectResourceFactoryBase::AsyncTask>> xf8_loadTransactions;

public:

    class CChainIterator
    {
        CGameArea* m_area;
    public:
        CChainIterator(CGameArea* area) : m_area(area) {}
        CGameArea& operator*() const { return *m_area; }
        CGameArea* operator->() const { return m_area; }
        CChainIterator& operator++() { m_area = m_area->GetNext(); return *this; }
        bool operator!=(const CChainIterator& other) const { return m_area != other.m_area; }
        bool operator==(const CChainIterator& other) const { return m_area == other.m_area; }
    };

    class CConstChainIterator
    {
        const CGameArea* m_area;
    public:
        CConstChainIterator(const CGameArea* area) : m_area(area) {}
        const CGameArea& operator*() const { return *m_area; }
        const CGameArea* operator->() const { return m_area; }
        CConstChainIterator& operator++() { m_area = m_area->GetNext(); return *this; }
        bool operator!=(const CConstChainIterator& other) const { return m_area != other.m_area; }
        bool operator==(const CConstChainIterator& other) const { return m_area == other.m_area; }
    };

    class CAreaObjectList : public CObjectList
    {
    private:
        TAreaId x200c_areaIdx = 0;
    public:
        CAreaObjectList(TAreaId areaIdx)
            : CObjectList(EGameObjectList::Invalid)
            , x200c_areaIdx(areaIdx)
        {
        }

        bool IsQualified(const CEntity& ent);
    };

    enum class EOcclusionState
    {
        Occluded,
        Visible
    };

    class CAreaFog
    {
        ERglFogMode x0_fogMode = ERglFogMode::None;
        zeus::CVector2f x4_rangeCur = {0.f, 1024.f};
        zeus::CVector2f xc_rangeTarget = {0.f, 1024.f};
        zeus::CVector2f x14_rangeDelta;
        zeus::CColor x1c_colorCur = {0.5f, 0.5f, 0.5f, 1.f};
        zeus::CColor x28_colorTarget = {0.5f, 0.5f, 0.5f, 1.f};
        float x34_colorDelta = 0.f;
    public:
        void SetCurrent() const;
        void Update(float dt);
        void RollFogOut(float rangeDelta, float colorDelta, const zeus::CColor& color);
        void FadeFog(ERglFogMode, const zeus::CColor& color, const zeus::CVector2f& vec1,
                     float, const zeus::CVector2f& vec2);
        void SetFogExplicit(ERglFogMode mode, const zeus::CColor& color, const zeus::CVector2f& range);
        bool IsFogDisabled() const;
        void DisableFog();
    };

    struct CPostConstructed
    {
        std::unique_ptr<CAreaOctTree> x0_collision;
        u32 x8_collisionSize = 0;
        std::experimental::optional<CAreaRenderOctTree> xc_octTree;
        std::vector<CMetroidModelInstance> x4c_insts;
        //std::unique_ptr<from unknown, pointless MREA section> x5c_;
        std::vector<CWorldLight> x60_lightsA;
        std::vector<CLight> x70_gfxLightsA;
        std::vector<CWorldLight> x80_lightsB;
        std::vector<CLight> x90_gfxLightsB;
        std::unique_ptr<CPVSAreaSet> xa0_pvs;
        u32 xa4_elemCount = 1024;
        struct MapEntry
        {
            s16 x0_id = -1;
            TUniqueId x4_uid = kInvalidUniqueId;
        } xa8_pvsEntityMap[1024];
        u32 x10a8_pvsVersion = 0;
        TLockedToken<CPFArea> x10ac_path;
        // bool x10b8_ = 0; optional flag for CToken
        u32 x10bc_ = 0;
        std::unique_ptr<CAreaObjectList> x10c0_areaObjs;
        std::unique_ptr<CAreaFog> x10c4_areaFog;
        const u8* x10c8_sclyBuf = nullptr;
        u32 x10d0_sclySize = 0;
        u32 x10d4_ = 0;
        const CScriptAreaAttributes* x10d8_areaAttributes = nullptr;
        EOcclusionState x10dc_occlusionState = EOcclusionState::Occluded;
        u32 x10e0_ = 0;
        float x10e4_ = 5.f;
        u32 x10e8_ = -1;
        u32 x10ec_ = 0;
        // std::vector<CAramToken> x10f0_tokens;
        u32 x1100_ = 0;
        u32 x1104_ = 0;
        union
        {
            struct
            {
                bool x1108_24_ : 1;
                bool x1108_25_ : 1;
                bool x1108_26_ : 1;
                bool x1108_27_ : 1;
                bool x1108_28_ : 1;
                bool x1108_29_ : 1;
                bool x1108_30_ : 1;
            };
            u8 _dummy = 0;
        };
        std::vector<std::pair<const u8*, u32>> x110c_layerPtrs;
        float x111c_thermalCurrent = 0.f;
        float x1120_thermalSpeed = 0.f;
        float x1124_thermalTarget = 0.f;
        float x1128_worldLightingLevel = 1.f;
        float x112c_xraySpeed = 0.f;
        float x1130_xrayTarget = 1.f;
        float x1134_ = 0.f;
        float x1138_ = 1.f;
        u32 x113c_ = 0;
    };
private:
    std::vector<std::pair<std::unique_ptr<u8[]>, int>> x110_mreaSecBufs;
    std::vector<std::pair<const u8*, int>> m_resolvedBufs;
    u32 x124_secCount = 0;
    u32 x128_mreaDataOffset = 0;
    std::unique_ptr<CPostConstructed> x12c_postConstructed;

    CGameArea* x130_next = nullptr;
    CGameArea* x134_prev = nullptr;
    EChain x138_curChain = EChain::Zero;

    void UpdateFog(float dt);
    void UpdateThermalVisor(float dt);

    struct MREAHeader
    {
        u32 version = 0;
        zeus::CTransform xf;
        u32 modelCount;
        u32 secCount;
        u32 geomSecIdx;
        u32 sclySecIdx;
        u32 collisionSecIdx;
        u32 unkSecIdx;
        u32 lightSecIdx;
        u32 visiSecIdx;
        u32 pathSecIdx;
        u32 arotSecIdx;
        std::vector<u32> secSizes;
    };

public:

    CGameArea(CInputStream& in, int idx, int mlvlVersion);

    bool IsFinishedOccluding() const;
    void ReadDependencyList();
    void SetPauseState(bool paused);

    bool IGetScriptingMemoryAlways() const;
    TAreaId GetAreaId() const { return x4_selfIdx; }
    TAreaId IGetAreaId() const { return x4_selfIdx; }
    ResId IGetAreaAssetId() const { return x84_mrea; }
    bool IIsActive() const;
    TAreaId IGetAttachedAreaId(int) const;
    u32 IGetNumAttachedAreas() const;
    ResId IGetStringTableAssetId() const;
    const zeus::CTransform& IGetTM() const;

    void SetXRaySpeedAndTarget(float f1, float f2);
    void SetThermalSpeedAndTarget(float f1, float f2);

    ResId GetAreaAssetId() const { return x84_mrea; }
    const CAreaFog* GetAreaFog() const { return GetPostConstructed()->x10c4_areaFog.get(); }
    CAreaFog* AreaFog() { return const_cast<CAreaFog*>(GetAreaFog()); }
    float GetXRayFogDistance() const;
    bool DoesAreaNeedEnvFx() const;
    bool DoesAreaNeedSkyNow() const;
    bool OtherAreaOcclusionChanged();
    void PingOcclusionState();
    void PreRender();
    void AliveUpdate(float dt);
    void SetOcclusionState(EOcclusionState state);
    EOcclusionState GetOcclusionState() const { return GetPostConstructed()->x10dc_occlusionState; }
    void RemoveStaticGeometry();
    void AddStaticGeometry();
    //void TransferTokensToARAM();
    //void TransferARAMTokensOver();
    EChain SetChain(CGameArea* prev, EChain chain);
    bool StartStreamingMainArea();
    //void UnloadAllLoadedTextures();
    //void ReloadAllLoadedTextures();
    void ReloadAllUnloadedTextures();
    void PrepTokens();
    u32 GetNumPartSizes() const;
    void AllocNewAreaData(int, int);
    bool Invalidate(CStateManager& mgr);
    void CullDeadAreaRequests();
    void StartStreamIn(CStateManager& mgr);
    void Validate(CStateManager& mgr);
    void LoadScriptObjects(CStateManager& mgr);
    std::pair<const u8*, u32> GetLayerScriptBuffer(int layer);
    void PostConstructArea();
    void FillInStaticGeometry();
    void VerifyTokenList(CStateManager& stateMgr);
    void ClearTokenList();
    u32 GetPreConstructedSize() const;
    MREAHeader VerifyHeader() const;
    TUniqueId LookupPVSUniqueID(TUniqueId id) const;
    s16 LookupPVSID(TUniqueId id) const;

    const zeus::CTransform& GetTransform() const {return xc_transform;}
    const zeus::CTransform& GetInverseTransform() const {return x3c_invTransform;}
    const zeus::CAABox& GetAABB() const {return x6c_aabb;}

    const std::vector<Dock>& GetDocks() const {return xcc_docks;}
    const Dock* GetDock(s32 dock) const { return &xcc_docks[dock]; }
    s32 GetDockCount() const { return xcc_docks.size(); }
    Dock* DockNC(s32 dock) { return &xcc_docks[dock]; }

    bool IsPostConstructed() const {return xf0_24_postConstructed;}
    const CPostConstructed* GetPostConstructed() const {return x12c_postConstructed.get();}

    bool IsValidated() const { return xf0_28_validated; }

    void SetAreaAttributes(const CScriptAreaAttributes* areaAttributes);
    bool GetActive() const { return xf0_25_active; }
    void SetActive(bool active) { xf0_25_active = active; }
    CObjectList& GetAreaObjects() const { return *GetPostConstructed()->x10c0_areaObjs.get(); }

    CGameArea* GetNext() const { return x130_next; }
};

}

#endif // __URDE_CGAMEAREA_HPP__
