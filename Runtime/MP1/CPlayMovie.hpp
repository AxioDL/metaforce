#pragma once

#include "Runtime/CPlayMovieBase.hpp"
#include "Runtime/RetroTypes.hpp"

namespace urde::MP1 {

class CPlayMovie : public CPlayMovieBase {
public:
  enum class EWhichMovie {
    WinGameBad,
    WinGameGood,
    WinGameBest,
    LoseGame,
    TalonTest,
    AfterCredits,
    SpecialEnding,
    CreditBG
  };

private:
  EWhichMovie x18_which;
  bool x78_24_ : 1 = false;
  bool x78_25_ : 1 = false;
  bool x78_26_resultsScreen : 1 = false;
  bool x78_27_ : 1 = false;

  static bool IsResultsScreen(EWhichMovie which);

public:
  explicit CPlayMovie(EWhichMovie which);
};

} // namespace urde::MP1
