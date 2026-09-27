#ifndef _CPLAYMOVIE
#define _CPLAYMOVIE

#include "Kyoto/TToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CMoviePlayer;
class CQuitGameScreen;
class CStaticAudioPlayer;
class CStringTable;
class CRasterFont;
class CGuiTextSupport;
class CFinalInput;

class CPlayMovie : public CIOWin {
public:
  enum EWhichMovie {
    kWM_WinGameBad,
    kWM_WinGameGood,
    kWM_WinGameBest,
    kWM_LoseGame,
    kWM_TalonText,
    kWM_AfterCredits,
    kWM_SpecialEnding,
    kWM_CreditBG
  };
  ~CPlayMovie() override;
  EMessageReturn OnMessage(const CArchitectureMessage& message, CArchitectureQueue& queue) override;
  bool GetIsContinueDraw() const override;
  void Draw() const override;

  CPlayMovie(EWhichMovie movie);

private:
  bool IsGameWon(EWhichMovie movie);
  void DrawVideo() const;
  void DrawText() const;
  void UpdateText(float dt);
  EMessageReturn ProcessUserInput(const CFinalInput& input);
  void SetMovieIndex(int index);

  int mState;
  EWhichMovie mWhich;
  rstl::reserved_vector< rstl::auto_ptr< CMoviePlayer >, 3 > mMovies;
  CMoviePlayer* mMoviePlayer;
  int mMovieIndex;
  rstl::single_ptr< CQuitGameScreen > mQuitScreen;
  rstl::single_ptr< CStaticAudioPlayer > mAudioPlayer;
  TToken< CStringTable > mCompletionScreenStrings;
  TToken< CRasterFont > mLargeFont;
  rstl::single_ptr< CGuiTextSupport > mTitleText;
  rstl::single_ptr< CGuiTextSupport > mResultsText;
  rstl::single_ptr< CGuiTextSupport > mUnlockText;
  rstl::single_ptr< CGuiTextSupport > mContinueText;
  float mTextDelay;
  float mResultsTime;
  float mPulseTime;
  float mPrintedCharacters;
#if VERSION >= VERSION_GM8P_00
  CColor mPulseStartColor;
  CColor mPulseEndColor;
#endif
  bool mFinished : 1;
  bool mExit : 1;
  bool mResultsScreen : 1;
  bool mQuitScreenFlag : 1;
};
#if VERSION >= VERSION_GM8P_00
CHECK_SIZEOF(CPlayMovie, 0x84)
#else
CHECK_SIZEOF(CPlayMovie, 0x7c)
#endif

#endif // _CPLAYMOVIE
