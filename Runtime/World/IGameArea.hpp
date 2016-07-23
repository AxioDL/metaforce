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
            u32 x0_area;
            s16 x4_dock;
            union
            {
                struct
                {
                    bool x6_16_ : 1;
                };
                u16 x6_ = 0;
            };
            SDockReference() = default;
        };
    private:
        u32 x0_ = 0;
        std::vector<SDockReference> x4_dockReferences;
        rstl::reserved_vector<zeus::CVector3f, 4> x14_planeVertices;
        bool x48_;
    public:

        u32 GetReferenceCount() const { return x0_; }
        Dock(CInputStream& in, const zeus::CTransform& xf);
        TAreaId GetConnectedAreaId(s32 other) const;
        s16 GetOtherDockNumber(s32 other) const;
        bool GetShouldLoadOther(s32 other) const;
        void SetShouldLoadOther(s32 other, bool should);
        bool ShouldLoadOtherArea(s32 other) const;
        zeus::CVector3f GetPoint(s32 idx) const;
    };

    struct IAreaObjectList
    {
        virtual bool IsQualified(const CEntity& ent)=0;
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
}

#endif // __URDE_IGAMEAREA_HPP__
