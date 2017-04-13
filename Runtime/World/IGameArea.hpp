#ifndef __URDE_IGAMEAREA_HPP__
#define __URDE_IGAMEAREA_HPP__

#include "RetroTypes.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{
class CEntity;

class IGameArea
{
public:
    class Dock
    {
    public:
        struct SDockReference
        {
            u32 x0_area = 0;
            s16 x4_dock = 0;
            bool x6_loadOther = false;
            SDockReference() = default;
        };
    private:
        u32 x0_referenceCount = 0;
        std::vector<SDockReference> x4_dockReferences;
        rstl::reserved_vector<zeus::CVector3f, 4> x14_planeVertices;
        bool x48_isReferenced;
    public:

        const rstl::reserved_vector<zeus::CVector3f, 4>& GetPlaneVertices() const {return x14_planeVertices;}
        u32 GetReferenceCount() const { return x0_referenceCount; }
        const std::vector<SDockReference>& GetDockRefs() const { return x4_dockReferences; }
        Dock(CInputStream& in, const zeus::CTransform& xf);
        TAreaId GetConnectedAreaId(s32 other) const;
        s16 GetOtherDockNumber(s32 other) const;
        bool GetShouldLoadOther(s32 other) const;
        void SetShouldLoadOther(s32 other, bool should);
        bool ShouldLoadOtherArea(s32 other) const;
        zeus::CVector3f GetPoint(s32 idx) const;
        bool IsReferenced() const { return x48_isReferenced; }
        void SetReferenceCount(s32 v) { x0_referenceCount = v; x48_isReferenced = true; }
    };

    virtual bool IGetScriptingMemoryAlways() const=0;
    virtual TAreaId IGetAreaId() const=0;
    virtual ResId IGetAreaAssetId() const=0;
    virtual bool IIsActive() const=0;
    virtual TAreaId IGetAttachedAreaId(int) const=0;
    virtual u32 IGetNumAttachedAreas() const=0;
    virtual ResId IGetStringTableAssetId() const=0;
    virtual const zeus::CTransform& IGetTM() const=0;
};

enum class EChain
{
    Invalid = -1,
    ToDeallocate,
    Deallocated,
    Loading,
    Alive,
    AliveJudgement
};

}

#endif // __URDE_IGAMEAREA_HPP__
