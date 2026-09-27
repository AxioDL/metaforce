#ifndef _SPOSITIONHISTORY
#define _SPOSITIONHISTORY

#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"

struct SPositionHistory {
private:
  float mMagSquared;
  rstl::reserved_vector< CVector3f, 16 > mValues;

public:
  explicit SPositionHistory(float mag);
  CVector3f GetValue(CVector3f pos, CVector3f face);
  void AddValue(CVector3f pos);
  void Clear() { mValues.clear(); }
};

#endif // _SPOSITIONHISTORY
