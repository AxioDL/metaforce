#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/math.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

template < typename T, int N >
static inline void AddSample(rstl::reserved_vector< T, N >& samples, const T& value) {
  if (samples.size() < N) {
    samples.push_back(value);
  }
  for (int i = samples.size() - 1; i > 0; --i) {
    samples[i] = samples[i - 1];
  }
  samples[0] = value;
}

CPlayer::CPlayerStuckTracker::CPlayerStuckTracker() {}

void CPlayer::CPlayerStuckTracker::AddState(EPlayerState state, const CVector3f& position,
                                            const CVector3f& velocity, const CVector2f& input) {
  AddSample(mStates, static_cast< int >(state));
  AddSample(mPositions, position);
  AddSample(mVelocities, velocity);
  AddSample(mInputs, input);
}

template < typename T, int N >
rstl::optional_object< T > _getElementBoundsCheck(const rstl::reserved_vector< T, N >& v, int idx);

template <>
rstl::optional_object< CVector3f >
_getElementBoundsCheck(const rstl::reserved_vector< CVector3f, 20 >& v, int idx) {
  if (idx >= v.size()) {
    return rstl::optional_object_null();
  }

  return v[idx];
}
template <>
rstl::optional_object< CVector2f >
_getElementBoundsCheck(const rstl::reserved_vector< CVector2f, 20 >& v, int idx) {
  if (idx >= v.size()) {
    return rstl::optional_object_null();
  }

  return v[idx];
}

template <>
rstl::optional_object< int > _getElementBoundsCheck(const rstl::reserved_vector< int, 20 >& v,
                                                    int idx) {
  if (idx >= v.size()) {
    return rstl::optional_object_null();
  }

  return v[idx];
}

bool CPlayer::CPlayerStuckTracker::IsPlayerStuck() const {
  if (mStates.size() == 20) {
    float distance = 0.f;
    CAABox velocityBounds(*_getElementBoundsCheck(mVelocities, 0),
                          *_getElementBoundsCheck(mVelocities, 0));
    CAABox positionBounds(*_getElementBoundsCheck(mPositions, 0),
                          *_getElementBoundsCheck(mPositions, 0));
    CAABox inputBounds(CVector3f(*_getElementBoundsCheck(mInputs, 0), 0.f),
                       CVector3f(*_getElementBoundsCheck(mInputs, 0), 0.f));
    int stateChanges = 0;
    float minSpeed = _getElementBoundsCheck(mVelocities, 0)->Magnitude();
    float maxSpeed = minSpeed;
    for (int i = 1; i < 20; ++i) {
      const CVector3f delta =
          *_getElementBoundsCheck(mPositions, i - 1) - *_getElementBoundsCheck(mPositions, i);
      const float deltaSq = delta.MagSquared();
      if (deltaSq > FLT_EPSILON) {
        distance += CMath::FastSqrtF(deltaSq);
      }
      positionBounds.AccumulateBounds(*_getElementBoundsCheck(mPositions, i));
      const CVector3f velocity = *_getElementBoundsCheck(mVelocities, i);
      velocityBounds.AccumulateBounds(velocity);
      const float speed = velocity.Magnitude();
      minSpeed = rstl::min_val(speed, minSpeed);
      maxSpeed = rstl::max_val(speed, maxSpeed);
      inputBounds.AccumulateBounds(CVector3f(*_getElementBoundsCheck(mInputs, i), 0.f));
      stateChanges +=
          *_getElementBoundsCheck(mStates, i - 1) != *_getElementBoundsCheck(mStates, i);
    }
    const bool stopped = distance < 1.f / 30.f || distance < (1.f / 30.f) * minSpeed;
    if (stateChanges == 0 && *_getElementBoundsCheck(mStates, 0) == kPS_StartingJump) {
      const float inputRange = (inputBounds.GetMaxPoint() - inputBounds.GetMinPoint()).Magnitude();
      CAABox inputWithZero(inputBounds.GetMinPoint(), inputBounds.GetMaxPoint());
      inputWithZero.AccumulateBounds(CVector3f::Zero());
      const float inputExtent =
          (inputWithZero.GetMaxPoint() - inputWithZero.GetMinPoint()).Magnitude();
      const bool unusualInput = inputExtent < 0.01f || inputRange > 1.5f;
      if (stopped && unusualInput) {
        return true;
      }
    }
  }
  return false;
}

void CPlayer::CPlayerStuckTracker::ResetStats() {
  mStates.clear();
  mPositions.clear();
  mVelocities.clear();
  mInputs.clear();
}
