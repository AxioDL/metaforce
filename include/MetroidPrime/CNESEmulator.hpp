#ifndef _CNESEMULATOR
#define _CNESEMULATOR

#include "rstl/single_ptr.hpp"
#include "types.h"

class CColor;
class CFinalInput;
class CDvdRequest;
struct ksNesCommonWorkObj;
struct ksNesStateObj;
struct OSModuleHeader;

class CNESEmulator {
public:
  enum EPasswordEntryState { kPES_NotPasswordScreen, kPES_NotEntered, kPES_Entered };

  CNESEmulator();
  ~CNESEmulator();

  void Draw(const CColor& color, bool enableFiltering);
  void Update();
  void ProcessUserInput(const CFinalInput& input, int);

  const uchar* GetPassword() const;
  bool IsGameOver() const;
  void LoadPassword(const uchar* password);
  EPasswordEntryState GetPasswordEntryState() const;

private:
  static bool CheckForGameOver(const uchar* vram, uchar* passwordOut);
  static bool SetPasswordIntoEntryScreen(uchar* vram, uchar* wram, const uchar* password);
  static EPasswordEntryState CheckForPasswordEntryScreen(const uchar* vram);

  rstl::single_ptr< CDvdRequest > mDvdRequest;
  rstl::single_ptr< uchar > mResultBuffer;
  ksNesCommonWorkObj* mWork;
  ksNesStateObj* mState;
  OSModuleHeader* mModule;
  void* mBss;
  uchar* mChrRam;
  uchar* mBbRam;
  bool mGameOver;
  uchar mPassword[18];
  EPasswordEntryState mPasswordEntryState;
  bool mPasswordPending;
  uchar mPasswordToLoad[18];
  uchar mPad[0x54 - 0x4b];
};
CHECK_SIZEOF(CNESEmulator, 0x54)

#endif // _CNESEMULATOR
