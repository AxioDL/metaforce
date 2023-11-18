#pragma once

#include "Runtime/CIOWin.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Audio/CStaticAudioPlayer.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"

#include "zeus/CVector2i.hpp"

namespace metaforce {
class CGuiTextSupport;
class CStringTable;
class CRasterFont;
class CMoviePlayer;
struct CFinalInput;

namespace MP1 {
class CCredits : public CIOWin {
  u32 x14_ = 0;
  TLockedToken<CStringTable> x18_creditsTable;
  TLockedToken<CRasterFont> x20_creditsFont;
  std::unique_ptr<CMoviePlayer> x28_;
  std::unique_ptr<CStaticAudioPlayer> x2c_;
  std::vector<std::pair<std::unique_ptr<CGuiTextSupport>, zeus::CVector2i>> x30_text;
  int x44_textSupport = 0;
  float x48_ = 0.f;
  float x4c_ = 0.f;
  float x50_ = 8.f;
  float x54_;
  float x58_ = 0.f;
  bool x5c_24_ : 1 = false;
  bool x5c_25_ : 1 = false;
  bool x5c_26_ : 1 = false;
  bool x5c_27_ : 1 = true;
  bool x5c_28_ : 1 = false;
  void DrawVideo();
  void DrawText();
  static void DrawText(CGuiTextSupport&, const zeus::CVector3f& translation);

public:
  CCredits();
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override { return false; }
  void Draw() override;

  EMessageReturn Update(float, CArchitectureQueue& queue);
  EMessageReturn ProcessUserInput(const CFinalInput& input);
};
} // namespace MP1
} // namespace metaforce
