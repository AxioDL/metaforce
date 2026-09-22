#include "Kyoto/CARAMManager.hpp"

#include <dolphin/arq.h>
#include <dolphin/os.h>

bool CARAMManager::mbInitialized = false;
u32 CARAMManager::mpARAMStart = 0;
uint CARAMManager::mChunkSize = 0;
uint CARAMManager::mNumChunks = 0;
uint* CARAMManager::mpBookKeepingMemory;
uint CARAMManager::mPreInitializeAlloc = 16 * 1024;
uint CARAMManager::mDMAUniqueID = 0;
uint CARAMManager::mChunksAllocated = 0;
const int CARAMManager::kInvalidAlloc = -1;
const int CARAMManager::kInvalidHandle = -1;

rstl::list< CARAMManager::SAramDMARequest* > CARAMManager::mActiveDMAs;

bool CARAMManager::Initialize(uint chunkSize) {
  uint numChunks = (ARGetSize() - mPreInitializeAlloc) / chunkSize;
  mChunkSize = chunkSize;
  mNumChunks = numChunks;
  mpARAMStart = ARAlloc(chunkSize * numChunks);
  mpBookKeepingMemory = (uint*)CMemory::Alloc(numChunks * 4, IAllocator::kHI_None,
                                              IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                                              CCallStack(-1, "\?\?(\?\?)"));
  CMemory::OffsetFakeStatics(mNumChunks * 4);

  for (uint i = 0; i < numChunks; ++i) {
    mpBookKeepingMemory[i] = 0;
  }

  mDMAUniqueID = 0;
  mbInitialized = true;
  return true;
}

void CARAMManager::Shutdown() {
  WaitForAllDMAsToComplete();
  CMemory::Free(mpBookKeepingMemory);
  u32 unk = 0;
  CMemory::OffsetFakeStatics(-mNumChunks * 4);
  ARFree(&unk);
  mbInitialized = false;
}

void* CARAMManager::Alloc(const uint len) {
  uint chunkCount = (mChunkSize - 1 + len) / mChunkSize;
  uint block = FindFreeBlocks(0, mNumChunks, chunkCount);

  if (block == -1) {
    return (void*)-1;
  }

  mChunksAllocated += chunkCount;
  uint blockOffset = mpARAMStart + block * mChunkSize;
  mpBookKeepingMemory[block] = chunkCount;

  while (--chunkCount != 0) {
    ++block;
    mpBookKeepingMemory[block] = -1;
  }

  return (void*)blockOffset;
}

uint CARAMManager::FindFreeBlocks(uint arg0, uint arg1, uint arg2) {
  while (arg0 < arg1) {
    if (mpBookKeepingMemory[arg0] == 0) {
      if (arg2 == 1) {
        return arg0;
      }

      ++arg0;
      int r8 = 1;
      while (arg0 < arg1) {
        uint tmp = mpBookKeepingMemory[arg0];
        if (tmp != 0) {
          arg0 += tmp;
          break;
        }

        r8++;
        if (r8 == arg2) {
          return arg0 - (arg2 - 1);
        }
        ++arg0;
      }
    } else {
      arg0 += mpBookKeepingMemory[arg0];
    }
  }
  return -1;
}

bool CARAMManager::Free(const void* ptr) {
  if (GetInvalidAlloc() == ptr) {
    return false;
  }

  uint blockStart = (reinterpret_cast< uintptr_t >(ptr) - mpARAMStart) / mChunkSize;
  uint blockCount = mpBookKeepingMemory[blockStart];
  mChunksAllocated -= blockCount;
  while (blockCount--) {
    mpBookKeepingMemory[blockStart++] = 0;
  }
  return true;
}

uint CARAMManager::DMAToARAM(void* src, void* dest, uint len, EDMAPriority priority) {
  DCStoreRange(src, len);
  SAramDMARequest* req = rs_new SAramDMARequest();
  req->mComplete = false;
  req->mUniqueID = mDMAUniqueID;
  mActiveDMAs.push_back(req);
  ARQPostRequest(&req->mRequest, req->mUniqueID, ARQ_TYPE_MRAM_TO_ARAM,
                 (priority == kDMAPrio_One) ? ARQ_PRIORITY_HIGH : ARQ_PRIORITY_LOW,
                 reinterpret_cast< uintptr_t >(src), reinterpret_cast< uintptr_t >(dest), len,
                 AramManagerDMACallback);

  GetAndIncrementUniqueID();
  return req->mUniqueID;
}

int CARAMManager::DMAToMRAM(void* src, void* dest, uint len, EDMAPriority priority) {
  DCInvalidateRange(dest, len);
  SAramDMARequest* req = rs_new SAramDMARequest();
  req->mComplete = false;
  req->mUniqueID = mDMAUniqueID;
  mActiveDMAs.push_back(req);
  ARQPostRequest(&req->mRequest, req->mUniqueID, ARQ_TYPE_ARAM_TO_MRAM,
                 (priority == kDMAPrio_One) ? ARQ_PRIORITY_HIGH : ARQ_PRIORITY_LOW,
                 reinterpret_cast< uintptr_t >(src), reinterpret_cast< uintptr_t >(dest), len,
                 AramManagerDMACallback);

  GetAndIncrementUniqueID();
  return req->mUniqueID;
}

bool CARAMManager::IsDMACompleted(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      if ((*it)->mComplete) {
        delete (*it);
        mActiveDMAs.erase(it);
        return true;
      }
      return false;
    }
  }

  return true;
}

void CARAMManager::WaitForDMACompletion(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      // Spin until complete!
      while (!(*it)->mComplete)
        ;
      delete (*it);
      mActiveDMAs.erase(it);
      return;
    }
  }
}

void CARAMManager::WaitForAllDMAsToComplete() {
  while ((int)mActiveDMAs.size() > 0) {
    RefreshActiveDMAList();
  }
}

bool CARAMManager::CancelDMA(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      return (*it)->mComplete != false;
    }
  }
  return true;
}

void CARAMManager::AramManagerDMACallback(uintptr_t result) {
  SAramDMARequest* req = reinterpret_cast< SAramDMARequest* >(result);
  req->mComplete = true;
  if (req->mRequest.type == ARQ_TYPE_ARAM_TO_MRAM) {
    DCInvalidateRange(reinterpret_cast< void* >(req->mRequest.dest), req->mRequest.length);
  }
}

void CARAMManager::RefreshActiveDMAList() {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  while (it != mActiveDMAs.end()) {
    if ((*it)->mComplete) {
      delete (*it);
      it = mActiveDMAs.erase(it);
      continue;
    }
    ++it;
  }
}

void CARAMManager::CollectGarbage() {
  RefreshActiveDMAList();
}
