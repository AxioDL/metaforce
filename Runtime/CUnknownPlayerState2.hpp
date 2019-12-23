#pragma once

#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

#include "RetroTypes.hpp"

namespace urde {

class CUnknownPlayerState2 {
private:
  zeus::CVector3f x7c_ = zeus::skZero3f;

public:
  zeus::CVector2f x0_ = zeus::skZero2f;
  zeus::CVector3f x8_ = zeus::skZero3f;
  float x18_ = 0.f;
  float x1c_ = 0.f;
  float x20_ = 0.f;
  float x24_ = 0.f;
  zeus::CVector3f x28_ = zeus::skRight.normalized();
  float x34_ = 0.f;
  bool x38_ = false;
  int x40_ = 0; // unk type
  int x44_ = 0; // unk type
  zeus::CVector3f x48_ = zeus::skZero3f;
  zeus::CVector3f x54_ = zeus::skZero3f;
  zeus::CVector3f x60_ = zeus::skZero3f;
  float x6c_ = 0.f;
  zeus::CVector2f x70_ = zeus::skZero2f;
  float x78_ = 0.f;
  TUniqueId x88_ = kInvalidUniqueId;
  int x8c_ = 0; // unk type
  float x90_ = 0.f;
  float x94_ = 1.f;
  u8 x98_;
  float x9c_ = 0.f;
  float xa0_ = 0.f;
  int xa4_ = 0; // unk type
  int xa8_; // unk type

  CUnknownPlayerState2(int i1, int i2);

  const zeus::CVector3f& Getx7c() const { return x7c_; };
};
} // namespace urde
