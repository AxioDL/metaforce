#include "CGBASupport.hpp"
#include "CDvdRequest.hpp"
#include "CBasics.hpp"
#include "jbus/Listener.hpp"
#include "jbus/Endpoint.hpp"

namespace urde::MP1
{

static jbus::Listener g_JbusListener;
static std::unique_ptr<jbus::Endpoint> g_JbusEndpoint;

void CGBASupport::Initialize()
{
    jbus::Initialize();
    g_JbusListener.start();
}

void CGBASupport::GlobalPoll()
{
    if (g_JbusEndpoint && !g_JbusEndpoint->connected())
        g_JbusEndpoint.reset();
    if (!g_JbusEndpoint)
    {
        g_JbusEndpoint = g_JbusListener.accept();
        if (g_JbusEndpoint)
            g_JbusEndpoint->setChan(3);
    }
}

CGBASupport::CGBASupport()
: CDvdFile("client_pad.bin")
{
    x28_fileSize = ROUND_UP_32(Length());
    x2c_buffer.reset(new u8[x28_fileSize]);
    x30_dvdReq = AsyncRead(x2c_buffer.get(), x28_fileSize);
}

CGBASupport::~CGBASupport()
{
    if (x30_dvdReq)
        x30_dvdReq->PostCancelRequest();
}

u8 CGBASupport::CalculateFusionJBusChecksum(const u8* data, size_t len)
{
    u32 sum = -1;
    for (int i = 0 ; i < len; ++i)
    {
        u8 ch = *data++;
        sum ^= ch;
        for (int j = 0; j < 8; ++j)
        {
            if ((sum & 1))
            {
                sum >>= 1;
                sum ^= 0xb010;
            }
            else
                sum >>= 1;
        }
    }
    return sum;
}

bool CGBASupport::PollResponse()
{
    if (!g_JbusEndpoint)
        return false;

    u8 status;
    if (g_JbusEndpoint->GBAReset(&status) == jbus::GBA_NOT_READY)
        if (g_JbusEndpoint->GBAReset(&status) == jbus::GBA_NOT_READY)
            return false;

    if (g_JbusEndpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
        return false;
    if (status != (jbus::GBA_JSTAT_PSF1 | jbus::GBA_JSTAT_SEND))
        return false;

    u8 bytes[4];
    if (g_JbusEndpoint->GBARead(bytes, &status) == jbus::GBA_NOT_READY)
        return false;
    if (reinterpret_cast<u32&>(bytes) != SBIG('AMTE'))
        return false;

    if (g_JbusEndpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
        return false;
    if (status != jbus::GBA_JSTAT_PSF1)
        return false;

    if (g_JbusEndpoint->GBAWrite((unsigned char*)"AMTE", &status) == jbus::GBA_NOT_READY)
        return false;

    if (g_JbusEndpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
        return false;
    if ((status & jbus::GBA_JSTAT_FLAGS_MASK) != jbus::GBA_JSTAT_FLAGS_MASK)
        return false;

    u64 profStart = jbus::GetGCTicks();
    const u64 timeToSpin = jbus::GetGCTicksPerSec() / 8000;
    for (;;)
    {
        u64 curTime = jbus::GetGCTicks();
        if (curTime - profStart > timeToSpin)
            return true;

        if (g_JbusEndpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
            continue;
        if (!(status & jbus::GBA_JSTAT_SEND))
            continue;

        if (g_JbusEndpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
            continue;
        if (status != (jbus::GBA_JSTAT_FLAGS_MASK | jbus::GBA_JSTAT_SEND))
            continue;
        break;
    }

    if (g_JbusEndpoint->GBARead(bytes, &status) != jbus::GBA_READY)
        return false;

    if (bytes[3] != CalculateFusionJBusChecksum(bytes, 3))
        return false;

    x44_fusionLinked = (bytes[2] & 0x2) == 0;
    if (x44_fusionLinked && (bytes[2] & 0x1) != 0)
        x45_fusionBeat = true;

    return true;
}

static void JoyBootDone(jbus::ThreadLocalEndpoint& endpoint, jbus::EJoyReturn status) {}

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
        if (!g_JbusEndpoint)
        {
            x34_phase = EPhase::Failed;
            break;
        }
        x40_siChan = g_JbusEndpoint->getChan();
        x34_phase = EPhase::StartJoyBusBoot;

    case EPhase::StartJoyBusBoot:
        x34_phase = EPhase::PollJoyBusBoot;
        if (!g_JbusEndpoint || g_JbusEndpoint->GBAJoyBootAsync(x40_siChan * 2, 2,
                               x2c_buffer.get(), x28_fileSize, &x3c_status,
                               std::bind(JoyBootDone,
                                         std::placeholders::_1,
                                         std::placeholders::_2)) != jbus::GBA_READY)
            x34_phase = EPhase::Failed;
        break;

    case EPhase::PollJoyBusBoot:
        u8 percent;
        if (g_JbusEndpoint && g_JbusEndpoint->GBAGetProcessStatus(percent) == jbus::GBA_BUSY)
            break;
        if (!g_JbusEndpoint || g_JbusEndpoint->GBAGetStatus(&x3c_status) == jbus::GBA_NOT_READY)
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
