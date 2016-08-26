#ifndef __URDE_IANIMREADER_HPP__
#define __URDE_IANIMREADER_HPP__

#include "RetroTypes.hpp"
#include "CCharAnimTime.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CQuaternion.hpp"
#include "CParticleData.hpp"
#include "CToken.hpp"

namespace urde
{
class CSegId;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;
class CSegIdList;
class CSegStatementSet;

struct SAdvancementDeltas
{
    zeus::CVector3f x0_posDelta;
    zeus::CQuaternion xc_rotDelta;
};

struct SAdvancementResults
{
    CCharAnimTime x0_remTime;
    SAdvancementDeltas x8_deltas;
};

struct CSteadyStateAnimInfo
{
    CCharAnimTime x64_duration;
    zeus::CVector3f x6c_curRootOffset;
    bool x78_ = false;
};

template <class T>
using TSubAnimTypeToken = TCachedToken<T>;

class IAnimReader
{
public:
    virtual ~IAnimReader() = default;
    virtual bool IsCAnimTreeNode() const {return false;}
    virtual SAdvancementResults VAdvanceView(const CCharAnimTime& a)=0;
    virtual CCharAnimTime VGetTimeRemaining() const=0;
    virtual CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const=0;
    virtual bool VHasOffset(const CSegId& seg) const=0;
    virtual zeus::CVector3f VGetOffset(const CSegId& seg) const=0;
    virtual zeus::CQuaternion VGetRotation(const CSegId& seg) const=0;
    virtual u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const=0;
    virtual u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const=0;
    virtual u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const=0;
    virtual u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const=0;
    virtual bool VGetBoolPOIState(const char*) const=0;
    virtual s32 VGetInt32POIState(const char*) const=0;
    virtual CParticleData::EParentedMode VGetParticlePOIState(const char*) const=0;
    virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const=0;
    virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const=0;
    virtual std::shared_ptr<IAnimReader> VClone() const=0;
    virtual std::shared_ptr<IAnimReader> VSimplified() {return {};}
    virtual void VSetPhase(float)=0;
    virtual SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;

    u32 GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
};

}

#endif // __URDE_IANIMREADER_HPP__
