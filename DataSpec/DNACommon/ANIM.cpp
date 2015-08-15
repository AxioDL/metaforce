#include "ANIM.hpp"

namespace Retro
{
namespace DNAANIM
{

size_t ComputeBitstreamSize(size_t keyFrameCount, const std::vector<Channel>& channels)
{
    size_t bitsPerKeyFrame = 0;
    for (const Channel& chan : channels)
    {
        switch (chan.type)
        {
        case Channel::ROTATION:
            bitsPerKeyFrame += 1;
        case Channel::TRANSLATION:
        case Channel::SCALE:
        {
            bitsPerKeyFrame += chan.q[0];
            bitsPerKeyFrame += chan.q[1];
            bitsPerKeyFrame += chan.q[2];
            break;
        }
        default: break;
        }
    }
    return (bitsPerKeyFrame * keyFrameCount + 31) / 32 * 4;
}

static inline QuantizedRot QuantizeRotation(const Value& quat, atUint32 div)
{
    float q = M_PI / 2.0 / div;
    return
    {
        {
            atInt16(asinf(quat.v4.vec[1]) / q),
            atInt16(asinf(quat.v4.vec[2]) / q),
            atInt16(asinf(quat.v4.vec[3]) / q),
        },
        (quat.v4.vec[0] < 0) ? true : false
    };
}
static inline Value DequantizeRotation(const QuantizedRot& v, atUint32 div)
{
    float q = M_PI / 2.0 / div;
    Value retval =
    {
        0.0,
        sinf(v.v[0] * q),
        sinf(v.v[1] * q),
        sinf(v.v[2] * q),
    };
    retval.v4.vec[0] = sqrtf(MAX((1.0 -
                               (retval.v4.vec[1] * retval.v4.vec[1] +
                                retval.v4.vec[2] * retval.v4.vec[2] +
                                retval.v4.vec[3] * retval.v4.vec[3])), 0.0));
    retval.v4.vec[0] = v.w ? -retval.v4.vec[0] : retval.v4.vec[0];
    return retval;
}

bool BitstreamReader::dequantizeBit(const atUint8* data)
{
    atUint32 byteCur = (m_bitCur / 32) * 4;
    atUint32 bitRem = m_bitCur % 32;

    /* Fill 32 bit buffer with region containing bits */
    /* Make them least significant */
    atUint32 tempBuf = HECL::SBig(*(atUint32*)(data + byteCur)) >> bitRem;

    /* That's it */
    m_bitCur += 1;
    return tempBuf & 0x1;
}

atInt16 BitstreamReader::dequantize(const atUint8* data, atUint8 q)
{
    atUint32 byteCur = (m_bitCur / 32) * 4;
    atUint32 bitRem = m_bitCur % 32;

    /* Fill 32 bit buffer with region containing bits */
    /* Make them least significant */
    atUint32 tempBuf = HECL::SBig(*(atUint32*)(data + byteCur)) >> bitRem;

    /* If this shift underflows the value, buffer the next 32 bits */
    /* And tack onto shifted buffer */
    if ((bitRem + q) > 32)
    {
        atUint32 tempBuf2 = HECL::SBig(*(atUint32*)(data + byteCur + 4));
        tempBuf |= (tempBuf2 << (32 - bitRem));
    }

    /* Pick out sign */
    atUint32 sign = (tempBuf >> (q - 1)) & 0x1;
    if (sign)
        tempBuf = ~tempBuf;

    /* mask it (excluding sign bit) */
    atUint32 mask = (1 << (q - 1)) - 1;
    tempBuf &= mask;

    /* Return delta value */
    m_bitCur += q;
    return atInt32(sign ? (tempBuf + 1) * -1 : tempBuf);
}

std::vector<std::vector<Value>>
BitstreamReader::read(const atUint8* data,
                      size_t keyFrameCount,
                      const std::vector<Channel>& channels,
                      atUint32 rotDiv,
                      float transMult)
{
    m_bitCur = 0;
    std::vector<std::vector<Value>> chanKeys;
    std::vector<QuantizedValue> chanAccum;
    chanKeys.reserve(channels.size());
    chanAccum.reserve(channels.size());
    for (const Channel& chan : channels)
    {
        chanAccum.push_back({chan.i[0], chan.i[1], chan.i[2]});
        QuantizedValue& accum = chanAccum.back();

        chanKeys.emplace_back();
        std::vector<Value>& keys = chanKeys.back();
        keys.reserve(keyFrameCount);
        switch (chan.type)
        {
        case Channel::ROTATION:
        {
            QuantizedRot qr = {{chan.i[0], chan.i[1], chan.i[2]}, false};
            keys.emplace_back(DequantizeRotation(qr, rotDiv));
            break;
        }
        case Channel::TRANSLATION:
        {
            keys.push_back({chan.i[0] * transMult, chan.i[1] * transMult, chan.i[2] * transMult});
            break;
        }
        case Channel::SCALE:
        {
            keys.push_back({chan.i[0] / (float)rotDiv, chan.i[1] / (float)rotDiv, chan.i[2] / (float)rotDiv});
            break;
        }
        default: break;
        }
    }
    for (size_t f=0 ; f<keyFrameCount ; ++f)
    {
        auto kit = chanKeys.begin();
        auto ait = chanAccum.begin();
        for (const Channel& chan : channels)
        {
            QuantizedValue& p = *ait;
            switch (chan.type)
            {
            case Channel::ROTATION:
            {
                bool wBit = dequantizeBit(data);
                p[0] += dequantize(data, chan.q[0]);
                p[1] += dequantize(data, chan.q[1]);
                p[2] += dequantize(data, chan.q[2]);
                QuantizedRot qr = {{p[0], p[1], p[2]}, wBit};
                kit->emplace_back(DequantizeRotation(qr, rotDiv));
                break;
            }
            case Channel::TRANSLATION:
            {
                p[0] += dequantize(data, chan.q[0]);
                p[1] += dequantize(data, chan.q[1]);
                p[2] += dequantize(data, chan.q[2]);
                kit->push_back({p[0] * transMult, p[1] * transMult, p[2] * transMult});
                break;
            }
            case Channel::SCALE:
            {
                p[0] += dequantize(data, chan.q[0]);
                p[1] += dequantize(data, chan.q[1]);
                p[2] += dequantize(data, chan.q[2]);
                kit->push_back({p[0] / (float)rotDiv, p[1] / (float)rotDiv, p[2] / (float)rotDiv});
                break;
            }
            default: break;
            }
            ++kit;
            ++ait;
        }
    }
    return chanKeys;
}

void BitstreamWriter::quantizeBit(atUint8* data, bool val)
{
    atUint32 byteCur = (m_bitCur / 32) * 4;
    atUint32 bitRem = m_bitCur % 32;

    /* Fill 32 bit buffer with region containing bits */
    /* Make them least significant */
    *(atUint32*)(data + byteCur) =
    HECL::SBig(HECL::SBig(*(atUint32*)(data + byteCur)) | (val << bitRem));

    m_bitCur += 1;
}

void BitstreamWriter::quantize(atUint8* data, atUint8 q, atInt16 val)
{
    atUint32 byteCur = (m_bitCur / 32) * 4;
    atUint32 bitRem = m_bitCur % 32;

    atUint32 masked = val & ((1 << q) - 1);

    /* Fill 32 bit buffer with region containing bits */
    /* Make them least significant */
    *(atUint32*)(data + byteCur) =
    HECL::SBig(HECL::SBig(*(atUint32*)(data + byteCur)) | (masked << bitRem));

    /* If this shift underflows the value, buffer the next 32 bits */
    /* And tack onto shifted buffer */
    if ((bitRem + q) > 32)
    {
        *(atUint32*)(data + byteCur + 4) =
        HECL::SBig(HECL::SBig(*(atUint32*)(data + byteCur + 4)) | (masked >> (32 - bitRem)));
    }

    m_bitCur += q;
}

std::unique_ptr<atUint8[]>
BitstreamWriter::write(const std::vector<std::vector<Value>>& chanKeys,
                       size_t keyFrameCount, std::vector<Channel>& channels,
                       atUint32& rotDivOut,
                       float& transMultOut,
                       size_t& sizeOut)
{
    m_bitCur = 0;
    rotDivOut = 32767; /* Normalized range of values */

    /* Pre-pass to calculate translation multiplier */
    float maxTransDiff = 0.0;
    auto kit = chanKeys.begin();
    for (Channel& chan : channels)
    {
        switch (chan.type)
        {
        case Channel::TRANSLATION:
        {
            const Value* last = &(*kit)[0];
            for (auto it=kit->begin() + 1;
                 it != kit->end();
                 ++it)
            {
                const Value* current = &*it;
                maxTransDiff = MAX(maxTransDiff, current->v3.vec[0] - last->v3.vec[0]);
                maxTransDiff = MAX(maxTransDiff, current->v3.vec[1] - last->v3.vec[1]);
                maxTransDiff = MAX(maxTransDiff, current->v3.vec[2] - last->v3.vec[2]);
                last = current;
            }
            break;
        }
        default: break;
        }
        ++kit;
    }
    transMultOut = maxTransDiff / 32767;

    /* Output channel inits */
    kit = chanKeys.begin();
    for (Channel& chan : channels)
    {
        chan.q[0] = 1;
        chan.q[1] = 1;
        chan.q[2] = 1;
        switch (chan.type)
        {
        case Channel::ROTATION:
        {
            QuantizedRot qr = QuantizeRotation((*kit)[0], rotDivOut);
            chan.i = qr.v;
            break;
        }
        case Channel::TRANSLATION:
        {
            chan.i = {atInt16((*kit)[0].v3.vec[0] / transMultOut),
                      atInt16((*kit)[0].v3.vec[1] / transMultOut),
                      atInt16((*kit)[0].v3.vec[2] / transMultOut)};
            break;
        }
        case Channel::SCALE:
        {
            chan.i = {atInt16((*kit)[0].v3.vec[0] * rotDivOut),
                      atInt16((*kit)[0].v3.vec[1] * rotDivOut),
                      atInt16((*kit)[0].v3.vec[2] * rotDivOut)};
            break;
        }
        default: break;
        }
        ++kit;
    }

    /* Pre-pass to analyze quantization factors for channels */
    kit = chanKeys.begin();
    for (Channel& chan : channels)
    {
        switch (chan.type)
        {
        case Channel::ROTATION:
        {
            QuantizedRot qrLast = QuantizeRotation((*kit)[0], rotDivOut);
            for (auto it=kit->begin() + 1;
                 it != kit->end();
                 ++it)
            {
                QuantizedRot qrCur = QuantizeRotation(*it, rotDivOut);
                chan.q[0] = MAX(chan.q[0], ceilf(log2f(qrCur.v[0] - qrLast.v[0])));
                chan.q[1] = MAX(chan.q[1], ceilf(log2f(qrCur.v[1] - qrLast.v[1])));
                chan.q[2] = MAX(chan.q[2], ceilf(log2f(qrCur.v[2] - qrLast.v[2])));
                qrLast = qrCur;
            }
            break;
        }
        case Channel::TRANSLATION:
        {
            QuantizedValue last = {atInt16((*kit)[0].v3.vec[0] / transMultOut),
                                   atInt16((*kit)[0].v3.vec[1] / transMultOut),
                                   atInt16((*kit)[0].v3.vec[2] / transMultOut)};
            for (auto it=kit->begin() + 1;
                 it != kit->end();
                 ++it)
            {
                QuantizedValue cur = {atInt16(it->v3.vec[0] / transMultOut),
                                      atInt16(it->v3.vec[1] / transMultOut),
                                      atInt16(it->v3.vec[2] / transMultOut)};
                chan.q[0] = MAX(chan.q[0], ceilf(log2f(cur[0] - last[0])));
                chan.q[1] = MAX(chan.q[1], ceilf(log2f(cur[1] - last[1])));
                chan.q[2] = MAX(chan.q[2], ceilf(log2f(cur[2] - last[2])));
                last = cur;
            }
            break;
        }
        case Channel::SCALE:
        {
            QuantizedValue last = {atInt16((*kit)[0].v3.vec[0] * rotDivOut),
                                   atInt16((*kit)[0].v3.vec[1] * rotDivOut),
                                   atInt16((*kit)[0].v3.vec[2] * rotDivOut)};
            for (auto it=kit->begin() + 1;
                 it != kit->end();
                 ++it)
            {
                QuantizedValue cur = {atInt16(it->v3.vec[0] * rotDivOut),
                                      atInt16(it->v3.vec[1] * rotDivOut),
                                      atInt16(it->v3.vec[2] * rotDivOut)};
                chan.q[0] = MAX(chan.q[0], ceilf(log2f(cur[0] - last[0])));
                chan.q[1] = MAX(chan.q[1], ceilf(log2f(cur[1] - last[1])));
                chan.q[2] = MAX(chan.q[2], ceilf(log2f(cur[2] - last[2])));
                last = cur;
            }
            break;
        }
        default: break;
        }
        ++kit;
    }

    /* Generate Bitstream */
    sizeOut = ComputeBitstreamSize(keyFrameCount, channels);
    atUint8* newData = new atUint8[sizeOut];
    for (size_t f=0 ; f<keyFrameCount ; ++f)
    {
        kit = chanKeys.begin();
        for (const Channel& chan : channels)
        {
            switch (chan.type)
            {
            case Channel::ROTATION:
            {
                QuantizedRot qrLast = QuantizeRotation((*kit)[0], rotDivOut);
                for (auto it=kit->begin() + 1;
                     it != kit->end();
                     ++it)
                {
                    QuantizedRot qrCur = QuantizeRotation(*it, rotDivOut);
                    quantizeBit(newData, qrCur.w);
                    quantize(newData, chan.q[0], qrCur.v[0] - qrLast.v[0]);
                    quantize(newData, chan.q[1], qrCur.v[1] - qrLast.v[0]);
                    quantize(newData, chan.q[2], qrCur.v[2] - qrLast.v[0]);
                    qrLast = qrCur;
                }
                break;
            }
            case Channel::TRANSLATION:
            {
                QuantizedValue last = {atInt16((*kit)[0].v3.vec[0] / transMultOut),
                                       atInt16((*kit)[0].v3.vec[1] / transMultOut),
                                       atInt16((*kit)[0].v3.vec[2] / transMultOut)};
                for (auto it=kit->begin() + 1;
                     it != kit->end();
                     ++it)
                {
                    QuantizedValue cur = {atInt16(it->v3.vec[0] / transMultOut),
                                          atInt16(it->v3.vec[1] / transMultOut),
                                          atInt16(it->v3.vec[2] / transMultOut)};
                    quantize(newData, chan.q[0], cur[0] - last[0]);
                    quantize(newData, chan.q[1], cur[1] - last[0]);
                    quantize(newData, chan.q[2], cur[2] - last[0]);
                    last = cur;
                }
                break;
            }
            case Channel::SCALE:
            {
                QuantizedValue last = {atInt16((*kit)[0].v3.vec[0] * rotDivOut),
                                       atInt16((*kit)[0].v3.vec[1] * rotDivOut),
                                       atInt16((*kit)[0].v3.vec[2] * rotDivOut)};
                for (auto it=kit->begin() + 1;
                     it != kit->end();
                     ++it)
                {
                    QuantizedValue cur = {atInt16(it->v3.vec[0] * rotDivOut),
                                          atInt16(it->v3.vec[1] * rotDivOut),
                                          atInt16(it->v3.vec[2] * rotDivOut)};
                    quantize(newData, chan.q[0], cur[0] - last[0]);
                    quantize(newData, chan.q[1], cur[1] - last[0]);
                    quantize(newData, chan.q[2], cur[2] - last[0]);
                    last = cur;
                }
                break;
            }
            default: break;
            }
            ++kit;
        }
    }
    return std::unique_ptr<atUint8[]>(newData);
}

}
}
