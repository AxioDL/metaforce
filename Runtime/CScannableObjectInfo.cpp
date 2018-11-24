#include "CScannableObjectInfo.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
CScannableObjectInfo::CScannableObjectInfo(CInputStream& in, CAssetId resId) : x0_scannableObjectId(resId)
{
    u32 version = in.readUint32Big();
    Load(in, version);

    for (u32 i = 0 ; i < x14_buckets.size(); ++i)
    {
        x14_buckets[i].x4_appearanceRange *= x8_totalDownloadTime;
    }

    float appearanceOffset = g_tweakGui->GetScanAppearanceDuration();
    for (u32 i = 0 ; i < x14_buckets.size(); ++i)
    {
        if (x14_buckets[i].x8_imagePos != -1)
        {
            x8_totalDownloadTime += appearanceOffset;
            for (u32 j = i; j < x14_buckets.size(); j++)
                x14_buckets[j].x4_appearanceRange += appearanceOffset;
        }
    }

    for (u32 i = 0; i < x14_buckets.size() - 1; ++i)
    {
        for (u32 j = i + 1; j < x14_buckets.size(); ++j)
        {
            if (x14_buckets[i].x8_imagePos == x14_buckets[j].x8_imagePos && x14_buckets[i].x8_imagePos != -1)
                x14_buckets[j].x8_imagePos = -1;
        }
    }
}

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
