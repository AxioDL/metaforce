#ifndef _CARAMTOKEN
#define _CARAMTOKEN

#include "types.h"

class CARAMToken {
#if !defined(TARGET_PC)
  static CARAMToken* sLists[7];
#endif
public:
  enum EStatus {
    kS_Zero,
    kS_One,
    kS_Two,
    kS_Three,
    kS_Four,
    kS_Five,
    kS_Six,
  };

  CARAMToken();
  CARAMToken(void* ptr, uint len, int unk);
  CARAMToken(const CARAMToken& other);
  ~CARAMToken();
  void PostConstruct(void* ptr, uint len, int unk);
  CARAMToken& operator=(const CARAMToken& other);
  const EStatus GetStatus() const { return mStatus; }
  int GetSize() const { return mDataLen; }
  bool LoadToMRAM();
  bool LoadToARAM();
  bool RefreshStatus();
  static void UpdateAllDMAs();
#if !defined(TARGET_PC)
  void InitiallyMoveToList();
  void MoveToList(EStatus status);
  void RemoveFromList();
#endif
  void MakeInvalid();

  void* ForceSyncMRAM();
  void ForceSyncARAM();

  void* GetMRAMSafe();

private:
#if defined(TARGET_PC)
  mutable EStatus mStatus;
  mutable void* mMramPtr;
  mutable int mDataLen;
#else
  EStatus mStatus;
  void* mMramPtr;
  const void* mAramPtr;
  int mDataLen;
  uint mDmaHandle;
  CARAMToken* mPrev;
  CARAMToken* mNext;
  bool x1c_24_ : 1;
#endif
};

#endif // _CARAMTOKEN
