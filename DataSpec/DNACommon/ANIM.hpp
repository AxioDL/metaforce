#ifndef _DNACOMMON_ANIMBITSTREAM_HPP_
#define _DNACOMMON_ANIMBITSTREAM_HPP_

#include "DNACommon.hpp"
#include <cmath>

namespace DataSpec::DNAANIM
{

union Value
{
    atVec3f v3;
    atVec4f v4;
    Value(atVec3f v) : v3(v) {}
    Value(atVec4f v) : v4(v) {}
    Value(float x, float y, float z)
    {
        v3.vec[0] = x;
        v3.vec[1] = y;
        v3.vec[2] = z;
        v4.vec[3] = 0.0;
    }
    Value(float w, float x, float y, float z)
    {
        v4.vec[0] = w;
        v4.vec[1] = x;
        v4.vec[2] = y;
        v4.vec[3] = z;
    }
};
struct QuantizedValue
{
    atInt32 v[4];
    atInt32& operator[] (size_t idx)
    {return v[idx];}
    const atInt32& operator[] (size_t idx) const
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

#endif // _DNACOMMON_ANIMBITSTREAM_HPP_
