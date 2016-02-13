#ifndef __PSHAG_CDVDREQUEST_HPP__
#define __PSHAG_CDVDREQUEST_HPP__

namespace pshag
{

class IDvdRequest
{
public:
    virtual void WaitUntilComplete()=0;
    virtual bool IsComplete()=0;
    virtual void PostCancelRequest()=0;

    enum class EMediaType
    {
        ARAM = 0,
        Real = 1,
        NOD = 2
    };
    virtual EMediaType GetMediaType() const=0;
};

class CNODDvdRequest : public IDvdRequest
{
public:
    void WaitUntilComplete();
    bool IsComplete();
    void PostCancelRequest();
    EMediaType GetMediaType() const {return EMediaType::NOD;}
};

class CDvdRequest : public IDvdRequest
{
    void WaitUntilComplete();
    bool IsComplete();
    void PostCancelRequest();
    EMediaType GetMediaType() const { return EMediaType::Real; }
};

}

#endif // __PSHAG_CDVDREQUEST_HPP__
