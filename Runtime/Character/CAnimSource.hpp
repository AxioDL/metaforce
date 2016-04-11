#ifndef __PSHAG_CANIMSOURCE_HPP__
#define __PSHAG_CANIMSOURCE_HPP__

#include "RetroTypes.hpp"
#include "CCharAnimTime.hpp"
#include "zeus/CQuaternion.hpp"
#include "zeus/CVector3f.hpp"
#include "CToken.hpp"

namespace urde
{
class IObjectStore;
class CSegIdList;
class CSegStatementSet;
class CSegId;
class CAnimPOIData;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;

class RotationAndOffsetStorage
{
    friend class CAnimSource;
    std::unique_ptr<float[]> x0_storage;
    u32 x8_frameCount;
    u32 xc_rotPerFrame;
    u32 x10_transPerFrame;

    std::unique_ptr<float[]> GetRotationsAndOffsets(const std::vector<zeus::CQuaternion>& rots,
                                                    const std::vector<zeus::CVector3f>& offs,
                                                    u32 frameCount);
    static void CopyRotationsAndOffsets(const std::vector<zeus::CQuaternion>& rots,
                                        const std::vector<zeus::CVector3f>& offs,
                                        u32 frameCount, float*);
    static u32 DataSizeInBytes(u32 rotPerFrame, u32 transPerFrame, u32 frameCount);

public:
    struct CRotationAndOffsetVectors
    {
        std::vector<zeus::CQuaternion> x0_rotations;
        std::vector<zeus::CVector3f> x10_offsets;
        CRotationAndOffsetVectors(CInputStream& in);
    };
    u32 GetFrameSizeInBytes() const;
    RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors, u32 frameCount);
};

class CAnimSource
{
    float x0_duration;
    u32 x4_;
    float x8_interval;
    u32 xc_;
    u32 x10_frameCount;
    u32 x1c_;
    std::vector<u8> x20_rotationChannels;
    std::vector<u8> x30_translationChannels;
    RotationAndOffsetStorage x40_data;
    TResId x54_evntId;
    TCachedToken<CAnimPOIData> x58_evntData;
    float x60_averageVelocity;

    void CalcAverageVelocity();

public:
    CAnimSource(CInputStream& in, IObjectStore& store);

    void GetSegStatementSet(const CSegIdList& list,
                            CSegStatementSet& set,
                            const CCharAnimTime& time) const;

    const std::vector<CSoundPOINode>& GetSoundPOIStream() const;
    const std::vector<CParticlePOINode>& GetParticlePOIStream() const;
    const std::vector<CInt32POINode>& GetInt32POIStream() const;
    const std::vector<CBoolPOINode>& GetBoolPOIStream() const;
    zeus::CQuaternion GetRotation(const CSegId& seg, const CCharAnimTime& time) const;
    zeus::CVector3f GetOffset(const CSegId& seg, const CCharAnimTime& time) const;
    bool HasOffset(const CSegId& seg) const;
};

}

#endif // __PSHAG_CANIMSOURCE_HPP__
