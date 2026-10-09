#include "Kyoto/Input/CInputFilter.hpp"

float CInputQuantizer::Quantize(float value) {
  float scale = 1000000.f;
  int bucket;
  int scaleInt = static_cast< int >(scale);
  int step = static_cast< int >(mStep * scale);
  int input = static_cast< int >(value * scale) + scaleInt * 2;
  int halfStep = step / 2;

  if (mIncreasing) {
    bucket = input / step;
    if (bucket > mBucket) {
      mBucket = bucket;
    } else {
      bucket = (input + halfStep) / step;
      if (bucket < mBucket) {
        mIncreasing = false;
        mBucket = bucket;
      } else {
        bucket = mBucket;
      }
    }
  } else {
    bucket = (input + halfStep) / step;
    if (bucket < mBucket) {
      mBucket = bucket;
    } else {
      bucket = input / step;
      if (bucket > mBucket) {
        mIncreasing = true;
        mBucket = bucket;
      } else {
        bucket = mBucket;
      }
    }
  }

  return static_cast< float >(bucket * step - scaleInt * 2) / scale;
}

CScalarInputFilter::CScalarInputFilter(int profile, uint quantizationMode, float step)
: mSamples(0.f)
, mProfile(profile)
, mQuantizationMode(quantizationMode)
, mQuantizer(step) {}

CAdaptiveInputFilter::CAdaptiveInputFilter(int algorithm, int profile, uint quantizationMode,
                                         float step)
: CScalarInputFilter(profile, quantizationMode, step)
, mAlgorithm(algorithm)
, mInputs(0.f)
, mOutputs(0.f)
, mFeedforward(0.f)
, mFeedback(0.f) {
  SetProfile(profile);
}

void CAdaptiveInputFilter::SetProfile(int profile) {
  mProfile = profile;
  switch (profile) {
  case 0:
    mFeedforward[0] = 0.f;
    mFeedforward[1] = 0.9f;
    mFeedback[0] = 0.f;
    mFeedback[1] = 0.1f;
    break;
  case 1:
    mFeedforward[0] = 0.f;
    mFeedforward[1] = 0.5f;
    mFeedback[0] = 0.1f;
    mFeedback[1] = 0.4f;
    break;
  case 2:
    mFeedforward[0] = 0.f;
    mFeedforward[1] = 0.3f;
    mFeedback[0] = 0.3f;
    mFeedback[1] = 0.4f;
    break;
  case 3:
    mFeedforward[0] = 0.f;
    mFeedforward[1] = 0.2f;
    mFeedback[0] = 0.4f;
    mFeedback[1] = 0.4f;
    break;
  case 4:
    mFeedforward[0] = 0.f;
    mFeedforward[1] = 0.1f;
    mFeedback[0] = 0.4f;
    mFeedback[1] = 0.5f;
    break;
  }
}

float CAdaptiveInputFilter::FilterRecursive(float value) {
  for (int i = 0; i < mInputs.size() - 1; ++i) {
    mInputs[i] = mInputs[i + 1U];
  }
  mInputs[mInputs.size() - 1] = value;

  const float result =
      mFeedback[1] * mOutputs[mOutputs.size() - 1] +
      (mFeedback[0] * mOutputs[mOutputs.size() - 2] +
       (mFeedforward[0] * mInputs[mInputs.size() - 2] +
        mFeedforward[1] * mInputs[mInputs.size() - 1]));

  for (int i = 0; i < mOutputs.size() - 1; ++i) {
    mOutputs[i] = mOutputs[i + 1U];
  }
  mOutputs[mOutputs.size() - 1] = result;
  return result;
}

float CScalarInputFilter::UpdateDeviation(float value) {
  float next;
  float sample;
  float average;
  float mean = 0.f;
  for (int i = 0; i < 10;) {
    sample = mSamples[i++];
    mean += sample;
  }
  average = mean / 10.f;

  for (int i = 0; i < 9;) {
    next = mSamples[i + 1];
    mSamples[i++] = next;
  }
  mSamples[9] = value;
  if (average > value) {
    return average - value;
  }
  return value - average;
}

float CAdaptiveInputFilter::FilterAdaptiveMean(float value) {
  float next;
  float previousWeight;
  float weight = UpdateDeviation(value) / 0.025f;
  if (weight < 1.f) {
    previousWeight = 1.f - weight;
  } else {
    weight = 1.f;
    previousWeight = 0.f;
  }

  for (int i = 0; i < 9;) {
    next = mInputs[i + 1];
    mInputs[i++] = next;
  }
  mInputs[9] = value;

  float mean = 0.f;
  for (int i = 0; i < 10;) {
    mean += mInputs[i++];
  }
  const float average = mean / 10.f;
  const float result = weight * average + previousWeight * mOutputs[9];

  for (int i = 0; i < 9; ++i) {
    mOutputs[i] = mOutputs[i + 1];
  }
  mOutputs[9] = result;
  return result;
}

float CAdaptiveInputFilter::FilterAdaptiveSlow(float value) {
  float next;
  float weight = UpdateDeviation(value) / 0.25f;
  float previousWeight;
  if (weight < 0.3f) {
    previousWeight = 1.f - weight;
  } else {
    weight = 0.3f;
    previousWeight = 0.7f;
  }

  for (int i = 0; i < 9;) {
    next = mInputs[i + 1];
    mInputs[i++] = next;
  }
  mInputs[9] = value;

  const float result = weight * mInputs[9] + previousWeight * mOutputs[9];

  for (int i = 0; i < 9; ++i) {
    mOutputs[i] = mOutputs[i + 1];
  }
  mOutputs[9] = result;
  return result;
}

float CAdaptiveInputFilter::FilterAdaptiveFast(float value) {
  float next;
  float weight = UpdateDeviation(value) / 0.25f;
  float previousWeight;
  if (weight < 0.8f) {
    previousWeight = 1.f - weight;
  } else {
    weight = 0.8f;
    previousWeight = 0.2f;
  }

  for (int i = 0; i < 9;) {
    next = mInputs[i + 1];
    mInputs[i++] = next;
  }
  mInputs[9] = value;

  const float result = weight * mInputs[9] + previousWeight * mOutputs[9];

  for (int i = 0; i < 9; ++i) {
    mOutputs[i] = mOutputs[i + 1];
  }
  mOutputs[9] = result;
  return result;
}

float CAdaptiveInputFilter::Filter(float value) {
  float result = 0.f;
  switch (mAlgorithm) {
  case 0:
    result = FilterRecursive(value);
    break;
  case 1:
    result = FilterAdaptiveMean(value);
    break;
  case 2:
    result = FilterAdaptiveSlow(value);
    break;
  case 3:
    result = FilterAdaptiveFast(value);
    break;
  }

  if (mQuantizationMode == 1) {
    result = mQuantizer.Quantize(result);
  }
  return result;
}
