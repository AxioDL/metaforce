#include "Runtime/CMayaSpline.hpp"

#include "Runtime/Streams/CInputStream.hpp"

namespace metaforce {
void ValidateTangent(zeus::CVector2f& tangent) {
  if (tangent.x() < 0.f) {
    tangent.x() = 0.f;
  }

  const float mag = tangent.magnitude();
  if (mag != 0.f) {
    tangent /= mag;
  }

  if (tangent.x() == 0.f && tangent.y() != 0.f) {
    const float mul = tangent.y() >= 0.f ? 1.f : -1.f;
    tangent.x() = 0.0001f;
    tangent.y() = 5729578.0f * tangent.x() * mul;
  }
}

CMayaSplineKnot::CMayaSplineKnot(CInputStream& in) {
  x0_time = in.ReadFloat();
  x4_amplitude = in.ReadFloat();
  x8_ = in.ReadInt8();
  x9_ = in.ReadInt8();
  if (x8_ == 5) {
    float x = in.ReadFloat();
    float y = in.ReadFloat();
    xc_cachedTangentA = {x, y};
  }

  if (x9_ == 5) {
    float x = in.ReadFloat();
    float y = in.ReadFloat();
    x14_cachedTangentB = {x, y};
  }
}

void CMayaSplineKnot::GetTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next, zeus::CVector2f& tangentA,
                                  zeus::CVector2f& tangentB) {
  if (xa_24_dirty) {
    CalculateTangents(prev, next);
  }

  tangentA = xc_cachedTangentA;
  tangentB = x14_cachedTangentB;
}

void CMayaSplineKnot::CalculateTangents(CMayaSplineKnot* prev, CMayaSplineKnot* next) {
  xa_24_dirty = false;
  bool calculateTangents = false;
  if (x8_ == 4 && prev != nullptr) {
    float fVar2 = std::abs(prev->GetAmplitude() - GetAmplitude());
    float fVar3 = fVar2;
    if (next != nullptr) {
      fVar3 = std::abs(next->GetAmplitude() - GetAmplitude());
    }
    if (fVar3 <= 0.05f || fVar2 <= 0.05f) {
      x8_ = 1;
    }
  }

  if (x8_ == 0) {
    if (prev == nullptr) {
      xc_cachedTangentA = {1.f, 0.f};
    } else {
      xc_cachedTangentA = {GetTime() - prev->GetTime(), GetAmplitude() - prev->GetAmplitude()};
    }
  } else if (x8_ == 1) {
    float fVar1 = 0.f;
    if (prev != nullptr) {
      fVar1 = GetTime() - prev->GetTime();
    } else if (next != nullptr) {
      fVar1 = next->GetTime() - GetTime();
    }
    xc_cachedTangentA = {fVar1, 0.f};
  } else if (x8_ == 2) {
    calculateTangents = true;
  } else if (x8_ == 3) {
    xc_cachedTangentA = zeus::skOne2f;
  } else if (x8_ == 4) {
    x8_ = 2;
    calculateTangents = true;
  }

  if (x9_ == 0) {
    if (next == nullptr) {
      x14_cachedTangentB = {1.f, 0.f};
    } else {
      x14_cachedTangentB = {next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude()};
    }
  } else if (x9_ == 1) {
    float fVar1 = 0.f;
    if (next != nullptr) {
      fVar1 = next->GetTime() - GetTime();
    } else if (prev != nullptr) {
      fVar1 = GetTime() - prev->GetTime();
    }

    x14_cachedTangentB = {fVar1, 0.f};
  } else if (x9_ == 2) {
    calculateTangents = true;
  } else if (x9_ == 3) {
    x14_cachedTangentB = {1.f, 0.f};
  } else if (x9_ == 4 && next != nullptr) {
    float fVar1 = next->GetAmplitude() - GetAmplitude();
    float fVar2 = fVar1;
    if (prev != nullptr) {
      fVar2 = prev->GetAmplitude() - GetAmplitude();
    }

    if (fVar1 <= 0.05f || fVar2 <= 0.05f) {
      x9_ = 1;
    }
    calculateTangents = true;
  }

  if (calculateTangents) {
    zeus::CVector2f tangentA;
    zeus::CVector2f tangentB;

    if (prev == nullptr && next != nullptr) {
      tangentA = tangentB = {next->GetTime() - GetTime(), next->GetAmplitude() - GetAmplitude()};
    } else if (prev != nullptr && next == nullptr) {
      tangentA = tangentB = {GetTime() - prev->GetTime(), GetAmplitude() - prev->GetAmplitude()};
    } else if (prev != nullptr && next != nullptr) {
      float timeDiff = next->GetTime() - prev->GetTime();
      float ampDiff = next->GetAmplitude() - prev->GetAmplitude();
      float amp = timeDiff >= 0.0001f ? ampDiff / timeDiff : (ampDiff <= 0.f ? -5729578.0f : 5729578.0f);
      float nextTimeDiff = next->GetTime() - GetTime();
      float prevTimeDiff = GetTime() - prev->GetTime();
      float ampA = 0.f;
      float ampB = 0.;
      float timeA = 0.;
      float timeB = 0.;
      if (nextTimeDiff >= 0.f) {
        ampA = nextTimeDiff * amp;
      } else {
        timeA = 0.f;
        ampA = amp;
      }

      if (prevTimeDiff >= 0.f) {
        ampB = prevTimeDiff * amp;
      } else {
        timeB = 0.f;
      }
      tangentB = {timeB, ampB};
      tangentA = {timeA, ampA};

    } else {
      tangentA.zeroOut();
      tangentB.zeroOut();
    }

    if (x8_ == 2) {
      xc_cachedTangentA = tangentA;
    }
    if (x9_ == 2) {
      x14_cachedTangentB = tangentB;
    }
  }
  ValidateTangent(xc_cachedTangentA);
  ValidateTangent(x14_cachedTangentB);
}

CMayaSpline::CMayaSpline(CInputStream& in, s32 count) : x0_preInfinity(in.ReadInt8()), x4_postInfinity(in.ReadInt8()) {

  u32 knotCount = in.ReadLong();
  x8_knots.reserve(knotCount);
  for (size_t i = 0; i < knotCount; ++i) {
    x8_knots.emplace_back(in);
  }
  x18_clampMode = in.ReadInt8();
  x1c_minAmplitudeTime = in.ReadFloat();
  x20_maxAmplitudeTime = in.ReadFloat();
}

float CMayaSpline::GetMinTime() const { return x8_knots.empty() ? 0.f : x8_knots[0].GetTime(); }
float CMayaSpline::GetMaxTime() const { return x8_knots.empty() ? 0.f : x8_knots[GetKnotCount() - 1].GetTime(); }
float CMayaSpline::GetDuration() const { return x8_knots.empty() ? 0.f : GetMaxTime() - GetMinTime(); }

float CMayaSpline::EvaluateAt(float time) {
  float amplitude = EvaluateAtUnclamped(time);
  if (x18_clampMode == 1) {
    if (x1c_minAmplitudeTime > amplitude) {
      return x1c_minAmplitudeTime;
    }
    if (x20_maxAmplitudeTime < amplitude) {
      return x20_maxAmplitudeTime;
    }
    return amplitude;
  } else if (x18_clampMode == 2) {
    float center = x20_maxAmplitudeTime - x1c_minAmplitudeTime;

    if (center > 0.f) {
      if (amplitude <= FLT_EPSILON + x20_maxAmplitudeTime) {
        return amplitude - (center * static_cast<float>(s32((amplitude - x20_maxAmplitudeTime) / center) + 1));
      }
      if (amplitude < x1c_minAmplitudeTime - FLT_EPSILON) {
        return amplitude + (center * static_cast<float>(std::abs(s32((amplitude - x1c_minAmplitudeTime) / center))));
      }
    }
  }

  return amplitude;
}

float CMayaSpline::EvaluateAtUnclamped(float time) {
  if (x8_knots.empty()) {
    return 0.f;
  }

  u32 lastIdx = x8_knots.size() - 1;
  bool bVar2 = false;
  float retVal;
  if (time < x8_knots[0].GetTime()) {
    if (x0_preInfinity == 0) {
      return x8_knots[0].GetAmplitude();
    }
    return EvaluateInfinities(time, true);
  } else if (x8_knots[lastIdx].GetTime() >= time) {
    bVar2 = false;
    s32 local_68 = -1;
    s32 iVar1 = x24_chachedKnotIndex;
    if (iVar1 != -1) {
      if (lastIdx <= iVar1 || x8_knots[lastIdx].GetTime() >= time) {
        if (iVar1 > 0 && x8_knots[iVar1].GetTime() > time) {
          s32 iVar3 = iVar1 - 1;
          bVar2 = x8_knots[iVar3].GetTime() < time;
          if (bVar2) {
            local_68 = iVar1;
          }
          if (x8_knots[iVar3].GetTime() == time) {
            x24_chachedKnotIndex = iVar3;
            return x8_knots[x24_chachedKnotIndex].GetAmplitude();
          }
        }
      } else {
        retVal = x8_knots[iVar1 + 1].GetTime();
        if (retVal == time) {
          x24_chachedKnotIndex = lastIdx;
          return x8_knots[x24_chachedKnotIndex].GetAmplitude();
        }

        if (retVal > time) {
          bVar2 = true;
          local_68 = iVar1 + 1;
        }
      }
    }

    if (!bVar2 && (FindKnot(time, local_68))) {
      if (local_68 == 0) {
        x24_chachedKnotIndex = 0;
        return x8_knots[0].GetAmplitude();
      }
      if (local_68 == x8_knots.size()) {
        x24_chachedKnotIndex = 0;
        return x8_knots[lastIdx].GetAmplitude();
      }
    }

    lastIdx = local_68 - 1;
    if (x28_ != lastIdx) {
      x24_chachedKnotIndex = lastIdx;
      x28_ = lastIdx;
      if (x8_knots[x24_chachedKnotIndex].GetX9() == 3) {
        x2c_24_dirty = true;
      } else {
        x2c_24_dirty = false;
        rstl::reserved_vector<zeus::CVector2f, 4> points;
        FindControlPoints(x24_chachedKnotIndex, points);
        CalculateHermiteCoefficients(points, x34_cachedHermitCoefs);
        x30_cachedMinTime = points[0].x();
      }
    }

    if (x2c_24_dirty) {
      return x8_knots[x24_chachedKnotIndex].GetTime();
    } else {
      return EvaluateHermite(time);
    }
  }

  if (x4_postInfinity == 0) {
    return x8_knots[lastIdx].GetAmplitude();
  }

  return EvaluateInfinities(time, false);
}

float CMayaSpline::EvaluateInfinities(float time, bool pre) {
  if (x8_knots.empty()) {
    return 0.f;
  }

  s32 lastIdx = x8_knots.size() - 1;
  CMayaSplineKnot* curKnot = &x8_knots[0];
  const float startTime = x8_knots[0].GetTime();
  const float endTime = x8_knots[lastIdx].GetAmplitude();
  float center = endTime - startTime;
  if (zeus::close_enough(center, 0)) {
    return curKnot->GetAmplitude();
  }

  double tmp = 0.f;
  float divTime =
      (time <= endTime) ? std::modf((time - startTime) / center, &tmp) : std::modf((time - endTime) / center, &tmp);

  center = center * std::abs(divTime);
  tmp = 1.f + std::abs(tmp);

  if (!pre) {
    if (x4_postInfinity == 4) {
      divTime = std::fmod(tmp, 2.f);
      if (zeus::close_enough(divTime, 0.f)) {
        center = startTime + center;
      } else {
        center = endTime - center;
      }
    } else if (x4_postInfinity == 2 || x4_postInfinity == 3) {
      center = startTime + center;
    } else if (x4_postInfinity == 1) {
      center = time - endTime;
      zeus::CVector2f tangentA;
      zeus::CVector2f tangentB;
      x8_knots[0].GetTangents((lastIdx < 1) ? nullptr : &x8_knots[lastIdx - 2], nullptr, tangentA, tangentB);
      if (!zeus::close_enough(tangentB.x(), 0.f)) {
        return x8_knots[lastIdx].GetAmplitude() + (center * tangentB.y() / tangentB.x());
      }
      return x8_knots[lastIdx].GetAmplitude();
    }
  } else if (x0_preInfinity == 4) {
    divTime = std::fmod(tmp, 2.f);
    if (zeus::close_enough(divTime, 0.f)) {
      center = endTime - center;
    } else {
      center = startTime + center;
    }
  } else if (x0_preInfinity == 2 || x0_preInfinity == 3) {
    center = endTime - center;
  } else if (x0_preInfinity == 1) {
    center = (startTime - time);
    zeus::CVector2f tangentA;
    zeus::CVector2f tangentB;
    x8_knots[0].GetTangents(nullptr, &x8_knots[1], tangentA, tangentB);
    if (!zeus::close_enough(tangentA.x(), 0)) {
      return (x8_knots[0].GetAmplitude() - (center * tangentA.y() / tangentA.x()));
    }
    return x8_knots[0].GetAmplitude();
  }

  float eval = EvaluateAt(center);
  if (pre && x0_preInfinity == 3) {
    return eval - (tmp * x8_knots[lastIdx].GetAmplitude() - x8_knots[0].GetAmplitude());
  }

  if (!pre && x4_postInfinity == 3) {
    return eval + (tmp * x8_knots[lastIdx].GetAmplitude() - x8_knots[0].GetAmplitude());
  }
  return eval;
}

float CMayaSpline::EvaluateHermite(float time) {
  const float timeDiff = time - x30_cachedMinTime;
  return x34_cachedHermitCoefs[0] + (timeDiff * x34_cachedHermitCoefs[1]) + (timeDiff * x34_cachedHermitCoefs[2]) +
         (timeDiff * x34_cachedHermitCoefs[3]);
}

bool CMayaSpline::FindKnot(float time, s32& knotIndex) {
  if (x8_knots.empty()) {
    return false;
  }

  u32 lower = 0;
  u32 upper = x8_knots.size();
  while (lower < upper) {
    u32 index = (lower + upper) / 2;
    const auto& knot = x8_knots[index];
    if (knot.GetTime() > time) {
      upper = index - 1;
    } else if (time > knot.GetTime()) {
      lower = index + 1;
    } else {
      knotIndex = index;
      return true;
    }
  }

  knotIndex = lower;
  return false;
}

void CMayaSpline::FindControlPoints(s32 knotIndex, rstl::reserved_vector<zeus::CVector2f, 4>& controlPoints) {
  CMayaSplineKnot* knot = &x8_knots[knotIndex];
  controlPoints.emplace_back(knot->GetTime(), knot->GetAmplitude());

  zeus::CVector2f tangentA;
  zeus::CVector2f tangentB;
  CMayaSplineKnot* next = (knotIndex + 1 < x8_knots.size()) ? &x8_knots[knotIndex + 1] : nullptr;
  CMayaSplineKnot* prev = (knotIndex - 1 >= 0) ? &x8_knots[knotIndex - 1] : nullptr;
  knot->GetTangents(prev, next, tangentA, tangentB);

  knot = &x8_knots[knotIndex + 1];
  controlPoints.emplace_back(controlPoints[0] + (tangentB * zeus::CVector2f{1.f / 3.f}));
  next = (knotIndex + 2 < x8_knots.size()) ? &x8_knots[knotIndex + 2] : nullptr;
  prev = (knotIndex - 2 >= 0) ? &x8_knots[knotIndex - 2] : nullptr;
  knot->GetTangents(prev, next, tangentA, tangentB);
  zeus::CVector2f knotV = {knot->GetTime(), knot->GetAmplitude()};
  controlPoints.emplace_back(knotV - (tangentA * zeus::CVector2f{1.f / 3.f}));
  controlPoints.emplace_back(knotV);
}

void CMayaSpline::CalculateHermiteCoefficients(const rstl::reserved_vector<zeus::CVector2f, 4>& controlPoints,
                                               float* coefs) {
  const zeus::CVector2f point1 = controlPoints[3] - controlPoints[0];
  const zeus::CVector2f point2 = controlPoints[1] - controlPoints[0];
  const zeus::CVector2f point3 = controlPoints[3] - controlPoints[2];
  float dVar7 = point2.x() != 0.f ? point2.y() / point2.x() : 5729578.0f;
  float dVar6 = point3.x() != 0.f ? point3.y() / point3.x() : 5729578.0f;
  const float point1XSq = point1.x() * point1.x();
  coefs[0] =
      ((1.f / (point1XSq)) * (((dVar7 * point1.x()) + (dVar6 * point1.x()) - point1.y()) - point1.y())) / point1.x();
  coefs[1] = ((1.f / (point1XSq)) *
              ((((point1.y() + (point1.y() + point1.y())) - (dVar7 * point1.x())) - (dVar7 * point1.x())) -
               (dVar6 * point1.x())));
  coefs[2] = dVar7;
  coefs[3] = controlPoints[0].y();
}

} // namespace metaforce
