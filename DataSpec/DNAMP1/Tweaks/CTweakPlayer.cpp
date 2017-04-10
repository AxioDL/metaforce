#include "CTweakPlayer.hpp"
#include "zeus/Math.hpp"

namespace DataSpec
{
namespace DNAMP1
{
void CTweakPlayer::read(athena::io::IStreamReader& __dna_reader)
{
    /* x4_[0] */
    x4_[0] = __dna_reader.readFloatBig();
    /* x4_[1] */
    x4_[1] = __dna_reader.readFloatBig();
    /* x4_[2] */
    x4_[2] = __dna_reader.readFloatBig();
    /* x4_[3] */
    x4_[3] = __dna_reader.readFloatBig();
    /* x4_[4] */
    x4_[4] = __dna_reader.readFloatBig();
    /* x4_[5] */
    x4_[5] = __dna_reader.readFloatBig();
    /* x4_[6] */
    x4_[6] = __dna_reader.readFloatBig();
    /* x4_[7] */
    x4_[7] = __dna_reader.readFloatBig();
    /* x24_[0] */
    x24_[0] = __dna_reader.readFloatBig();
    /* x24_[1] */
    x24_[1] = __dna_reader.readFloatBig();
    /* x24_[2] */
    x24_[2] = __dna_reader.readFloatBig();
    /* x24_[3] */
    x24_[3] = __dna_reader.readFloatBig();
    /* x24_[4] */
    x24_[4] = __dna_reader.readFloatBig();
    /* x24_[5] */
    x24_[5] = __dna_reader.readFloatBig();
    /* x24_[6] */
    x24_[6] = __dna_reader.readFloatBig();
    /* x24_[7] */
    x24_[7] = __dna_reader.readFloatBig();
    /* x44_[0] */
    x44_[0] = __dna_reader.readFloatBig();
    /* x44_[1] */
    x44_[1] = __dna_reader.readFloatBig();
    /* x44_[2] */
    x44_[2] = __dna_reader.readFloatBig();
    /* x44_[3] */
    x44_[3] = __dna_reader.readFloatBig();
    /* x44_[4] */
    x44_[4] = __dna_reader.readFloatBig();
    /* x44_[5] */
    x44_[5] = __dna_reader.readFloatBig();
    /* x44_[6] */
    x44_[6] = __dna_reader.readFloatBig();
    /* x44_[7] */
    x44_[7] = __dna_reader.readFloatBig();
    /* x64_[0] */
    x64_[0] = __dna_reader.readFloatBig();
    /* x64_[1] */
    x64_[1] = __dna_reader.readFloatBig();
    /* x64_[2] */
    x64_[2] = __dna_reader.readFloatBig();
    /* x64_[3] */
    x64_[3] = __dna_reader.readFloatBig();
    /* x64_[4] */
    x64_[4] = __dna_reader.readFloatBig();
    /* x64_[5] */
    x64_[5] = __dna_reader.readFloatBig();
    /* x64_[6] */
    x64_[6] = __dna_reader.readFloatBig();
    /* x64_[7] */
    x64_[7] = __dna_reader.readFloatBig();
    /* x84_[0] */
    x84_[0] = __dna_reader.readFloatBig();
    /* x84_[1] */
    x84_[1] = __dna_reader.readFloatBig();
    /* x84_[2] */
    x84_[2] = __dna_reader.readFloatBig();
    /* x84_[3] */
    x84_[3] = __dna_reader.readFloatBig();
    /* x84_[4] */
    x84_[4] = __dna_reader.readFloatBig();
    /* x84_[5] */
    x84_[5] = __dna_reader.readFloatBig();
    /* x84_[6] */
    x84_[6] = __dna_reader.readFloatBig();
    /* x84_[7] */
    x84_[7] = __dna_reader.readFloatBig();
    /* xa4_[0] */
    xa4_[0] = __dna_reader.readFloatBig();
    /* xa4_[1] */
    xa4_[1] = __dna_reader.readFloatBig();
    /* xa4_[2] */
    xa4_[2] = __dna_reader.readFloatBig();
    /* xa4_[3] */
    xa4_[3] = __dna_reader.readFloatBig();
    /* xa4_[4] */
    xa4_[4] = __dna_reader.readFloatBig();
    /* xa4_[5] */
    xa4_[5] = __dna_reader.readFloatBig();
    /* xa4_[6] */
    xa4_[6] = __dna_reader.readFloatBig();
    /* xa4_[7] */
    xa4_[7] = __dna_reader.readFloatBig();
    /* xc4_ */
    xc4_ = __dna_reader.readFloatBig();
    /* xc8_ */
    xc8_ = __dna_reader.readFloatBig();
    /* xcc_ */
    xcc_ = __dna_reader.readFloatBig();
    /* xd0_ */
    xd0_ = __dna_reader.readFloatBig();
    /* xd4_ */
    xd4_ = __dna_reader.readFloatBig();
    /* xd8_ */
    xd8_ = __dna_reader.readFloatBig();
    /* xdc_ */
    xdc_ = __dna_reader.readFloatBig();
    /* xe0_ */
    xe0_ = __dna_reader.readFloatBig();
    /* xe4_ */
    xe4_ = __dna_reader.readFloatBig();
    /* xe8_ */
    xe8_ = __dna_reader.readFloatBig();
    /* xec_ */
    xec_ = __dna_reader.readFloatBig();
    /* xf0_ */
    xf0_ = __dna_reader.readFloatBig();
    /* xf4_ */
    xf4_ = __dna_reader.readFloatBig();
    /* xf8_ */
    xf8_ = __dna_reader.readFloatBig();
    /* xfc_ */
    xfc_ = __dna_reader.readFloatBig();
    /* x100_ */
    x100_ = __dna_reader.readFloatBig();
    /* x104_ */
    x104_ = __dna_reader.readFloatBig();
    /* x108_ */
    x108_ = __dna_reader.readFloatBig();
    /* x10c_ */
    x10c_ = __dna_reader.readFloatBig();
    /* x110_ */
    x110_ = __dna_reader.readFloatBig();
    /* x114_ */
    x114_ = __dna_reader.readFloatBig();
    /* x118_ */
    x118_ = __dna_reader.readFloatBig();
    /* x11c_ */
    x11c_ = __dna_reader.readFloatBig();
    /* x120_ */
    x120_ = __dna_reader.readFloatBig();
    /* x124_ */
    x124_ = __dna_reader.readFloatBig();
    /* x128_ */
    x128_ = __dna_reader.readFloatBig();
    /* x12c_ */
    x12c_ = __dna_reader.readFloatBig();
    /* x130_ */
    x130_ = __dna_reader.readFloatBig();
    /* x134_ */
    x134_ = __dna_reader.readFloatBig();
    /* x138_ */
    x138_hudLagAmount = __dna_reader.readFloatBig();
    /* x13c_ */
    x13c_ = __dna_reader.readFloatBig();
    /* x140_ */
    x140_ = __dna_reader.readFloatBig();
    /* x144_ */
    x144_ = __dna_reader.readFloatBig();
    /* x148_ */
    x148_ = __dna_reader.readFloatBig();
    /* x14c_ */
    x14c_ = __dna_reader.readFloatBig();
    /* x150_leftDiv */
    x150_leftDiv = __dna_reader.readFloatBig();
    /* x154_rightDiv */
    x154_rightDiv = __dna_reader.readFloatBig();
    /* x228_24_ */
    x228_24_ = __dna_reader.readBool();
    /* x228_25_ */
    x228_25_ = __dna_reader.readBool();
    /* x228_26_ */
    x228_26_ = __dna_reader.readBool();
    /* x228_27_ */
    x228_27_ = __dna_reader.readBool();
    /* x228_28_ */
    x228_28_ = __dna_reader.readBool();
    /* x228_29_ */
    x228_29_ = __dna_reader.readBool();
    /* x228_30_ */
    x228_30_ = __dna_reader.readBool();
    /* x228_31_ */
    x228_31_ = __dna_reader.readBool();
    /* x229_24_ */
    x229_24_ = __dna_reader.readBool();
    /* x229_25_ */
    x229_25_ = __dna_reader.readBool();
    /* x229_26_ */
    x229_26_ = __dna_reader.readBool();
    /* x229_27_ */
    x229_27_ = __dna_reader.readBool();
    /* x229_28_ */
    x229_28_ = __dna_reader.readBool();
    /* x229_29_ */
    x229_29_ = __dna_reader.readBool();
    /* x229_30_ */
    x229_30_ = __dna_reader.readBool();
    /* x229_31_ */
    x229_31_ = __dna_reader.readBool();
    /* x22a_24_ */
    x22a_24_ = __dna_reader.readBool();
    /* x22a_25_ */
    x22a_25_ = __dna_reader.readBool();
    /* x22a_26_ */
    x22a_26_ = __dna_reader.readBool();
    /* x22a_27_ */
    x22a_27_ = __dna_reader.readBool();
    /* x22a_28_ */
    x22a_28_ = __dna_reader.readBool();
    /* x22c_ */
    x22c_ = __dna_reader.readFloatBig();
    /* x230_ */
    x230_ = __dna_reader.readFloatBig();
    /* x234_ */
    x234_ = __dna_reader.readFloatBig();
    /* x238_ */
    x238_ = __dna_reader.readFloatBig();
    /* x23c_ */
    x23c_ = __dna_reader.readFloatBig();
    /* x240_ */
    x240_ = __dna_reader.readFloatBig();
    /* x244_ */
    x244_ = __dna_reader.readFloatBig();
    /* x248_ */
    x248_ = __dna_reader.readFloatBig();
    /* x24c_ */
    x24c_ = __dna_reader.readFloatBig();
    /* x250_ */
    x250_ = __dna_reader.readFloatBig();
    /* x254_ */
    x254_ = __dna_reader.readFloatBig();
    /* x258_ */
    x258_ = __dna_reader.readFloatBig();
    /* x25c_ */
    x25c_ = __dna_reader.readFloatBig();
    /* x260_ */
    x260_ = __dna_reader.readFloatBig();
    /* x264_ */
    x264_ = __dna_reader.readFloatBig();
    /* x268_ */
    x268_ = __dna_reader.readFloatBig();
    /* x158_[0] */
    x158_[0] = __dna_reader.readFloatBig();
    /* x164_[0] */
    x164_[0] = __dna_reader.readFloatBig();
    /* x170_[0] */
    x170_[0] = __dna_reader.readFloatBig();
    /* x158_[1] */
    x158_[1] = __dna_reader.readFloatBig();
    /* x164_[1] */
    x164_[1] = __dna_reader.readFloatBig();
    /* x170_[1] */
    x170_[1] = __dna_reader.readFloatBig();
    /* x158_[2] */
    x158_[2] = __dna_reader.readFloatBig();
    /* x164_[2] */
    x164_[2] = __dna_reader.readFloatBig();
    /* x170_[2] */
    x170_[2] = __dna_reader.readFloatBig();
    /* x17c_ */
    x17c_ = __dna_reader.readFloatBig();
    /* x180_ */
    x180_ = __dna_reader.readFloatBig();
    /* x184_ */
    x184_ = __dna_reader.readFloatBig();
    /* x188_ */
    x188_ = __dna_reader.readFloatBig();
    /* x18c_ */
    x18c_ = __dna_reader.readFloatBig();
    /* x190_ */
    x190_ = __dna_reader.readFloatBig();
    /* x194_ */
    x194_ = __dna_reader.readFloatBig();
    /* x198_ */
    x198_ = __dna_reader.readFloatBig();
    /* x19c_ */
    x19c_ = __dna_reader.readFloatBig();
    /* x1a0_ */
    x1a0_ = __dna_reader.readFloatBig();
    /* x1a4_ */
    x1a4_ = __dna_reader.readFloatBig();
    /* x1a8_[0] */
    x1a8_[0] = __dna_reader.readUint32Big();
    /* x1b0_[0] */
    x1b0_[0] = __dna_reader.readUint32Big();
    /* x1b8_[0] */
    x1b8_[0] = __dna_reader.readUint32Big();
    /* x1c0_[0] */
    x1c0_[0] = __dna_reader.readUint32Big();
    /* x1c8_[0] */
    x1c8_[0] = __dna_reader.readUint32Big();
    /* x1d0_[0] */
    x1d0_[0] = __dna_reader.readUint32Big();
    /* x1a8_[1] */
    x1a8_[1] = __dna_reader.readUint32Big();
    /* x1b0_[1] */
    x1b0_[1] = __dna_reader.readUint32Big();
    /* x1b8_[1] */
    x1b8_[1] = __dna_reader.readUint32Big();
    /* x1c0_[1] */
    x1c0_[1] = __dna_reader.readUint32Big();
    /* x1c8_[1] */
    x1c8_[1] = __dna_reader.readUint32Big();
    /* x1d0_[1] */
    x1d0_[1] = __dna_reader.readUint32Big();
    /* x1d8_ */
    x1d8_ = __dna_reader.readFloatBig();
    /* x1dc_ */
    x1dc_ = __dna_reader.readFloatBig();
    /* x1e0_ */
    x1e0_ = __dna_reader.readFloatBig();
    /* x1e4_ */
    x1e4_ = __dna_reader.readFloatBig();
    /* x1e8_ */
    x1e8_ = __dna_reader.readFloatBig();
    /* x1ec_ */
    x1ec_ = __dna_reader.readFloatBig();
    /* x1f0_ */
    x1f0_ = __dna_reader.readFloatBig();
    /* x1f4_ */
    x1f4_ = __dna_reader.readFloatBig();
    /* x1f8_ */
    x1f8_ = __dna_reader.readFloatBig();
    /* x1fc_ */
    x1fc_ = __dna_reader.readFloatBig();
    /* x200_24_ */
    x200_24_ = __dna_reader.readBool();
    /* x200_25_ */
    x200_25_ = __dna_reader.readBool();
    /* x204_ */
    x204_ = __dna_reader.readFloatBig();
    /* x208_ */
    x208_ = __dna_reader.readFloatBig();
    /* x20c_ */
    x20c_ = __dna_reader.readFloatBig();
    /* x210_ */
    x210_ = __dna_reader.readFloatBig();
    /* x214_ */
    x214_ = __dna_reader.readFloatBig();
    /* x218_ */
    x218_scanningRange = __dna_reader.readFloatBig();
    /* x21c_24_ */
    x21c_24_ = __dna_reader.readBool();
    /* x21c_25_ */
    x21c_25_scanFreezesGame = __dna_reader.readBool();
    /* x21c_26_ */
    x21c_26_ = __dna_reader.readBool();
    /* x220_ */
    x220_ = __dna_reader.readFloatBig();
    /* x224_ */
    x224_ = __dna_reader.readFloatBig();
    /* x2a0_ */
    x2a0_ = __dna_reader.readFloatBig();
    /* x2a4_ */
    x2a4_ = __dna_reader.readFloatBig();
    /* x2a8_ */
    x2a8_ = __dna_reader.readFloatBig();
    /* x2ac_ */
    x2ac_ = __dna_reader.readFloatBig();
    /* x2b0_ */
    x2b0_ = __dna_reader.readFloatBig();
    /* x2b4_ */
    x2b4_ = __dna_reader.readFloatBig();
    /* x2b8_ */
    x2b8_ = __dna_reader.readFloatBig();
    /* x2bc_ */
    x2bc_ = __dna_reader.readFloatBig();
    /* x2c0_ */
    x2c0_ = __dna_reader.readFloatBig();
    /* x2c4_ */
    x2c4_ = __dna_reader.readFloatBig();
    /* x2c8_ */
    x2c8_ = __dna_reader.readFloatBig();
    /* x2cc_ */
    x2cc_ = __dna_reader.readFloatBig();
    /* x2d0_ */
    x2d0_ = __dna_reader.readUint32Big();
    /* x2d4_ */
    x2d4_ = __dna_reader.readBool();
    /* x2d5_ */
    x2d5_ = __dna_reader.readBool();
    /* x2d8_ */
    x2d8_ = __dna_reader.readFloatBig();
    /* x2dc_ */
    x2dc_ = __dna_reader.readFloatBig();
    /* x2e0_ */
    x2e0_ = __dna_reader.readFloatBig();
    /* x2e4_ */
    x2e4_ = __dna_reader.readFloatBig();
    /* x26c_playerHeight */
    x26c_playerHeight = __dna_reader.readFloatBig();
    /* x270_playerXYHalfExtent */
    x270_playerXYHalfExtent = __dna_reader.readFloatBig();
    /* x274_ */
    x274_ = __dna_reader.readFloatBig();
    /* x278_ */
    x278_ = __dna_reader.readFloatBig();
    /* x27c_playerBallHalfExtent */
    x27c_playerBallHalfExtent = __dna_reader.readFloatBig();
    /* x280_ */
    x280_ = __dna_reader.readFloatBig();
    /* x284_ */
    x284_ = __dna_reader.readFloatBig();
    /* x288_ */
    x288_ = __dna_reader.readFloatBig();
    /* x28c_ */
    x28c_ = __dna_reader.readFloatBig();
    /* x290_ */
    x290_ = __dna_reader.readFloatBig();
    /* x294_ */
    x294_ = __dna_reader.readFloatBig();
    /* x298_ */
    x298_ = __dna_reader.readFloatBig();
    /* x29c_ */
    x29c_ = __dna_reader.readFloatBig();
    /* x2e8_ */
    x2e8_ = __dna_reader.readFloatBig();
    /* x2ec_ */
    x2ec_ = __dna_reader.readFloatBig();
    /* x2f0_ */
    x2f0_ = __dna_reader.readFloatBig();
    /* x2f4_ */
    x2f4_ = __dna_reader.readBool();
    /* x2f8_ */
    x2f8_ = __dna_reader.readFloatBig();
    /* x2fc_ */
    x2fc_ = __dna_reader.readUint32Big();
    /* x300_ */
    x300_variaDamageReduction = __dna_reader.readFloatBig();
    /* x304_ */
    x304_gravityDamageReduction = __dna_reader.readFloatBig();
    /* x308_ */
    x308_phazonDamageReduction = __dna_reader.readFloatBig();
}

void CTweakPlayer::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* x4_[0] */
    __dna_writer.writeFloatBig(x4_[0]);
    /* x4_[1] */
    __dna_writer.writeFloatBig(x4_[1]);
    /* x4_[2] */
    __dna_writer.writeFloatBig(x4_[2]);
    /* x4_[3] */
    __dna_writer.writeFloatBig(x4_[3]);
    /* x4_[4] */
    __dna_writer.writeFloatBig(x4_[4]);
    /* x4_[5] */
    __dna_writer.writeFloatBig(x4_[5]);
    /* x4_[6] */
    __dna_writer.writeFloatBig(x4_[6]);
    /* x4_[7] */
    __dna_writer.writeFloatBig(x4_[7]);
    /* x24_[0] */
    __dna_writer.writeFloatBig(x24_[0]);
    /* x24_[1] */
    __dna_writer.writeFloatBig(x24_[1]);
    /* x24_[2] */
    __dna_writer.writeFloatBig(x24_[2]);
    /* x24_[3] */
    __dna_writer.writeFloatBig(x24_[3]);
    /* x24_[4] */
    __dna_writer.writeFloatBig(x24_[4]);
    /* x24_[5] */
    __dna_writer.writeFloatBig(x24_[5]);
    /* x24_[6] */
    __dna_writer.writeFloatBig(x24_[6]);
    /* x24_[7] */
    __dna_writer.writeFloatBig(x24_[7]);
    /* x44_[0] */
    __dna_writer.writeFloatBig(x44_[0]);
    /* x44_[1] */
    __dna_writer.writeFloatBig(x44_[1]);
    /* x44_[2] */
    __dna_writer.writeFloatBig(x44_[2]);
    /* x44_[3] */
    __dna_writer.writeFloatBig(x44_[3]);
    /* x44_[4] */
    __dna_writer.writeFloatBig(x44_[4]);
    /* x44_[5] */
    __dna_writer.writeFloatBig(x44_[5]);
    /* x44_[6] */
    __dna_writer.writeFloatBig(x44_[6]);
    /* x44_[7] */
    __dna_writer.writeFloatBig(x44_[7]);
    /* x64_[0] */
    __dna_writer.writeFloatBig(x64_[0]);
    /* x64_[1] */
    __dna_writer.writeFloatBig(x64_[1]);
    /* x64_[2] */
    __dna_writer.writeFloatBig(x64_[2]);
    /* x64_[3] */
    __dna_writer.writeFloatBig(x64_[3]);
    /* x64_[4] */
    __dna_writer.writeFloatBig(x64_[4]);
    /* x64_[5] */
    __dna_writer.writeFloatBig(x64_[5]);
    /* x64_[6] */
    __dna_writer.writeFloatBig(x64_[6]);
    /* x64_[7] */
    __dna_writer.writeFloatBig(x64_[7]);
    /* x84_[0] */
    __dna_writer.writeFloatBig(x84_[0]);
    /* x84_[1] */
    __dna_writer.writeFloatBig(x84_[1]);
    /* x84_[2] */
    __dna_writer.writeFloatBig(x84_[2]);
    /* x84_[3] */
    __dna_writer.writeFloatBig(x84_[3]);
    /* x84_[4] */
    __dna_writer.writeFloatBig(x84_[4]);
    /* x84_[5] */
    __dna_writer.writeFloatBig(x84_[5]);
    /* x84_[6] */
    __dna_writer.writeFloatBig(x84_[6]);
    /* x84_[7] */
    __dna_writer.writeFloatBig(x84_[7]);
    /* xa4_[0] */
    __dna_writer.writeFloatBig(xa4_[0]);
    /* xa4_[1] */
    __dna_writer.writeFloatBig(xa4_[1]);
    /* xa4_[2] */
    __dna_writer.writeFloatBig(xa4_[2]);
    /* xa4_[3] */
    __dna_writer.writeFloatBig(xa4_[3]);
    /* xa4_[4] */
    __dna_writer.writeFloatBig(xa4_[4]);
    /* xa4_[5] */
    __dna_writer.writeFloatBig(xa4_[5]);
    /* xa4_[6] */
    __dna_writer.writeFloatBig(xa4_[6]);
    /* xa4_[7] */
    __dna_writer.writeFloatBig(xa4_[7]);
    /* xc4_ */
    __dna_writer.writeFloatBig(xc4_);
    /* xc8_ */
    __dna_writer.writeFloatBig(xc8_);
    /* xcc_ */
    __dna_writer.writeFloatBig(xcc_);
    /* xd0_ */
    __dna_writer.writeFloatBig(xd0_);
    /* xd4_ */
    __dna_writer.writeFloatBig(xd4_);
    /* xd8_ */
    __dna_writer.writeFloatBig(xd8_);
    /* xdc_ */
    __dna_writer.writeFloatBig(xdc_);
    /* xe0_ */
    __dna_writer.writeFloatBig(xe0_);
    /* xe4_ */
    __dna_writer.writeFloatBig(xe4_);
    /* xe8_ */
    __dna_writer.writeFloatBig(xe8_);
    /* xec_ */
    __dna_writer.writeFloatBig(xec_);
    /* xf0_ */
    __dna_writer.writeFloatBig(xf0_);
    /* xf4_ */
    __dna_writer.writeFloatBig(xf4_);
    /* xf8_ */
    __dna_writer.writeFloatBig(xf8_);
    /* xfc_ */
    __dna_writer.writeFloatBig(xfc_);
    /* x100_ */
    __dna_writer.writeFloatBig(x100_);
    /* x104_ */
    __dna_writer.writeFloatBig(x104_);
    /* x108_ */
    __dna_writer.writeFloatBig(x108_);
    /* x10c_ */
    __dna_writer.writeFloatBig(x10c_);
    /* x110_ */
    __dna_writer.writeFloatBig(x110_);
    /* x114_ */
    __dna_writer.writeFloatBig(x114_);
    /* x118_ */
    __dna_writer.writeFloatBig(x118_);
    /* x11c_ */
    __dna_writer.writeFloatBig(x11c_);
    /* x120_ */
    __dna_writer.writeFloatBig(x120_);
    /* x124_ */
    __dna_writer.writeFloatBig(x124_);
    /* x128_ */
    __dna_writer.writeFloatBig(x128_);
    /* x12c_ */
    __dna_writer.writeFloatBig(x12c_);
    /* x130_ */
    __dna_writer.writeFloatBig(x130_);
    /* x134_ */
    __dna_writer.writeFloatBig(x134_);
    /* x138_ */
    __dna_writer.writeFloatBig(x138_hudLagAmount);
    /* x13c_ */
    __dna_writer.writeFloatBig(x13c_);
    /* x140_ */
    __dna_writer.writeFloatBig(x140_);
    /* x144_ */
    __dna_writer.writeFloatBig(x144_);
    /* x148_ */
    __dna_writer.writeFloatBig(x148_);
    /* x14c_ */
    __dna_writer.writeFloatBig(x14c_);
    /* x150_leftDiv */
    __dna_writer.writeFloatBig(x150_leftDiv);
    /* x154_rightDiv */
    __dna_writer.writeFloatBig(x154_rightDiv);
    /* x228_24_ */
    __dna_writer.writeBool(x228_24_);
    /* x228_25_ */
    __dna_writer.writeBool(x228_25_);
    /* x228_26_ */
    __dna_writer.writeBool(x228_26_);
    /* x228_27_ */
    __dna_writer.writeBool(x228_27_);
    /* x228_28_ */
    __dna_writer.writeBool(x228_28_);
    /* x228_29_ */
    __dna_writer.writeBool(x228_29_);
    /* x228_30_ */
    __dna_writer.writeBool(x228_30_);
    /* x228_31_ */
    __dna_writer.writeBool(x228_31_);
    /* x229_24_ */
    __dna_writer.writeBool(x229_24_);
    /* x229_25_ */
    __dna_writer.writeBool(x229_25_);
    /* x229_26_ */
    __dna_writer.writeBool(x229_26_);
    /* x229_27_ */
    __dna_writer.writeBool(x229_27_);
    /* x229_28_ */
    __dna_writer.writeBool(x229_28_);
    /* x229_29_ */
    __dna_writer.writeBool(x229_29_);
    /* x229_30_ */
    __dna_writer.writeBool(x229_30_);
    /* x229_31_ */
    __dna_writer.writeBool(x229_31_);
    /* x22a_24_ */
    __dna_writer.writeBool(x22a_24_);
    /* x22a_25_ */
    __dna_writer.writeBool(x22a_25_);
    /* x22a_26_ */
    __dna_writer.writeBool(x22a_26_);
    /* x22a_27_ */
    __dna_writer.writeBool(x22a_27_);
    /* x22a_28_ */
    __dna_writer.writeBool(x22a_28_);
    /* x22c_ */
    __dna_writer.writeFloatBig(x22c_);
    /* x230_ */
    __dna_writer.writeFloatBig(x230_);
    /* x234_ */
    __dna_writer.writeFloatBig(x234_);
    /* x238_ */
    __dna_writer.writeFloatBig(x238_);
    /* x23c_ */
    __dna_writer.writeFloatBig(x23c_);
    /* x240_ */
    __dna_writer.writeFloatBig(x240_);
    /* x244_ */
    __dna_writer.writeFloatBig(x244_);
    /* x248_ */
    __dna_writer.writeFloatBig(x248_);
    /* x24c_ */
    __dna_writer.writeFloatBig(x24c_);
    /* x250_ */
    __dna_writer.writeFloatBig(x250_);
    /* x254_ */
    __dna_writer.writeFloatBig(x254_);
    /* x258_ */
    __dna_writer.writeFloatBig(x258_);
    /* x25c_ */
    __dna_writer.writeFloatBig(x25c_);
    /* x260_ */
    __dna_writer.writeFloatBig(x260_);
    /* x264_ */
    __dna_writer.writeFloatBig(x264_);
    /* x268_ */
    __dna_writer.writeFloatBig(x268_);
    /* x158_[0] */
    __dna_writer.writeFloatBig(x158_[0]);
    /* x164_[0] */
    __dna_writer.writeFloatBig(x164_[0]);
    /* x170_[0] */
    __dna_writer.writeFloatBig(x170_[0]);
    /* x158_[1] */
    __dna_writer.writeFloatBig(x158_[1]);
    /* x164_[1] */
    __dna_writer.writeFloatBig(x164_[1]);
    /* x170_[1] */
    __dna_writer.writeFloatBig(x170_[1]);
    /* x158_[2] */
    __dna_writer.writeFloatBig(x158_[2]);
    /* x164_[2] */
    __dna_writer.writeFloatBig(x164_[2]);
    /* x170_[2] */
    __dna_writer.writeFloatBig(x170_[2]);
    /* x17c_ */
    __dna_writer.writeFloatBig(x17c_);
    /* x180_ */
    __dna_writer.writeFloatBig(x180_);
    /* x184_ */
    __dna_writer.writeFloatBig(x184_);
    /* x188_ */
    __dna_writer.writeFloatBig(x188_);
    /* x18c_ */
    __dna_writer.writeFloatBig(x18c_);
    /* x190_ */
    __dna_writer.writeFloatBig(x190_);
    /* x194_ */
    __dna_writer.writeFloatBig(x194_);
    /* x198_ */
    __dna_writer.writeFloatBig(x198_);
    /* x19c_ */
    __dna_writer.writeFloatBig(x19c_);
    /* x1a0_ */
    __dna_writer.writeFloatBig(x1a0_);
    /* x1a4_ */
    __dna_writer.writeFloatBig(x1a4_);
    /* x1a8_[0] */
    __dna_writer.writeUint32Big(x1a8_[0]);
    /* x1b0_[0] */
    __dna_writer.writeUint32Big(x1b0_[0]);
    /* x1b8_[0] */
    __dna_writer.writeUint32Big(x1b8_[0]);
    /* x1c0_[0] */
    __dna_writer.writeUint32Big(x1c0_[0]);
    /* x1c8_[0] */
    __dna_writer.writeUint32Big(x1c8_[0]);
    /* x1d0_[0] */
    __dna_writer.writeUint32Big(x1d0_[0]);
    /* x1a8_[1] */
    __dna_writer.writeUint32Big(x1a8_[1]);
    /* x1b0_[1] */
    __dna_writer.writeUint32Big(x1b0_[1]);
    /* x1b8_[1] */
    __dna_writer.writeUint32Big(x1b8_[1]);
    /* x1c0_[1] */
    __dna_writer.writeUint32Big(x1c0_[1]);
    /* x1c8_[1] */
    __dna_writer.writeUint32Big(x1c8_[1]);
    /* x1d0_[1] */
    __dna_writer.writeUint32Big(x1d0_[1]);
    /* x1d8_ */
    __dna_writer.writeFloatBig(x1d8_);
    /* x1dc_ */
    __dna_writer.writeFloatBig(x1dc_);
    /* x1e0_ */
    __dna_writer.writeFloatBig(x1e0_);
    /* x1e4_ */
    __dna_writer.writeFloatBig(x1e4_);
    /* x1e8_ */
    __dna_writer.writeFloatBig(x1e8_);
    /* x1ec_ */
    __dna_writer.writeFloatBig(x1ec_);
    /* x1f0_ */
    __dna_writer.writeFloatBig(x1f0_);
    /* x1f4_ */
    __dna_writer.writeFloatBig(x1f4_);
    /* x1f8_ */
    __dna_writer.writeFloatBig(x1f8_);
    /* x1fc_ */
    __dna_writer.writeFloatBig(x1fc_);
    /* x200_24_ */
    __dna_writer.writeBool(x200_24_);
    /* x200_25_ */
    __dna_writer.writeBool(x200_25_);
    /* x204_ */
    __dna_writer.writeFloatBig(x204_);
    /* x208_ */
    __dna_writer.writeFloatBig(x208_);
    /* x20c_ */
    __dna_writer.writeFloatBig(x20c_);
    /* x210_ */
    __dna_writer.writeFloatBig(x210_);
    /* x214_ */
    __dna_writer.writeFloatBig(x214_);
    /* x218_ */
    __dna_writer.writeFloatBig(x218_scanningRange);
    /* x21c_24_ */
    __dna_writer.writeBool(x21c_24_);
    /* x21c_25_ */
    __dna_writer.writeBool(x21c_25_scanFreezesGame);
    /* x21c_26_ */
    __dna_writer.writeBool(x21c_26_);
    /* x220_ */
    __dna_writer.writeFloatBig(x220_);
    /* x224_ */
    __dna_writer.writeFloatBig(x224_);
    /* x2a0_ */
    __dna_writer.writeFloatBig(x2a0_);
    /* x2a4_ */
    __dna_writer.writeFloatBig(x2a4_);
    /* x2a8_ */
    __dna_writer.writeFloatBig(x2a8_);
    /* x2ac_ */
    __dna_writer.writeFloatBig(x2ac_);
    /* x2b0_ */
    __dna_writer.writeFloatBig(x2b0_);
    /* x2b4_ */
    __dna_writer.writeFloatBig(x2b4_);
    /* x2b8_ */
    __dna_writer.writeFloatBig(x2b8_);
    /* x2bc_ */
    __dna_writer.writeFloatBig(x2bc_);
    /* x2c0_ */
    __dna_writer.writeFloatBig(x2c0_);
    /* x2c4_ */
    __dna_writer.writeFloatBig(x2c4_);
    /* x2c8_ */
    __dna_writer.writeFloatBig(x2c8_);
    /* x2cc_ */
    __dna_writer.writeFloatBig(x2cc_);
    /* x2d0_ */
    __dna_writer.writeUint32Big(x2d0_);
    /* x2d4_ */
    __dna_writer.writeBool(x2d4_);
    /* x2d5_ */
    __dna_writer.writeBool(x2d5_);
    /* x2d8_ */
    __dna_writer.writeFloatBig(x2d8_);
    /* x2dc_ */
    __dna_writer.writeFloatBig(x2dc_);
    /* x2e0_ */
    __dna_writer.writeFloatBig(x2e0_);
    /* x2e4_ */
    __dna_writer.writeFloatBig(x2e4_);
    /* x26c_playerHeight */
    __dna_writer.writeFloatBig(x26c_playerHeight);
    /* x270_playerXYHalfExtent */
    __dna_writer.writeFloatBig(x270_playerXYHalfExtent);
    /* x274_ */
    __dna_writer.writeFloatBig(x274_);
    /* x278_ */
    __dna_writer.writeFloatBig(x278_);
    /* x27c_playerBallHalfExtent */
    __dna_writer.writeFloatBig(x27c_playerBallHalfExtent);
    /* x280_ */
    __dna_writer.writeFloatBig(x280_);
    /* x284_ */
    __dna_writer.writeFloatBig(x284_);
    /* x288_ */
    __dna_writer.writeFloatBig(x288_);
    /* x28c_ */
    __dna_writer.writeFloatBig(x28c_);
    /* x290_ */
    __dna_writer.writeFloatBig(x290_);
    /* x294_ */
    __dna_writer.writeFloatBig(x294_);
    /* x298_ */
    __dna_writer.writeFloatBig(x298_);
    /* x29c_ */
    __dna_writer.writeFloatBig(x29c_);
    /* x2e8_ */
    __dna_writer.writeFloatBig(x2e8_);
    /* x2ec_ */
    __dna_writer.writeFloatBig(x2ec_);
    /* x2f0_ */
    __dna_writer.writeFloatBig(x2f0_);
    /* x2f4_ */
    __dna_writer.writeBool(x2f4_);
    /* x2f8_ */
    __dna_writer.writeFloatBig(x2f8_);
    /* x2fc_ */
    __dna_writer.writeUint32Big(x2fc_);
    /* x300_ */
    __dna_writer.writeFloatBig(x300_variaDamageReduction);
    /* x304_ */
    __dna_writer.writeFloatBig(x304_gravityDamageReduction);
    /* x308_ */
    __dna_writer.writeFloatBig(x308_phazonDamageReduction);
}

void CTweakPlayer::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* x4_ */
    size_t __x4_Count;
    if (auto v = __dna_docin.enterSubVector("x4_", __x4_Count))
    {
    /* x4_[0] */
    x4_[0] = __dna_docin.readFloat("x4_");
    /* x4_[1] */
    x4_[1] = __dna_docin.readFloat("x4_");
    /* x4_[2] */
    x4_[2] = __dna_docin.readFloat("x4_");
    /* x4_[3] */
    x4_[3] = __dna_docin.readFloat("x4_");
    /* x4_[4] */
    x4_[4] = __dna_docin.readFloat("x4_");
    /* x4_[5] */
    x4_[5] = __dna_docin.readFloat("x4_");
    /* x4_[6] */
    x4_[6] = __dna_docin.readFloat("x4_");
    /* x4_[7] */
    x4_[7] = __dna_docin.readFloat("x4_");
    }
    /* x24_ */
    size_t __x24_Count;
    if (auto v = __dna_docin.enterSubVector("x24_", __x24_Count))
    {
    /* x24_[0] */
    x24_[0] = __dna_docin.readFloat("x24_");
    /* x24_[1] */
    x24_[1] = __dna_docin.readFloat("x24_");
    /* x24_[2] */
    x24_[2] = __dna_docin.readFloat("x24_");
    /* x24_[3] */
    x24_[3] = __dna_docin.readFloat("x24_");
    /* x24_[4] */
    x24_[4] = __dna_docin.readFloat("x24_");
    /* x24_[5] */
    x24_[5] = __dna_docin.readFloat("x24_");
    /* x24_[6] */
    x24_[6] = __dna_docin.readFloat("x24_");
    /* x24_[7] */
    x24_[7] = __dna_docin.readFloat("x24_");
    }
    /* x44_ */
    size_t __x44_Count;
    if (auto v = __dna_docin.enterSubVector("x44_", __x44_Count))
    {
    /* x44_[0] */
    x44_[0] = __dna_docin.readFloat("x44_");
    /* x44_[1] */
    x44_[1] = __dna_docin.readFloat("x44_");
    /* x44_[2] */
    x44_[2] = __dna_docin.readFloat("x44_");
    /* x44_[3] */
    x44_[3] = __dna_docin.readFloat("x44_");
    /* x44_[4] */
    x44_[4] = __dna_docin.readFloat("x44_");
    /* x44_[5] */
    x44_[5] = __dna_docin.readFloat("x44_");
    /* x44_[6] */
    x44_[6] = __dna_docin.readFloat("x44_");
    /* x44_[7] */
    x44_[7] = __dna_docin.readFloat("x44_");
    }
    /* x64_ */
    size_t __x64_Count;
    if (auto v = __dna_docin.enterSubVector("x64_", __x64_Count))
    {
    /* x64_[0] */
    x64_[0] = __dna_docin.readFloat("x64_");
    /* x64_[1] */
    x64_[1] = __dna_docin.readFloat("x64_");
    /* x64_[2] */
    x64_[2] = __dna_docin.readFloat("x64_");
    /* x64_[3] */
    x64_[3] = __dna_docin.readFloat("x64_");
    /* x64_[4] */
    x64_[4] = __dna_docin.readFloat("x64_");
    /* x64_[5] */
    x64_[5] = __dna_docin.readFloat("x64_");
    /* x64_[6] */
    x64_[6] = __dna_docin.readFloat("x64_");
    /* x64_[7] */
    x64_[7] = __dna_docin.readFloat("x64_");
    }
    /* x84_ */
    size_t __x84_Count;
    if (auto v = __dna_docin.enterSubVector("x84_", __x84_Count))
    {
    /* x84_[0] */
    x84_[0] = __dna_docin.readFloat("x84_");
    /* x84_[1] */
    x84_[1] = __dna_docin.readFloat("x84_");
    /* x84_[2] */
    x84_[2] = __dna_docin.readFloat("x84_");
    /* x84_[3] */
    x84_[3] = __dna_docin.readFloat("x84_");
    /* x84_[4] */
    x84_[4] = __dna_docin.readFloat("x84_");
    /* x84_[5] */
    x84_[5] = __dna_docin.readFloat("x84_");
    /* x84_[6] */
    x84_[6] = __dna_docin.readFloat("x84_");
    /* x84_[7] */
    x84_[7] = __dna_docin.readFloat("x84_");
    }
    /* xa4_ */
    size_t __xa4_Count;
    if (auto v = __dna_docin.enterSubVector("xa4_", __xa4_Count))
    {
    /* xa4_[0] */
    xa4_[0] = __dna_docin.readFloat("xa4_");
    /* xa4_[1] */
    xa4_[1] = __dna_docin.readFloat("xa4_");
    /* xa4_[2] */
    xa4_[2] = __dna_docin.readFloat("xa4_");
    /* xa4_[3] */
    xa4_[3] = __dna_docin.readFloat("xa4_");
    /* xa4_[4] */
    xa4_[4] = __dna_docin.readFloat("xa4_");
    /* xa4_[5] */
    xa4_[5] = __dna_docin.readFloat("xa4_");
    /* xa4_[6] */
    xa4_[6] = __dna_docin.readFloat("xa4_");
    /* xa4_[7] */
    xa4_[7] = __dna_docin.readFloat("xa4_");
    }
    /* xc4_ */
    xc4_ = __dna_docin.readFloat("xc4_");
    /* xc8_ */
    xc8_ = __dna_docin.readFloat("xc8_");
    /* xcc_ */
    xcc_ = __dna_docin.readFloat("xcc_");
    /* xd0_ */
    xd0_ = __dna_docin.readFloat("xd0_");
    /* xd4_ */
    xd4_ = __dna_docin.readFloat("xd4_");
    /* xd8_ */
    xd8_ = __dna_docin.readFloat("xd8_");
    /* xdc_ */
    xdc_ = __dna_docin.readFloat("xdc_");
    /* xe0_ */
    xe0_ = __dna_docin.readFloat("xe0_");
    /* xe4_ */
    xe4_ = __dna_docin.readFloat("xe4_");
    /* xe8_ */
    xe8_ = __dna_docin.readFloat("xe8_");
    /* xec_ */
    xec_ = __dna_docin.readFloat("xec_");
    /* xf0_ */
    xf0_ = __dna_docin.readFloat("xf0_");
    /* xf4_ */
    xf4_ = __dna_docin.readFloat("xf4_");
    /* xf8_ */
    xf8_ = __dna_docin.readFloat("xf8_");
    /* xfc_ */
    xfc_ = __dna_docin.readFloat("xfc_");
    /* x100_ */
    x100_ = __dna_docin.readFloat("x100_");
    /* x104_ */
    x104_ = __dna_docin.readFloat("x104_");
    /* x108_ */
    x108_ = __dna_docin.readFloat("x108_");
    /* x10c_ */
    x10c_ = __dna_docin.readFloat("x10c_");
    /* x110_ */
    x110_ = __dna_docin.readFloat("x110_");
    /* x114_ */
    x114_ = __dna_docin.readFloat("x114_");
    /* x118_ */
    x118_ = __dna_docin.readFloat("x118_");
    /* x11c_ */
    x11c_ = __dna_docin.readFloat("x11c_");
    /* x120_ */
    x120_ = __dna_docin.readFloat("x120_");
    /* x124_ */
    x124_ = __dna_docin.readFloat("x124_");
    /* x128_ */
    x128_ = __dna_docin.readFloat("x128_");
    /* x12c_ */
    x12c_ = __dna_docin.readFloat("x12c_");
    /* x130_ */
    x130_ = __dna_docin.readFloat("x130_");
    /* x134_ */
    x134_ = __dna_docin.readFloat("x134_");
    /* x138_hudLagAmount */
    x138_hudLagAmount = __dna_docin.readFloat("x138_hudLagAmount");
    /* x13c_ */
    x13c_ = __dna_docin.readFloat("x13c_");
    /* x140_ */
    x140_ = __dna_docin.readFloat("x140_");
    /* x144_ */
    x144_ = __dna_docin.readFloat("x144_");
    /* x148_ */
    x148_ = __dna_docin.readFloat("x148_");
    /* x14c_ */
    x14c_ = __dna_docin.readFloat("x14c_");
    /* x150_leftDiv */
    x150_leftDiv = __dna_docin.readFloat("x150_leftDiv");
    /* x154_rightDiv */
    x154_rightDiv = __dna_docin.readFloat("x154_rightDiv");
    /* x228_24_ */
    x228_24_ = __dna_docin.readBool("x228_24_");
    /* x228_25_ */
    x228_25_ = __dna_docin.readBool("x228_25_");
    /* x228_26_ */
    x228_26_ = __dna_docin.readBool("x228_26_");
    /* x228_27_ */
    x228_27_ = __dna_docin.readBool("x228_27_");
    /* x228_28_ */
    x228_28_ = __dna_docin.readBool("x228_28_");
    /* x228_29_ */
    x228_29_ = __dna_docin.readBool("x228_29_");
    /* x228_30_ */
    x228_30_ = __dna_docin.readBool("x228_30_");
    /* x228_31_ */
    x228_31_ = __dna_docin.readBool("x228_31_");
    /* x229_24_ */
    x229_24_ = __dna_docin.readBool("x229_24_");
    /* x229_25_ */
    x229_25_ = __dna_docin.readBool("x229_25_");
    /* x229_26_ */
    x229_26_ = __dna_docin.readBool("x229_26_");
    /* x229_27_ */
    x229_27_ = __dna_docin.readBool("x229_27_");
    /* x229_28_ */
    x229_28_ = __dna_docin.readBool("x229_28_");
    /* x229_29_ */
    x229_29_ = __dna_docin.readBool("x229_29_");
    /* x229_30_ */
    x229_30_ = __dna_docin.readBool("x229_30_");
    /* x229_31_ */
    x229_31_ = __dna_docin.readBool("x229_31_");
    /* x22a_24_ */
    x22a_24_ = __dna_docin.readBool("x22a_24_");
    /* x22a_25_ */
    x22a_25_ = __dna_docin.readBool("x22a_25_");
    /* x22a_26_ */
    x22a_26_ = __dna_docin.readBool("x22a_26_");
    /* x22a_27_ */
    x22a_27_ = __dna_docin.readBool("x22a_27_");
    /* x22a_28_ */
    x22a_28_ = __dna_docin.readBool("x22a_28_");
    /* x22c_ */
    x22c_ = __dna_docin.readFloat("x22c_");
    /* x230_ */
    x230_ = __dna_docin.readFloat("x230_");
    /* x234_ */
    x234_ = __dna_docin.readFloat("x234_");
    /* x238_ */
    x238_ = __dna_docin.readFloat("x238_");
    /* x23c_ */
    x23c_ = __dna_docin.readFloat("x23c_");
    /* x240_ */
    x240_ = __dna_docin.readFloat("x240_");
    /* x244_ */
    x244_ = __dna_docin.readFloat("x244_");
    /* x248_ */
    x248_ = __dna_docin.readFloat("x248_");
    /* x24c_ */
    x24c_ = __dna_docin.readFloat("x24c_");
    /* x250_ */
    x250_ = __dna_docin.readFloat("x250_");
    /* x254_ */
    x254_ = __dna_docin.readFloat("x254_");
    /* x258_ */
    x258_ = __dna_docin.readFloat("x258_");
    /* x25c_ */
    x25c_ = __dna_docin.readFloat("x25c_");
    /* x260_ */
    x260_ = __dna_docin.readFloat("x260_");
    /* x264_ */
    x264_ = __dna_docin.readFloat("x264_");
    /* x268_ */
    x268_ = __dna_docin.readFloat("x268_");
    /* x158_ */
    size_t __x158_Count;
    if (auto v = __dna_docin.enterSubVector("x158_", __x158_Count))
    {
    /* x158_[0] */
    x158_[0] = __dna_docin.readFloat("x158_");
    /* x158_[1] */
    x158_[1] = __dna_docin.readFloat("x158_");
    /* x158_[2] */
    x158_[2] = __dna_docin.readFloat("x158_");
    }
    /* x164_ */
    size_t __x164_Count;
    if (auto v = __dna_docin.enterSubVector("x164_", __x164_Count))
    {
    /* x164_[0] */
    x164_[0] = __dna_docin.readFloat("x164_");
    /* x164_[1] */
    x164_[1] = __dna_docin.readFloat("x164_");
    /* x164_[2] */
    x164_[2] = __dna_docin.readFloat("x164_");
    }
    /* x170_ */
    size_t __x170_Count;
    if (auto v = __dna_docin.enterSubVector("x170_", __x170_Count))
    {
    /* x170_[0] */
    x170_[0] = __dna_docin.readFloat("x170_");
    /* x170_[1] */
    x170_[1] = __dna_docin.readFloat("x170_");
    /* x170_[2] */
    x170_[2] = __dna_docin.readFloat("x170_");
    }
    /* x17c_ */
    x17c_ = __dna_docin.readFloat("x17c_");
    /* x180_ */
    x180_ = __dna_docin.readFloat("x180_");
    /* x184_ */
    x184_ = __dna_docin.readFloat("x184_");
    /* x188_ */
    x188_ = __dna_docin.readFloat("x188_");
    /* x18c_ */
    x18c_ = __dna_docin.readFloat("x18c_");
    /* x190_ */
    x190_ = __dna_docin.readFloat("x190_");
    /* x194_ */
    x194_ = __dna_docin.readFloat("x194_");
    /* x198_ */
    x198_ = __dna_docin.readFloat("x198_");
    /* x19c_ */
    x19c_ = __dna_docin.readFloat("x19c_");
    /* x1a0_ */
    x1a0_ = __dna_docin.readFloat("x1a0_");
    /* x1a4_ */
    x1a4_ = __dna_docin.readFloat("x1a4_");
    /* x1a8_ */
    size_t __x1a8_Count;
    if (auto v = __dna_docin.enterSubVector("x1a8_", __x1a8_Count))
    {
    /* x1a8_[0] */
    x1a8_[0] = __dna_docin.readUint32("x1a8_");
    /* x1a8_[1] */
    x1a8_[1] = __dna_docin.readUint32("x1a8_");
    }
    /* x1b0_ */
    size_t __x1b0_Count;
    if (auto v = __dna_docin.enterSubVector("x1b0_", __x1b0_Count))
    {
    /* x1b0_[0] */
    x1b0_[0] = __dna_docin.readUint32("x1b0_");
    /* x1b0_[1] */
    x1b0_[1] = __dna_docin.readUint32("x1b0_");
    }
    /* x1b8_ */
    size_t __x1b8_Count;
    if (auto v = __dna_docin.enterSubVector("x1b8_", __x1b8_Count))
    {
    /* x1b8_[0] */
    x1b8_[0] = __dna_docin.readUint32("x1b8_");
    /* x1b8_[1] */
    x1b8_[1] = __dna_docin.readUint32("x1b8_");
    }
    /* x1c0_ */
    size_t __x1c0_Count;
    if (auto v = __dna_docin.enterSubVector("x1c0_", __x1c0_Count))
    {
    /* x1c0_[0] */
    x1c0_[0] = __dna_docin.readUint32("x1c0_");
    /* x1c0_[1] */
    x1c0_[1] = __dna_docin.readUint32("x1c0_");
    }
    /* x1c8_ */
    size_t __x1c8_Count;
    if (auto v = __dna_docin.enterSubVector("x1c8_", __x1c8_Count))
    {
    /* x1c8_[0] */
    x1c8_[0] = __dna_docin.readUint32("x1c8_");
    /* x1c8_[1] */
    x1c8_[1] = __dna_docin.readUint32("x1c8_");
    }
    /* x1d0_ */
    size_t __x1d0_Count;
    if (auto v = __dna_docin.enterSubVector("x1d0_", __x1d0_Count))
    {
    /* x1d0_[0] */
    x1d0_[0] = __dna_docin.readUint32("x1d0_");
    /* x1d0_[1] */
    x1d0_[1] = __dna_docin.readUint32("x1d0_");
    }
    /* x1d8_ */
    x1d8_ = __dna_docin.readFloat("x1d8_");
    /* x1dc_ */
    x1dc_ = __dna_docin.readFloat("x1dc_");
    /* x1e0_ */
    x1e0_ = __dna_docin.readFloat("x1e0_");
    /* x1e4_ */
    x1e4_ = __dna_docin.readFloat("x1e4_");
    /* x1e8_ */
    x1e8_ = __dna_docin.readFloat("x1e8_");
    /* x1ec_ */
    x1ec_ = __dna_docin.readFloat("x1ec_");
    /* x1f0_ */
    x1f0_ = __dna_docin.readFloat("x1f0_");
    /* x1f4_ */
    x1f4_ = __dna_docin.readFloat("x1f4_");
    /* x1f8_ */
    x1f8_ = __dna_docin.readFloat("x1f8_");
    /* x1fc_ */
    x1fc_ = __dna_docin.readFloat("x1fc_");
    /* x200_24_ */
    x200_24_ = __dna_docin.readBool("x200_24_");
    /* x200_25_ */
    x200_25_ = __dna_docin.readBool("x200_25_");
    /* x204_ */
    x204_ = __dna_docin.readFloat("x204_");
    /* x208_ */
    x208_ = __dna_docin.readFloat("x208_");
    /* x20c_ */
    x20c_ = __dna_docin.readFloat("x20c_");
    /* x210_ */
    x210_ = __dna_docin.readFloat("x210_");
    /* x214_ */
    x214_ = __dna_docin.readFloat("x214_");
    /* x218_scanningRange */
    x218_scanningRange = __dna_docin.readFloat("x218_scanningRange");
    /* x21c_24_ */
    x21c_24_ = __dna_docin.readBool("x21c_24_");
    /* x21c_25_ */
    x21c_25_scanFreezesGame = __dna_docin.readBool("x21c_25_");
    /* x21c_26_ */
    x21c_26_ = __dna_docin.readBool("x21c_26_");
    /* x220_ */
    x220_ = __dna_docin.readFloat("x220_");
    /* x224_ */
    x224_ = __dna_docin.readFloat("x224_");
    /* x2a0_ */
    x2a0_ = __dna_docin.readFloat("x2a0_");
    /* x2a4_ */
    x2a4_ = __dna_docin.readFloat("x2a4_");
    /* x2a8_ */
    x2a8_ = __dna_docin.readFloat("x2a8_");
    /* x2ac_ */
    x2ac_ = __dna_docin.readFloat("x2ac_");
    /* x2b0_ */
    x2b0_ = __dna_docin.readFloat("x2b0_");
    /* x2b4_ */
    x2b4_ = __dna_docin.readFloat("x2b4_");
    /* x2b8_ */
    x2b8_ = __dna_docin.readFloat("x2b8_");
    /* x2bc_ */
    x2bc_ = __dna_docin.readFloat("x2bc_");
    /* x2c0_ */
    x2c0_ = __dna_docin.readFloat("x2c0_");
    /* x2c4_ */
    x2c4_ = __dna_docin.readFloat("x2c4_");
    /* x2c8_ */
    x2c8_ = __dna_docin.readFloat("x2c8_");
    /* x2cc_ */
    x2cc_ = __dna_docin.readFloat("x2cc_");
    /* x2d0_ */
    x2d0_ = __dna_docin.readUint32("x2d0_");
    /* x2d4_ */
    x2d4_ = __dna_docin.readBool("x2d4_");
    /* x2d5_ */
    x2d5_ = __dna_docin.readBool("x2d5_");
    /* x2d8_ */
    x2d8_ = __dna_docin.readFloat("x2d8_");
    /* x2dc_ */
    x2dc_ = __dna_docin.readFloat("x2dc_");
    /* x2e0_ */
    x2e0_ = __dna_docin.readFloat("x2e0_");
    /* x2e4_ */
    x2e4_ = __dna_docin.readFloat("x2e4_");
    /* x26c_playerHeight */
    x26c_playerHeight = __dna_docin.readFloat("x26c_playerHeight");
    /* x270_playerXYHalfExtent */
    x270_playerXYHalfExtent = __dna_docin.readFloat("x270_playerXYHalfExtent");
    /* x274_ */
    x274_ = __dna_docin.readFloat("x274_");
    /* x278_ */
    x278_ = __dna_docin.readFloat("x278_");
    /* x27c_playerBallHalfExtent */
    x27c_playerBallHalfExtent = __dna_docin.readFloat("x27c_playerBallHalfExtent");
    /* x280_ */
    x280_ = __dna_docin.readFloat("x280_");
    /* x284_ */
    x284_ = __dna_docin.readFloat("x284_");
    /* x288_ */
    x288_ = __dna_docin.readFloat("x288_");
    /* x28c_ */
    x28c_ = __dna_docin.readFloat("x28c_");
    /* x290_ */
    x290_ = __dna_docin.readFloat("x290_");
    /* x294_ */
    x294_ = __dna_docin.readFloat("x294_");
    /* x298_ */
    x298_ = __dna_docin.readFloat("x298_");
    /* x29c_ */
    x29c_ = __dna_docin.readFloat("x29c_");
    /* x2e8_ */
    x2e8_ = __dna_docin.readFloat("x2e8_");
    /* x2ec_ */
    x2ec_ = __dna_docin.readFloat("x2ec_");
    /* x2f0_ */
    x2f0_ = __dna_docin.readFloat("x2f0_");
    /* x2f4_ */
    x2f4_ = __dna_docin.readBool("x2f4_");
    /* x2f8_ */
    x2f8_ = __dna_docin.readFloat("x2f8_");
    /* x2fc_ */
    x2fc_ = __dna_docin.readUint32("x2fc_");
    /* x300_variaDamageReduction */
    x300_variaDamageReduction = __dna_docin.readFloat("x300_variaDamageReduction");
    /* x304_gravityDamageReduction */
    x304_gravityDamageReduction = __dna_docin.readFloat("x304_gravityDamageReduction");
    /* x308_phazonDamageReduction */
    x308_phazonDamageReduction = __dna_docin.readFloat("x308_phazonDamageReduction");
}

void CTweakPlayer::CTweakPlayer::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* x4_ */
    if (auto v = __dna_docout.enterSubVector("x4_"))
    {
    /* x4_[0] */
    __dna_docout.writeFloat("x4_", x4_[0]);
    /* x4_[1] */
    __dna_docout.writeFloat("x4_", x4_[1]);
    /* x4_[2] */
    __dna_docout.writeFloat("x4_", x4_[2]);
    /* x4_[3] */
    __dna_docout.writeFloat("x4_", x4_[3]);
    /* x4_[4] */
    __dna_docout.writeFloat("x4_", x4_[4]);
    /* x4_[5] */
    __dna_docout.writeFloat("x4_", x4_[5]);
    /* x4_[6] */
    __dna_docout.writeFloat("x4_", x4_[6]);
    /* x4_[7] */
    __dna_docout.writeFloat("x4_", x4_[7]);
    }
    /* x24_ */
    if (auto v = __dna_docout.enterSubVector("x24_"))
    {
    /* x24_[0] */
    __dna_docout.writeFloat("x24_", x24_[0]);
    /* x24_[1] */
    __dna_docout.writeFloat("x24_", x24_[1]);
    /* x24_[2] */
    __dna_docout.writeFloat("x24_", x24_[2]);
    /* x24_[3] */
    __dna_docout.writeFloat("x24_", x24_[3]);
    /* x24_[4] */
    __dna_docout.writeFloat("x24_", x24_[4]);
    /* x24_[5] */
    __dna_docout.writeFloat("x24_", x24_[5]);
    /* x24_[6] */
    __dna_docout.writeFloat("x24_", x24_[6]);
    /* x24_[7] */
    __dna_docout.writeFloat("x24_", x24_[7]);
    }
    /* x44_ */
    if (auto v = __dna_docout.enterSubVector("x44_"))
    {
    /* x44_[0] */
    __dna_docout.writeFloat("x44_", x44_[0]);
    /* x44_[1] */
    __dna_docout.writeFloat("x44_", x44_[1]);
    /* x44_[2] */
    __dna_docout.writeFloat("x44_", x44_[2]);
    /* x44_[3] */
    __dna_docout.writeFloat("x44_", x44_[3]);
    /* x44_[4] */
    __dna_docout.writeFloat("x44_", x44_[4]);
    /* x44_[5] */
    __dna_docout.writeFloat("x44_", x44_[5]);
    /* x44_[6] */
    __dna_docout.writeFloat("x44_", x44_[6]);
    /* x44_[7] */
    __dna_docout.writeFloat("x44_", x44_[7]);
    }
    /* x64_ */
    if (auto v = __dna_docout.enterSubVector("x64_"))
    {
    /* x64_[0] */
    __dna_docout.writeFloat("x64_", x64_[0]);
    /* x64_[1] */
    __dna_docout.writeFloat("x64_", x64_[1]);
    /* x64_[2] */
    __dna_docout.writeFloat("x64_", x64_[2]);
    /* x64_[3] */
    __dna_docout.writeFloat("x64_", x64_[3]);
    /* x64_[4] */
    __dna_docout.writeFloat("x64_", x64_[4]);
    /* x64_[5] */
    __dna_docout.writeFloat("x64_", x64_[5]);
    /* x64_[6] */
    __dna_docout.writeFloat("x64_", x64_[6]);
    /* x64_[7] */
    __dna_docout.writeFloat("x64_", x64_[7]);
    }
    /* x84_ */
    if (auto v = __dna_docout.enterSubVector("x84_"))
    {
    /* x84_[0] */
    __dna_docout.writeFloat("x84_", x84_[0]);
    /* x84_[1] */
    __dna_docout.writeFloat("x84_", x84_[1]);
    /* x84_[2] */
    __dna_docout.writeFloat("x84_", x84_[2]);
    /* x84_[3] */
    __dna_docout.writeFloat("x84_", x84_[3]);
    /* x84_[4] */
    __dna_docout.writeFloat("x84_", x84_[4]);
    /* x84_[5] */
    __dna_docout.writeFloat("x84_", x84_[5]);
    /* x84_[6] */
    __dna_docout.writeFloat("x84_", x84_[6]);
    /* x84_[7] */
    __dna_docout.writeFloat("x84_", x84_[7]);
    }
    /* xa4_ */
    if (auto v = __dna_docout.enterSubVector("xa4_"))
    {
    /* xa4_[0] */
    __dna_docout.writeFloat("xa4_", xa4_[0]);
    /* xa4_[1] */
    __dna_docout.writeFloat("xa4_", xa4_[1]);
    /* xa4_[2] */
    __dna_docout.writeFloat("xa4_", xa4_[2]);
    /* xa4_[3] */
    __dna_docout.writeFloat("xa4_", xa4_[3]);
    /* xa4_[4] */
    __dna_docout.writeFloat("xa4_", xa4_[4]);
    /* xa4_[5] */
    __dna_docout.writeFloat("xa4_", xa4_[5]);
    /* xa4_[6] */
    __dna_docout.writeFloat("xa4_", xa4_[6]);
    /* xa4_[7] */
    __dna_docout.writeFloat("xa4_", xa4_[7]);
    }
    /* xc4_ */
    __dna_docout.writeFloat("xc4_", xc4_);
    /* xc8_ */
    __dna_docout.writeFloat("xc8_", xc8_);
    /* xcc_ */
    __dna_docout.writeFloat("xcc_", xcc_);
    /* xd0_ */
    __dna_docout.writeFloat("xd0_", xd0_);
    /* xd4_ */
    __dna_docout.writeFloat("xd4_", xd4_);
    /* xd8_ */
    __dna_docout.writeFloat("xd8_", xd8_);
    /* xdc_ */
    __dna_docout.writeFloat("xdc_", xdc_);
    /* xe0_ */
    __dna_docout.writeFloat("xe0_", xe0_);
    /* xe4_ */
    __dna_docout.writeFloat("xe4_", xe4_);
    /* xe8_ */
    __dna_docout.writeFloat("xe8_", xe8_);
    /* xec_ */
    __dna_docout.writeFloat("xec_", xec_);
    /* xf0_ */
    __dna_docout.writeFloat("xf0_", xf0_);
    /* xf4_ */
    __dna_docout.writeFloat("xf4_", xf4_);
    /* xf8_ */
    __dna_docout.writeFloat("xf8_", xf8_);
    /* xfc_ */
    __dna_docout.writeFloat("xfc_", xfc_);
    /* x100_ */
    __dna_docout.writeFloat("x100_", x100_);
    /* x104_ */
    __dna_docout.writeFloat("x104_", x104_);
    /* x108_ */
    __dna_docout.writeFloat("x108_", x108_);
    /* x10c_ */
    __dna_docout.writeFloat("x10c_", x10c_);
    /* x110_ */
    __dna_docout.writeFloat("x110_", x110_);
    /* x114_ */
    __dna_docout.writeFloat("x114_", x114_);
    /* x118_ */
    __dna_docout.writeFloat("x118_", x118_);
    /* x11c_ */
    __dna_docout.writeFloat("x11c_", x11c_);
    /* x120_ */
    __dna_docout.writeFloat("x120_", x120_);
    /* x124_ */
    __dna_docout.writeFloat("x124_", x124_);
    /* x128_ */
    __dna_docout.writeFloat("x128_", x128_);
    /* x12c_ */
    __dna_docout.writeFloat("x12c_", x12c_);
    /* x130_ */
    __dna_docout.writeFloat("x130_", x130_);
    /* x134_ */
    __dna_docout.writeFloat("x134_", x134_);
    /* x138_hudLagAmount */
    __dna_docout.writeFloat("x138_hudLagAmount", x138_hudLagAmount);
    /* x13c_ */
    __dna_docout.writeFloat("x13c_", x13c_);
    /* x140_ */
    __dna_docout.writeFloat("x140_", x140_);
    /* x144_ */
    __dna_docout.writeFloat("x144_", x144_);
    /* x148_ */
    __dna_docout.writeFloat("x148_", x148_);
    /* x14c_ */
    __dna_docout.writeFloat("x14c_", x14c_);
    /* x150_leftDiv */
    __dna_docout.writeFloat("x150_leftDiv", x150_leftDiv);
    /* x154_rightDiv */
    __dna_docout.writeFloat("x154_rightDiv", x154_rightDiv);
    /* x228_24_ */
    __dna_docout.writeBool("x228_24_", x228_24_);
    /* x228_25_ */
    __dna_docout.writeBool("x228_25_", x228_25_);
    /* x228_26_ */
    __dna_docout.writeBool("x228_26_", x228_26_);
    /* x228_27_ */
    __dna_docout.writeBool("x228_27_", x228_27_);
    /* x228_28_ */
    __dna_docout.writeBool("x228_28_", x228_28_);
    /* x228_29_ */
    __dna_docout.writeBool("x228_29_", x228_29_);
    /* x228_30_ */
    __dna_docout.writeBool("x228_30_", x228_30_);
    /* x228_31_ */
    __dna_docout.writeBool("x228_31_", x228_31_);
    /* x229_24_ */
    __dna_docout.writeBool("x229_24_", x229_24_);
    /* x229_25_ */
    __dna_docout.writeBool("x229_25_", x229_25_);
    /* x229_26_ */
    __dna_docout.writeBool("x229_26_", x229_26_);
    /* x229_27_ */
    __dna_docout.writeBool("x229_27_", x229_27_);
    /* x229_28_ */
    __dna_docout.writeBool("x229_28_", x229_28_);
    /* x229_29_ */
    __dna_docout.writeBool("x229_29_", x229_29_);
    /* x229_30_ */
    __dna_docout.writeBool("x229_30_", x229_30_);
    /* x229_31_ */
    __dna_docout.writeBool("x229_31_", x229_31_);
    /* x22a_24_ */
    __dna_docout.writeBool("x22a_24_", x22a_24_);
    /* x22a_25_ */
    __dna_docout.writeBool("x22a_25_", x22a_25_);
    /* x22a_26_ */
    __dna_docout.writeBool("x22a_26_", x22a_26_);
    /* x22a_27_ */
    __dna_docout.writeBool("x22a_27_", x22a_27_);
    /* x22a_28_ */
    __dna_docout.writeBool("x22a_28_", x22a_28_);
    /* x22c_ */
    __dna_docout.writeFloat("x22c_", x22c_);
    /* x230_ */
    __dna_docout.writeFloat("x230_", x230_);
    /* x234_ */
    __dna_docout.writeFloat("x234_", x234_);
    /* x238_ */
    __dna_docout.writeFloat("x238_", x238_);
    /* x23c_ */
    __dna_docout.writeFloat("x23c_", x23c_);
    /* x240_ */
    __dna_docout.writeFloat("x240_", x240_);
    /* x244_ */
    __dna_docout.writeFloat("x244_", x244_);
    /* x248_ */
    __dna_docout.writeFloat("x248_", x248_);
    /* x24c_ */
    __dna_docout.writeFloat("x24c_", x24c_);
    /* x250_ */
    __dna_docout.writeFloat("x250_", x250_);
    /* x254_ */
    __dna_docout.writeFloat("x254_", x254_);
    /* x258_ */
    __dna_docout.writeFloat("x258_", x258_);
    /* x25c_ */
    __dna_docout.writeFloat("x25c_", x25c_);
    /* x260_ */
    __dna_docout.writeFloat("x260_", x260_);
    /* x264_ */
    __dna_docout.writeFloat("x264_", x264_);
    /* x268_ */
    __dna_docout.writeFloat("x268_", x268_);
    /* x158_ */
    if (auto v = __dna_docout.enterSubVector("x158_"))
    {
    /* x158_[0] */
    __dna_docout.writeFloat("x158_", x158_[0]);
    /* x158_[1] */
    __dna_docout.writeFloat("x158_", x158_[1]);
    /* x158_[2] */
    __dna_docout.writeFloat("x158_", x158_[2]);
    }
    /* x164_ */
    if (auto v = __dna_docout.enterSubVector("x164_"))
    {
    /* x164_[0] */
    __dna_docout.writeFloat("x164_", x164_[0]);
    /* x164_[1] */
    __dna_docout.writeFloat("x164_", x164_[1]);
    /* x164_[2] */
    __dna_docout.writeFloat("x164_", x164_[2]);
    }
    /* x170_ */
    if (auto v = __dna_docout.enterSubVector("x170_"))
    {
    /* x170_[0] */
    __dna_docout.writeFloat("x170_", x170_[0]);
    /* x170_[1] */
    __dna_docout.writeFloat("x170_", x170_[1]);
    /* x170_[2] */
    __dna_docout.writeFloat("x170_", x170_[2]);
    }
    /* x17c_ */
    __dna_docout.writeFloat("x17c_", x17c_);
    /* x180_ */
    __dna_docout.writeFloat("x180_", x180_);
    /* x184_ */
    __dna_docout.writeFloat("x184_", x184_);
    /* x188_ */
    __dna_docout.writeFloat("x188_", x188_);
    /* x18c_ */
    __dna_docout.writeFloat("x18c_", x18c_);
    /* x190_ */
    __dna_docout.writeFloat("x190_", x190_);
    /* x194_ */
    __dna_docout.writeFloat("x194_", x194_);
    /* x198_ */
    __dna_docout.writeFloat("x198_", x198_);
    /* x19c_ */
    __dna_docout.writeFloat("x19c_", x19c_);
    /* x1a0_ */
    __dna_docout.writeFloat("x1a0_", x1a0_);
    /* x1a4_ */
    __dna_docout.writeFloat("x1a4_", x1a4_);
    /* x1a8_ */
    if (auto v = __dna_docout.enterSubVector("x1a8_"))
    {
    /* x1a8_[0] */
    __dna_docout.writeUint32("x1a8_", x1a8_[0]);
    /* x1a8_[1] */
    __dna_docout.writeUint32("x1a8_", x1a8_[1]);
    }
    /* x1b0_ */
    if (auto v = __dna_docout.enterSubVector("x1b0_"))
    {
    /* x1b0_[0] */
    __dna_docout.writeUint32("x1b0_", x1b0_[0]);
    /* x1b0_[1] */
    __dna_docout.writeUint32("x1b0_", x1b0_[1]);
    }
    /* x1b8_ */
    if (auto v = __dna_docout.enterSubVector("x1b8_"))
    {
    /* x1b8_[0] */
    __dna_docout.writeUint32("x1b8_", x1b8_[0]);
    /* x1b8_[1] */
    __dna_docout.writeUint32("x1b8_", x1b8_[1]);
    }
    /* x1c0_ */
    if (auto v = __dna_docout.enterSubVector("x1c0_"))
    {
    /* x1c0_[0] */
    __dna_docout.writeUint32("x1c0_", x1c0_[0]);
    /* x1c0_[1] */
    __dna_docout.writeUint32("x1c0_", x1c0_[1]);
    }
    /* x1c8_ */
    if (auto v = __dna_docout.enterSubVector("x1c8_"))
    {
    /* x1c8_[0] */
    __dna_docout.writeUint32("x1c8_", x1c8_[0]);
    /* x1c8_[1] */
    __dna_docout.writeUint32("x1c8_", x1c8_[1]);
    }
    /* x1d0_ */
    if (auto v = __dna_docout.enterSubVector("x1d0_"))
    {
    /* x1d0_[0] */
    __dna_docout.writeUint32("x1d0_", x1d0_[0]);
    /* x1d0_[1] */
    __dna_docout.writeUint32("x1d0_", x1d0_[1]);
    }
    /* x1d8_ */
    __dna_docout.writeFloat("x1d8_", x1d8_);
    /* x1dc_ */
    __dna_docout.writeFloat("x1dc_", x1dc_);
    /* x1e0_ */
    __dna_docout.writeFloat("x1e0_", x1e0_);
    /* x1e4_ */
    __dna_docout.writeFloat("x1e4_", x1e4_);
    /* x1e8_ */
    __dna_docout.writeFloat("x1e8_", x1e8_);
    /* x1ec_ */
    __dna_docout.writeFloat("x1ec_", x1ec_);
    /* x1f0_ */
    __dna_docout.writeFloat("x1f0_", x1f0_);
    /* x1f4_ */
    __dna_docout.writeFloat("x1f4_", x1f4_);
    /* x1f8_ */
    __dna_docout.writeFloat("x1f8_", x1f8_);
    /* x1fc_ */
    __dna_docout.writeFloat("x1fc_", x1fc_);
    /* x200_24_ */
    __dna_docout.writeBool("x200_24_", x200_24_);
    /* x200_25_ */
    __dna_docout.writeBool("x200_25_", x200_25_);
    /* x204_ */
    __dna_docout.writeFloat("x204_", x204_);
    /* x208_ */
    __dna_docout.writeFloat("x208_", x208_);
    /* x20c_ */
    __dna_docout.writeFloat("x20c_", x20c_);
    /* x210_ */
    __dna_docout.writeFloat("x210_", x210_);
    /* x214_ */
    __dna_docout.writeFloat("x214_", x214_);
    /* x218_scanningRange */
    __dna_docout.writeFloat("x218_scanningRange", x218_scanningRange);
    /* x21c_24_ */
    __dna_docout.writeBool("x21c_24_", x21c_24_);
    /* x21c_25_ */
    __dna_docout.writeBool("x21c_25_", x21c_25_scanFreezesGame);
    /* x21c_26_ */
    __dna_docout.writeBool("x21c_26_", x21c_26_);
    /* x220_ */
    __dna_docout.writeFloat("x220_", x220_);
    /* x224_ */
    __dna_docout.writeFloat("x224_", x224_);
    /* x2a0_ */
    __dna_docout.writeFloat("x2a0_", x2a0_);
    /* x2a4_ */
    __dna_docout.writeFloat("x2a4_", x2a4_);
    /* x2a8_ */
    __dna_docout.writeFloat("x2a8_", x2a8_);
    /* x2ac_ */
    __dna_docout.writeFloat("x2ac_", x2ac_);
    /* x2b0_ */
    __dna_docout.writeFloat("x2b0_", x2b0_);
    /* x2b4_ */
    __dna_docout.writeFloat("x2b4_", x2b4_);
    /* x2b8_ */
    __dna_docout.writeFloat("x2b8_", x2b8_);
    /* x2bc_ */
    __dna_docout.writeFloat("x2bc_", x2bc_);
    /* x2c0_ */
    __dna_docout.writeFloat("x2c0_", x2c0_);
    /* x2c4_ */
    __dna_docout.writeFloat("x2c4_", x2c4_);
    /* x2c8_ */
    __dna_docout.writeFloat("x2c8_", x2c8_);
    /* x2cc_ */
    __dna_docout.writeFloat("x2cc_", x2cc_);
    /* x2d0_ */
    __dna_docout.writeUint32("x2d0_", x2d0_);
    /* x2d4_ */
    __dna_docout.writeBool("x2d4_", x2d4_);
    /* x2d4_ */
    __dna_docout.writeBool("x2d5_", x2d5_);
    /* x2d8_ */
    __dna_docout.writeFloat("x2d8_", x2d8_);
    /* x2dc_ */
    __dna_docout.writeFloat("x2dc_", x2dc_);
    /* x2e0_ */
    __dna_docout.writeFloat("x2e0_", x2e0_);
    /* x2e4_ */
    __dna_docout.writeFloat("x2e4_", x2e4_);
    /* x26c_playerHeight */
    __dna_docout.writeFloat("x26c_playerHeight", x26c_playerHeight);
    /* x270_playerXYHalfExtent */
    __dna_docout.writeFloat("x270_playerXYHalfExtent", x270_playerXYHalfExtent);
    /* x274_ */
    __dna_docout.writeFloat("x274_", x274_);
    /* x278_ */
    __dna_docout.writeFloat("x278_", x278_);
    /* x27c_playerBallHalfExtent */
    __dna_docout.writeFloat("x27c_playerBallHalfExtent", x27c_playerBallHalfExtent);
    /* x280_ */
    __dna_docout.writeFloat("x280_", x280_);
    /* x284_ */
    __dna_docout.writeFloat("x284_", x284_);
    /* x288_ */
    __dna_docout.writeFloat("x288_", x288_);
    /* x28c_ */
    __dna_docout.writeFloat("x28c_", x28c_);
    /* x290_ */
    __dna_docout.writeFloat("x290_", x290_);
    /* x294_ */
    __dna_docout.writeFloat("x294_", x294_);
    /* x298_ */
    __dna_docout.writeFloat("x298_", x298_);
    /* x29c_ */
    __dna_docout.writeFloat("x29c_", x29c_);
    /* x2e8_ */
    __dna_docout.writeFloat("x2e8_", x2e8_);
    /* x2ec_ */
    __dna_docout.writeFloat("x2ec_", x2ec_);
    /* x2f0_ */
    __dna_docout.writeFloat("x2f0_", x2f0_);
    /* x2f4_ */
    __dna_docout.writeBool("x2f4_", x2f4_);
    /* x2f8_ */
    __dna_docout.writeFloat("x2f8_", x2f8_);
    /* x2fc_ */
    __dna_docout.writeUint32("x2fc_", x2fc_);
    /* x300_variaDamageReduction */
    __dna_docout.writeFloat("x300_variaDamageReduction", x300_variaDamageReduction);
    /* x304_gravityDamageReduction */
    __dna_docout.writeFloat("x304_gravityDamageReduction", x304_gravityDamageReduction);
    /* x308_phazonDamageReduction */
    __dna_docout.writeFloat("x308_phazonDamageReduction", x308_phazonDamageReduction);
}

void CTweakPlayer::FixupValues()
{
    x130_ = zeus::degToRad(x130_);
    x134_ = zeus::degToRad(x134_);
    x138_hudLagAmount = zeus::degToRad(x138_hudLagAmount);
    x13c_ = zeus::degToRad(x13c_);
    x140_ = zeus::degToRad(x140_);
    x144_ = zeus::degToRad(x144_);
    x23c_ = zeus::degToRad(x23c_);
    x240_ = zeus::degToRad(x240_);
    x244_ = zeus::degToRad(x244_);
    x248_ = zeus::degToRad(x248_);
    x250_ = zeus::degToRad(x250_);
    x264_ = zeus::degToRad(x264_);
    x268_ = zeus::degToRad(x268_);
    x17c_ = zeus::degToRad(x17c_);
    x184_ = zeus::degToRad(x184_);
    x188_ = zeus::degToRad(x188_);
    x18c_ = zeus::degToRad(x18c_);
    x190_ = zeus::degToRad(x190_);
    x194_ = zeus::degToRad(x194_);
    x198_ = zeus::degToRad(x198_);
    x1f0_ = zeus::degToRad(x1f0_);
    x1f4_ = zeus::degToRad(x1f4_);
    x2b0_ = zeus::degToRad(x2b0_);
    x2c0_ = zeus::degToRad(x2c0_);
    x280_ = zeus::degToRad(x280_);
    x284_ = zeus::degToRad(x284_);
    x290_ = zeus::degToRad(x290_);
    x29c_ = zeus::degToRad(x29c_);
}

const char* CTweakPlayer::DNAType()
{
    return "DataSpec::DNAMP1::CTweakPlayer";
}

size_t CTweakPlayer::binarySize(size_t __isz) const
{
    return __isz + 785;
}
}
}
