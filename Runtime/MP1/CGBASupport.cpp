#include "CGBASupport.hpp"
#include "CDvdRequest.hpp"
#include "CBasics.hpp"

namespace urde
{
namespace MP1
{

#define GBA_JSTAT_MASK                  0x3a
#define GBA_JSTAT_FLAGS_SHIFT           4
#define GBA_JSTAT_FLAGS_MASK            0x30
#define GBA_JSTAT_PSF1                  0x20
#define GBA_JSTAT_PSF0                  0x10
#define GBA_JSTAT_SEND                  0x08
#define GBA_JSTAT_RECV                  0x02

#define GBA_READY                       0
#define GBA_NOT_READY                   1
#define GBA_BUSY                        2
#define GBA_JOYBOOT_UNKNOWN_STATE       3
#define GBA_JOYBOOT_ERR_INVALID         4

static s32 GBAJoyBootAsync(s32 chan, s32 paletteColor, s32 paletteSpeed,
                           u8* programp, s32 length, u8* status)
{
    return GBA_READY;
}

static s32 GBAGetProcessStatus(s32 chan, u8* percentp)
{
    return GBA_READY;
}

static s32 GBAGetStatus(s32 chan, u8* status)
{
    return GBA_READY;
}

static s32 GBAReset(s32 chan, u8* status)
{
    return GBA_READY;
}

static s32 GBARead(s32 chan, u8* dst, u8* status)
{
    return GBA_READY;
}

static s32 GBAWrite(s32 chan, u8* src, u8* status)
{
    return GBA_READY;
}

CGBASupport* CGBASupport::SharedInstance = nullptr;

CGBASupport::CGBASupport()
: CDvdFile("client_pad.bin")
{
    x28_fileSize = ROUND_UP_32(Length());
    x2c_buffer.reset(new u8[x28_fileSize]);
    x30_dvdReq = AsyncRead(x2c_buffer.get(), x28_fileSize);
    //GBAInit();
    SharedInstance = this;
}

CGBASupport::~CGBASupport()
{
    SharedInstance = nullptr;
}

bool CGBASupport::PollResponse()
{
    u8 status;
    if (GBAReset(x40_siChan, &status) == GBA_NOT_READY)
        if (GBAReset(x40_siChan, &status) == GBA_NOT_READY)
            return false;

    if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
        return false;
    if (status != (GBA_JSTAT_PSF1 | GBA_JSTAT_SEND))
        return false;

    u8 bytes[4];
    if (GBARead(x40_siChan, bytes, &status) == GBA_NOT_READY)
        return false;
    if (reinterpret_cast<u32&>(bytes) != SBIG('AMTE'))
        return false;

    if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
        return false;
    if (status != GBA_JSTAT_PSF1)
        return false;

    if (GBAWrite(x40_siChan, (unsigned char*)"AMTE", &status) == GBA_NOT_READY)
        return false;

    if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
        return false;
    if ((status & GBA_JSTAT_FLAGS_MASK) != GBA_JSTAT_FLAGS_MASK)
        return false;

    u64 profStart = CBasics::GetGCTicks();
    const u64 timeToSpin = 486000000 / 8000;
    for (;;)
    {
        u64 curTime = CBasics::GetGCTicks();
        if (curTime - profStart > timeToSpin)
            return true;

        if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
            continue;
        if (!(status & GBA_JSTAT_SEND))
            continue;

        if (GBAGetStatus(x40_siChan, &status) == GBA_NOT_READY)
            continue;
        if (status != (GBA_JSTAT_FLAGS_MASK | GBA_JSTAT_SEND))
            continue;
        break;
    }

    if (GBARead(x40_siChan, bytes, &status) != GBA_READY)
        return false;

    /* CRC Here */

    x44_fusionLinked = (bytes[2] & 0x2) != 0;
    if (x44_fusionLinked && (bytes[2] & 0x1) != 0)
        x45_fusionBeat = true;

    return true;
}

void CGBASupport::Update(float dt)
{
    switch (x34_phase)
    {
    case EPhase::LoadClientPad:
        IsReady();
        break;

    case EPhase::StartProbeTimeout:
        x38_timeout = 4.f;
        x34_phase = EPhase::PollProbe;

    case EPhase::PollProbe:
        /* SIProbe poll normally occurs here with 4 second timeout */
        x34_phase = EPhase::StartJoyBusBoot;

    case EPhase::StartJoyBusBoot:
        x34_phase = EPhase::PollJoyBusBoot;
        GBAJoyBootAsync(x40_siChan, x40_siChan * 2, 2, x2c_buffer.get(), x28_fileSize, &x3c_status);
        break;

    case EPhase::PollJoyBusBoot:
        if (GBAGetProcessStatus(x40_siChan, &x3c_status) == GBA_BUSY)
            break;
        if (GBAGetStatus(x40_siChan, &x3c_status) == GBA_NOT_READY)
        {
            x34_phase = EPhase::Failed;
            break;
        }
        x38_timeout = 4.f;
        x34_phase = EPhase::DataTransfer;
        break;

    case EPhase::DataTransfer:
        if (PollResponse())
        {
            x34_phase = EPhase::Complete;
            break;
        }
        x38_timeout = std::max(0.f, x38_timeout - dt);
        if (x38_timeout == 0.f)
            x34_phase = EPhase::Failed;
        break;

    default: break;
    }
}

bool CGBASupport::IsReady()
{
    if (x34_phase != EPhase::LoadClientPad)
        return true;

    if (x30_dvdReq->IsComplete())
    {
        x30_dvdReq.reset();
        x34_phase = EPhase::Standby;
        /* Conveniently already little-endian */
        reinterpret_cast<u32&>(x2c_buffer[0xc8]) = u32(CBasics::GetGCTicks());
        x2c_buffer[0xaf] = 'E';
        x2c_buffer[0xbd] = 0xc9;
        return true;
    }
    return false;
}

void CGBASupport::InitializeSupport()
{
    x34_phase = EPhase::Standby;
    x38_timeout = 0.f;
    x3c_status = false;
    x40_siChan = -1;
    x44_fusionLinked = false;
    x45_fusionBeat = false;
}

void CGBASupport::StartLink()
{
    x34_phase = EPhase::StartProbeTimeout;
    x40_siChan = -1;
}

}
}
