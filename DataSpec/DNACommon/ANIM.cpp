#include "zeus/Math.hpp"
#include "ANIM.hpp"

#define DUMP_KEYS 0

namespace DataSpec::DNAANIM {

size_t ComputeBitstreamSize(size_t keyFrameCount, const std::vector<Channel>& channels) {
  size_t bitsPerKeyFrame = 0;
  for (const Channel& chan : channels) {
    switch (chan.type) {
    case Channel::Type::Rotation:
      bitsPerKeyFrame += 1;
      [[fallthrough]];
    case Channel::Type::Translation:
    case Channel::Type::Scale:
      bitsPerKeyFrame += chan.q[0];
      bitsPerKeyFrame += chan.q[1];
      bitsPerKeyFrame += chan.q[2];
      break;
    case Channel::Type::KfHead:
      bitsPerKeyFrame += 1;
      break;
    case Channel::Type::RotationMP3:
      bitsPerKeyFrame += chan.q[0];
      bitsPerKeyFrame += chan.q[1];
      bitsPerKeyFrame += chan.q[2];
      bitsPerKeyFrame += chan.q[3];
      break;
    default:
      break;
    }
  }
  return (bitsPerKeyFrame * keyFrameCount + 31) / 32 * 4;
}

static QuantizedRot QuantizeRotation(const Value& quat, atUint32 div) {
  float q = float(div) / (M_PIF / 2.0f);
  zeus::simd_floats f(quat.simd);
  assert(std::abs(f[1]) <= 1.f && "Out of range quat X component");
  assert(std::abs(f[2]) <= 1.f && "Out of range quat Y component");
  assert(std::abs(f[3]) <= 1.f && "Out of range quat Z component");
  return {{
              atInt32(std::asin(f[1]) * q),
              atInt32(std::asin(f[2]) * q),
              atInt32(std::asin(f[3]) * q),
          },
          (f[0] < 0.f)};
}

static Value DequantizeRotation(const QuantizedRot& v, atUint32 div) {
  float q = (M_PIF / 2.0f) / float(div);
  athena::simd_floats f = {
      0.0f,
      std::sin(v.v[0] * q),
      std::sin(v.v[1] * q),
      std::sin(v.v[2] * q),
  };
  f[0] = std::sqrt(std::max((1.0f - (f[1] * f[1] + f[2] * f[2] + f[3] * f[3])), 0.0f));
  f[0] = v.w ? -f[0] : f[0];
  Value retval;
  retval.simd.copy_from(f);
  return retval;
}

static Value DequantizeRotation_3(const QuantizedRot& v, atUint32 div) {
  float q = 1.0f / float(div);
  athena::simd_floats f = {
      0.0f,
      v.v[0] * q,
      v.v[1] * q,
      v.v[2] * q,
  };
  f[0] = std::sqrt(std::max((1.0f - (f[1] * f[1] + f[2] * f[2] + f[3] * f[3])), 0.0f));
  f[0] = v.w ? -f[0] : f[0];
  Value retval;
  retval.simd.copy_from(f);
  return retval;
}

bool BitstreamReader::dequantizeBit(const atUint8* data) {
  atUint32 byteCur = (m_bitCur / 32) * 4;
  atUint32 bitRem = m_bitCur % 32;

  /* Fill 32 bit buffer with region containing bits */
  /* Make them least significant */
  atUint32 tempBuf = hecl::SBig(*reinterpret_cast<const atUint32*>(data + byteCur)) >> bitRem;

  /* That's it */
  m_bitCur += 1;
  return tempBuf & 0x1;
}

atInt32 BitstreamReader::dequantize(const atUint8* data, atUint8 q) {
  atUint32 byteCur = (m_bitCur / 32) * 4;
  atUint32 bitRem = m_bitCur % 32;

  /* Fill 32 bit buffer with region containing bits */
  /* Make them least significant */
  atUint32 tempBuf = hecl::SBig(*reinterpret_cast<const atUint32*>(data + byteCur)) >> bitRem;

  /* If this shift underflows the value, buffer the next 32 bits */
  /* And tack onto shifted buffer */
  if ((bitRem + q) > 32) {
    atUint32 tempBuf2 = hecl::SBig(*reinterpret_cast<const atUint32*>(data + byteCur + 4));
    tempBuf |= (tempBuf2 << (32 - bitRem));
  }

  /* Mask it */
  atUint32 mask = (1 << q) - 1;
  tempBuf &= mask;

  /* Sign extend */
  atUint32 sign = (tempBuf >> (q - 1)) & 0x1;
  if (sign)
    tempBuf |= ~0u << q;

  /* Return delta value */
  m_bitCur += q;
  return atInt32(tempBuf);
}

std::vector<std::vector<Value>> BitstreamReader::read(const atUint8* data, size_t keyFrameCount,
                                                      const std::vector<Channel>& channels, atUint32 rotDiv,
                                                      float transMult, float scaleMult) {
  m_bitCur = 0;
  std::vector<std::vector<Value>> chanKeys;
  std::vector<QuantizedValue> chanAccum;
  chanKeys.reserve(channels.size());
  chanAccum.reserve(channels.size());
  for (const Channel& chan : channels) {
    chanAccum.push_back(chan.i);

    chanKeys.emplace_back();
    std::vector<Value>& keys = chanKeys.back();
    keys.reserve(keyFrameCount);
    switch (chan.type) {
    case Channel::Type::Rotation: {
      QuantizedRot qr = {{chan.i[0], chan.i[1], chan.i[2]}, false};
      keys.emplace_back(DequantizeRotation(qr, rotDiv));
      break;
    }
    case Channel::Type::Translation: {
      keys.push_back({chan.i[0] * transMult, chan.i[1] * transMult, chan.i[2] * transMult});
      break;
    }
    case Channel::Type::Scale: {
      keys.push_back({chan.i[0] * scaleMult, chan.i[1] * scaleMult, chan.i[2] * scaleMult});
      break;
    }
    case Channel::Type::KfHead: {
      break;
    }
    case Channel::Type::RotationMP3: {
      QuantizedRot qr = {{chan.i[1], chan.i[2], chan.i[3]}, bool(chan.i[0] & 0x1)};
      keys.emplace_back(DequantizeRotation_3(qr, rotDiv));
      break;
    }
    default:
      break;
    }
  }

  for (size_t f = 0; f < keyFrameCount; ++f) {
#if DUMP_KEYS
    fprintf(stderr, "\nFRAME %" PRISize " %u %u\n", f, (m_bitCur / 32) * 4, m_bitCur % 32);
    int lastId = -1;
#endif
    auto kit = chanKeys.begin();
    auto ait = chanAccum.begin();
    for (const Channel& chan : channels) {
#if DUMP_KEYS
      if (chan.id != lastId) {
        lastId = chan.id;
        fprintf(stderr, "\n");
      }
#endif
      QuantizedValue& p = *ait;
      switch (chan.type) {
      case Channel::Type::Rotation: {
        bool wBit = dequantizeBit(data);
        p[0] += dequantize(data, chan.q[0]);
        p[1] += dequantize(data, chan.q[1]);
        p[2] += dequantize(data, chan.q[2]);
        QuantizedRot qr = {{p[0], p[1], p[2]}, wBit};
        kit->emplace_back(DequantizeRotation(qr, rotDiv));
#if DUMP_KEYS
        fprintf(stderr, "%d R: %d %d %d %d\t", chan.id, wBit, p[0], p[1], p[2]);
#endif
        break;
      }
      case Channel::Type::Translation: {
        atInt32 val1 = dequantize(data, chan.q[0]);
        p[0] += val1;
        atInt32 val2 = dequantize(data, chan.q[1]);
        p[1] += val2;
        atInt32 val3 = dequantize(data, chan.q[2]);
        p[2] += val3;
        kit->push_back({p[0] * transMult, p[1] * transMult, p[2] * transMult});
#if DUMP_KEYS
        fprintf(stderr, "%d T: %d %d %d\t", chan.id, p[0], p[1], p[2]);
#endif
        break;
      }
      case Channel::Type::Scale: {
        p[0] += dequantize(data, chan.q[0]);
        p[1] += dequantize(data, chan.q[1]);
        p[2] += dequantize(data, chan.q[2]);
        kit->push_back({p[0] * scaleMult, p[1] * scaleMult, p[2] * scaleMult});
#if DUMP_KEYS
        fprintf(stderr, "%d S: %d %d %d\t", chan.id, p[0], p[1], p[2]);
#endif
        break;
      }
      case Channel::Type::KfHead: {
        dequantizeBit(data);
        break;
      }
      case Channel::Type::RotationMP3: {
        atInt32 val1 = dequantize(data, chan.q[0]);
        p[0] += val1;
        atInt32 val2 = dequantize(data, chan.q[1]);
        p[1] += val2;
        atInt32 val3 = dequantize(data, chan.q[2]);
        p[2] += val3;
        atInt32 val4 = dequantize(data, chan.q[3]);
        p[3] += val4;
        QuantizedRot qr = {{p[1], p[2], p[3]}, bool(p[0] & 0x1)};
        kit->emplace_back(DequantizeRotation_3(qr, rotDiv));
        break;
      }
      default:
        break;
      }
      ++kit;
      ++ait;
    }
#if DUMP_KEYS
    fprintf(stderr, "\n");
#endif
  }

  return chanKeys;
}

void BitstreamWriter::quantizeBit(atUint8* data, bool val) {
  atUint32 byteCur = (m_bitCur / 32) * 4;
  atUint32 bitRem = m_bitCur % 32;

  /* Fill 32 bit buffer with region containing bits */
  /* Make them least significant */
  *(atUint32*)(data + byteCur) = hecl::SBig(hecl::SBig(*(atUint32*)(data + byteCur)) | (val << bitRem));

  m_bitCur += 1;
}

void BitstreamWriter::quantize(atUint8* data, atUint8 q, atInt32 val) {
  atUint32 byteCur = (m_bitCur / 32) * 4;
  atUint32 bitRem = m_bitCur % 32;

  atUint32 masked = val & ((1 << q) - 1);
  assert(((((val >> 31) & 0x1) == 0x1) || (((masked >> (q - 1)) & 0x1) == 0)) && "Twos compliment fail");

  /* Fill 32 bit buffer with region containing bits */
  /* Make them least significant */
  *(atUint32*)(data + byteCur) = hecl::SBig(hecl::SBig(*(atUint32*)(data + byteCur)) | (masked << bitRem));

  /* If this shift underflows the value, buffer the next 32 bits */
  /* And tack onto shifted buffer */
  if ((bitRem + q) > 32) {
    *(atUint32*)(data + byteCur + 4) =
        hecl::SBig(hecl::SBig(*(atUint32*)(data + byteCur + 4)) | (masked >> (32 - bitRem)));
  }

  m_bitCur += q;
}

std::unique_ptr<atUint8[]> BitstreamWriter::write(const std::vector<std::vector<Value>>& chanKeys, size_t keyFrameCount,
                                                  std::vector<Channel>& channels, atUint32 quantRange,
                                                  atUint32& rotDivOut, float& transMultOut, float& scaleMultOut,
                                                  size_t& sizeOut) {
  m_bitCur = 0;
  rotDivOut = quantRange; /* Normalized range of values */
  float quantRangeF = float(quantRange);

  /* Pre-pass to calculate translation multiplier */
  float maxTransDelta = 0.0f;
  float maxScaleDelta = 0.0f;
  auto kit = chanKeys.begin();
  for (Channel& chan : channels) {
    switch (chan.type) {
    case Channel::Type::Translation: {
      zeus::simd<float> lastVal = {};
      for (auto it = kit->begin(); it != kit->end(); ++it) {
        const Value* key = &*it;
        zeus::simd_floats f(key->simd - lastVal);
        lastVal = key->simd;
        maxTransDelta = std::max(maxTransDelta, std::fabs(f[0]));
        maxTransDelta = std::max(maxTransDelta, std::fabs(f[1]));
        maxTransDelta = std::max(maxTransDelta, std::fabs(f[2]));
      }
      break;
    }
    case Channel::Type::Scale: {
      zeus::simd<float> lastVal = {};
      for (auto it = kit->begin(); it != kit->end(); ++it) {
        const Value* key = &*it;
        zeus::simd_floats f(key->simd - lastVal);
        lastVal = key->simd;
        maxScaleDelta = std::max(maxScaleDelta, std::fabs(f[0]));
        maxScaleDelta = std::max(maxScaleDelta, std::fabs(f[1]));
        maxScaleDelta = std::max(maxScaleDelta, std::fabs(f[2]));
      }
      break;
    }
    default:
      break;
    }
    ++kit;
  }
  transMultOut = maxTransDelta / quantRangeF + FLT_EPSILON;
  scaleMultOut = maxScaleDelta / quantRangeF + FLT_EPSILON;

  /* Output channel inits */
  std::vector<QuantizedValue> initVals;
  initVals.reserve(channels.size());
  kit = chanKeys.begin();
  for (Channel& chan : channels) {
    chan.q[0] = 1;
    chan.q[1] = 1;
    chan.q[2] = 1;
    switch (chan.type) {
    case Channel::Type::Rotation: {
      QuantizedRot qr = QuantizeRotation((*kit)[0], rotDivOut);
      chan.i = qr.v;
      initVals.push_back(chan.i);
      break;
    }
    case Channel::Type::Translation: {
      zeus::simd_floats f((*kit)[0].simd);
      chan.i = {atInt32(f[0] / transMultOut), atInt32(f[1] / transMultOut), atInt32(f[2] / transMultOut)};
      initVals.push_back(chan.i);
      break;
    }
    case Channel::Type::Scale: {
      zeus::simd_floats f((*kit)[0].simd);
      chan.i = {atInt32(f[0] / scaleMultOut), atInt32(f[1] / scaleMultOut), atInt32(f[2] / scaleMultOut)};
      initVals.push_back(chan.i);
      break;
    }
    default:
      break;
    }
    ++kit;
  }

  /* Pre-pass to analyze quantization factors for channels */
  std::vector<QuantizedValue> lastVals = initVals;
  kit = chanKeys.begin();
  auto vit = lastVals.begin();
  for (Channel& chan : channels) {
    QuantizedValue& last = *vit++;
    switch (chan.type) {
    case Channel::Type::Rotation: {
      for (auto it = kit->begin() + 1; it != kit->end(); ++it) {
        QuantizedRot qrCur = QuantizeRotation(*it, rotDivOut);
        chan.q[0] = std::max(chan.q[0], atUint8(qrCur.v.qFrom(last, 0)));
        chan.q[1] = std::max(chan.q[1], atUint8(qrCur.v.qFrom(last, 1)));
        chan.q[2] = std::max(chan.q[2], atUint8(qrCur.v.qFrom(last, 2)));
        last = qrCur.v;
      }
      break;
    }
    case Channel::Type::Translation: {
      for (auto it = kit->begin() + 1; it != kit->end(); ++it) {
        zeus::simd_floats f(it->simd);
        QuantizedValue cur = {atInt32(f[0] / transMultOut), atInt32(f[1] / transMultOut), atInt32(f[2] / transMultOut)};
        chan.q[0] = std::max(chan.q[0], atUint8(cur.qFrom(last, 0)));
        chan.q[1] = std::max(chan.q[1], atUint8(cur.qFrom(last, 1)));
        chan.q[2] = std::max(chan.q[2], atUint8(cur.qFrom(last, 2)));
        last = cur;
      }
      break;
    }
    case Channel::Type::Scale: {
      for (auto it = kit->begin() + 1; it != kit->end(); ++it) {
        zeus::simd_floats f(it->simd);
        QuantizedValue cur = {atInt32(f[0] / scaleMultOut), atInt32(f[1] / scaleMultOut), atInt32(f[2] / scaleMultOut)};
        chan.q[0] = std::max(chan.q[0], atUint8(cur.qFrom(last, 0)));
        chan.q[1] = std::max(chan.q[1], atUint8(cur.qFrom(last, 1)));
        chan.q[2] = std::max(chan.q[2], atUint8(cur.qFrom(last, 2)));
        last = cur;
      }
      break;
    }
    default:
      break;
    }
    ++kit;
  }

  /* Generate Bitstream */
  sizeOut = ComputeBitstreamSize(keyFrameCount, channels);
  std::unique_ptr<atUint8[]> newData(new atUint8[sizeOut]);
  memset(newData.get(), 0, sizeOut);

  lastVals = initVals;
  for (size_t frame = 0; frame < keyFrameCount; ++frame) {
    kit = chanKeys.begin();
    vit = lastVals.begin();
    for (const Channel& chan : channels) {
      const Value& val = (*kit++)[frame + 1];
      QuantizedValue& last = *vit++;
      switch (chan.type) {
      case Channel::Type::Rotation: {
        QuantizedRot qrCur = QuantizeRotation(val, rotDivOut);
        quantizeBit(newData.get(), qrCur.w);
        quantize(newData.get(), chan.q[0], qrCur.v[0] - last.v[0]);
        quantize(newData.get(), chan.q[1], qrCur.v[1] - last.v[1]);
        quantize(newData.get(), chan.q[2], qrCur.v[2] - last.v[2]);
        last = qrCur.v;
        break;
      }
      case Channel::Type::Translation: {
        zeus::simd_floats f(val.simd);
        QuantizedValue cur = {atInt32(f[0] / transMultOut), atInt32(f[1] / transMultOut), atInt32(f[2] / transMultOut)};
        quantize(newData.get(), chan.q[0], cur[0] - last[0]);
        quantize(newData.get(), chan.q[1], cur[1] - last[1]);
        quantize(newData.get(), chan.q[2], cur[2] - last[2]);
        last = cur;
        break;
      }
      case Channel::Type::Scale: {
        zeus::simd_floats f(val.simd);
        QuantizedValue cur = {atInt32(f[0] / scaleMultOut), atInt32(f[1] / scaleMultOut), atInt32(f[2] / scaleMultOut)};
        quantize(newData.get(), chan.q[0], cur[0] - last[0]);
        quantize(newData.get(), chan.q[1], cur[1] - last[1]);
        quantize(newData.get(), chan.q[2], cur[2] - last[2]);
        last = cur;
        break;
      }
      default:
        break;
      }
    }
  }
  return newData;
}

} // namespace DataSpec::DNAANIM
