#include "CGBASupport.hpp"

namespace urde
{
namespace MP1
{

CGBASupport* CGBASupport::SharedInstance = nullptr;

CGBASupport::CGBASupport()
: CDvdFile("client_pad.bin")
{
    x28_fileSize = ROUND_UP_32(Length());
    x2c_buffer.reset(new u8[x28_fileSize]);
    x30_dvdReq = AsyncRead(x2c_buffer.get(), x28_fileSize);
    //InitDSPComm();
    SharedInstance = this;
}

CGBASupport::~CGBASupport()
{
    SharedInstance = nullptr;
}

}
}
