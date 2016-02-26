#ifndef __PSHAG_CSTATICINTERFERENCE_HPP__
#define __PSHAG_CSTATICINTERFERENCE_HPP__

#include <vector>
#include "RetroTypes.hpp"

namespace pshag
{
class CStateManager;

struct CStaticInterferenceSource
{
    TUniqueId id;
    float magnitude;
    float timeLeft;
};

class CStaticInterference
{
    std::vector<CStaticInterferenceSource> m_sources;
public:
    CStaticInterference(int sourceCount)
    {
        m_sources.reserve(sourceCount);
    }
    void AddSource(TUniqueId id, float magnitude, float duration)
    {
        for (CStaticInterferenceSource& src : m_sources)
        {
            if (src.id == id)
            {
                src.magnitude = magnitude;
                src.timeLeft = duration;
                return;
            }
        }
        m_sources.push_back({id, magnitude, duration});
    }

    void RemoveSource(TUniqueId id)
    {
        auto iter = std::find_if(m_sources.begin(), m_sources.end(), [&id](const CStaticInterferenceSource& src)->bool{
            return src.id == id;
        });
        if (iter != m_sources.end())
            m_sources.erase(iter);
    }

    void Update(CStateManager&, float dt)
    {
        std::vector<CStaticInterferenceSource> newSources;
        newSources.reserve(m_sources.size());
        for (CStaticInterferenceSource& src : m_sources)
        {
            if (src.timeLeft >= 0.0)
            {
                src.timeLeft -= dt;
                newSources.push_back(src);
            }
        }
        m_sources = std::move(newSources);
    }

    float GetTotalInterference() const
    {
        float validAccum = 0.0;
        float invalidAccum = 0.0;
        for (const CStaticInterferenceSource& src : m_sources)
        {
            if (src.id == kInvalidUniqueId)
                invalidAccum += src.magnitude;
            else
                validAccum += src.magnitude;
        }
        if (validAccum > 0.80000001)
            validAccum = 0.80000001;
        validAccum += invalidAccum;
        if (validAccum > 1.0)
            return 1.0;
        return validAccum;
    }
};

}

#endif // __PSHAG_CSTATICINTERFERENCE_HPP__
