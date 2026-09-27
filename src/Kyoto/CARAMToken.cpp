#include "Kyoto/CARAMToken.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/CARAMManager.hpp"

#include "rstl/construct.hpp"

#include "dolphin/os/OSCache.h"

CARAMToken* CARAMToken::sLists[7];
CARAMToken::CARAMToken() {
  mStatus = kS_Six;
  mMramPtr = nullptr;
  mAramPtr = CARAMManager::GetInvalidAlloc();
  mDataLen = 0;
  mDmaHandle = CARAMManager::GetInvalidDMAHandle();
  mPrev = nullptr;
  mNext = nullptr;
  x1c_24_ = false;
  InitiallyMoveToList();
}

CARAMToken::CARAMToken(void* ptr, uint len, int unk) {
  mStatus = kS_One;
  mMramPtr = ptr;
  mAramPtr = CARAMManager::GetInvalidAlloc();
  mDataLen = len;
  mDmaHandle = CARAMManager::GetInvalidDMAHandle();
  mPrev = nullptr;
  mNext = nullptr;
  x1c_24_ = !unk;

  InitiallyMoveToList();
  if (x1c_24_) {
    mAramPtr = CARAMManager::Alloc(mDataLen);
    mDmaHandle = CARAMManager::DMAToARAM(mMramPtr, (void*)mAramPtr, mDataLen,
                                            CARAMManager::kDMAPrio_One);
    CARAMManager::WaitForDMACompletion(mDmaHandle);
    mDmaHandle = CARAMManager::GetInvalidDMAHandle();
  }
}

CARAMToken::CARAMToken(const CARAMToken& other)
: mStatus(other.mStatus)
, mMramPtr(other.mMramPtr)
, mAramPtr(other.mAramPtr)
, mDataLen(other.mDataLen)
, mDmaHandle(other.mDmaHandle)
, mPrev(nullptr)
, mNext(nullptr)
, x1c_24_(other.x1c_24_) {
  const_cast< CARAMToken& >(other).MakeInvalid();
  InitiallyMoveToList();
}

CARAMToken::~CARAMToken() {
  if (mDmaHandle != CARAMManager::GetInvalidDMAHandle() &&
      !CARAMManager::CancelDMA(mDmaHandle)) {
    CARAMManager::WaitForDMACompletion(mDmaHandle);
  }

  RemoveFromList();
  CMemory::Free(mMramPtr);
  CARAMManager::Free(mAramPtr);
}

void CARAMToken::PostConstruct(void* ptr, uint len, int unk) {
  MoveToList(kS_One);
  mMramPtr = ptr;
  mDataLen = len;
  x1c_24_ = unk == 0;
}

CARAMToken& CARAMToken::operator=(const CARAMToken& other) {
  if (&other == this) {
    return *this;
  }

  rstl::destroy(this);
  rstl::construct(this, other);
  return *this;
}

bool CARAMToken::LoadToMRAM() {
  switch (mStatus) {
  case kS_Three: {
    break;
  }
  case kS_Four:
  case kS_One: {
    return true;
  }
  case kS_Two: {
    MoveToList(kS_Four);
    if (CARAMManager::CancelDMA(mDmaHandle)) {
      RefreshStatus();
    }
    return true;
  }
  case kS_Five: {
    MoveToList(kS_Three);
    break;
  }
  case kS_Zero: {
    mMramPtr = CMemory::Alloc(mDataLen, IAllocator::kHI_RoundUpLen);
    DCInvalidateRange(mMramPtr, mDataLen);
    mDmaHandle = CARAMManager::DMAToMRAM((void*)mAramPtr, mMramPtr, mDataLen,
                                            CARAMManager::kDMAPrio_One);
    MoveToList(kS_Three);
    break;
  }
  default:
    break;
  }

  return RefreshStatus();
}

bool CARAMToken::LoadToARAM() {
  switch (mStatus) {
  case kS_Zero:
  case kS_Five:
    return true;
  case kS_Three: {
    MoveToList(kS_Five);
    if (CARAMManager::CancelDMA(mDmaHandle)) {
      RefreshStatus();
    }
    return true;
  }
  case kS_Four: {
    MoveToList(kS_Two);
    break;
  }
  case kS_One: {
    if (!x1c_24_) {
      mAramPtr = CARAMManager::Alloc(mDataLen);
      if (CARAMManager::GetInvalidAlloc() == mAramPtr) {
        return false;
      }

      mDmaHandle = CARAMManager::DMAToARAM(mMramPtr, (void*)mAramPtr, mDataLen,
                                              CARAMManager::kDMAPrio_One);
    }
    MoveToList(kS_Two);
    break;
  }
  default:
    break;
  }
  return RefreshStatus();
}

bool CARAMToken::RefreshStatus() {
  if (mStatus == kS_One || mStatus == kS_Zero) {
    return true;
  }

  if (!CARAMManager::IsDMACompleted(mDmaHandle)) {
    return false;
  }

  mDmaHandle = CARAMManager::GetInvalidDMAHandle();

  switch (mStatus) {
  case kS_Three:
  case kS_Four: {
    if (!x1c_24_) {
      CARAMManager::Free(mAramPtr);
      mAramPtr = CARAMManager::GetInvalidAlloc();
    }
    MoveToList(kS_One);
    break;
  }
  case kS_Two:
  case kS_Five: {
    CMemory::Free(mMramPtr);
    mMramPtr = nullptr;
    MoveToList(kS_Zero);
    break;
  }
  default:
    break;
  }

  return true;
}

void CARAMToken::UpdateAllDMAs() {
  for (int i = kS_Two; i <= kS_Five; ++i) {
    CARAMToken* ptr = sLists[i];
    while (ptr != nullptr) {
      CARAMToken* tmp = ptr->mNext;
      ptr->RefreshStatus();
      ptr = tmp;
    }
  }
}

void CARAMToken::InitiallyMoveToList() {
  mPrev = nullptr;
  mNext = sLists[mStatus];
  sLists[mStatus] = this;
  if (mNext != nullptr) {
    mNext->mPrev = this;
  }
}

void CARAMToken::MoveToList(EStatus status) {
  if (mStatus == status) {
    return;
  }

  RemoveFromList();
  mStatus = status;
  InitiallyMoveToList();
}

void CARAMToken::RemoveFromList() {
  if (mPrev == nullptr) {
    sLists[mStatus] = mNext;
  } else {
    mPrev->mNext = mNext;
  }

  if (mNext != nullptr) {
    mNext->mPrev = mPrev;
  }
}

void CARAMToken::MakeInvalid() {
  MoveToList(kS_Six);
  mMramPtr = nullptr;
  mAramPtr = CARAMManager::GetInvalidAlloc();
  mDataLen = 0;
  mDmaHandle = CARAMManager::GetInvalidDMAHandle();
}

void* CARAMToken::ForceSyncMRAM() {
  void* ptr = GetMRAMSafe();
  MakeInvalid();
  return ptr;
}

void CARAMToken::ForceSyncARAM() {
  if (mStatus >= kS_Two && mStatus <= kS_Five) {
    CARAMManager::WaitForDMACompletion(mDmaHandle);
    RefreshStatus();
  }
}

void* CARAMToken::GetMRAMSafe() {
  if (mStatus == kS_One) {
    return mMramPtr;
  }
  LoadToMRAM();
  while (!RefreshStatus())
    ;
  return mMramPtr;
}
