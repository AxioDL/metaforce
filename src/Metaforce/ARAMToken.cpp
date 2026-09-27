#include "Kyoto/CARAMToken.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CARAMToken::CARAMToken() : mStatus(kS_Six), mMramPtr(nullptr), mDataLen(0) {}

CARAMToken::CARAMToken(void* ptr, uint len, int)
: mStatus(ptr ? kS_One : kS_Six), mMramPtr(ptr), mDataLen(ptr ? len : 0) {}

CARAMToken::CARAMToken(const CARAMToken& other)
: mStatus(other.mStatus), mMramPtr(other.mMramPtr), mDataLen(other.mDataLen) {
  other.mStatus = kS_Six;
  other.mMramPtr = nullptr;
  other.mDataLen = 0;
}

CARAMToken::~CARAMToken() { CMemory::Free(mMramPtr); }

CARAMToken& CARAMToken::operator=(const CARAMToken& other) {
  if (this != &other) {
    CMemory::Free(mMramPtr);
    mStatus = other.mStatus;
    mMramPtr = other.mMramPtr;
    mDataLen = other.mDataLen;
    other.mStatus = kS_Six;
    other.mMramPtr = nullptr;
    other.mDataLen = 0;
  }
  return *this;
}

void CARAMToken::PostConstruct(void* ptr, uint len, int) {
  if (ptr != mMramPtr) {
    CMemory::Free(mMramPtr);
  }
  mMramPtr = ptr;
  mDataLen = ptr ? len : 0;
  mStatus = ptr ? kS_One : kS_Six;
}

bool CARAMToken::LoadToMRAM() { return mStatus == kS_One; }
bool CARAMToken::LoadToARAM() { return mStatus == kS_One; }
bool CARAMToken::RefreshStatus() { return mStatus == kS_One; }
void CARAMToken::UpdateAllDMAs() {}
void CARAMToken::ForceSyncARAM() {}
void* CARAMToken::GetMRAMSafe() { return mMramPtr; }

void CARAMToken::MakeInvalid() {
  mStatus = kS_Six;
  mMramPtr = nullptr;
  mDataLen = 0;
}

void* CARAMToken::ForceSyncMRAM() {
  void* ptr = mMramPtr;
  MakeInvalid();
  return ptr;
}
