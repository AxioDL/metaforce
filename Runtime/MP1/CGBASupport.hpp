#ifndef __URDE_CGBASUPPORT_HPP__
#define __URDE_CGBASUPPORT_HPP__

#include "CDvdFile.hpp"

namespace urde
{
namespace MP1
{

class CGBASupport : public CDvdFile
{
public:
    enum class EPhase
    {
        LoadClientPad,
        Standby,
        StartProbeTimeout,
        PollProbe,
        StartJoyBusBoot,
        PollJoyBusBoot,
        DataTransfer,
        Complete,
        Failed
    };

private:
    u32 x28_fileSize;
    std::unique_ptr<u8[]> x2c_buffer;
    std::shared_ptr<IDvdRequest> x30_dvdReq;
    EPhase x34_phase = EPhase::LoadClientPad;
    float x38_timeout = 0.f;
    u8 x3c_status = 0;
    u32 x40_siChan = -1;
    bool x44_fusionLinked = false;
    bool x45_fusionBeat = false;
    static CGBASupport* SharedInstance;

public:
    CGBASupport();
    ~CGBASupport();
    bool PollResponse();
    void Update(float dt);
    bool IsReady();
    void InitializeSupport();
    void StartLink();
    EPhase GetPhase() const { return x34_phase; }
    bool IsFusionLinked() const { return x44_fusionLinked; }
    bool IsFusionBeat() const { return x45_fusionBeat; }
};

}
}

#endif // __URDE_CGBASUPPORT_HPP__
