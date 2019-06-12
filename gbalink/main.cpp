#include <cstdio>
#include "GCNTypes.hpp"
#include <memory>
#include <thread>
#include <queue>
#include <optional>
#include "jbus/Endpoint.hpp"
#include "jbus/Listener.hpp"

#undef min
#undef max

class CGBASupport {
public:
  enum class EPhase {
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
  std::unique_ptr<jbus::Endpoint> m_endpoint;
  u32 x28_fileSize;
  std::unique_ptr<u8[]> x2c_buffer;
  EPhase x34_phase = EPhase::LoadClientPad;
  float x38_timeout = 0.f;
  u8 x3c_status = 0;
  u32 x40_siChan = -1;
  bool x44_fusionLinked = false;
  bool x45_fusionBeat = false;
  static CGBASupport* SharedInstance;

  static u8 CalculateFusionJBusChecksum(const u8* data, size_t len);

public:
  CGBASupport(const char* clientPadPath, std::unique_ptr<jbus::Endpoint>&& ep);
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

CGBASupport* CGBASupport::SharedInstance;

CGBASupport::CGBASupport(const char* clientPadPath, std::unique_ptr<jbus::Endpoint>&& ep) : m_endpoint(std::move(ep)) {
  FILE* fp = fopen(clientPadPath, "rb");
  if (!fp) {
    fprintf(stderr, "No file at %s\n", clientPadPath);
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  x28_fileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  x2c_buffer.reset(new u8[x28_fileSize]);
  fread(x2c_buffer.get(), 1, x28_fileSize, fp);
  fclose(fp);
  SharedInstance = this;
}

CGBASupport::~CGBASupport() { SharedInstance = nullptr; }

u8 CGBASupport::CalculateFusionJBusChecksum(const u8* data, size_t len) {
  u32 sum = -1;
  for (size_t i = 0; i < len; ++i) {
    u8 ch = *data++;
    sum ^= ch;
    for (int j = 0; j < 8; ++j) {
      if ((sum & 1)) {
        sum >>= 1;
        sum ^= 0xb010;
      } else
        sum >>= 1;
    }
  }
  return sum;
}

bool CGBASupport::PollResponse() {
  u8 status;
  if (m_endpoint->GBAReset(&status) == jbus::GBA_NOT_READY)
    if (m_endpoint->GBAReset(&status) == jbus::GBA_NOT_READY)
      return false;

  if (m_endpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
    return false;
  if (status != (jbus::GBA_JSTAT_PSF1 | jbus::GBA_JSTAT_SEND))
    return false;

  u8 bytes[4];
  if (m_endpoint->GBARead(bytes, &status) == jbus::GBA_NOT_READY)
    return false;
  if (reinterpret_cast<u32&>(bytes) != SBIG('AMTE'))
    return false;

  if (m_endpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
    return false;
  if (status != jbus::GBA_JSTAT_PSF1)
    return false;

  if (m_endpoint->GBAWrite((unsigned char*)"AMTE", &status) == jbus::GBA_NOT_READY)
    return false;

  if (m_endpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
    return false;
  if ((status & jbus::GBA_JSTAT_FLAGS_MASK) != jbus::GBA_JSTAT_FLAGS_MASK)
    return false;

  u64 profStart = jbus::GetGCTicks();
  const u64 timeToSpin = jbus::GetGCTicksPerSec() / 8000;
  for (;;) {
    u64 curTime = jbus::GetGCTicks();
    if (curTime - profStart > timeToSpin)
      return true;

    if (m_endpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
      continue;
    if (!(status & jbus::GBA_JSTAT_SEND))
      continue;

    if (m_endpoint->GBAGetStatus(&status) == jbus::GBA_NOT_READY)
      continue;
    if (status != (jbus::GBA_JSTAT_FLAGS_MASK | jbus::GBA_JSTAT_SEND))
      continue;
    break;
  }

  if (m_endpoint->GBARead(bytes, &status) != jbus::GBA_READY)
    return false;

  if (bytes[3] != CalculateFusionJBusChecksum(bytes, 3))
    return false;

  x44_fusionLinked = (bytes[2] & 0x2) == 0;
  if (x44_fusionLinked && (bytes[2] & 0x1) != 0)
    x45_fusionBeat = true;

  return true;
}

static void JoyBootDone(jbus::ThreadLocalEndpoint& endpoint, jbus::EJoyReturn status) {}

void CGBASupport::Update(float dt) {
  switch (x34_phase) {
  case EPhase::LoadClientPad:
    IsReady();
    break;

  case EPhase::StartProbeTimeout:
    x38_timeout = 4.f;
    x34_phase = EPhase::PollProbe;
    [[fallthrough]];

  case EPhase::PollProbe:
    /* SIProbe poll normally occurs here with 4 second timeout */
    x40_siChan = m_endpoint->getChan();
    x34_phase = EPhase::StartJoyBusBoot;
    [[fallthrough]];

  case EPhase::StartJoyBusBoot:
    x34_phase = EPhase::PollJoyBusBoot;
    if (m_endpoint->GBAJoyBootAsync(x40_siChan * 2, 2, x2c_buffer.get(), x28_fileSize, &x3c_status,
                                    std::bind(JoyBootDone, std::placeholders::_1, std::placeholders::_2)) !=
        jbus::GBA_READY)
      x34_phase = EPhase::Failed;
    break;

  case EPhase::PollJoyBusBoot:
    u8 percent;
    if (m_endpoint->GBAGetProcessStatus(percent) == jbus::GBA_BUSY)
      break;
    if (m_endpoint->GBAGetStatus(&x3c_status) == jbus::GBA_NOT_READY) {
      x34_phase = EPhase::Failed;
      break;
    }
    x38_timeout = 4.f;
    x34_phase = EPhase::DataTransfer;
    break;

  case EPhase::DataTransfer:
    if (PollResponse()) {
      x34_phase = EPhase::Complete;
      break;
    }
    x38_timeout = std::max(0.f, x38_timeout - dt);
    if (x38_timeout == 0.f)
      x34_phase = EPhase::Failed;
    break;

  default:
    break;
  }
}

bool CGBASupport::IsReady() {
  if (x34_phase != EPhase::LoadClientPad)
    return true;

  x34_phase = EPhase::Standby;
  reinterpret_cast<u32&>(x2c_buffer[0xc8]) = u32(jbus::GetGCTicks());
  x2c_buffer[0xaf] = 'E';
  x2c_buffer[0xbd] = 0xc9;
  return true;
}

void CGBASupport::InitializeSupport() {
  x34_phase = EPhase::Standby;
  x38_timeout = 0.f;
  x3c_status = false;
  x40_siChan = -1;
  x44_fusionLinked = false;
  x45_fusionBeat = false;
}

void CGBASupport::StartLink() {
  x34_phase = EPhase::StartProbeTimeout;
  x40_siChan = -1;
}

int main(int argc, char** argv) {
  jbus::Initialize();
  printf("Listening for client\n");
  jbus::Listener listener;
  listener.start();
  std::unique_ptr<jbus::Endpoint> endpoint;
  while (true) {
    s64 frameStart = jbus::GetGCTicks();
    endpoint = listener.accept();
    if (endpoint)
      break;
    s64 frameEnd = jbus::GetGCTicks();
    s64 waitTicks = jbus::GetGCTicksPerSec() / 60;
    if (waitTicks > 0)
      jbus::WaitGCTicks(waitTicks);
  }

  CGBASupport gba("client_pad.bin", std::move(endpoint));
  gba.Update(0.f);
  gba.InitializeSupport();
  gba.StartLink();

  printf("Waiting 5 sec\n");
  jbus::WaitGCTicks(jbus::GetGCTicksPerSec() * 5);

  printf("Connecting\n");
  while (gba.GetPhase() < CGBASupport::EPhase::Complete) {
    gba.Update(1.f / 60.f);
    s64 waitTicks = jbus::GetGCTicksPerSec() / 60;
    if (waitTicks > 0)
      jbus::WaitGCTicks(waitTicks);
  }

  CGBASupport::EPhase finalPhase = gba.GetPhase();
  printf("%s Linked: %d Beat: %d\n", finalPhase == CGBASupport::EPhase::Complete ? "Complete" : "Failed",
         gba.IsFusionLinked(), gba.IsFusionBeat());

  return 0;
}
