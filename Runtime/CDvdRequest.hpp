#pragma once

namespace urde
{

class IDvdRequest
{
public:
    virtual ~IDvdRequest() = default;

    virtual void WaitUntilComplete()=0;
    virtual bool IsComplete()=0;
    virtual void PostCancelRequest()=0;

    enum class EMediaType
    {
        ARAM = 0,
        Real = 1,
        File = 2,
        NOD = 3
    };
    virtual EMediaType GetMediaType() const=0;
};

}

