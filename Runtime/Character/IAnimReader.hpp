#ifndef __PSHAG_IANIMREADER_HPP__
#define __PSHAG_IANIMREADER_HPP__

#include "RetroTypes.hpp"
#include "CCharAnimTime.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CQuaternion.hpp"

namespace urde
{
class CSegId;
class CBoolPOINode;
class CInt32POINode;
class CParticlePOINode;
class CSoundPOINode;
class CSegIdList;
class CSegStatementSet;

struct SAdvancementResults
{
    CCharAnimTime x0_animTime;
    zeus::CVector3f x8_posDelta;
    zeus::CQuaternion x14_rotDelta;
};

class IAnimReader
{
public:
    virtual ~IAnimReader() = default;
    virtual bool IsCAnimTreeNode() const {return false;}
    virtual void VAdvanceView(const CCharAnimTime& a)=0;
    virtual void VGetTimeRemaining() const=0;
    virtual void VGetSteadyStateAnimInfo() const=0;
    virtual bool VHasOffset(const CSegId& seg) const=0;
    virtual void VGetOffset(const CSegId& seg) const=0;
    virtual void VGetRotation(const CSegId& seg) const=0;
    virtual void VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32, u32, u32) const=0;
    virtual void VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32, u32, u32) const=0;
    virtual void VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32, u32, u32) const=0;
    virtual void VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32, u32, u32) const=0;
    virtual void VGetBoolPOIState(const char*) const=0;
    virtual void VGetInt32POIState(const char*) const=0;
    virtual void VGetParticlePOIState(const char*) const=0;
    virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const=0;
    virtual void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const=0;
    virtual void VClone() const=0;
    virtual std::unique_ptr<IAnimReader> VSimplified() {return {};}
    virtual void VSetPhase(float)=0;
    virtual SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
    virtual void Depth() const=0;
    virtual void VGetContributionOfHighestInfluence() const=0;
    virtual void VGetNumChildren() const=0;
    virtual void VGetBestUnblendedChild() const=0;
};

}

#endif // __PSHAG_IANIMREADER_HPP__
