#ifndef __RETRO_CGRAPHICS_HPP__
#define __RETRO_CGRAPHICS_HPP__

#include "RetroTypes.hpp"
#include "CTransform.hpp"

namespace Retro
{

enum class ERglLight : u8
{
    None  = 0,
    Zero  = 1,
    One   = 1 << 1,
    Two   = 1 << 2,
    Three = 1 << 3,
    Four  = 1 << 4,
    Five  = 1 << 5,
    Six   = 1 << 6,
    Seven = 1 << 7
};
ENABLE_BITWISE_ENUM(ERglLight)

enum class ERglEnum
{
    Never   = 0,
    Less    = 1,
    Equal   = 2,
    LEqual  = 3,
    Greater = 4,
    NEqual  = 5,
    GEqual  = 6,
    Always  = 7
};

enum class ERglBlendMode
{
    None     = 0,
    Blend    = 1,
    Logic    = 2,
    Subtract = 3
};

enum class ERglBlendFactor
{
    Zero        = 0,
    One         = 1,
    SrcColor    = 2,
    InvSrcColor = 3,
    SrcAlpha    = 4,
    InvSrcAlpha = 5,
    DstAlpha    = 6,
    InvDstAlpha = 7
};

enum class ERglLogicOp
{
    Clear   = 0,
    And     = 1,
    RevAnd  = 2,
    Copy    = 3,
    InvAnd  = 4,
    NoOp    = 5,
    Xor     = 6,
    Or      = 7,
    Nor     = 8,
    Equiv   = 9,
    Inv     = 10,
    RevOr   = 11,
    InvCopy = 12,
    InvOr   = 13,
    NAnd    = 14,
    Set     = 15
};

enum class ERglCullMode
{
    None  = 0,
    Front = 1,
    Back  = 2,
    All   = 3
};

enum class ERglAlphaFunc
{
    Never   = 0,
    Less    = 1,
    Equal   = 2,
    LEqual  = 3,
    Greater = 4,
    NEqual  = 5,
    GEqual  = 6,
    Always  = 7
};

enum class ERglAlphaOp
{
    And  = 0,
    Or   = 1,
    Xor  = 2,
    XNor = 3
};

class CGraphics
{
public:
    static u32 g_NumLightsActive;
    static ERglLight g_LightActive;
    static ERglLight g_LightsWereOn;
    static Zeus::CTransform g_ViewMatrix;
    static void DisableAllLights();
    static void EnableLight(ERglLight light);
    static void SetLightState(ERglLight lightState);
    static void SetDepthWriteMode(bool, ERglEnum, bool);
    static void SetBlendMode(ERglBlendMode, ERglBlendFactor, ERglBlendFactor, ERglLogicOp);
    static void SetCullMode(ERglCullMode);
    static void SetAlphaCompare(ERglAlphaFunc comp0, u8 ref0, ERglAlphaOp op, ERglAlphaFunc comp1, u8 ref1);
    static void SetModelMatrix(const Zeus::CTransform& xf);
};

}

#endif // __RETRO_CGRAPHICS_HPP__
