#pragma once

#include "Runtime/CIOWin.hpp"
#include "Runtime/Graphics/CMoviePlayer.hpp"

namespace metaforce {

class CPlayMovieBase : public CIOWin {
  CMoviePlayer x18_moviePlayer;

public:
  CPlayMovieBase(const char* iowName, const char* path) : CIOWin(iowName), x18_moviePlayer(path, 0.0, false, false) {}
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override { return EMessageReturn::Normal; }
  void Draw() override {}
};

} // namespace metaforce
