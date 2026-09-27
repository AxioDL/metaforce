#ifndef _CGSCOMBOFIRE
#define _CGSCOMBOFIRE

class CAnimData;
class CStateManager;
class CGSComboFire {
public:
  CGSComboFire();

  bool IsComboOver() const { return mOver; }
  int GetLoopState() const { return mLoopState; }
  void SetLoopState(const int l) { mLoopState = l; }
  void SetIdle(const bool i) { mIdle = i; }
  int GetGunId() const { return mGunId; }
  bool Update(CAnimData& data, float dt, CStateManager& mgr);
  int SetAnim(CAnimData& data, int gunId, int loopState, CStateManager& mgr, float delay);

private:
  float mDelay;
  int mLoopState; // In, loop, out
  int mCueAnimId;
  int mGunId;
  bool mOver : 1;
  bool mIdle : 1;
};

#endif // _CGSCOMBOFIRE
