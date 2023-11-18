#include "Runtime/MP1/CPlayMovie.hpp"

#include "Runtime/GuiSys/CGuiFrame.hpp"

namespace metaforce::MP1 {

const char* kMovies[] = {
    "Video/wingame.thp",       "Video/wingame_good.thp", "Video/wingame_best.thp",  "Video/losegame.thp",
    "Video/05_tallonText.thp", "Video/AfterCredits.thp", "Video/SpecialEnding.thp", "Video/creditBG.thp",
};

bool CPlayMovie::IsResultsScreen(EWhichMovie which) { return int(which) <= 2; }

CPlayMovie::CPlayMovie(EWhichMovie which) : CIOWin("CPlayMovie"), x18_which(which) {}

CIOWin::EMessageReturn CPlayMovie::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  return EMessageReturn::RemoveIOWinAndExit;
}

void CPlayMovie::Draw() {
  if (x14_ != 3) {
    return;
  }

  DrawVideo();
  if (x78_27_) {
    x40_quitScreen->Draw();
  } else if (x78_26_resultsScreen) {
    DrawText();
  }
}

void CPlayMovie::DrawVideo() {
  if (x38_moviePlayer) {
    x38_moviePlayer->DrawVideo();
  }
}

void CPlayMovie::DrawText() {
  // TODO
}

} // namespace metaforce::MP1
