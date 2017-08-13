#include "CAnimSource.hpp"
#include "CAnimPOIData.hpp"
#include "CSegId.hpp"
#include "CSegIdList.hpp"
#include "CSegStatementSet.hpp"

namespace urde
{

static float ClampZeroToOne(float in)
{
    return std::max(0.f, std::min(1.f, in));
}

u32 RotationAndOffsetStorage::DataSizeInBytes(u32 rotPerFrame, u32 transPerFrame, u32 frameCount)
{
    return (transPerFrame * 12 + rotPerFrame * 16) * frameCount;
}

void RotationAndOffsetStorage::CopyRotationsAndOffsets(const std::vector<zeus::CQuaternion>& rots,
                                                       const std::vector<zeus::CVector3f>& offs,
                                                       u32 frameCount, float* arrOut)
{
    std::vector<zeus::CQuaternion>::const_iterator rit = rots.cbegin();
    std::vector<zeus::CVector3f>::const_iterator oit = offs.cbegin();
    u32 rotsPerFrame = rots.size() / frameCount;
    u32 offsPerFrame = offs.size() / frameCount;
    for (u32 i=0 ; i<frameCount ; ++i)
    {
        for (u32 j=0 ; j<rotsPerFrame ; ++j)
        {
            const zeus::CQuaternion& rot = *rit++;
            arrOut[0] = rot.w;
            arrOut[1] = rot.x;
            arrOut[2] = rot.y;
            arrOut[3] = rot.z;
            arrOut += 4;
        }
        for (u32 j=0 ; j<offsPerFrame ; ++j)
        {
            const zeus::CVector3f& off = *oit++;
            arrOut[0] = off.x;
            arrOut[1] = off.y;
            arrOut[2] = off.z;
            arrOut += 3;
        }
    }
}

std::unique_ptr<float[]>
RotationAndOffsetStorage::GetRotationsAndOffsets(const std::vector<zeus::CQuaternion>& rots,
                                                 const std::vector<zeus::CVector3f>& offs,
                                                 u32 frameCount)
{
    u32 size = DataSizeInBytes(rots.size() / frameCount, offs.size() / frameCount, frameCount);
    std::unique_ptr<float[]> ret(new float[(size / 4 + 1) * 4]);
    CopyRotationsAndOffsets(rots, offs, frameCount, ret.get());
    return ret;
}

RotationAndOffsetStorage::CRotationAndOffsetVectors::CRotationAndOffsetVectors(CInputStream& in)
{
    u32 quatCount = in.readUint32Big();
    x0_rotations.reserve(quatCount);
    for (u32 i=0 ; i<quatCount ; ++i)
    {
        x0_rotations.emplace_back();
        x0_rotations.back().readBig(in);
    }

    u32 vecCount = in.readUint32Big();
    x10_offsets.reserve(vecCount);
    for (u32 i=0 ; i<vecCount ; ++i)
    {
        x10_offsets.emplace_back();
        x10_offsets.back().readBig(in);
    }
}

u32 RotationAndOffsetStorage::GetFrameSizeInBytes() const
{
    return (x10_transPerFrame * 12 + xc_rotPerFrame * 16);
}

RotationAndOffsetStorage::RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors,
                                                   u32 frameCount)
{
    x0_storage = GetRotationsAndOffsets(vectors.x0_rotations, vectors.x10_offsets, frameCount);
    x8_frameCount = frameCount;
    xc_rotPerFrame = vectors.x0_rotations.size() / frameCount;
    x10_transPerFrame = vectors.x10_offsets.size() / frameCount;
}

static std::vector<u8> ReadIndexTable(CInputStream& in)
{
    std::vector<u8> ret;
    u32 count = in.readUint32Big();
    ret.reserve(count);
    for (u32 i=0 ; i<count ; ++i)
        ret.push_back(in.readUByte());
    return ret;
}

void CAnimSource::CalcAverageVelocity()
{
    u8 rootIdx = x20_rotationChannels[3];
    u8 rootTransIdx = x30_translationChannels[rootIdx];
    float accum = 0.f;
    const u32 floatsPerFrame = x40_data.x10_transPerFrame * 3 + x40_data.xc_rotPerFrame * 4;
    const u32 rotFloatsPerFrame = x40_data.xc_rotPerFrame * 4;
    for (u32 i=1 ; i<x10_frameCount ; ++i)
    {
        const float* frameDataA =
            &x40_data.x0_storage[(i-1)*floatsPerFrame+rotFloatsPerFrame+rootTransIdx*3];
        const float* frameDataB =
            &x40_data.x0_storage[i*floatsPerFrame+rotFloatsPerFrame+rootTransIdx*3];
        zeus::CVector3f vecA(frameDataA[0], frameDataA[1], frameDataA[2]);
        zeus::CVector3f vecB(frameDataB[0], frameDataB[1], frameDataB[2]);
        float frameVel = (vecB - vecA).magnitude();
        if (frameVel > 0.00001f)
            accum += frameVel;
    }
    x60_averageVelocity = accum / x0_duration.GetSeconds();
}

CAnimSource::CAnimSource(CInputStream& in, IObjectStore& store)
: x0_duration(in),
  x8_interval(in),
  x10_frameCount(in.readUint32Big()),
  x1c_rootBone(in),
  x20_rotationChannels(ReadIndexTable(in)),
  x30_translationChannels(ReadIndexTable(in)),
  x40_data(RotationAndOffsetStorage::CRotationAndOffsetVectors(in), x10_frameCount),
  x54_evntId(in.readUint32Big())
{
    if (x54_evntId.IsValid())
    {
        x58_evntData = store.GetObj({SBIG('EVNT'), x54_evntId});
        x58_evntData.GetObj();
    }
    CalcAverageVelocity();
}

void CAnimSource::GetSegStatementSet(const CSegIdList& list,
                                     CSegStatementSet& set,
                                     const CCharAnimTime& time) const
{
    u32 frameIdx = unsigned(time / x8_interval);
    float remTime = time.GetSeconds() - frameIdx * x8_interval.GetSeconds();
    if (std::fabs(remTime) < 0.00001f)
        remTime = 0.f;
    float t = ClampZeroToOne(remTime / x8_interval.GetSeconds());
    const u32 floatsPerFrame = x40_data.x10_transPerFrame * 3 + x40_data.xc_rotPerFrame * 4;
    const u32 rotFloatsPerFrame = x40_data.xc_rotPerFrame * 4;

    for (const CSegId& id : list.GetList())
    {
        u8 rotIdx = x20_rotationChannels[id];
        if (rotIdx != 0xff)
        {
            const float* frameDataA =
                &x40_data.x0_storage[frameIdx*floatsPerFrame+rotIdx*4];
            const float* frameDataB =
                &x40_data.x0_storage[(frameIdx+1)*floatsPerFrame+rotIdx*4];

            zeus::CQuaternion quatA(frameDataA[0], frameDataA[1], frameDataA[2], frameDataA[3]);
            zeus::CQuaternion quatB(frameDataB[0], frameDataB[1], frameDataB[2], frameDataB[3]);
            set.x4_segData[id].x0_rotation = zeus::CQuaternion::slerp(quatA, quatB, t);

            u8 transIdx = x30_translationChannels[rotIdx];
            if (transIdx != 0xff)
            {
                const float* frameDataA =
                    &x40_data.x0_storage[frameIdx*floatsPerFrame+rotFloatsPerFrame+transIdx*3];
                const float* frameDataB =
                    &x40_data.x0_storage[(frameIdx-1)*floatsPerFrame+rotFloatsPerFrame+transIdx*3];
                zeus::CVector3f vecA(frameDataA[0], frameDataA[1], frameDataA[2]);
                zeus::CVector3f vecB(frameDataB[0], frameDataB[1], frameDataB[2]);
                set.x4_segData[id].x10_offset = zeus::CVector3f::lerp(vecA, vecB, t);
                set.x4_segData[id].x1c_hasOffset = true;
            }
        }
    }
}

const std::vector<CSoundPOINode>& CAnimSource::GetSoundPOIStream() const
{
    return x58_evntData->GetSoundPOIStream();
}

const std::vector<CParticlePOINode>& CAnimSource::GetParticlePOIStream() const
{
    return x58_evntData->GetParticlePOIStream();
}

const std::vector<CInt32POINode>& CAnimSource::GetInt32POIStream() const
{
    return x58_evntData->GetInt32POIStream();
}

const std::vector<CBoolPOINode>& CAnimSource::GetBoolPOIStream() const
{
    return x58_evntData->GetBoolPOIStream();
}

zeus::CQuaternion CAnimSource::GetRotation(const CSegId& seg, const CCharAnimTime& time) const
{
    u8 rotIdx = x20_rotationChannels[seg];
    if (rotIdx != 0xff)
    {
        u32 frameIdx = unsigned(time / x8_interval);
        float remTime = time.GetSeconds() - frameIdx * x8_interval.GetSeconds();
        if (std::fabs(remTime) < 0.00001f)
            remTime = 0.f;
        float t = ClampZeroToOne(remTime / x8_interval.GetSeconds());

        const u32 floatsPerFrame = x40_data.x10_transPerFrame * 3 + x40_data.xc_rotPerFrame * 4;
        const float* frameDataA =
            &x40_data.x0_storage[frameIdx*floatsPerFrame+rotIdx*4];
        const float* frameDataB =
            &x40_data.x0_storage[(frameIdx+1)*floatsPerFrame+rotIdx*4];

        zeus::CQuaternion quatA(frameDataA[0], frameDataA[1], frameDataA[2], frameDataA[3]);
        zeus::CQuaternion quatB(frameDataB[0], frameDataB[1], frameDataB[2], frameDataB[3]);
        return zeus::CQuaternion::slerp(quatA, quatB, t);
    }
    else
    {
        return {};
    }
}

zeus::CVector3f CAnimSource::GetOffset(const CSegId& seg, const CCharAnimTime& time) const
{
    u8 rotIdx = x20_rotationChannels[seg];
    if (rotIdx != 0xff)
    {
        u8 transIdx = x30_translationChannels[rotIdx];
        if (transIdx == 0xff)
            return {};

        u32 frameIdx = unsigned(time / x8_interval);
        float remTime = time.GetSeconds() - frameIdx * x8_interval.GetSeconds();
        if (std::fabs(remTime) < 0.00001f)
            remTime = 0.f;
        float t = ClampZeroToOne(remTime / x8_interval.GetSeconds());

        const u32 floatsPerFrame = x40_data.x10_transPerFrame * 3 + x40_data.xc_rotPerFrame * 4;
        const u32 rotFloatsPerFrame = x40_data.xc_rotPerFrame * 4;
        const float* frameDataA =
            &x40_data.x0_storage[frameIdx*floatsPerFrame+rotFloatsPerFrame+transIdx*3];
        const float* frameDataB =
            &x40_data.x0_storage[(frameIdx-1)*floatsPerFrame+rotFloatsPerFrame+transIdx*3];
        zeus::CVector3f vecA(frameDataA[0], frameDataA[1], frameDataA[2]);
        zeus::CVector3f vecB(frameDataB[0], frameDataB[1], frameDataB[2]);
        return zeus::CVector3f::lerp(vecA, vecB, t);
    }
    else
    {
        return {};
    }
}

bool CAnimSource::HasOffset(const CSegId& seg) const
{
    u8 rotIdx = x20_rotationChannels[seg];
    if (rotIdx == 0xff)
        return false;
    u8 transIdx = x30_translationChannels[rotIdx];
    return transIdx != 0xff;
}

}
