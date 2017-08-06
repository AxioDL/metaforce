#ifndef __DNAMP1_CTWEAKBALL_HPP__
#define __DNAMP1_CTWEAKBALL_HPP__

#include "../../DNACommon/Tweaks/ITweakBall.hpp"
#include "zeus/CVector3f.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakBall : public ITweakBall
{
    DECL_YAML
    Delete __d;
    Value<float> x4_maxTranslationAcceleration[8];
    Value<float> x24_translationFriction[8];
    Value<float> x44_translationMaxSpeed[8];
    Value<float> x64_;
    Value<float> x68_;
    Value<float> x6c_;
    Value<float> x70_;
    Value<float> x74_;
    Value<zeus::CVector3f> x78_;
    Value<float> x84_;
    Value<float> x88_;
    Value<float> x8c_;
    Value<float> x90_;
    Value<float> x94_;
    Value<float> x98_;
    Value<float> x9c_;
    Value<float> xa0_;
    Value<float> xa4_;
    Value<float> xa8_;
    Value<float> xac_;
    Value<float> xb0_;
    Value<float> xb4_;
    Value<float> xb8_;
    Value<float> xbc_;
    Value<float> xc0_;
    Value<float> xc4_[8];
    Value<float> xe4_;
    Value<float> xe8_;
    float xec_ = 10000.f;
    float xf0_ = 1000.f;
    float xf4_ = 40000.f;
    float xf8_ = 40000.f;
    float xfc_ = 40000.f;
    float x100_ = 40000.f;
    float x104_ = 40000.f;
    float x108_ = 40000.f;
    float x10c_ = 10000.f;
    float x110_ = 1000.f;
    float x114_ = 40000.f;
    float x118_ = 40000.f;
    float x11c_ = 40000.f;
    float x120_ = 40000.f;
    float x124_ = 40000.f;
    float x128_ = 40000.f;
    float x12c_ = 10000.f;
    float x130_ = 10000.f;
    float x134_ = 1000.f;
    float x138_ = 10000.f;
    float x13c_ = 2000.f;
    float x140_ = 2000.f;
    float x144_ = 2000.f;
    float x148_ = 2000.f;
    Value<float> x14c_;
    Value<float> x150_;
    Value<float> x158_;
    Value<float> x154_;
    Value<float> x15c_;
    Value<float> x160_;
    Value<float> x164_;
    Value<float> x168_;
    Value<float> x16c_;
    Value<float> x170_;
    Value<float> x174_;
    Value<float> x178_;
    Value<float> x17c_;
    Value<float> x180_;
    Value<float> x184_;
    Value<float> x188_;
    Value<zeus::CVector3f> x18c_;
    Value<float> x198_;
    Value<float> x19c_;
    Value<float> x1a0_;
    Value<float> x1a4_;
    Value<float> x1a8_;
    Value<float> x1ac_;
    Value<float> x1b0_;
    Value<float> x1b4_;
    Value<zeus::CVector3f> x1b8_;
    Value<float> x1c4_;
    Value<float> x1c8_;
    Value<float> x1cc_;
    Value<float> x1d0_ballCameraControlDistance;
    Value<float> x1d4_;
    Value<float> x1d8_;
    Value<float> x1dc_;
    Value<float> x1e0_;
    Value<float> x1ec_;
    Value<float> x1f0_;
    Value<float> x1f4_;
    Value<float> x1f8_;
    Value<float> x1fc_;
    Value<float> x1e4_leftStickDivisor;
    Value<float> x1e8_rightStickDivisor;
    Value<float> x200_;
    Value<float> x204_;
    float x208_;
    Value<float> x20c_;
    Value<float> x218_;
    Value<float> x21c_;
    Value<float> x220_;
    Value<float> x224_;
    Value<float> x210_;
    float x228_;
    Value<float> x22c_;
    Value<float> x230_;
    Value<float> x234_;

    CTweakBall()=default;
    CTweakBall(athena::io::IStreamReader& r)
    {
        this->read(r);
        x74_ = zeus::degToRad(x74_);
        x90_ = zeus::degToRad(x90_);
        x15c_ = zeus::degToRad(x15c_);
        x16c_ = zeus::degToRad(x16c_);
        x174_ = zeus::degToRad(x174_);
        x17c_ = zeus::degToRad(x17c_);
        x184_ = zeus::degToRad(x184_);
        x188_ = zeus::degToRad(x188_);
        x1a8_ = zeus::degToRad(x1a8_);
        x1b0_ = zeus::degToRad(x1b0_);
        x1b4_ = zeus::degToRad(x1b4_);
        x1ec_ = zeus::degToRad(x1ec_);
    }

    float GetMaxBallTranslationAcceleration(int s) const { return x4_maxTranslationAcceleration[s]; }
    float GetBallTranslationFriction(int s) const { return x24_translationFriction[s]; }
    float GetBallTranslationMaxSpeed(int s) const { return x44_translationMaxSpeed[s]; }
    float GetBallCameraControlDistance() const { return x1d0_ballCameraControlDistance; }
    float GetLeftStickDivisor() const { return x1e4_leftStickDivisor; }
    float GetRightStickDivisor() const { return x1e8_rightStickDivisor; }
};
}
}
#endif // __DNAMP1_CTWEAKBALL_HPP__
