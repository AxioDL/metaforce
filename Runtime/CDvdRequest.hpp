#ifndef __RETRO_CDVDREQUEST_HPP__
#define __RETRO_CDVDREQUEST_HPP__

namespace Retro
{

class IDvdRequest
{
public:
    virtual void WaitUntilComplete()=0;
    virtual bool IsComplete()=0;
    virtual void PostCancelRequest()=0;

    enum EMediaType
    {
        MediaARAM = 0,
        MediaReal = 1,
        MediaNOD = 2
    };
    virtual EMediaType GetMediaType() const=0;
};

class CNODDvdRequest : public IDvdRequest
{
public:
    void WaitUntilComplete();
    bool IsComplete();
    void PostCancelRequest();
    EMediaType GetMediaType() const {return MediaNOD;}
};

class CDvdRequest : public IDvdRequest
{
    void WaitUntilComplete();
    bool IsComplete();
    void PostCancelRequest();
    EMediaType GetMediaType() const { return MediaReal; }
};

}

#endif // __RETRO_CDVDREQUEST_HPP__
