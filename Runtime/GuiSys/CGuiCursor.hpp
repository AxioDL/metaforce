#pragma once

#include <array>
#include <vector>
#include <zeus/CColor.hpp>

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CModel.hpp"

namespace urde {

enum class ECursorState {
  x0,
  x1,
  x2,
};

struct SGuiCursorTransferInner {
  int x0_;  // unk type
  int x4_;  // unk type
  int x8_;  // unk type
  int xc_;  // unk type
  int x10_; // unk type
  int x14_; // unk type
  int x18_; // unk type
};

class CGuiCursorTransfer {
private:
public:
  int x0_; // unk type
  int x4_; // unk type
  std::vector<SGuiCursorTransferInner> x8_;
  int x14_; // unk type
  int x18_; // unk type
  int x1c_; // unk type
  int x20_; // unk type
  int x24_; // unk type
  union {
    struct {
      bool x28_ : 1;
    };
    u32 _dummy = 0;
  };
  int x2c_; // unk type
  int x30_; // unk type
  int x34_; // unk type
  int x38_; // unk type
  int x3c_; // unk type

  CGuiCursorTransfer(const CGuiCursorTransfer& other);
  CGuiCursorTransfer(float f1, float f2, float f3, float f4);

  double GetSomeDouble();
};

class CGuiCursor {
private:
  static const char* GetCursorName(ECursorState state);
  static const zeus::CColor GetCursorColor(ECursorState state);

public:
  TLockedToken<CModel> x0_cursorReticle;
  TLockedToken<CModel> xc_cursorPan;
  zeus::CVector2f x18_ = zeus::skZero2f;
  zeus::CVector3f x20_ = zeus::skOne3f;
  CGuiCursorTransfer x2c_;
  CGuiCursorTransfer x6c_;
  int xac_; // unk type
  float xb0_ = 0.f;
  float xb4_ = 1.f;
  float xb8_ = 1.f;
  float xbc_ = 1.f;
  float xc0_ = 0.f;
  float xc4_ = 0.f;
  float xc8_ = 0.f;
  int xcc_; // unk type
  int xd0_; // unk type
  float xd4_ = 0.f;
  float xd8_ = 0.f;
  float xdc_ = 67.5f;
  zeus::CColor xe0_cursorColor;
  ECursorState xe4_;
  union {
    struct {
      bool xe8_ : 1;
    };
    u32 _dummy = 0;
  };

  CGuiCursor(const CGuiCursorTransfer& t1, const CGuiCursorTransfer& t2, ECursorState cursorState);

  void Draw();
  void Update(double dt);
  zeus::CVector3f ProjectCursorPosition();
};
} // namespace urde
