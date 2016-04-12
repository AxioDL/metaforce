#include "CAnimSourceReader.hpp"
#include "CBoolPOINode.hpp"
#include "CInt32POINode.hpp"
#include "CParticlePOINode.hpp"
#include "CSoundPOINode.hpp"

namespace urde
{

CAnimSourceInfo::CAnimSourceInfo(const TSubAnimTypeToken<CAnimSource>& token)
: x4_token(token) {}

bool CAnimSourceInfo::HasPOIData() const
{
    return x4_token->x58_evntData;
}

const std::vector<CBoolPOINode>& CAnimSourceInfo::GetBoolPOIStream() const
{
    return x4_token->GetBoolPOIStream();
}

const std::vector<CInt32POINode>& CAnimSourceInfo::GetInt32POIStream() const
{
    return x4_token->GetInt32POIStream();
}

const std::vector<CParticlePOINode>& CAnimSourceInfo::GetParticlePOIStream() const
{
    return x4_token->GetParticlePOIStream();
}

const std::vector<CSoundPOINode>& CAnimSourceInfo::GetSoundPOIStream() const
{
    return x4_token->GetSoundPOIStream();
}

CCharAnimTime CAnimSourceInfo::GetAnimationDuration() const
{
    return x4_token->GetDuration();
}

std::map<std::string, CParticleData::EParentedMode>
CAnimSourceReaderBase::GetUniqueParticlePOIs() const
{
    const std::vector<CParticlePOINode>& particleNodes = x4_sourceInfo->GetParticlePOIStream();
    std::map<std::string, CParticleData::EParentedMode> ret;
    for (const CParticlePOINode& node : particleNodes)
        ret[node.GetName()] = node.GetData().GetParentedMode();
    return ret;
}

std::map<std::string, s32>
CAnimSourceReaderBase::GetUniqueInt32POIs() const
{
    const std::vector<CInt32POINode>& int32Nodes = x4_sourceInfo->GetInt32POIStream();
    std::map<std::string, s32> ret;
    for (const CInt32POINode& node : int32Nodes)
        ret[node.GetName()] = node.GetValue();
    return ret;
}

std::map<std::string, bool>
CAnimSourceReaderBase::GetUniqueBoolPOIs() const
{
    const std::vector<CBoolPOINode>& boolNodes = x4_sourceInfo->GetBoolPOIStream();
    std::map<std::string, bool> ret;
    for (const CBoolPOINode& node : boolNodes)
        ret[node.GetName()] = node.GetValue();
    return ret;
}

void CAnimSourceReaderBase::PostConstruct(const CCharAnimTime& time)
{
    x14_passedBoolCount = 0;
    x18_passedIntCount = 0;
    x1c_passedParticleCount = 0;
    x20_passedSoundCount = 0;

    if (x4_sourceInfo->HasPOIData())
    {
        std::map<std::string, bool> boolPOIs = GetUniqueBoolPOIs();
        std::map<std::string, s32> int32POIs = GetUniqueInt32POIs();
        std::map<std::string, CParticleData::EParentedMode> particlePOIs = GetUniqueParticlePOIs();

        x24_boolStates.reserve(boolPOIs.size());
        for (const auto& poi : boolPOIs)
            x24_boolStates.push_back(poi);

        x34_int32States.reserve(int32POIs.size());
        for (const auto& poi : int32POIs)
            x34_int32States.push_back(poi);

        x44_particleStates.reserve(particlePOIs.size());
        for (const auto& poi : particlePOIs)
            x44_particleStates.push_back(poi);
    }

    CCharAnimTime tmpTime = time;
    if (tmpTime.GreaterThanZero())
    {
        while (tmpTime.GreaterThanZero())
        {
            SAdvancementResults res = VAdvanceView(tmpTime);
            tmpTime = res.x0_remTime;
        }
    }
    else if (x4_sourceInfo->HasPOIData())
    {
        UpdatePOIStates();
        if (!time.GreaterThanZero())
        {
            x14_passedBoolCount = 0;
            x18_passedIntCount = 0;
            x1c_passedParticleCount = 0;
            x20_passedSoundCount = 0;
        }
    }
}

void CAnimSourceReaderBase::UpdatePOIStates()
{
    const std::vector<CBoolPOINode>& boolNodes = x4_sourceInfo->GetBoolPOIStream();
    const std::vector<CInt32POINode>& int32Nodes = x4_sourceInfo->GetInt32POIStream();
    const std::vector<CParticlePOINode>& particleNodes = x4_sourceInfo->GetParticlePOIStream();
    const std::vector<CSoundPOINode>& soundNodes = x4_sourceInfo->GetSoundPOIStream();

    for (const CBoolPOINode& node : boolNodes)
    {
        if (node.GetTime() > xc_curTime)
            break;
        if (node.GetIndex() != -1)
            x24_boolStates[node.GetIndex()].second = node.GetValue();
        ++x14_passedBoolCount;
    }

    for (const CInt32POINode& node : int32Nodes)
    {
        if (node.GetTime() > xc_curTime)
            break;
        if (node.GetIndex() != -1)
            x34_int32States[node.GetIndex()].second = node.GetValue();
        ++x18_passedIntCount;
    }

    for (const CParticlePOINode& node : particleNodes)
    {
        if (node.GetTime() > xc_curTime)
            break;
        if (node.GetIndex() != -1)
            x44_particleStates[node.GetIndex()].second = node.GetData().GetParentedMode();
        ++x1c_passedParticleCount;
    }

    for (const CSoundPOINode& node : soundNodes)
    {
        if (node.GetTime() > xc_curTime)
            break;
        ++x20_passedSoundCount;
    }
}

u32 CAnimSourceReaderBase::VGetBoolPOIList(const CCharAnimTime& time,
                                           CBoolPOINode* listOut,
                                           u32 capacity, u32 iterator, u32 unk) const
{
    if (x4_sourceInfo->HasPOIData())
    {
        const std::vector<CBoolPOINode>& boolNodes = x4_sourceInfo->GetBoolPOIStream();
        return CBoolPOINode::_getPOIList(time, listOut, capacity, iterator, unk, boolNodes,
                                         xc_curTime, *x4_sourceInfo, x14_passedBoolCount);
    }
    return 0;
}

u32 CAnimSourceReaderBase::VGetInt32POIList(const CCharAnimTime& time,
                                            CInt32POINode* listOut,
                                            u32 capacity, u32 iterator, u32 unk) const
{
    if (x4_sourceInfo->HasPOIData())
    {
        const std::vector<CInt32POINode>& int32Nodes = x4_sourceInfo->GetInt32POIStream();
        return CInt32POINode::_getPOIList(time, listOut, capacity, iterator, unk, int32Nodes,
                                          xc_curTime, *x4_sourceInfo, x18_passedIntCount);
    }
    return 0;
}

u32 CAnimSourceReaderBase::VGetParticlePOIList(const CCharAnimTime& time,
                                               CParticlePOINode* listOut,
                                               u32 capacity, u32 iterator, u32 unk) const
{
    if (x4_sourceInfo->HasPOIData())
    {
        const std::vector<CParticlePOINode>& particleNodes = x4_sourceInfo->GetParticlePOIStream();
        return CParticlePOINode::_getPOIList(time, listOut, capacity, iterator, unk, particleNodes,
                                             xc_curTime, *x4_sourceInfo, x1c_passedParticleCount);
    }
    return 0;
}

u32 CAnimSourceReaderBase::VGetSoundPOIList(const CCharAnimTime& time,
                                            CSoundPOINode* listOut,
                                            u32 capacity, u32 iterator, u32 unk) const
{
    if (x4_sourceInfo->HasPOIData())
    {
        const std::vector<CSoundPOINode>& soundNodes = x4_sourceInfo->GetSoundPOIStream();
        return CSoundPOINode::_getPOIList(time, listOut, capacity, iterator, unk, soundNodes,
                                          xc_curTime, *x4_sourceInfo, x20_passedSoundCount);
    }
    return 0;
}

bool CAnimSourceReaderBase::VGetBoolPOIState(const char* name) const
{
    for (const auto& node : x24_boolStates)
        if (!node.first.compare(name))
            return node.second;
    return false;
}

s32 CAnimSourceReaderBase::VGetInt32POIState(const char* name) const
{
    for (const auto& node : x34_int32States)
        if (!node.first.compare(name))
            return node.second;
    return 0;
}

CParticleData::EParentedMode
CAnimSourceReaderBase::VGetParticlePOIState(const char* name) const
{
    for (const auto& node : x44_particleStates)
        if (!node.first.compare(name))
            return node.second;
    return CParticleData::EParentedMode::Initial;
}

CAnimSourceReaderBase::CAnimSourceReaderBase(std::unique_ptr<IAnimSourceInfo>&& sourceInfo,
                                             const CCharAnimTime& time)
: x4_sourceInfo(std::move(sourceInfo)), xc_curTime(time) {}

SAdvancementResults CAnimSourceReader::VGetAdvancementResults(const CCharAnimTime& a,
                                                              const CCharAnimTime& b) const
{
}

void CAnimSourceReader::VSetPhase(float)
{
}

SAdvancementResults CAnimSourceReader::VReverseView(const CCharAnimTime& time)
{
}

std::shared_ptr<IAnimReader> CAnimSourceReader::VClone() const
{
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list,
                                            CSegStatementSet& setOut) const
{
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list,
                                            CSegStatementSet& setOut,
                                            const CCharAnimTime& time) const
{
}

SAdvancementResults CAnimSourceReader::VAdvanceView(const CCharAnimTime& a)
{
}

CCharAnimTime CAnimSourceReader::VGetTimeRemaining() const
{
}

void CAnimSourceReader::VGetSteadyStateAnimInfo() const
{
}

bool CAnimSourceReader::VHasOffset(const CSegId& seg) const
{
}

zeus::CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg) const
{
}

zeus::CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg,
                                              const CCharAnimTime& time) const
{
}

zeus::CQuaternion CAnimSourceReader::VGetRotation(const CSegId& seg) const
{
}

CAnimSourceReader::CAnimSourceReader(const TSubAnimTypeToken<CAnimSource>& source,
                                     const CCharAnimTime& time)
: CAnimSourceReaderBase(std::make_unique<CAnimSourceInfo>(source), CCharAnimTime()),
  x54_source(source)
{
    CAnimSource* sourceData = x54_source.GetObj();
    x64_duration = sourceData->GetDuration();
    x6c_curRootOffset = sourceData->GetOffset(sourceData->GetRootBoneId(), time);
    PostConstruct(time);
}


}
