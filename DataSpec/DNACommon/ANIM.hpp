#pragma once

#include "DNACommon.hpp"
#include <cmath>

namespace DataSpec::DNAANIM
{

struct Value
{
    athena::simd<float> simd;
    Value() = default;
    Value(const athena::simd<float>& s) : simd(s) {}
    Value(const atVec3f& v) : simd(v.simd) {}
    Value(const atVec4f& v) : simd(v.simd) {}
    Value(float x, float y, float z) : simd(x, y, z, 0.f) {}
    Value(float w, float x, float y, float z) : simd(w, x, y, z) {}
};
struct QuantizedValue
{
    atInt32 v[4];
    atInt32& operator[] (size_t idx)
    {return v[idx];}
    atInt32 operator[] (size_t idx) const
    {return v[idx];}

    int qFrom(const QuantizedValue& other, size_t idx) const
    {
        atInt32 delta = std::abs(v[idx] - other.v[idx]);
        if (delta == 0)
            return 1;
        return int(std::ceil(std::log2(delta))) + 1;
    }
};
struct QuantizedRot
{
    QuantizedValue v;
    bool w;
};
struct Channel
{
    enum class Type
    {
        Rotation,
        Translation,
        Scale,
        KfHead,
        RotationMP3
    } type;
    atInt32 id = -1;
    QuantizedValue i = {};
    atUint8 q[4] = {};
};

size_t ComputeBitstreamSize(size_t keyFrameCount, const std::vector<Channel>& channels);

class BitstreamReader
{
    size_t m_bitCur;
    atInt32 dequantize(const atUint8* data, atUint8 q);
    bool dequantizeBit(const atUint8* data);
public:
    std::vector<std::vector<Value>>
    read(const atUint8* data,
         size_t keyFrameCount,
         const std::vector<Channel>& channels,
         atUint32 rotDiv,
         float transMult,
         float scaleMult);
};

class BitstreamWriter
{
    size_t m_bitCur;
    void quantize(atUint8* data, atUint8 q, atInt32 val);
    void quantizeBit(atUint8* data, bool val);
public:
    std::unique_ptr<atUint8[]>
    write(const std::vector<std::vector<Value>>& chanKeys,
          size_t keyFrameCount, std::vector<Channel>& channels,
          atUint32 quantRange,
          atUint32& rotDivOut,
          float& transMultOut,
          float& scaleMultOut,
          size_t& sizeOut);
};

}

