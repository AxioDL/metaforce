#include <Kyoto/Alloc/CMemory.hpp>
#include <Kyoto/CTimeProvider.hpp>
#include <Kyoto/Graphics/CGraphics.hpp>

CTimeProvider* CTimeProvider::gCurrentTimeProvider = nullptr;

CTimeProvider::CTimeProvider(const float& time)
: mCurrentTime(time), mFirst(true), mLastProvider(gCurrentTimeProvider) {
  if (mLastProvider != nullptr) {
    mLastProvider->mFirst = false;
  }

  gCurrentTimeProvider = this;

  CGraphics::SetExternalTimeProvider(this);
}

CTimeProvider::~CTimeProvider() {
  gCurrentTimeProvider = mLastProvider;
  if (gCurrentTimeProvider != nullptr) {
    gCurrentTimeProvider->mFirst = true;
    CGraphics::SetExternalTimeProvider(gCurrentTimeProvider);
  } else {
    CGraphics::SetExternalTimeProvider(nullptr);
  }
}
