#include "Kyoto/Alloc/CMediumAllocPool.hpp"

CMediumAllocPool* CMediumAllocPool::gMediumAllocPtr = nullptr;

CMediumAllocPool::CMediumAllocPool() : mLastNodePrev(mList.begin()) { gMediumAllocPtr = this; }

void CMediumAllocPool::ClearPuddles() {
  mLastNodePrev = mList.end();
  for (AUTO(it, mList.begin()); it != mList.end(); ++it) {
    // The debug build checks each puddle here before releasing the pool.
  }
  mList.clear();
  gMediumAllocPtr = nullptr;
}

bool CMediumAllocPool::HasPuddles() const { return mList.size() != 0; }

void* CMediumAllocPool::Alloc(uint len) {
  SMediumAllocPuddle* puddle = &*mLastNodePrev;
  void* ret;
  uint blockCount = 1;
  if (len >= 32) {
    blockCount = (len + 31) / 32;
  }

  ret = puddle->FindFree(blockCount);

  if (ret == nullptr) {
    for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end();
         ++it) {
      if (it == mLastNodePrev) {
        continue;
      }

      ret = it->FindFree(blockCount);
      if (ret != nullptr) {
        mLastNodePrev = it;
        break;
      }
    }
  }

  return ret;
}

int CMediumAllocPool::Free(const void* ptr) {
  rstl::list< SMediumAllocPuddle >::node* node = mList.begin().get_node();
  for (; node != mList.end().get_node(); node = node->get_next()) {
    SMediumAllocPuddle* puddle = node->get_value();
    if (puddle->GetPtrOffset(ptr) < puddle->GetNumEntries() * 32) {
      puddle->Free(ptr);
      if (node->get_value()->GetNumAllocs() == 0 && node->get_value()->CanErase()) {
        if (mLastNodePrev == node) {
          mLastNodePrev = mList.begin().get_node();
        }

        mList.erase(node);
      }

      return 2;
    }
  }
  return 1;
}

uint CMediumAllocPool::GetNumAllocs() {
  uint ret = 0;
  for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end(); ++it) {
    ret += it->GetNumAllocs();
  }

  return ret;
}
uint CMediumAllocPool::GetTotalEntries() {
  uint ret = 0;
  for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end(); ++it) {
    ret += it->GetNumEntries();
  }

  return ret;
}

uint CMediumAllocPool::GetNumBlocksAvailable() {
  uint ret = 0;
  for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end(); ++it) {
    ret += it->GetNumBlocks();
  }

  return ret;
}

void CMediumAllocPool::AddPuddle(uint len, void* data, const bool unk) {
  mList.push_back(SMediumAllocPuddle(len, data, unk));
  mLastNodePrev = mList.end();
  --mLastNodePrev;
}

SMediumAllocPuddle::SMediumAllocPuddle(const uint numBlocks, void* data, const bool canErase)
: mMainData(static_cast< uchar* >(data))
, mBookKeeping(static_cast< uchar* >(data) + numBlocks * 32)
, mCachedBookKeepingAddr(nullptr)
, mUnused(-1)
, mNumBlocks(numBlocks)
, mNumAllocs(0)
, mNumEntries(numBlocks)
, mCanErase(canErase) {
  SMediumAllocPuddle::InitBookKeeping(mBookKeeping, numBlocks);
}

SMediumAllocPuddle::~SMediumAllocPuddle() {}

void* SMediumAllocPuddle::FindFree(uint blockCount) {
  void* bookKeepingptr;
  uchar* entryPtr;
  void* ret;

  entryPtr = (uchar*)FindFreeEntry(blockCount);
  if (entryPtr == NULL) {
    return NULL;
  }

  bookKeepingptr = mBookKeeping;
  ret = mMainData.get();
  entryPtr[0] = (uchar)blockCount;
  ret = (void*)((uchar*)ret + ((uchar*)entryPtr - (uchar*)bookKeepingptr) * 0x20);
  entryPtr[blockCount - 1] = blockCount;
  mNumBlocks -= blockCount;
  mNumAllocs++;
  return ret;
}

void* SMediumAllocPuddle::FindFreeEntry(uint numBlocks) {
  if (GetNumBlocks() >= numBlocks) {
    uchar* cachedBookPtr = mCachedBookKeepingAddr;
    uchar* bookPtr = mBookKeeping;

    if (cachedBookPtr == nullptr) {
      cachedBookPtr = bookPtr;
    }

    uchar* ptr2 = bookPtr + GetNumEntries();
    uchar* ptr1 = cachedBookPtr;
    do {

      if ((*ptr1 & 0x80) == 0 || ptr1 == ptr2) {
        if (ptr1 == ptr2) {
          ptr1 = bookPtr;
        } else {
          ptr1 += *ptr1;
        }
      } else {
        uint offset = GetBlockOffset(ptr1, ptr2);

        if (offset >= numBlocks) {
          if ((ushort)(offset - numBlocks)) {
            SMediumAllocPuddle::InitBookKeeping(ptr1 + numBlocks, offset - numBlocks);
          }

          mCachedBookKeepingAddr = ptr1;
          return ptr1;
        }

        ptr1 += offset;

        if (ptr1 == cachedBookPtr) {
          break;
        }

        if (ptr1 == ptr2) {
          ptr1 = bookPtr;
        }
      }
    } while (ptr1 != cachedBookPtr);
  }
  return nullptr;
}

void SMediumAllocPuddle::Free(const void* ptr) {
  uchar* bookKeepingStart;
  uchar* bookKeepingPtr;
  ushort mergedCount;
  bool isCached;
  size_t blockOffset =
      (reinterpret_cast< uintptr_t >(ptr) - reinterpret_cast< uintptr_t >(mMainData.get())) / 32;
  uint blockCount = mBookKeeping[blockOffset];
  mergedCount = blockCount;
  isCached = false;
  mNumBlocks += blockCount;
  --mNumAllocs;

  bookKeepingStart = mBookKeeping;
  uchar* cachedBookKeep = mCachedBookKeepingAddr;
  uchar* block = bookKeepingStart + blockOffset;
  bookKeepingPtr = block;
  uchar* bookKeepingEndPtr = bookKeepingStart + GetNumEntries();
  if (cachedBookKeep == block) {
    isCached = true;
  }

  if (block > bookKeepingStart && block[-1] & 0x80) {
    const ushort previousCount =
        !(block[-1] & 0x60)
            ? static_cast< ushort >(block[-2] + (block[-1] & 0x7f) * 256)
            : static_cast< ushort >(
                  (block[-1] & 0x60) == 0x60ul ? 3 : ((block[-1] & 0x60) == 0x40 ? 2 : 1));

    bookKeepingPtr -= previousCount;
    mergedCount = static_cast< ushort >(mergedCount + previousCount);
  }

  uchar* ptr1 = block + blockCount;
  if (ptr1 < bookKeepingEndPtr && (ptr1[0] & 0x80) > 0) {
    mergedCount = static_cast< ushort >(mergedCount + GetBlockOffset(ptr1, bookKeepingEndPtr));
  }

  InitBookKeeping(bookKeepingPtr, mergedCount);
  if (!isCached) {
    return;
  }
  if (bookKeepingPtr == bookKeepingStart) {
    mCachedBookKeepingAddr = nullptr;
  } else {
    mCachedBookKeepingAddr = bookKeepingPtr - bookKeepingPtr[-1];
  }
}

ushort SMediumAllocPuddle::GetBlockOffset(const void* ptr1, const void* ptr2) {
  unsigned char tmp = (uchar*)ptr2 - (uchar*)ptr1 > 1 ? ((uchar*)ptr1)[1] : 0;

  ushort x = tmp + (*(uchar*)(ptr1) & 0x7f) * 0x100;
  if ((x & 0x6000) == 0) {
    return x;
  }
  return (x & 0x6000u) == 0x6000u ? 3 : (((x & 0x6000u) != 0x4000) ? 0 : 1) + 1;
}

void SMediumAllocPuddle::InitBookKeeping(uchar* bookKeepingPtr, ushort blockCount) {
  if (blockCount < 4) {
    const uchar tmp = (blockCount == 3 ? 96 : blockCount == 2 ? 64 : 32) | 0x80;
    bookKeepingPtr[0] = tmp;
    if (blockCount > 1) {
      bookKeepingPtr[blockCount - 1] = tmp;
    }
  } else {
    bookKeepingPtr[0] = (blockCount >> 8) | 0x80;
    bookKeepingPtr[1] = blockCount;
    bookKeepingPtr[blockCount - 2] = blockCount;
    bookKeepingPtr[blockCount - 1] = (blockCount >> 8) | 0x80;
  }
}
