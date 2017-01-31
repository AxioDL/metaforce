#ifndef __URDE_CSCANNABLEOBJECTINFO_HPP__
#define __URDE_CSCANNABLEOBJECTINFO_HPP__

#include "RetroTypes.hpp"
#include "IFactory.hpp"
#include "CToken.hpp"
#include "zeus/CVector2i.hpp"

namespace urde
{
class CScannableObjectInfo
{
public:
    enum class EPanelType
    {
    };

    struct SBucket
    {
        ResId x0_texture = -1;
        float x4_appearanceRange = 0.f;
        u32 x8_imagePos = 0;
        zeus::CVector2i xc_size;
        float x14_interval = 0.f;
        float x18_fadeDuration = 0.f;
        SBucket(CInputStream&, u32 version);
    };

private:
    void Load(CInputStream&, u32);
    ResId x0_scannableObjectId;
    u32 x4_stringId = -1;
    float x8_totalDownloadTime = 0.f;
    u32 xc_category = 0;
    bool x10_important = false;
    rstl::reserved_vector<SBucket, 4> x14_buckets;

public:
    CScannableObjectInfo(CInputStream&, ResId);
    ResId GetStringTableId() const;
    ResId GetScannableObjectId() const;
    float GetTotalDownloadTime() const;
    const SBucket& GetBucket(s32) const;
    bool IsImportant() const;
};

CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&,
                                             CObjectReference* selfRef);
}
#endif // __URDE_CSCANNABLEOBJECTINFO_HPP__
