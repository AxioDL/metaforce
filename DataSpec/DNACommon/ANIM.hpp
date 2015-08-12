#ifndef _DNACOMMON_ANIMBITSTREAM_HPP_
#define _DNACOMMON_ANIMBITSTREAM_HPP_

#include <math.h>
#include "DNACommon.hpp"

namespace Retro
{
namespace DNAANIM
{

union Value
{
    atVec3f v3;
    atVec4f v4;
    float scale;
    Value(atVec3f v) : v3(v) {}
    Value(atVec4f v) : v4(v) {}
    Value(float v) : scale(v) {}
    Value(float x, float y, float z)
    {
        v3.vec[0] = x;
        v3.vec[1] = y;
        v3.vec[2] = z;
        v4.vec[3] = 0.0;
    }
};
struct QuantizedValue
{
    atInt16 v[3];
    atInt16& operator[] (size_t idx)
    {return v[idx];}
    const atInt16& operator[] (size_t idx) const
    {return v[idx];}
};
struct QuantizedRot
{
    QuantizedValue v;
    bool w;
};
struct Channel
{
    enum Type
    {
        ROTATION,
        TRANSLATION,
        SCALE
    } type;
    QuantizedValue i = {};
    atUint8 q[3] = {};
};

size_t ComputeBitstreamSize(size_t keyFrameCount, const std::vector<Channel>& channels);

class BitstreamReader
{
    size_t m_bitCur;
    atInt16 dequantize(const atUint8* data, atUint8 q);
public:
    std::vector<std::vector<Value>>
    read(const atUint8* data,
         size_t keyFrameCount,
         const std::vector<Channel>& channels,
         atUint32 rotDiv,
         float transMult);
};

class BitstreamWriter
{
    size_t m_bitCur;
    void quantize(atUint8* data, atUint8 q, atInt16 val);
public:
    std::unique_ptr<atUint8[]>
    write(const std::vector<std::vector<Value>>& chanKeys,
          size_t keyFrameCount, std::vector<Channel>& channels,
          atUint32& rotDivOut,
          float& transMultOut,
          size_t& sizeOut);
};

}
}

#endif // _DNACOMMON_ANIMBITSTREAM_HPP_
