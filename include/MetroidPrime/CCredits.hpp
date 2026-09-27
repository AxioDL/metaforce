#ifndef _CCREDITS
#define _CCREDITS

#include "MetroidPrime/CIOWin.hpp"

#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"

#include "rstl/single_ptr.hpp"

class CFinalInput;
class CStringTable;
class CRasterFont;
class CMoviePlayer;
class CStaticAudioPlayer;
class CGuiTextSupport;
class CVector3f;
class CTransform4f;

class CCredits : public CIOWin {
public:
  ~CCredits() override;

  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override;
  void Draw() const override;

  CCredits();

  EMessageReturn Update(float, CArchitectureQueue& queue);
  EMessageReturn ProcessUserInput(const CFinalInput& input);

#if VERSION >= VERSION_GM8P_00
  static void DrawText(CGuiTextSupport&, const CTransform4f& transform);
#else
  static void DrawText(CGuiTextSupport&, const CVector3f& translation);
#endif

private:
  int mState;
  TToken< CStringTable > mCreditsTable;
#if VERSION < VERSION_GM8P_00
  TToken< CRasterFont > mCreditsFont;
#endif
  rstl::single_ptr< CMoviePlayer > mMoviePlayer;
  rstl::single_ptr< CStaticAudioPlayer > mAudioPlayer;
  rstl::list< rstl::pair< rstl::ncrc_ptr< CGuiTextSupport >, CVector2i > > mText;
  float mScrollPosition;
  float mTotalScrollDistance;
  float mScrollSpeed;
  float mTextFadeRemaining;
  float mVideoFadeTime;
  bool mFinished : 1;
  bool mVideoFaded : 1;
  bool mTextFaded : 1;
  bool mFadingIn : 1;
  bool mFadingOut : 1;

  void DrawVideo() const;
  void DrawText() const;
};
#if VERSION >= VERSION_GM8P_00
CHECK_SIZEOF(CCredits, 0x58)
#else
CHECK_SIZEOF(CCredits, 0x60)
#endif

#endif // _CCREDITS
