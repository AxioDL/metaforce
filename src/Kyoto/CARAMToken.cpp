#include "Kyoto/CARAMToken.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/CARAMManager.hpp"

#include "rstl/construct.hpp"

#include "dolphin/os/OSCache.h"

CARAMToken* CARAMToken::sLists[7];
CARAMToken::CARAMToken() {
  x0_status = kS_Six;
  x4_mramPtr = nullptr;
  x8_aramPtr = CARAMManager::GetInvalidAlloc();
  xc_dataLen = 0;
  x10_dmaHandle = CARAMManager::GetInvalidDMAHandle();
  x14_prev = nullptr;
  x18_next = nullptr;
  x1c_24_ = false;
  InitiallyMoveToList();
}

CARAMToken::CARAMToken(void* ptr, uint len, int unk) {
  x0_status = kS_One;
  x4_mramPtr = ptr;
  x8_aramPtr = CARAMManager::GetInvalidAlloc();
  xc_dataLen = len;
  x10_dmaHandle = CARAMManager::GetInvalidDMAHandle();
  x14_prev = nullptr;
  x18_next = nullptr;
  x1c_24_ = !unk;

  InitiallyMoveToList();
  if (x1c_24_) {
    x8_aramPtr = CARAMManager::Alloc(xc_dataLen);
    x10_dmaHandle = CARAMManager::DMAToARAM(x4_mramPtr, (void*)x8_aramPtr, xc_dataLen,
                                            CARAMManager::kDMAPrio_One);
    CARAMManager::WaitForDMACompletion(x10_dmaHandle);
    x10_dmaHandle = CARAMManager::GetInvalidDMAHandle();
  }
}

CARAMToken::CARAMToken(const CARAMToken& other)
: x0_status(other.x0_status)
, x4_mramPtr(other.x4_mramPtr)
, x8_aramPtr(other.x8_aramPtr)
, xc_dataLen(other.xc_dataLen)
, x10_dmaHandle(other.x10_dmaHandle)
, x14_prev(nullptr)
, x18_next(nullptr)
, x1c_24_(other.x1c_24_) {
  const_cast< CARAMToken& >(other).MakeInvalid();
  InitiallyMoveToList();
}

CARAMToken::~CARAMToken() {
  if (x10_dmaHandle != CARAMManager::GetInvalidDMAHandle() &&
      !CARAMManager::CancelDMA(x10_dmaHandle)) {
    CARAMManager::WaitForDMACompletion(x10_dmaHandle);
  }

  RemoveFromList();
  CMemory::Free(x4_mramPtr);
  CARAMManager::Free(x8_aramPtr);
}

void CARAMToken::PostConstruct(void* ptr, uint len, int unk) {
  MoveToList(kS_One);
  x4_mramPtr = ptr;
  xc_dataLen = len;
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
  switch (x0_status) {
  case kS_Three: {
    break;
  }
  case kS_Four:
  case kS_One: {
    return true;
  }
  case kS_Two: {
    MoveToList(kS_Four);
    if (CARAMManager::CancelDMA(x10_dmaHandle)) {
      RefreshStatus();
    }
    return true;
  }
  case kS_Five: {
    MoveToList(kS_Three);
    break;
  }
  case kS_Zero: {
    x4_mramPtr = CMemory::Alloc(xc_dataLen, IAllocator::kHI_RoundUpLen);
    DCInvalidateRange(x4_mramPtr, xc_dataLen);
    x10_dmaHandle = CARAMManager::DMAToMRAM((void*)x8_aramPtr, x4_mramPtr, xc_dataLen,
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
  switch (x0_status) {
  case kS_Zero:
  case kS_Five:
    return true;
  case kS_Three: {
    MoveToList(kS_Five);
    if (CARAMManager::CancelDMA(x10_dmaHandle)) {
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
      x8_aramPtr = CARAMManager::Alloc(xc_dataLen);
      if (CARAMManager::GetInvalidAlloc() == x8_aramPtr) {
        return false;
      }

      x10_dmaHandle = CARAMManager::DMAToARAM(x4_mramPtr, (void*)x8_aramPtr, xc_dataLen,
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
  if (x0_status == kS_One || x0_status == kS_Zero) {
    return true;
  }

  if (!CARAMManager::IsDMACompleted(x10_dmaHandle)) {
    return false;
  }

  x10_dmaHandle = CARAMManager::GetInvalidDMAHandle();

  switch (x0_status) {
  case kS_Three:
  case kS_Four: {
    if (!x1c_24_) {
      CARAMManager::Free(x8_aramPtr);
      x8_aramPtr = CARAMManager::GetInvalidAlloc();
    }
    MoveToList(kS_One);
    break;
  }
  case kS_Two:
  case kS_Five: {
    CMemory::Free(x4_mramPtr);
    x4_mramPtr = nullptr;
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
      CARAMToken* tmp = ptr->x18_next;
      ptr->RefreshStatus();
      ptr = tmp;
    }
  }
}

void CARAMToken::InitiallyMoveToList() {
  x14_prev = nullptr;
  x18_next = sLists[x0_status];
  sLists[x0_status] = this;
  if (x18_next != nullptr) {
    x18_next->x14_prev = this;
  }
}

void CARAMToken::MoveToList(EStatus status) {
  if (x0_status == status) {
    return;
  }

  RemoveFromList();
  x0_status = status;
  InitiallyMoveToList();
}

void CARAMToken::RemoveFromList() {
  if (x14_prev == nullptr) {
    sLists[x0_status] = x18_next;
  } else {
    x14_prev->x18_next = x18_next;
  }

  if (x18_next != nullptr) {
    x18_next->x14_prev = x14_prev;
  }
}

void CARAMToken::MakeInvalid() {
  MoveToList(kS_Six);
  x4_mramPtr = nullptr;
  x8_aramPtr = CARAMManager::GetInvalidAlloc();
  xc_dataLen = 0;
  x10_dmaHandle = CARAMManager::GetInvalidDMAHandle();
}

void* CARAMToken::ForceSyncMRAM() {
  void* ptr = GetMRAMSafe();
  MakeInvalid();
  return ptr;
}

void CARAMToken::ForceSyncARAM() {
  if (x0_status >= kS_Two && x0_status <= kS_Five) {
    CARAMManager::WaitForDMACompletion(x10_dmaHandle);
    RefreshStatus();
  }
}

void* CARAMToken::GetMRAMSafe() {
  if (x0_status == kS_One) {
    return x4_mramPtr;
  }
  LoadToMRAM();
  while (!RefreshStatus())
    ;
  return x4_mramPtr;
}
