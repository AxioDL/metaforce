#ifndef __URDE_CGBASUPPORT_HPP__
#define __URDE_CGBASUPPORT_HPP__

#include "CDvdFile.hpp"

namespace urde
{
namespace MP1
{

class CGBASupport : public CDvdFile
{
    u32 x28_fileSize;
    std::unique_ptr<u8[]> x2c_buffer;
    std::shared_ptr<IDvdRequest> x30_dvdReq;
    u32 x34_ = 0;
    float x38_ = 0.f;
    bool x3c_ = false;
    u32 x40_ = -1;
    bool x44_ = false;
    bool x45_ = false;
    static CGBASupport* SharedInstance;
public:
    CGBASupport();
    ~CGBASupport();
    bool IsReady() const;
};

}
}

#endif // __URDE_CGBASUPPORT_HPP__
