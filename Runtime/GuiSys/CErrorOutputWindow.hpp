#pragma once

#include "Runtime/CIOWin.hpp"
#include "Runtime/RetroTypes.hpp"

namespace urde {

class CErrorOutputWindow : public CIOWin {
public:
  enum class State { Zero, One, Two };

private:
  State x14_state = State::Zero;
  union {
    struct {
      bool x18_24_;
      bool x18_25_;
      bool x18_26_;
      bool x18_27_;
      bool x18_28_;
    };
    u16 dummy = 0;
  };
  const wchar_t* x1c_msg;

public:
  CErrorOutputWindow(bool);
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override { return int(x14_state) < 2; }
  void Draw() const override;
};

} // namespace urde
