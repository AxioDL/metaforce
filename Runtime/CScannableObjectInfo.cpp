#include "CScannableObjectInfo.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
CScannableObjectInfo::CScannableObjectInfo(CInputStream& in, ResId resId) : x0_scannableObjectId(resId)
{
    u32 version = in.readUint32Big();
    Load(in, version);
}

ResId CScannableObjectInfo::GetScannableObjectId() const { return x0_scannableObjectId; }

ResId CScannableObjectInfo::GetStringTableId() const { return x4_stringId; }

float CScannableObjectInfo::GetTotalDownloadTime() const { return x8_totalDownloadTime; }

const CScannableObjectInfo::SBucket& CScannableObjectInfo::GetBucket(s32 idx) const { return x14_buckets[idx]; }

void CScannableObjectInfo::Load(CInputStream& in, u32 version)
{
    in.readUint32Big();
    in.readUint32Big();
    x4_stringId = in.readUint32Big();
    if (version < 4)
        x8_totalDownloadTime = in.readFloatBig();
    else
    {
        u32 scanSpeed = in.readUint32Big();
        x8_totalDownloadTime = g_tweakGui->GetScanSpeed(scanSpeed);
    }
    xc_category = in.readUint32Big();
    if (version > 4)
        x10_important = in.readBool();

    for (u32 i = 0; i < 4; i++)
        x14_buckets.emplace_back(in, version);
}

CScannableObjectInfo::SBucket::SBucket(CInputStream& in, u32 version)
{
    x0_texture = in.readUint32Big();
    x4_appearanceRange = in.readFloatBig();
    x8_imagePos = in.readUint32Big();
    if (version > 1)
    {
        xc_size.x = in.readUint32Big();
        xc_size.y = in.readUint32Big();
        x14_interval = in.readFloatBig();
        if (version >= 3)
            x18_fadeDuration = in.readFloatBig();
    }
}

CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer&,
                                             CObjectReference* selfRef)
{
    return TToken<CScannableObjectInfo>::GetIObjObjectFor(std::make_unique<CScannableObjectInfo>(in, tag.id));
}
}
