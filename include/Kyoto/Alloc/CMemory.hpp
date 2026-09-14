#ifndef _CMEMORY
#define _CMEMORY

#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/IAllocator.hpp"
#include "types.h"

class COsContext;
class CMemory {
  static IAllocator* mpAllocator;
  static bool mInitialized;

public:
  static void Startup(COsContext& ctx);
  static void Shutdown();
  static void SetAllocator(COsContext& ctx, IAllocator& allocator);
  static IAllocator* GetAllocator();
  static void* Alloc(size_t len, IAllocator::EHint hint = IAllocator::kHI_None,
                     IAllocator::EScope scope = IAllocator::kSC_Unk1,
                     IAllocator::EType type = IAllocator::kTP_Heap,
                     const CCallStack& callstack = CCallStack(-1, "\?\?(\?\?)"));
  static void Free(const void* ptr);
  static void SetOutOfMemoryCallback(IAllocator::FOutOfMemoryCb callback, const void* context);
  static void OffsetFakeStatics(int);
};

#if defined(__MWERKS__) || defined(CLANGD)
// placement new
inline void* operator new(size_t n, void* ptr) { return ptr; };
#else
#include <new>
#endif

#ifdef __MWERKS__
void* operator new(size_t sz, const char*, const char*);
void* operator new[](size_t sz, const char*, const char*);
inline void operator delete(void* ptr) { CMemory::Free(ptr); }
inline void operator delete[](void* ptr) { CMemory::Free(ptr); }
#define rs_new new ("\?\?(\?\?)", nullptr)
#else
// void operator delete(void* ptr);
// void operator delete[](void* ptr);
#define rs_new new
#endif

#endif // _CMEMORY
