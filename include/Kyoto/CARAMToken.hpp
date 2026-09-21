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
  const EStatus GetStatus() const { return x0_status; }
  int GetSize() const { return xc_dataLen; }
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
  mutable EStatus x0_status;
  mutable void* x4_mramPtr;
  mutable int xc_dataLen;
#else
  EStatus x0_status;
  void* x4_mramPtr;
  const void* x8_aramPtr;
  int xc_dataLen;
  uint x10_dmaHandle;
  CARAMToken* x14_prev;
  CARAMToken* x18_next;
  bool x1c_24_ : 1;
#endif
};

#endif // _CARAMTOKEN
