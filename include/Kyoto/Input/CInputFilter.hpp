#ifndef _CINPUTFILTER
#define _CINPUTFILTER

#include "types.h"

#include "rstl/reserved_vector.hpp"

// Names inferred from the Trilogy input implementation.
class CInputQuantizer {
public:
  explicit CInputQuantizer(float step) : mStep(step), mIncreasing(true), mBucket(0) {}
  float Quantize(float value);

private:
  float mStep;
  bool mIncreasing;
  int mBucket;
};
CHECK_SIZEOF(CInputQuantizer, 0xc)

class CScalarInputFilter {
public:
  virtual ~CScalarInputFilter() = 0;
  virtual void SetProfile(int profile) = 0;
  virtual float Filter(float value) = 0;

  CScalarInputFilter(int profile, uint quantizationMode, float step);

protected:
  float UpdateDeviation(float value);

  rstl::reserved_vector< float, 10 > mSamples;
  int mProfile;
  uint mQuantizationMode;
  CInputQuantizer mQuantizer;
};
CHECK_SIZEOF(CScalarInputFilter, 0x44)

inline CScalarInputFilter::~CScalarInputFilter() {}

class CAdaptiveInputFilter : public CScalarInputFilter {
public:
  ~CAdaptiveInputFilter() override {}
  void SetProfile(int profile) override;
  float Filter(float value) override;

  CAdaptiveInputFilter(int algorithm, int profile, uint quantizationMode, float step);

private:
  float FilterRecursive(float value);
  float FilterAdaptiveMean(float value);
  float FilterAdaptiveSlow(float value);
  float FilterAdaptiveFast(float value);

  int mAlgorithm;
  rstl::reserved_vector< float, 10 > mInputs;
  rstl::reserved_vector< float, 10 > mOutputs;
  rstl::reserved_vector< float, 2 > mFeedforward;
  rstl::reserved_vector< float, 2 > mFeedback;
};
CHECK_SIZEOF(CAdaptiveInputFilter, 0xb8)

#endif // _CINPUTFILTER
