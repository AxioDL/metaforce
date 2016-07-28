#ifndef __URDE_CGAMEAREA_HPP__
#define __URDE_CGAMEAREA_HPP__

#include "zeus/CVector2f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CAABox.hpp"
#include "CToken.hpp"
#include "RetroTypes.hpp"
#include "IGameArea.hpp"
#include "CAreaOctTree.hpp"
#include "hecl/ClientProcess.hpp"
#include "Graphics/CMetroidModelInstance.hpp"

namespace urde
{
class CStateManager;

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

    std::vector<u32> xbc_;
    std::vector<Dock> xcc_docks;
    std::vector<CToken> xdc_tokens;

    u32 xec_totalResourcesSize = 0;

    union
    {
        struct
        {
            bool xf0_24_postConstructed : 1;
            bool xf0_25_active : 1;
            bool xf0_26_ : 1;
            bool xf0_27_ : 1;
            bool xf0_28_ : 1;
        };
        u8 _dummy = 0;
    };

    std::list<std::shared_ptr<const hecl::ClientProcess::BufferTransaction>> xf8_loadTransactions;

public:
    struct CPostConstructed
    {
        std::unique_ptr<uint8_t[]> x0_;
        u32 x8_ = 0;
        std::experimental::optional<CAreaOctTree> xc_octTree;
        std::vector<CMetroidModelInstance> x4c_insts;
        std::unique_ptr<uint8_t> x5c_;
        //std::vector<Something 68 bytes> x60_;
        //std::vector<Something 80 bytes> x70_;
        //std::vector<Something 68 bytes> x80_;
        //std::vector<Something 80 bytes> x90_;
        std::unique_ptr<uint8_t> xa0_;
        u32 xa4_elemCount = 1024;
        struct MapEntry
        {
            s16 x0_id = -1;
            TUniqueId x4_uid = kInvalidUniqueId;
        } xa8_map[1024];
        u32 x10a8_ = 0;
        CToken x10ac_;
        // bool x10b8_ = 0; optional flag for CToken
        u32 x10bc_ = 0;
        std::unique_ptr<uint8_t[]> x10c0_;
        std::unique_ptr<uint8_t[]> x10c4_;
        std::unique_ptr<uint8_t> x10c8_;
        u32 x10d0_ = 0;
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
        // std::vector<Something 8 bytes> x110c_;
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
    std::unique_ptr<CPostConstructed> x12c_postConstructed;

    void UpdateFog(float dt);
    void UpdateThermalVisor(float dt);

public:

    struct CAreaObjectList : public IAreaObjectList
    {
        bool IsQualified(const CEntity& ent);
    };

    enum class EOcclusionState
    {
    };

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
    void VerifyTokenList();
    void ClearTokenList();
    u32 GetPreConstructedSize() const;
    bool VerifyHeader() const;

    const zeus::CTransform& GetTransform() const {return xc_transform;}
    const zeus::CTransform& GetInverseTransform() const {return x3c_invTransform;}
    const zeus::CAABox& GetAABB() const {return x6c_aabb;}

    const std::vector<Dock> GetDocks() const {return xcc_docks;}

    bool IsPostConstructed() const {return xf0_24_postConstructed;}
    const CPostConstructed* GetPostConstructed() const {return x12c_postConstructed.get();}

};

}

#endif // __URDE_CGAMEAREA_HPP__
