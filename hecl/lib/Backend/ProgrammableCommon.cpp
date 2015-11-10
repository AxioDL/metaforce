#include "HECL/Backend/ProgrammableCommon.hpp"
#include <map>

namespace HECL
{
namespace Backend
{

unsigned ProgrammableCommon::addTexCoordGen(TexGenSrc src, int uvIdx, int mtx)
{
    for (unsigned i=0 ; i<m_tcgs.size() ; ++i)
    {
        TexCoordGen& tcg = m_tcgs[i];
        if (tcg.m_src == src && tcg.m_uvIdx == uvIdx && tcg.m_mtx == mtx)
            return i;
    }
    m_tcgs.emplace_back();
    TexCoordGen& newTcg = m_tcgs.back();
    newTcg.m_src = src;
    newTcg.m_uvIdx = uvIdx;
    newTcg.m_mtx = mtx;
    return m_tcgs.size() - 1;
}

unsigned ProgrammableCommon::addTexSampling(unsigned mapIdx, unsigned tcgIdx)
{
    for (unsigned i=0 ; i<m_texSamplings.size() ; ++i)
    {
        TexSampling& samp = m_texSamplings[i];
        if (samp.mapIdx == mapIdx && samp.tcgIdx == tcgIdx)
            return i;
    }
    m_texSamplings.emplace_back();
    TexSampling& samp = m_texSamplings.back();
    samp.mapIdx = mapIdx;
    samp.tcgIdx = tcgIdx;
    return m_texSamplings.size() - 1;
}

unsigned ProgrammableCommon::RecursiveTraceTexGen(const IR& ir, Diagnostics& diag,
                                                  const IR::Instruction& inst, int mtx)
{
    if (inst.m_op != IR::OpCall)
        diag.reportBackendErr(inst.m_loc, "TexCoordGen resolution requires function");

    const std::string& tcgName = inst.m_call.m_name;
    if (!tcgName.compare("UV"))
    {
        if (inst.getChildCount() < 1)
            diag.reportBackendErr(inst.m_loc, "TexCoordGen UV(layerIdx) requires one argument");
        const IR::Instruction& idxInst = inst.getChildInst(ir, 0);
        const atVec4f& idxImm = idxInst.getImmVec();
        return addTexCoordGen(TG_UV, idxImm.vec[0], mtx);
    }
    else if (!tcgName.compare("Normal"))
        return addTexCoordGen(TG_NRM, -1, mtx);
    else if (!tcgName.compare("View"))
        return addTexCoordGen(TG_POS, -1, mtx);

    /* Otherwise treat as game-specific function */
    const IR::Instruction& tcgSrcInst = inst.getChildInst(ir, 0);
    unsigned idx = RecursiveTraceTexGen(ir, diag, tcgSrcInst, m_texMtxRefs.size());
    TexCoordGen& tcg = m_tcgs[idx];
    m_texMtxRefs.push_back(idx);
    tcg.m_gameFunction = tcgName;
    tcg.m_gameArgs.clear();
    for (int i=1 ; i<inst.getChildCount() ; ++i)
    {
        const IR::Instruction& ci = inst.getChildInst(ir, i);
        tcg.m_gameArgs.push_back(ci.getImmVec());
    }
    return idx;
}

std::string ProgrammableCommon::RecursiveTraceColor(const IR& ir, Diagnostics& diag,
                                                    const IR::Instruction& inst)
{
    switch (inst.m_op)
    {
    case IR::OpCall:
    {
        const std::string& name = inst.m_call.m_name;
        if (!name.compare("Texture"))
        {
            if (inst.getChildCount() < 2)
                diag.reportBackendErr(inst.m_loc, "Texture(map, texgen) requires 2 arguments");

            const IR::Instruction& mapInst = inst.getChildInst(ir, 0);
            const atVec4f& mapImm = mapInst.getImmVec();
            unsigned mapIdx = unsigned(mapImm.vec[0]);

            const IR::Instruction& tcgInst = inst.getChildInst(ir, 1);
            unsigned texGenIdx = RecursiveTraceTexGen(ir, diag, tcgInst, -1);

            return EmitSamplingUse(addTexSampling(mapIdx, texGenIdx));
        }
        else if (!name.compare("ColorReg"))
        {
            const IR::Instruction& idxInst = inst.getChildInst(ir, 0);
            unsigned idx = unsigned(idxInst.getImmVec().vec[0]);
            return EmitColorRegUse(idx);
        }
        else if (!name.compare("Lighting"))
        {
            m_lighting = true;
            return EmitLighting();
        }
        else
            diag.reportBackendErr(inst.m_loc, "unable to interpret '%s'", name.c_str());
        break;
    }
    case IR::OpLoadImm:
    {
        const atVec4f& vec = inst.m_loadImm.m_immVec;
        return EmitVec3(vec);
    }
    case IR::OpArithmetic:
    {
        ArithmeticOp op = inst.m_arithmetic.m_op;
        const IR::Instruction& aInst = inst.getChildInst(ir, 0);
        const IR::Instruction& bInst = inst.getChildInst(ir, 1);
        std::string aTrace = RecursiveTraceColor(ir, diag, aInst);
        std::string bTrace = RecursiveTraceColor(ir, diag, bInst);

        switch (op)
        {
        case ArithmeticOp::ArithmeticOpAdd:
        {
            return EmitAdd(aTrace, bTrace);
        }
        case ArithmeticOp::ArithmeticOpSubtract:
        {
            return EmitSub(aTrace, bTrace);
        }
        case ArithmeticOp::ArithmeticOpMultiply:
        {
            return EmitMult(aTrace, bTrace);
        }
        case ArithmeticOp::ArithmeticOpDivide:
        {
            return EmitDiv(aTrace, bTrace);
        }
        default:
            diag.reportBackendErr(inst.m_loc, "invalid arithmetic op");
        }
    }
    case IR::OpSwizzle:
    {
        const IR::Instruction& aInst = inst.getChildInst(ir, 0);
        std::string aTrace = RecursiveTraceColor(ir, diag, aInst);
        return EmitSwizzle3(diag, inst.m_loc, aTrace, inst.m_swizzle.m_idxs);
    }
    default:
        diag.reportBackendErr(inst.m_loc, "invalid color op");
    }

    return std::string();
}

std::string ProgrammableCommon::RecursiveTraceAlpha(const IR& ir, Diagnostics& diag,
                                                    const IR::Instruction& inst)
{
    switch (inst.m_op)
    {
    case IR::OpCall:
    {
        const std::string& name = inst.m_call.m_name;
        if (!name.compare("Texture"))
        {
            if (inst.getChildCount() < 2)
                diag.reportBackendErr(inst.m_loc, "Texture(map, texgen) requires 2 arguments");

            const IR::Instruction& mapInst = inst.getChildInst(ir, 0);
            const atVec4f& mapImm = mapInst.getImmVec();
            unsigned mapIdx = unsigned(mapImm.vec[0]);

            const IR::Instruction& tcgInst = inst.getChildInst(ir, 1);
            unsigned texGenIdx = RecursiveTraceTexGen(ir, diag, tcgInst, -1);

            return EmitSamplingUse(addTexSampling(mapIdx, texGenIdx));
        }
        else if (!name.compare("ColorReg"))
        {
            const IR::Instruction& idxInst = inst.getChildInst(ir, 0);
            unsigned idx = unsigned(idxInst.getImmVec().vec[0]);
            return EmitColorRegUse(idx);
        }
        else if (!name.compare("Lighting"))
        {
            m_lighting = true;
            return EmitLighting();
        }
        else
            diag.reportBackendErr(inst.m_loc, "unable to interpret '%s'", name.c_str());
        break;
    }
    case IR::OpLoadImm:
    {
        const atVec4f& vec = inst.m_loadImm.m_immVec;
        return EmitVal(vec.vec[0]);
    }
    case IR::OpArithmetic:
    {
        ArithmeticOp op = inst.m_arithmetic.m_op;
        const IR::Instruction& aInst = inst.getChildInst(ir, 0);
        const IR::Instruction& bInst = inst.getChildInst(ir, 1);
        std::string aTrace = RecursiveTraceAlpha(ir, diag, aInst);
        std::string bTrace = RecursiveTraceAlpha(ir, diag, bInst);

        switch (op)
        {
        case ArithmeticOp::ArithmeticOpAdd:
        {
            return EmitAdd(aTrace, bTrace);
        }
        case ArithmeticOp::ArithmeticOpSubtract:
        {
            return EmitSub(aTrace, bTrace);
        }
        case ArithmeticOp::ArithmeticOpMultiply:
        {
            return EmitMult(aTrace, bTrace);
        }
        case ArithmeticOp::ArithmeticOpDivide:
        {
            return EmitDiv(aTrace, bTrace);
        }
        default:
            diag.reportBackendErr(inst.m_loc, "invalid arithmetic op");
        }
    }
    case IR::OpSwizzle:
    {
        const IR::Instruction& aInst = inst.getChildInst(ir, 0);
        std::string aTrace = RecursiveTraceAlpha(ir, diag, aInst);
        return EmitSwizzle1(diag, inst.m_loc, aTrace, inst.m_swizzle.m_idxs);
    }
    default:
        diag.reportBackendErr(inst.m_loc, "invalid alpha op");
    }

    return std::string();
}

void ProgrammableCommon::reset(const IR& ir, Diagnostics& diag, const char* backendName)
{
    diag.setBackend(backendName);

    /* Final instruction is the root call by hecl convention */
    const IR::Instruction& rootCall = ir.m_instructions.back();
    bool doAlpha = false;
    if (!rootCall.m_call.m_name.compare("HECLOpaque"))
    {
        m_blendSrc = boo::BlendFactorOne;
        m_blendDst = boo::BlendFactorZero;
    }
    else if (!rootCall.m_call.m_name.compare("HECLAlpha"))
    {
        m_blendSrc = boo::BlendFactorSrcAlpha;
        m_blendDst = boo::BlendFactorInvSrcAlpha;
        doAlpha = true;
    }
    else if (!rootCall.m_call.m_name.compare("HECLAdditive"))
    {
        m_blendSrc = boo::BlendFactorSrcAlpha;
        m_blendDst = boo::BlendFactorOne;
        doAlpha = true;
    }
    else
    {
        diag.reportBackendErr(rootCall.m_loc, "%s backend doesn't handle '%s' root",
                              backendName, rootCall.m_call.m_name.c_str());
        return;
    }

    /* Follow Color Chain */
    const IR::Instruction& colorRoot =
    ir.m_instructions.at(rootCall.m_call.m_argInstIdxs.at(0));
    m_colorExpr = RecursiveTraceColor(ir, diag, colorRoot);

    /* Follow Alpha Chain */
    if (doAlpha)
    {
        const IR::Instruction& alphaRoot =
        ir.m_instructions.at(rootCall.m_call.m_argInstIdxs.at(1));
        m_alphaExpr = RecursiveTraceAlpha(ir, diag, alphaRoot);
    }
}

static const char SWIZZLE_CHARS[] = "rgba";

std::string ProgrammableCommon::EmitSwizzle3(Diagnostics& diag, const SourceLocation& loc,
                                             const std::string& a, const atInt8 swiz[4]) const
{
    std::string retval = a + '.';
    for (int i=0 ; i<3 ; ++i)
    {
        if (swiz[i] < 0 || swiz[i] > 3)
            diag.reportBackendErr(loc, "unable to use swizzle as RGB value");
        retval += SWIZZLE_CHARS[swiz[i]];
    }
    return retval;
}

std::string ProgrammableCommon::EmitSwizzle1(Diagnostics& diag, const SourceLocation& loc,
                                             const std::string& a, const atInt8 swiz[4]) const
{
    std::string retval = a + '.';
    if (swiz[0] < 0 || swiz[0] > 3)
        diag.reportBackendErr(loc, "unable to use swizzle as Alpha value");
    retval += SWIZZLE_CHARS[swiz[0]];
    return retval;
}

}
}
