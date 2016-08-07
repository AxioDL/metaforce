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

namespace urde
{
class CStateManager;
class CPFArea;

enum class ERglFogMode
{
    Four = 4
};

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

    std::unique_ptr<u8[]> x0_buf;
    u32 x8_bitmapCount;
    u32 xc_meshCount;
    u32 x10_nodeCount;
    u32 x14_bitmapWordCount;
    zeus::CAABox x18_aabb;
    u32* x30_bitmaps;
    u32* x34_indirectionTable;
    u8* x38_entries;

    CAreaRenderOctTree(std::unique_ptr<u8[]>&& buf);

    void FindOverlappingModels(std::vector<u32>& out, const zeus::CAABox& testAABB) const;
};

class CGameArea : public IGameArea
{
    friend class CWorld;

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
            bool xf0_27_ : 1;
            bool xf0_28_ : 1;
        };
        u8 _dummy = 0;
    };

    std::list<std::shared_ptr<const hecl::ClientProcess::BufferTransaction>> xf8_loadTransactions;

public:
    class CAreaFog
    {
        zeus::CVector2f x4_ = {0.f, 1024.f};
        zeus::CVector2f xc_ = {0.f, 1024.f};
        zeus::CVector2f x14_;
        zeus::CVector3f x1c_ = {0.5f};
        zeus::CVector3f x28_ = {0.5f};
        float x34_ = 0.f;
    public:
        void SetCurrent() const;
        void Update(float dt);
        void RollFogOut(float, float, const zeus::CColor& color);
        void FadeFog(ERglFogMode, const zeus::CColor& color, const zeus::CVector2f& vec1,
                     float, const zeus::CVector2f& vec2);
        void SetFogExplicit(ERglFogMode, const zeus::CColor& color, const zeus::CVector2f& vec);
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
        std::unique_ptr<CPVSAreaSet::CPVSAreaHolder> xa0_pvs;
        u32 xa4_elemCount = 1024;
        struct MapEntry
        {
            s16 x0_id = -1;
            TUniqueId x4_uid = kInvalidUniqueId;
        } xa8_map[1024];
        u32 x10a8_pvsVersion = 0;
        TLockedToken<CPFArea> x10ac_path;
        // bool x10b8_ = 0; optional flag for CToken
        u32 x10bc_ = 0;
        std::unique_ptr<CObjectList> x10c0_areaObjs;
        std::unique_ptr<CAreaFog> x10c4_areaFog;
        std::unique_ptr<u8[]> x10c8_sclyBuf;
        u32 x10d0_sclySize = 0;
        u32 x10d4_ = 0;
        u32 x10d8_ = 0;
        u32 x10dc_ = 0;
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
                bool x1108_28_ : 1;
                bool x1108_29_ : 1;
                bool x1108_30_ : 1;
            };
            u8 _dummy = 0;
        };
        std::vector<std::pair<u8*, u32>> x110c_layerPtrs;
        float x111c_thermalCurrent = 0.f;
        float x1120_thermalSpeed = 0.f;
        float x1124_thermalTarget = 0.f;
        float x1128_ = 1.f;
        float x112c_ = 0.f;
        float x1130_ = 1.f;
        float x1134_ = 0.f;
        float x1138_ = 1.f;
        u32 x113c_ = 0;
    };
private:
    std::vector<std::pair<std::unique_ptr<u8[]>, int>> x110_mreaSecBufs;
    std::unique_ptr<CPostConstructed> x12c_postConstructed;

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

    struct CAreaObjectList : public IAreaObjectList
    {
        bool IsQualified(const CEntity& ent);
    };

    enum class EOcclusionState
    {
    };

    CGameArea(CInputStream& in, int idx, int mlvlVersion);

    bool IsFinishedOccluding() const;
    void ReadDependencyList();

    bool IGetScriptingMemoryAlways() const;
    TAreaId IGetAreaId() const;
    ResId IGetAreaAssetId() const;
    bool IIsActive() const;
    TAreaId IGetAttachedAreaId(int) const;
    u32 IGetNumAttachedAreas() const;
    ResId IGetStringTableAssetId() const;
    const zeus::CTransform& IGetTM() const;

    bool DoesAreaNeedEnvFx() const;
    bool DoesAreaNeedSkyNow() const;
    bool OtherAreaOcclusionChanged();
    void PingOcclusionState();
    void PreRender();
    void AliveUpdate(float dt);
    void SetOcclusionState(EOcclusionState state);
    void RemoveStaticGeometry();
    void AddStaticGeometry();
    //void TransferTokensToARAM();
    //void TransferARAMTokensOver();
    void SetChain(CGameArea* other, int);
    void StartStreamingMainArea();
    //void UnloadAllLoadedTextures();
    //void ReloadAllLoadedTextures();
    u32 GetNumPartSizes() const;
    void AllocNewAreaData(int, int);
    void Invalidate(CStateManager& mgr);
    void CullDeadAreaRequests();
    void StartStreamIn(CStateManager& mgr);
    bool Validate(CStateManager& mgr);
    void PostConstructArea();
    void FillInStaticGeometry();
    void VerifyTokenList(CStateManager& stateMgr);
    void ClearTokenList();
    u32 GetPreConstructedSize() const;
    MREAHeader VerifyHeader() const;

    const zeus::CTransform& GetTransform() const {return xc_transform;}
    const zeus::CTransform& GetInverseTransform() const {return x3c_invTransform;}
    const zeus::CAABox& GetAABB() const {return x6c_aabb;}

    const std::vector<Dock> GetDocks() const {return xcc_docks;}

    bool IsPostConstructed() const {return xf0_24_postConstructed;}
    const CPostConstructed* GetPostConstructed() const {return x12c_postConstructed.get();}

};

}

#endif // __URDE_CGAMEAREA_HPP__
