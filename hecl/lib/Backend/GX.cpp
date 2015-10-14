#include "HECL/Backend/GX.hpp"
#include <map>

namespace HECL
{
namespace Backend
{

unsigned GX::addKColor(Diagnostics& diag, const SourceLocation& loc, const Color& color)
{
    for (unsigned i=0 ; i<m_kcolorCount ; ++i)
        if (m_kcolors[i] == color)
            return i;
    if (m_kcolorCount >= 4)
        diag.reportBackendErr(loc, "GX KColor overflow");
    m_kcolors[m_kcolorCount] = color;
    return m_kcolorCount++;
}

unsigned GX::addKAlpha(Diagnostics& diag, const SourceLocation& loc, float alpha)
{
    uint8_t ai = uint8_t(std::min(std::max(alpha * 255.f, 0.f), 255.f));
    for (unsigned i=0 ; i<m_kcolorCount ; ++i)
    {
        if (m_kcolors[i].color[3] == ai)
            return i;
        else if (m_kcolors[i].color[3] == 0)
        {
            m_kcolors[i].color[3] = ai;
            return i;
        }
    }
    if (m_kcolorCount >= 4)
        diag.reportBackendErr(loc, "GX KColor overflow");
    m_kcolors[m_kcolorCount] = ai;
    return m_kcolorCount++;
}

unsigned GX::addTexCoordGen(Diagnostics& diag, const SourceLocation& loc,
                            TexGenSrc src, TexMtx mtx)
{
    for (unsigned i=0 ; i<m_tcgCount ; ++i)
    {
        TexCoordGen& tcg = m_tcgs[i];
        if (tcg.m_src == src && tcg.m_mtx == mtx)
            return i;
    }
    if (m_tcgCount >= 8)
        diag.reportBackendErr(loc, "GX TexCoordGen overflow");
    GX::TexCoordGen& newTcg = m_tcgs[m_tcgCount];
    newTcg.m_src = src;
    newTcg.m_mtx = mtx;
    return m_tcgCount++;
}

GX::TEVStage& GX::addTEVStage(Diagnostics& diag, const SourceLocation& loc)
{
    if (m_tevCount >= 16)
        diag.reportBackendErr(loc, "GX TEV stage overflow");
    GX::TEVStage& newTEV = m_tevs[m_tevCount];
    if (m_tevCount)
    {
        newTEV.m_prev = &m_tevs[m_tevCount-1];
        newTEV.m_prev->m_next = &newTEV;
    }
    ++m_tevCount;
    return newTEV;
}

unsigned GX::RecursiveTraceTexGen(const IR& ir, Diagnostics& diag, const IR::Instruction& inst, TexMtx mtx)
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
        return addTexCoordGen(diag, inst.m_loc, TexGenSrc(TG_TEX0 + unsigned(idxImm.vec[0])), mtx);
    }
    else if (!tcgName.compare("Normal"))
        return addTexCoordGen(diag, inst.m_loc, TG_NRM, mtx);
    else if (!tcgName.compare("View"))
        return addTexCoordGen(diag, inst.m_loc, TG_POS, mtx);

    /* Otherwise treat as game-specific function */
    const IR::Instruction& tcgSrcInst = inst.getChildInst(ir, 0);
    unsigned idx = RecursiveTraceTexGen(ir, diag, tcgSrcInst, TexMtx(TEXMTX0 + m_texMtxCount * 3));
    GX::TexCoordGen& tcg = m_tcgs[idx];
    m_texMtxRefs[m_texMtxCount] = &tcg;
    ++m_texMtxCount;
    tcg.m_gameFunction = tcgName;
    tcg.m_gameArgs.clear();
    for (ssize_t i=1 ; i<inst.getChildCount() ; ++i)
    {
        const IR::Instruction& ci = inst.getChildInst(ir, i);
        tcg.m_gameArgs.push_back(ci.getImmVec());
    }
    return idx;
}

GX::TraceResult GX::RecursiveTraceColor(const IR& ir, Diagnostics& diag, const IR::Instruction& inst)
{
    switch (inst.m_op)
    {
    case IR::OpCall:
    {
        const std::string& name = inst.m_call.m_name;
        if (!name.compare("Texture"))
        {
            TEVStage& newStage = addTEVStage(diag, inst.m_loc);

            if (inst.getChildCount() < 2)
                diag.reportBackendErr(inst.m_loc, "Texture(map, texgen) requires 2 arguments");

            const IR::Instruction& mapInst = inst.getChildInst(ir, 0);
            const atVec4f& mapImm = mapInst.getImmVec();
            newStage.m_texMapIdx = unsigned(mapImm.vec[0]);
            newStage.m_color[0] = CC_TEXC;

            const IR::Instruction& tcgInst = inst.getChildInst(ir, 1);
            newStage.m_texGenIdx = RecursiveTraceTexGen(ir, diag, tcgInst, IDENTITY);

            return TraceResult(&newStage);
        }
        else if (!name.compare("ColorReg"))
        {
            const IR::Instruction& idxInst = inst.getChildInst(ir, 0);
            unsigned idx = unsigned(idxInst.getImmVec().vec[0]);
            return TraceResult(GX::TevColorArg(CC_C0 + idx * 2));
        }
        else if (!name.compare("Lighting"))
        {
            return TraceResult(CC_RASC);
        }
        else
            diag.reportBackendErr(inst.m_loc, "GX backend unable to interpret '%s'", name.c_str());
        break;
    }
    case IR::OpLoadImm:
    {
        const atVec4f& vec = inst.m_loadImm.m_immVec;
        if (vec.vec[0] == 0.f && vec.vec[1] == 0.f && vec.vec[2] == 0.f)
            return TraceResult(CC_ZERO);
        else if (vec.vec[0] == 1.f && vec.vec[1] == 1.f && vec.vec[2] == 1.f)
            return TraceResult(CC_ONE);
        unsigned idx = addKColor(diag, inst.m_loc, vec);
        return TraceResult(TevKColorSel(TEV_KCSEL_K0 + idx));
    }
    case IR::OpArithmetic:
    {
        ArithmeticOp op = inst.m_arithmetic.m_op;
        const IR::Instruction& aInst = inst.getChildInst(ir, 0);
        TraceResult aTrace = RecursiveTraceColor(ir, diag, aInst);
        const IR::Instruction& bInst = inst.getChildInst(ir, 1);
        TraceResult bTrace = RecursiveTraceColor(ir, diag, bInst);

        switch (op)
        {
        case ArithmeticOp::ArithmeticOpAdd:
        {
            if (aTrace.type == TraceResult::TraceTEVStage &&
                bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                    diag.reportBackendErr(inst.m_loc, "TEV stages must have monotonic progression");
                return TraceResult(b);
            }
            break;
        }
        case ArithmeticOp::ArithmeticOpSubtract:
        {
            if (aTrace.type == TraceResult::TraceTEVStage &&
                bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                    diag.reportBackendErr(inst.m_loc, "TEV stages must have monotonic progression");
                b->m_op = TEV_SUB;
                return TraceResult(b);
            }
            break;
        }
        case ArithmeticOp::ArithmeticOpMultiply:
        {
            if (aTrace.type == TraceResult::TraceTEVStage &&
                bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                    diag.reportBackendErr(inst.m_loc, "TEV stages must have monotonic progression");
                if (a->m_color[2] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                b->m_color[1] = b->m_color[0];
                b->m_color[0] = CC_ZERO;
                b->m_color[2] = CC_CPREV;
                b->m_color[3] = CC_ZERO;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::TraceTEVStage &&
                     bTrace.type == TraceResult::TraceTEVColorArg)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_color[1] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                a->m_color[1] = a->m_color[0];
                a->m_color[0] = CC_ZERO;
                a->m_color[2] = bTrace.tevColorArg;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::TraceTEVColorArg &&
                     bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* b = bTrace.tevStage;
                if (b->m_color[1] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                b->m_color[1] = b->m_color[0];
                b->m_color[0] = CC_ZERO;
                b->m_color[2] = bTrace.tevColorArg;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::TraceTEVStage &&
                     bTrace.type == TraceResult::TraceTEVKColorSel)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_kColor != TEV_KCSEL_1)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for KColor combine");
                if (a->m_color[1] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                a->m_color[1] = a->m_color[0];
                a->m_color[0] = CC_ZERO;
                a->m_color[2] = CC_KONST;
                a->m_kColor = bTrace.tevKColorSel;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::TraceTEVKColorSel &&
                     bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* b = bTrace.tevStage;
                if (b->m_kColor != TEV_KCSEL_1)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for KColor combine");
                if (b->m_color[1] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                b->m_color[1] = b->m_color[0];
                b->m_color[0] = CC_ZERO;
                b->m_color[2] = CC_KONST;
                b->m_kColor = aTrace.tevKColorSel;
                return TraceResult(b);
            }
            break;
        }
        default:
            diag.reportBackendErr(inst.m_loc, "invalid arithmetic op");
        }

        diag.reportBackendErr(inst.m_loc, "unable to convert arithmetic to TEV stage");
    }
    default:
        diag.reportBackendErr(inst.m_loc, "invalid color op");
    }

    return TraceResult();
}

GX::TraceResult GX::RecursiveTraceAlpha(const IR& ir, Diagnostics& diag, const IR::Instruction& inst)
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

            int foundStage = -1;
            for (int i=0 ; i<m_tevCount ; ++i)
            {
                TEVStage& testStage = m_tevs[i];
                if (testStage.m_texMapIdx == mapIdx && i > m_alphaTraceStage)
                {
                    foundStage = i;
                    break;
                }
            }

            if (foundStage >= 0)
            {
                m_alphaTraceStage = foundStage;
                TEVStage& stage = m_tevs[foundStage];
                stage.m_alpha[0] = CA_TEXA;
                return TraceResult(&stage);
            }

            TEVStage& newStage = addTEVStage(diag, inst.m_loc);

            newStage.m_texMapIdx = mapIdx;
            newStage.m_alpha[0] = CA_TEXA;

            const IR::Instruction& tcgInst = inst.getChildInst(ir, 1);
            newStage.m_texGenIdx = RecursiveTraceTexGen(ir, diag, tcgInst, IDENTITY);

            return TraceResult(&newStage);
        }
        else if (!name.compare("ColorReg"))
        {
            const IR::Instruction& idxInst = inst.getChildInst(ir, 0);
            unsigned idx = unsigned(idxInst.getImmVec().vec[0]);
            return TraceResult(GX::TevAlphaArg(CA_A0 + idx));
        }
        else if (!name.compare("Lighting"))
        {
            return TraceResult(CA_RASA);
        }
        else
            diag.reportBackendErr(inst.m_loc, "GX backend unable to interpret '%s'", name.c_str());
        break;
    }
    case IR::OpLoadImm:
    {
        const atVec4f& vec = inst.m_loadImm.m_immVec;
        if (vec.vec[0] == 0.f)
            return TraceResult(CA_ZERO);
        unsigned idx = addKAlpha(diag, inst.m_loc, vec.vec[0]);
        return TraceResult(TevKAlphaSel(TEV_KASEL_K0_A + idx));
    }
    case IR::OpArithmetic:
    {
        ArithmeticOp op = inst.m_arithmetic.m_op;
        const IR::Instruction& aInst = inst.getChildInst(ir, 0);
        TraceResult aTrace = RecursiveTraceAlpha(ir, diag, aInst);
        const IR::Instruction& bInst = inst.getChildInst(ir, 1);
        TraceResult bTrace = RecursiveTraceAlpha(ir, diag, bInst);

        switch (op)
        {
        case ArithmeticOp::ArithmeticOpAdd:
        {
            if (aTrace.type == TraceResult::TraceTEVStage &&
                bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                    diag.reportBackendErr(inst.m_loc, "TEV stages must have monotonic progression");
                return TraceResult(b);
            }
            break;
        }
        case ArithmeticOp::ArithmeticOpSubtract:
        {
            if (aTrace.type == TraceResult::TraceTEVStage &&
                bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                    diag.reportBackendErr(inst.m_loc, "TEV stages must have monotonic progression");
                if (b->m_op != TEV_SUB)
                    diag.reportBackendErr(inst.m_loc, "unable to integrate alpha subtraction into stage chain");
                return TraceResult(b);
            }
            break;
        }
        case ArithmeticOp::ArithmeticOpMultiply:
        {
            if (aTrace.type == TraceResult::TraceTEVStage &&
                bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                    diag.reportBackendErr(inst.m_loc, "TEV stages must have monotonic progression");
                if (a->m_alpha[2] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                b->m_alpha[1] = b->m_alpha[0];
                b->m_alpha[0] = CA_ZERO;
                b->m_alpha[2] = CA_APREV;
                b->m_alpha[3] = CA_ZERO;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::TraceTEVStage &&
                     bTrace.type == TraceResult::TraceTEVColorArg)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_alpha[1] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                a->m_alpha[1] = a->m_alpha[0];
                a->m_alpha[0] = CA_ZERO;
                a->m_alpha[2] = bTrace.tevAlphaArg;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::TraceTEVColorArg &&
                     bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* b = bTrace.tevStage;
                if (b->m_alpha[1] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                b->m_alpha[1] = b->m_alpha[0];
                b->m_alpha[0] = CA_ZERO;
                b->m_alpha[2] = bTrace.tevAlphaArg;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::TraceTEVStage &&
                     bTrace.type == TraceResult::TraceTEVKColorSel)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_kAlpha != TEV_KASEL_1)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for KAlpha combine");
                if (a->m_alpha[1] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                a->m_alpha[1] = a->m_alpha[0];
                a->m_alpha[0] = CA_ZERO;
                a->m_alpha[2] = CA_KONST;
                a->m_kAlpha = bTrace.tevKAlphaSel;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::TraceTEVKColorSel &&
                     bTrace.type == TraceResult::TraceTEVStage)
            {
                TEVStage* b = bTrace.tevStage;
                if (b->m_kAlpha != TEV_KASEL_1)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for KAlpha combine");
                if (b->m_alpha[1] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                b->m_alpha[1] = b->m_alpha[0];
                b->m_alpha[0] = CA_ZERO;
                b->m_alpha[2] = CA_KONST;
                b->m_kAlpha = aTrace.tevKAlphaSel;
                return TraceResult(b);
            }
            break;
        }
        default:
            diag.reportBackendErr(inst.m_loc, "invalid arithmetic op");
        }

        diag.reportBackendErr(inst.m_loc, "unable to convert arithmetic to TEV stage");
    }
    default:
        diag.reportBackendErr(inst.m_loc, "invalid alpha op");
    }

    return TraceResult();
}

void GX::reset(const IR& ir, Diagnostics& diag)
{
    m_tevCount = 0;
    m_tcgCount = 0;
    m_texMtxCount = 0;
    m_kcolorCount = 0;
    m_alphaTraceStage = -1;

    /* Final instruction is the root call by hecl convention */
    const IR::Instruction& rootCall = ir.m_instructions.back();
    bool doAlpha = false;
    if (!rootCall.m_call.m_name.compare("HECLOpaque"))
    {
        m_blendSrc = BL_ONE;
        m_blendDst = BL_ZERO;
    }
    else if (!rootCall.m_call.m_name.compare("HECLAlpha"))
    {
        m_blendSrc = BL_SRCALPHA;
        m_blendDst = BL_INVSRCALPHA;
        doAlpha = true;
    }
    else if (!rootCall.m_call.m_name.compare("HECLAdditive"))
    {
        m_blendSrc = BL_SRCALPHA;
        m_blendDst = BL_ONE;
        doAlpha = true;
    }

    /* Follow Color Chain */
    const IR::Instruction& colorRoot =
    ir.m_instructions.at(rootCall.m_call.m_argInstIdxs.at(0));
    RecursiveTraceColor(ir, diag, colorRoot);

    /* Follow Alpha Chain */
    if (doAlpha)
    {
        const IR::Instruction& alphaRoot =
        ir.m_instructions.at(rootCall.m_call.m_argInstIdxs.at(1));
        RecursiveTraceAlpha(ir, diag, alphaRoot);
    }
}

}
}
