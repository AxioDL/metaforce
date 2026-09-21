#include "Kyoto/CARAMToken.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CARAMToken::CARAMToken() : x0_status(kS_Six), x4_mramPtr(nullptr), xc_dataLen(0) {}

CARAMToken::CARAMToken(void* ptr, uint len, int)
: x0_status(ptr ? kS_One : kS_Six), x4_mramPtr(ptr), xc_dataLen(ptr ? len : 0) {}

CARAMToken::CARAMToken(const CARAMToken& other)
: x0_status(other.x0_status), x4_mramPtr(other.x4_mramPtr), xc_dataLen(other.xc_dataLen) {
  other.x0_status = kS_Six;
  other.x4_mramPtr = nullptr;
  other.xc_dataLen = 0;
}

CARAMToken::~CARAMToken() { CMemory::Free(x4_mramPtr); }

CARAMToken& CARAMToken::operator=(const CARAMToken& other) {
  if (this != &other) {
    CMemory::Free(x4_mramPtr);
    x0_status = other.x0_status;
    x4_mramPtr = other.x4_mramPtr;
    xc_dataLen = other.xc_dataLen;
    other.x0_status = kS_Six;
    other.x4_mramPtr = nullptr;
    other.xc_dataLen = 0;
  }
  return *this;
}

void CARAMToken::PostConstruct(void* ptr, uint len, int) {
  if (ptr != x4_mramPtr) {
    CMemory::Free(x4_mramPtr);
  }
  x4_mramPtr = ptr;
  xc_dataLen = ptr ? len : 0;
  x0_status = ptr ? kS_One : kS_Six;
}

bool CARAMToken::LoadToMRAM() { return x0_status == kS_One; }
bool CARAMToken::LoadToARAM() { return x0_status == kS_One; }
bool CARAMToken::RefreshStatus() { return x0_status == kS_One; }
void CARAMToken::UpdateAllDMAs() {}
void CARAMToken::ForceSyncARAM() {}
void* CARAMToken::GetMRAMSafe() { return x4_mramPtr; }

void CARAMToken::MakeInvalid() {
  x0_status = kS_Six;
  x4_mramPtr = nullptr;
  xc_dataLen = 0;
}

void* CARAMToken::ForceSyncMRAM() {
  void* ptr = x4_mramPtr;
  MakeInvalid();
  return ptr;
}
