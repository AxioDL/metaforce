#include "Kyoto/CDvdRequest.hpp"

#include "Kyoto/CARAMManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CRealDvdRequest::~CRealDvdRequest() {
  if (!IsComplete()) {
    PostCancelRequest();
    WaitUntilComplete();
  }
  DVDClose(&mFileInfo);
}

void CRealDvdRequest::WaitUntilComplete() {
  while (!CRealDvdRequest::IsComplete()) {}
}

bool CRealDvdRequest::IsComplete()  {
  s32 status = DVDGetCommandBlockStatus(&mFileInfo.cb);
  bool ret = false;
  if (status == DVD_STATE_END || status == DVD_STATE_CANCELED) {
    ret = true;
  }

  return ret;
}

void CRealDvdRequest::PostCancelRequest() {
  DVDCancelAsync(&mFileInfo.cb, nullptr);
}

int CRealDvdRequest::GetMediaType() const {
  return 1;
}

#if !defined(TARGET_PC)
void CARAMDvdRequest::WaitUntilComplete() {
  if (CARAMManager::GetInvalidDMAHandle() == mDmaReq) {
    return;
  }

  CARAMManager::WaitForDMACompletion(mDmaReq);
  mDmaReq = CARAMManager::GetInvalidDMAHandle();
}

bool CARAMDvdRequest::IsComplete() {
  if (mDmaReq != CARAMManager::GetInvalidDMAHandle()) {
    if (!CARAMManager::IsDMACompleted(mDmaReq)) {
      return false;
    }

    mDmaReq = CARAMManager::GetInvalidDMAHandle();
  }

  return true;
}

void CARAMDvdRequest::PostCancelRequest() {
  
}

int CARAMDvdRequest::GetMediaType() const {
  return 0;
}
#endif
