#include "CTweakBall.hpp"

namespace DataSpec
{
namespace DNAMP1
{

void CTweakBall::read(athena::io::IStreamReader& __dna_reader)
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
    /* x64_ */
    x64_ = __dna_reader.readFloatBig();
    /* x68_ */
    x68_ = __dna_reader.readFloatBig();
    /* x6c_ */
    x6c_ = __dna_reader.readFloatBig();
    /* x70_ */
    x70_ = __dna_reader.readFloatBig();
    /* xc4_[0] */
    xc4_[0] = __dna_reader.readFloatBig();
    /* xc4_[1] */
    xc4_[1] = __dna_reader.readFloatBig();
    /* xc4_[2] */
    xc4_[2] = __dna_reader.readFloatBig();
    /* xc4_[3] */
    xc4_[3] = __dna_reader.readFloatBig();
    /* xc4_[4] */
    xc4_[4] = __dna_reader.readFloatBig();
    /* xc4_[5] */
    xc4_[5] = __dna_reader.readFloatBig();
    /* xc4_[6] */
    xc4_[6] = __dna_reader.readFloatBig();
    /* xc4_[7] */
    xc4_[7] = __dna_reader.readFloatBig();
    /* xe4_ */
    xe4_ = __dna_reader.readFloatBig();
    /* xe8_ */
    xe8_ = __dna_reader.readFloatBig();
    /* x14c_ */
    x14c_ = __dna_reader.readFloatBig();
    /* x150_ */
    x150_ = __dna_reader.readFloatBig();
    /* x158_ */
    x158_ = __dna_reader.readFloatBig();
    /* x1dc_ */
    x1dc_ = __dna_reader.readFloatBig();
    /* x1e0_ */
    x1e0_ = __dna_reader.readFloatBig();
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
    /* x74_ */
    x74_ = __dna_reader.readFloatBig();
    /* x78_ */
    x78_ = __dna_reader.readVec3fBig();
    /* x84_ */
    x84_ = __dna_reader.readFloatBig();
    /* x88_ */
    x88_ = __dna_reader.readFloatBig();
    /* x8c_ */
    x8c_ = __dna_reader.readFloatBig();
    /* x90_ */
    x90_ = __dna_reader.readFloatBig();
    /* x94_ */
    x94_ = __dna_reader.readFloatBig();
    /* x98_ */
    x98_ = __dna_reader.readFloatBig();
    /* x9c_ */
    x9c_ = __dna_reader.readFloatBig();
    /* xa0_ */
    xa0_ = __dna_reader.readFloatBig();
    /* xa4_ */
    xa4_ = __dna_reader.readFloatBig();
    /* xa8_ */
    xa8_ = __dna_reader.readFloatBig();
    /* xac_ */
    xac_ = __dna_reader.readFloatBig();
    /* xb0_ */
    xb0_ = __dna_reader.readFloatBig();
    /* xb4_ */
    xb4_ = __dna_reader.readFloatBig();
    /* xb8_ */
    xb8_ = __dna_reader.readFloatBig();
    /* xbc_ */
    xbc_ = __dna_reader.readFloatBig();
    /* xc0_ */
    xc0_ = __dna_reader.readFloatBig();
    /* x154_ */
    x154_ = __dna_reader.readFloatBig();
    /* x15c_ */
    x15c_ = __dna_reader.readFloatBig();
    /* x160_ */
    x160_ = __dna_reader.readFloatBig();
    /* x164_ */
    x164_ = __dna_reader.readFloatBig();
    /* x168_ */
    x168_ = __dna_reader.readFloatBig();
    /* x16c_ */
    x16c_ = __dna_reader.readFloatBig();
    /* x170_ */
    x170_ = __dna_reader.readFloatBig();
    /* x174_ */
    x174_ = __dna_reader.readFloatBig();
    /* x178_ */
    x178_ = __dna_reader.readFloatBig();
    /* x17c_ */
    x17c_ = __dna_reader.readFloatBig();
    /* x180_ */
    x180_ = __dna_reader.readFloatBig();
    /* x184_ */
    x184_ = __dna_reader.readFloatBig();
    /* x188_ */
    x188_ = __dna_reader.readFloatBig();
    /* x18c_ */
    x18c_ = __dna_reader.readVec3fBig();
    /* x198_ */
    x198_ = __dna_reader.readFloatBig();
    /* x19c_ */
    x19c_ = __dna_reader.readFloatBig();
    /* x1a0_ */
    x1a0_ = __dna_reader.readFloatBig();
    /* x1a4_ */
    x1a4_ = __dna_reader.readFloatBig();
    /* x1a8_ */
    x1a8_ = __dna_reader.readFloatBig();
    /* x1ac_ */
    x1ac_ = __dna_reader.readFloatBig();
    /* x1b0_ */
    x1b0_ = __dna_reader.readFloatBig();
    /* x1b4_ */
    x1b4_ = __dna_reader.readFloatBig();
    /* x1b8_ */
    x1b8_ = __dna_reader.readVec3fBig();
    /* x1c4_ */
    x1c4_ = __dna_reader.readFloatBig();
    /* x1c8_ */
    x1c8_ = __dna_reader.readFloatBig();
    /* x1cc_ */
    x1cc_ = __dna_reader.readFloatBig();
    /* x1d0_ */
    x1d0_ = __dna_reader.readFloatBig();
    /* x1d4_ */
    x1d4_ = __dna_reader.readFloatBig();
    /* x1d8_ */
    x1d8_ = __dna_reader.readFloatBig();
    /* x1e4_ */
    x1e4_ = __dna_reader.readFloatBig();
    /* x1e8_ */
    x1e8_ = __dna_reader.readFloatBig();
    /* x200_ */
    x200_ = __dna_reader.readFloatBig();
    /* x204_ */
    x204_ = __dna_reader.readFloatBig();
    /* x20c_ */
    x20c_ = __dna_reader.readFloatBig();
    /* x218_ */
    x218_ = __dna_reader.readFloatBig();
    /* x21c_ */
    x21c_ = __dna_reader.readFloatBig();
    /* x220_ */
    x220_ = __dna_reader.readFloatBig();
    /* x224_ */
    x224_ = __dna_reader.readFloatBig();
    /* x210_ */
    x210_ = __dna_reader.readFloatBig();
    /* x22c_ */
    x22c_ = __dna_reader.readFloatBig();
    /* x230_ */
    x230_ = __dna_reader.readFloatBig();
    /* x234_ */
    x234_ = __dna_reader.readFloatBig();
}

void CTweakBall::write(athena::io::IStreamWriter& __dna_writer) const
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
    /* x64_ */
    __dna_writer.writeFloatBig(x64_);
    /* x68_ */
    __dna_writer.writeFloatBig(x68_);
    /* x6c_ */
    __dna_writer.writeFloatBig(x6c_);
    /* x70_ */
    __dna_writer.writeFloatBig(x70_);
    /* xc4_[0] */
    __dna_writer.writeFloatBig(xc4_[0]);
    /* xc4_[1] */
    __dna_writer.writeFloatBig(xc4_[1]);
    /* xc4_[2] */
    __dna_writer.writeFloatBig(xc4_[2]);
    /* xc4_[3] */
    __dna_writer.writeFloatBig(xc4_[3]);
    /* xc4_[4] */
    __dna_writer.writeFloatBig(xc4_[4]);
    /* xc4_[5] */
    __dna_writer.writeFloatBig(xc4_[5]);
    /* xc4_[6] */
    __dna_writer.writeFloatBig(xc4_[6]);
    /* xc4_[7] */
    __dna_writer.writeFloatBig(xc4_[7]);
    /* xe4_ */
    __dna_writer.writeFloatBig(xe4_);
    /* xe8_ */
    __dna_writer.writeFloatBig(xe8_);
    /* x14c_ */
    __dna_writer.writeFloatBig(x14c_);
    /* x150_ */
    __dna_writer.writeFloatBig(x150_);
    /* x158_ */
    __dna_writer.writeFloatBig(x158_);
    /* x1dc_ */
    __dna_writer.writeFloatBig(x1dc_);
    /* x1e0_ */
    __dna_writer.writeFloatBig(x1e0_);
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
    /* x74_ */
    __dna_writer.writeFloatBig(x74_);
    /* x78_ */
    __dna_writer.writeVec3fBig(x78_);
    /* x84_ */
    __dna_writer.writeFloatBig(x84_);
    /* x88_ */
    __dna_writer.writeFloatBig(x88_);
    /* x8c_ */
    __dna_writer.writeFloatBig(x8c_);
    /* x90_ */
    __dna_writer.writeFloatBig(x90_);
    /* x94_ */
    __dna_writer.writeFloatBig(x94_);
    /* x98_ */
    __dna_writer.writeFloatBig(x98_);
    /* x9c_ */
    __dna_writer.writeFloatBig(x9c_);
    /* xa0_ */
    __dna_writer.writeFloatBig(xa0_);
    /* xa4_ */
    __dna_writer.writeFloatBig(xa4_);
    /* xa8_ */
    __dna_writer.writeFloatBig(xa8_);
    /* xac_ */
    __dna_writer.writeFloatBig(xac_);
    /* xb0_ */
    __dna_writer.writeFloatBig(xb0_);
    /* xb4_ */
    __dna_writer.writeFloatBig(xb4_);
    /* xb8_ */
    __dna_writer.writeFloatBig(xb8_);
    /* xbc_ */
    __dna_writer.writeFloatBig(xbc_);
    /* xc0_ */
    __dna_writer.writeFloatBig(xc0_);
    /* x154_ */
    __dna_writer.writeFloatBig(x154_);
    /* x15c_ */
    __dna_writer.writeFloatBig(x15c_);
    /* x160_ */
    __dna_writer.writeFloatBig(x160_);
    /* x164_ */
    __dna_writer.writeFloatBig(x164_);
    /* x168_ */
    __dna_writer.writeFloatBig(x168_);
    /* x16c_ */
    __dna_writer.writeFloatBig(x16c_);
    /* x170_ */
    __dna_writer.writeFloatBig(x170_);
    /* x174_ */
    __dna_writer.writeFloatBig(x174_);
    /* x178_ */
    __dna_writer.writeFloatBig(x178_);
    /* x17c_ */
    __dna_writer.writeFloatBig(x17c_);
    /* x180_ */
    __dna_writer.writeFloatBig(x180_);
    /* x184_ */
    __dna_writer.writeFloatBig(x184_);
    /* x188_ */
    __dna_writer.writeFloatBig(x188_);
    /* x18c_ */
    __dna_writer.writeVec3fBig(x18c_);
    /* x198_ */
    __dna_writer.writeFloatBig(x198_);
    /* x19c_ */
    __dna_writer.writeFloatBig(x19c_);
    /* x1a0_ */
    __dna_writer.writeFloatBig(x1a0_);
    /* x1a4_ */
    __dna_writer.writeFloatBig(x1a4_);
    /* x1a8_ */
    __dna_writer.writeFloatBig(x1a8_);
    /* x1ac_ */
    __dna_writer.writeFloatBig(x1ac_);
    /* x1b0_ */
    __dna_writer.writeFloatBig(x1b0_);
    /* x1b4_ */
    __dna_writer.writeFloatBig(x1b4_);
    /* x1b8_ */
    __dna_writer.writeVec3fBig(x1b8_);
    /* x1c4_ */
    __dna_writer.writeFloatBig(x1c4_);
    /* x1c8_ */
    __dna_writer.writeFloatBig(x1c8_);
    /* x1cc_ */
    __dna_writer.writeFloatBig(x1cc_);
    /* x1d0_ */
    __dna_writer.writeFloatBig(x1d0_);
    /* x1d4_ */
    __dna_writer.writeFloatBig(x1d4_);
    /* x1d8_ */
    __dna_writer.writeFloatBig(x1d8_);
    /* x1e4_ */
    __dna_writer.writeFloatBig(x1e4_);
    /* x1e8_ */
    __dna_writer.writeFloatBig(x1e8_);
    /* x200_ */
    __dna_writer.writeFloatBig(x200_);
    /* x204_ */
    __dna_writer.writeFloatBig(x204_);
    /* x20c_ */
    __dna_writer.writeFloatBig(x20c_);
    /* x218_ */
    __dna_writer.writeFloatBig(x218_);
    /* x21c_ */
    __dna_writer.writeFloatBig(x21c_);
    /* x220_ */
    __dna_writer.writeFloatBig(x220_);
    /* x224_ */
    __dna_writer.writeFloatBig(x224_);
    /* x210_ */
    __dna_writer.writeFloatBig(x210_);
    /* x22c_ */
    __dna_writer.writeFloatBig(x22c_);
    /* x230_ */
    __dna_writer.writeFloatBig(x230_);
    /* x234_ */
    __dna_writer.writeFloatBig(x234_);
}

void CTweakBall::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* x4_ */
    size_t __x4_Count;
    __dna_docin.enterSubVector("x4_", __x4_Count);
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
    /*  */
    __dna_docin.leaveSubVector();
    /* x24_ */
    size_t __x24_Count;
    __dna_docin.enterSubVector("x24_", __x24_Count);
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
    /*  */
    __dna_docin.leaveSubVector();
    /* x44_ */
    size_t __x44_Count;
    __dna_docin.enterSubVector("x44_", __x44_Count);
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
    /*  */
    __dna_docin.leaveSubVector();
    /* x64_ */
    x64_ = __dna_docin.readFloat("x64_");
    /* x68_ */
    x68_ = __dna_docin.readFloat("x68_");
    /* x6c_ */
    x6c_ = __dna_docin.readFloat("x6c_");
    /* x70_ */
    x70_ = __dna_docin.readFloat("x70_");
    /* xc4_ */
    size_t __xc4_Count;
    __dna_docin.enterSubVector("xc4_", __xc4_Count);
    /* xc4_[0] */
    xc4_[0] = __dna_docin.readFloat("xc4_");
    /* xc4_[1] */
    xc4_[1] = __dna_docin.readFloat("xc4_");
    /* xc4_[2] */
    xc4_[2] = __dna_docin.readFloat("xc4_");
    /* xc4_[3] */
    xc4_[3] = __dna_docin.readFloat("xc4_");
    /* xc4_[4] */
    xc4_[4] = __dna_docin.readFloat("xc4_");
    /* xc4_[5] */
    xc4_[5] = __dna_docin.readFloat("xc4_");
    /* xc4_[6] */
    xc4_[6] = __dna_docin.readFloat("xc4_");
    /* xc4_[7] */
    xc4_[7] = __dna_docin.readFloat("xc4_");
    /*  */
    __dna_docin.leaveSubVector();
    /* xe4_ */
    xe4_ = __dna_docin.readFloat("xe4_");
    /* xe8_ */
    xe8_ = __dna_docin.readFloat("xe8_");
    /* x14c_ */
    x14c_ = __dna_docin.readFloat("x14c_");
    /* x150_ */
    x150_ = __dna_docin.readFloat("x150_");
    /* x158_ */
    x158_ = __dna_docin.readFloat("x158_");
    /* x1dc_ */
    x1dc_ = __dna_docin.readFloat("x1dc_");
    /* x1e0_ */
    x1e0_ = __dna_docin.readFloat("x1e0_");
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
    /* x74_ */
    x74_ = __dna_docin.readFloat("x74_");
    /* x78_ */
    x78_ = __dna_docin.readVec3f("x78_");
    /* x84_ */
    x84_ = __dna_docin.readFloat("x84_");
    /* x88_ */
    x88_ = __dna_docin.readFloat("x88_");
    /* x8c_ */
    x8c_ = __dna_docin.readFloat("x8c_");
    /* x90_ */
    x90_ = __dna_docin.readFloat("x90_");
    /* x94_ */
    x94_ = __dna_docin.readFloat("x94_");
    /* x98_ */
    x98_ = __dna_docin.readFloat("x98_");
    /* x9c_ */
    x9c_ = __dna_docin.readFloat("x9c_");
    /* xa0_ */
    xa0_ = __dna_docin.readFloat("xa0_");
    /* xa4_ */
    xa4_ = __dna_docin.readFloat("xa4_");
    /* xa8_ */
    xa8_ = __dna_docin.readFloat("xa8_");
    /* xac_ */
    xac_ = __dna_docin.readFloat("xac_");
    /* xb0_ */
    xb0_ = __dna_docin.readFloat("xb0_");
    /* xb4_ */
    xb4_ = __dna_docin.readFloat("xb4_");
    /* xb8_ */
    xb8_ = __dna_docin.readFloat("xb8_");
    /* xbc_ */
    xbc_ = __dna_docin.readFloat("xbc_");
    /* xc0_ */
    xc0_ = __dna_docin.readFloat("xc0_");
    /* x154_ */
    x154_ = __dna_docin.readFloat("x154_");
    /* x15c_ */
    x15c_ = __dna_docin.readFloat("x15c_");
    /* x160_ */
    x160_ = __dna_docin.readFloat("x160_");
    /* x164_ */
    x164_ = __dna_docin.readFloat("x164_");
    /* x168_ */
    x168_ = __dna_docin.readFloat("x168_");
    /* x16c_ */
    x16c_ = __dna_docin.readFloat("x16c_");
    /* x170_ */
    x170_ = __dna_docin.readFloat("x170_");
    /* x174_ */
    x174_ = __dna_docin.readFloat("x174_");
    /* x178_ */
    x178_ = __dna_docin.readFloat("x178_");
    /* x17c_ */
    x17c_ = __dna_docin.readFloat("x17c_");
    /* x180_ */
    x180_ = __dna_docin.readFloat("x180_");
    /* x184_ */
    x184_ = __dna_docin.readFloat("x184_");
    /* x188_ */
    x188_ = __dna_docin.readFloat("x188_");
    /* x18c_ */
    x18c_ = __dna_docin.readVec3f("x18c_");
    /* x198_ */
    x198_ = __dna_docin.readFloat("x198_");
    /* x19c_ */
    x19c_ = __dna_docin.readFloat("x19c_");
    /* x1a0_ */
    x1a0_ = __dna_docin.readFloat("x1a0_");
    /* x1a4_ */
    x1a4_ = __dna_docin.readFloat("x1a4_");
    /* x1a8_ */
    x1a8_ = __dna_docin.readFloat("x1a8_");
    /* x1ac_ */
    x1ac_ = __dna_docin.readFloat("x1ac_");
    /* x1b0_ */
    x1b0_ = __dna_docin.readFloat("x1b0_");
    /* x1b4_ */
    x1b4_ = __dna_docin.readFloat("x1b4_");
    /* x1b8_ */
    x1b8_ = __dna_docin.readVec3f("x1b8_");
    /* x1c4_ */
    x1c4_ = __dna_docin.readFloat("x1c4_");
    /* x1c8_ */
    x1c8_ = __dna_docin.readFloat("x1c8_");
    /* x1cc_ */
    x1cc_ = __dna_docin.readFloat("x1cc_");
    /* x1d0_ */
    x1d0_ = __dna_docin.readFloat("x1d0_");
    /* x1d4_ */
    x1d4_ = __dna_docin.readFloat("x1d4_");
    /* x1d8_ */
    x1d8_ = __dna_docin.readFloat("x1d8_");
    /* x1e4_ */
    x1e4_ = __dna_docin.readFloat("x1e4_");
    /* x1e8_ */
    x1e8_ = __dna_docin.readFloat("x1e8_");
    /* x200_ */
    x200_ = __dna_docin.readFloat("x200_");
    /* x204_ */
    x204_ = __dna_docin.readFloat("x204_");
    /* x20c_ */
    x20c_ = __dna_docin.readFloat("x20c_");
    /* x218_ */
    x218_ = __dna_docin.readFloat("x218_");
    /* x21c_ */
    x21c_ = __dna_docin.readFloat("x21c_");
    /* x220_ */
    x220_ = __dna_docin.readFloat("x220_");
    /* x224_ */
    x224_ = __dna_docin.readFloat("x224_");
    /* x210_ */
    x210_ = __dna_docin.readFloat("x210_");
    /* x22c_ */
    x22c_ = __dna_docin.readFloat("x22c_");
    /* x230_ */
    x230_ = __dna_docin.readFloat("x230_");
    /* x234_ */
    x234_ = __dna_docin.readFloat("x234_");
}

void CTweakBall::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* x4_ */
    __dna_docout.enterSubVector("x4_");
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
    /*  */
    __dna_docout.leaveSubVector();
    /* x24_ */
    __dna_docout.enterSubVector("x24_");
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
    /*  */
    __dna_docout.leaveSubVector();
    /* x44_ */
    __dna_docout.enterSubVector("x44_");
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
    /*  */
    __dna_docout.leaveSubVector();
    /* x64_ */
    __dna_docout.writeFloat("x64_", x64_);
    /* x68_ */
    __dna_docout.writeFloat("x68_", x68_);
    /* x6c_ */
    __dna_docout.writeFloat("x6c_", x6c_);
    /* x70_ */
    __dna_docout.writeFloat("x70_", x70_);
    /* xc4_ */
    __dna_docout.enterSubVector("xc4_");
    /* xc4_[0] */
    __dna_docout.writeFloat("xc4_", xc4_[0]);
    /* xc4_[1] */
    __dna_docout.writeFloat("xc4_", xc4_[1]);
    /* xc4_[2] */
    __dna_docout.writeFloat("xc4_", xc4_[2]);
    /* xc4_[3] */
    __dna_docout.writeFloat("xc4_", xc4_[3]);
    /* xc4_[4] */
    __dna_docout.writeFloat("xc4_", xc4_[4]);
    /* xc4_[5] */
    __dna_docout.writeFloat("xc4_", xc4_[5]);
    /* xc4_[6] */
    __dna_docout.writeFloat("xc4_", xc4_[6]);
    /* xc4_[7] */
    __dna_docout.writeFloat("xc4_", xc4_[7]);
    /*  */
    __dna_docout.leaveSubVector();
    /* xe4_ */
    __dna_docout.writeFloat("xe4_", xe4_);
    /* xe8_ */
    __dna_docout.writeFloat("xe8_", xe8_);
    /* x14c_ */
    __dna_docout.writeFloat("x14c_", x14c_);
    /* x150_ */
    __dna_docout.writeFloat("x150_", x150_);
    /* x158_ */
    __dna_docout.writeFloat("x158_", x158_);
    /* x1dc_ */
    __dna_docout.writeFloat("x1dc_", x1dc_);
    /* x1e0_ */
    __dna_docout.writeFloat("x1e0_", x1e0_);
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
    /* x74_ */
    __dna_docout.writeFloat("x74_", x74_);
    /* x78_ */
    __dna_docout.writeVec3f("x78_", x78_);
    /* x84_ */
    __dna_docout.writeFloat("x84_", x84_);
    /* x88_ */
    __dna_docout.writeFloat("x88_", x88_);
    /* x8c_ */
    __dna_docout.writeFloat("x8c_", x8c_);
    /* x90_ */
    __dna_docout.writeFloat("x90_", x90_);
    /* x94_ */
    __dna_docout.writeFloat("x94_", x94_);
    /* x98_ */
    __dna_docout.writeFloat("x98_", x98_);
    /* x9c_ */
    __dna_docout.writeFloat("x9c_", x9c_);
    /* xa0_ */
    __dna_docout.writeFloat("xa0_", xa0_);
    /* xa4_ */
    __dna_docout.writeFloat("xa4_", xa4_);
    /* xa8_ */
    __dna_docout.writeFloat("xa8_", xa8_);
    /* xac_ */
    __dna_docout.writeFloat("xac_", xac_);
    /* xb0_ */
    __dna_docout.writeFloat("xb0_", xb0_);
    /* xb4_ */
    __dna_docout.writeFloat("xb4_", xb4_);
    /* xb8_ */
    __dna_docout.writeFloat("xb8_", xb8_);
    /* xbc_ */
    __dna_docout.writeFloat("xbc_", xbc_);
    /* xc0_ */
    __dna_docout.writeFloat("xc0_", xc0_);
    /* x154_ */
    __dna_docout.writeFloat("x154_", x154_);
    /* x15c_ */
    __dna_docout.writeFloat("x15c_", x15c_);
    /* x160_ */
    __dna_docout.writeFloat("x160_", x160_);
    /* x164_ */
    __dna_docout.writeFloat("x164_", x164_);
    /* x168_ */
    __dna_docout.writeFloat("x168_", x168_);
    /* x16c_ */
    __dna_docout.writeFloat("x16c_", x16c_);
    /* x170_ */
    __dna_docout.writeFloat("x170_", x170_);
    /* x174_ */
    __dna_docout.writeFloat("x174_", x174_);
    /* x178_ */
    __dna_docout.writeFloat("x178_", x178_);
    /* x17c_ */
    __dna_docout.writeFloat("x17c_", x17c_);
    /* x180_ */
    __dna_docout.writeFloat("x180_", x180_);
    /* x184_ */
    __dna_docout.writeFloat("x184_", x184_);
    /* x188_ */
    __dna_docout.writeFloat("x188_", x188_);
    /* x18c_ */
    __dna_docout.writeVec3f("x18c_", x18c_);
    /* x198_ */
    __dna_docout.writeFloat("x198_", x198_);
    /* x19c_ */
    __dna_docout.writeFloat("x19c_", x19c_);
    /* x1a0_ */
    __dna_docout.writeFloat("x1a0_", x1a0_);
    /* x1a4_ */
    __dna_docout.writeFloat("x1a4_", x1a4_);
    /* x1a8_ */
    __dna_docout.writeFloat("x1a8_", x1a8_);
    /* x1ac_ */
    __dna_docout.writeFloat("x1ac_", x1ac_);
    /* x1b0_ */
    __dna_docout.writeFloat("x1b0_", x1b0_);
    /* x1b4_ */
    __dna_docout.writeFloat("x1b4_", x1b4_);
    /* x1b8_ */
    __dna_docout.writeVec3f("x1b8_", x1b8_);
    /* x1c4_ */
    __dna_docout.writeFloat("x1c4_", x1c4_);
    /* x1c8_ */
    __dna_docout.writeFloat("x1c8_", x1c8_);
    /* x1cc_ */
    __dna_docout.writeFloat("x1cc_", x1cc_);
    /* x1d0_ */
    __dna_docout.writeFloat("x1d0_", x1d0_);
    /* x1d4_ */
    __dna_docout.writeFloat("x1d4_", x1d4_);
    /* x1d8_ */
    __dna_docout.writeFloat("x1d8_", x1d8_);
    /* x1e4_ */
    __dna_docout.writeFloat("x1e4_", x1e4_);
    /* x1e8_ */
    __dna_docout.writeFloat("x1e8_", x1e8_);
    /* x200_ */
    __dna_docout.writeFloat("x200_", x200_);
    /* x204_ */
    __dna_docout.writeFloat("x204_", x204_);
    /* x20c_ */
    __dna_docout.writeFloat("x20c_", x20c_);
    /* x218_ */
    __dna_docout.writeFloat("x218_", x218_);
    /* x21c_ */
    __dna_docout.writeFloat("x21c_", x21c_);
    /* x220_ */
    __dna_docout.writeFloat("x220_", x220_);
    /* x224_ */
    __dna_docout.writeFloat("x224_", x224_);
    /* x210_ */
    __dna_docout.writeFloat("x210_", x210_);
    /* x22c_ */
    __dna_docout.writeFloat("x22c_", x22c_);
    /* x230_ */
    __dna_docout.writeFloat("x230_", x230_);
    /* x234_ */
    __dna_docout.writeFloat("x234_", x234_);
}

const char* CTweakBall::DNAType()
{
    return "DataSpec::DNAMP1::CTweakBall";
}

size_t CTweakBall::binarySize(size_t __isz) const
{
    return __isz + 456;
}

}
}
