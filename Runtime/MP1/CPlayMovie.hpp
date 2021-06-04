#pragma once

#include "Runtime/CIOWin.hpp"
#include "Runtime/Graphics/CMoviePlayer.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/MP1/CQuitGameScreen.hpp"

namespace metaforce::MP1 {

class CPlayMovie : public CIOWin {
public:
  enum class EWhichMovie {
    WinGameBad,
    WinGameGood,
    WinGameBest,
    LoseGame,
    TalonText,
    AfterCredits,
    SpecialEnding,
    CreditBG
  };

private:
  s32 x14_ = 0;
  EWhichMovie x18_which;
  std::unique_ptr<CMoviePlayer> x38_moviePlayer;
  std::unique_ptr<CQuitGameScreen> x40_quitScreen;
  bool x78_24_ : 1 = false;
  bool x78_25_ : 1 = false;
  bool x78_26_resultsScreen : 1 = false;
  bool x78_27_ : 1 = false;

  static bool IsResultsScreen(EWhichMovie which);

  void DrawVideo();
  void DrawText();
public:
  explicit CPlayMovie(EWhichMovie which);
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Draw() override;
  bool GetIsContinueDraw() const override { return false; }
};

} // namespace metaforce::MP1
