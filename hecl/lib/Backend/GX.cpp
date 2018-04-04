#include "hecl/Backend/GX.hpp"

namespace hecl::Backend
{

template <>
void GX::Color::Enumerate<athena::io::DNA<athena::Big>::Read>(typename Read::StreamT& reader)
{ reader.readUBytesToBuf(&num, 4); }
template <>
void GX::Color::Enumerate<athena::io::DNA<athena::Big>::Write>(typename Write::StreamT& writer)
{ writer.writeUBytes(reinterpret_cast<const atUint8*>(&num), 4); }
template <>
void GX::Color::Enumerate<athena::io::DNA<athena::Big>::BinarySize>(typename BinarySize::StreamT& s)
{ s += 4; }

unsigned GX::addKColor(Diagnostics& diag, const SourceLocation& loc, const Color& color)
{
    for (unsigned i=0 ; i<m_kcolorCount ; ++i)
        if (m_kcolors[i] == color)
            return i;
    if (m_kcolorCount >= 4)
        diag.reportBackendErr(loc, "GX KColor overflow");
    m_kcolors[m_kcolorCount] = color;
    m_kcolors[m_kcolorCount].color[3] = 0;
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
                            TexGenSrc src, TexMtx mtx, bool norm, PTTexMtx pmtx)
{
    for (unsigned i=0 ; i<m_tcgCount ; ++i)
    {
        TexCoordGen& tcg = m_tcgs[i];
        if (tcg.m_src == src && tcg.m_mtx == mtx &&
            tcg.m_norm == norm && tcg.m_pmtx == pmtx)
            return i;
    }
    if (m_tcgCount >= 8)
        diag.reportBackendErr(loc, "GX TexCoordGen overflow");
    GX::TexCoordGen& newTcg = m_tcgs[m_tcgCount];
    newTcg.m_src = src;
    newTcg.m_mtx = mtx;
    newTcg.m_norm = norm;
    newTcg.m_pmtx = pmtx;
    return m_tcgCount++;
}

GX::TEVStage& GX::addTEVStage(Diagnostics& diag, const SourceLocation& loc)
{
    if (m_tevCount >= 16)
        diag.reportBackendErr(loc, "GX TEV stage overflow");
    GX::TEVStage& newTEV = m_tevs[m_tevCount];
    newTEV.m_loc = loc;
    if (m_tevCount)
    {
        newTEV.m_prev = &m_tevs[m_tevCount-1];
        newTEV.m_prev->m_next = &newTEV;
    }
    ++m_tevCount;
    return newTEV;
}

GX::TEVStage& GX::addAlphaTEVStage(Diagnostics& diag, const SourceLocation& loc)
{
    ++m_alphaTraceStage;
    while (m_tevCount < m_alphaTraceStage + 1)
    {
        TEVStage& stage = addTEVStage(diag, loc);
        stage.m_color[3] = CC_CPREV;
        stage.m_alpha[3] = CA_APREV;
    }
    return m_tevs[m_alphaTraceStage];
}

unsigned GX::RecursiveTraceTexGen(const IR& ir, Diagnostics& diag, const IR::Instruction& inst, TexMtx mtx,
                                  bool normalize, PTTexMtx pmtx)
{
    if (inst.m_op != IR::OpType::Call)
        diag.reportBackendErr(inst.m_loc, "TexCoordGen resolution requires function");

    const std::string& tcgName = inst.m_call.m_name;
    if (!tcgName.compare("UV"))
    {
        if (inst.getChildCount() < 1)
            diag.reportBackendErr(inst.m_loc, "TexCoordGen UV(layerIdx) requires one argument");
        const IR::Instruction& idxInst = inst.getChildInst(ir, 0);
        auto& idxImm = idxInst.getImmVec();
        return addTexCoordGen(diag, inst.m_loc, TexGenSrc(TG_TEX0 + unsigned(idxImm.vec[0])), mtx, normalize, pmtx);
    }
    else if (!tcgName.compare("Normal"))
        return addTexCoordGen(diag, inst.m_loc, TG_NRM, mtx, normalize, pmtx);
    else if (!tcgName.compare("View"))
        return addTexCoordGen(diag, inst.m_loc, TG_POS, mtx, normalize, pmtx);

    /* Otherwise treat as game-specific function */
    const IR::Instruction& tcgSrcInst = inst.getChildInst(ir, 0);
    bool doNorm = normalize || tcgName.back() == 'N';
    unsigned idx = RecursiveTraceTexGen(ir, diag, tcgSrcInst, TexMtx(TEXMTX0 + m_texMtxCount * 3),
                                        doNorm, doNorm ? PTTexMtx(PTTEXMTX0 + m_texMtxCount * 3) : PTIDENTITY);
    GX::TexCoordGen& tcg = m_tcgs[idx];
    m_texMtxRefs[m_texMtxCount] = &tcg;
    ++m_texMtxCount;
    tcg.m_gameFunction = tcgName;
    tcg.m_gameArgs.clear();
    for (int i=1 ; i<inst.getChildCount() ; ++i)
    {
        const IR::Instruction& ci = inst.getChildInst(ir, i);
        tcg.m_gameArgs.push_back(ci.getImmVec());
    }
    return idx;
}

GX::TraceResult GX::RecursiveTraceColor(const IR& ir, Diagnostics& diag, const IR::Instruction& inst,
                                        bool swizzleAlpha)
{
    switch (inst.m_op)
    {
    case IR::OpType::Call:
    {
        const std::string& name = inst.m_call.m_name;
        bool normalize = false;
        if (!name.compare("Texture") || (normalize = true && !name.compare("TextureN")))
        {
            TEVStage& newStage = addTEVStage(diag, inst.m_loc);

            if (inst.getChildCount() < 2)
                diag.reportBackendErr(inst.m_loc, "Texture(map, texgen) requires 2 arguments");

            const IR::Instruction& mapInst = inst.getChildInst(ir, 0);
            auto& mapImm = mapInst.getImmVec();
            newStage.m_texMapIdx = unsigned(mapImm.vec[0]);
            newStage.m_color[0] = swizzleAlpha ? CC_TEXA : CC_TEXC;

            const IR::Instruction& tcgInst = inst.getChildInst(ir, 1);
            newStage.m_texGenIdx = RecursiveTraceTexGen(ir, diag, tcgInst, IDENTITY, normalize, PTIDENTITY);

            return TraceResult(&newStage);
        }
        else if (!name.compare("ColorReg"))
        {
            const IR::Instruction& idxInst = inst.getChildInst(ir, 0);
            unsigned idx = unsigned(idxInst.getImmVec().vec[0]);
            if (swizzleAlpha)
                m_aRegMask |= 1 << idx;
            else
                m_cRegMask |= 1 << idx;
            return TraceResult(TevColorArg((swizzleAlpha ? CC_A0 : CC_C0) + idx * 2));
        }
        else if (!name.compare("Lighting"))
        {
            return TraceResult(swizzleAlpha ? CC_RASA : CC_RASC);
        }
        else
            diag.reportBackendErr(inst.m_loc, "GX backend unable to interpret '%s'", name.c_str());
        break;
    }
    case IR::OpType::LoadImm:
    {
        const atVec4f& vec = inst.m_loadImm.m_immVec;
        if (vec.vec[0] == 0.f && vec.vec[1] == 0.f && vec.vec[2] == 0.f)
            return TraceResult(CC_ZERO);
        else if (vec.vec[0] == 1.f && vec.vec[1] == 1.f && vec.vec[2] == 1.f)
            return TraceResult(CC_ONE);
        unsigned idx = addKColor(diag, inst.m_loc, vec);
        return TraceResult(TevKColorSel(TEV_KCSEL_K0 + idx));
    }
    case IR::OpType::Arithmetic:
    {
        ArithmeticOp op = inst.m_arithmetic.m_op;
        const IR::Instruction& aInst = inst.getChildInst(ir, 0);
        const IR::Instruction& bInst = inst.getChildInst(ir, 1);
        TraceResult aTrace;
        TraceResult bTrace;
        if (aInst.m_op != IR::OpType::Arithmetic && bInst.m_op == IR::OpType::Arithmetic)
        {
            bTrace = RecursiveTraceColor(ir, diag, bInst);
            aTrace = RecursiveTraceColor(ir, diag, aInst);
        }
        else
        {
            aTrace = RecursiveTraceColor(ir, diag, aInst);
            bTrace = RecursiveTraceColor(ir, diag, bInst);
        }
        if (aTrace.type == TraceResult::Type::TEVStage &&
            bTrace.type == TraceResult::Type::TEVStage &&
            getStageIdx(aTrace.tevStage) > getStageIdx(bTrace.tevStage))
            std::swap(aTrace, bTrace);

        TevKColorSel newKColor = TEV_KCSEL_1;
        if (aTrace.type == TraceResult::Type::TEVKColorSel &&
            bTrace.type == TraceResult::Type::TEVKColorSel)
            diag.reportBackendErr(inst.m_loc, "unable to handle 2 KColors in one stage");
        else if (aTrace.type == TraceResult::Type::TEVKColorSel)
        {
            newKColor = aTrace.tevKColorSel;
            aTrace.type = TraceResult::Type::TEVColorArg;
            aTrace.tevColorArg = CC_KONST;
        }
        else if (bTrace.type == TraceResult::Type::TEVKColorSel)
        {
            newKColor = bTrace.tevKColorSel;
            bTrace.type = TraceResult::Type::TEVColorArg;
            bTrace.tevColorArg = CC_KONST;
        }

        switch (op)
        {
        case ArithmeticOp::Add:
        {
            if (aTrace.type == TraceResult::Type::TEVStage &&
                bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                {
                    a->m_cRegOut = TEVLAZY;
                    b->m_color[3] = CC_LAZY;
                    b->m_lazyCInIdx = m_cRegLazy;
                    a->m_lazyCOutIdx = m_cRegLazy++;
                }
                else if (b == &m_tevs[m_tevCount-1] &&
                         a->m_texMapIdx == b->m_texMapIdx && a->m_texGenIdx == b->m_texGenIdx &&
                         a->m_color[3] == CC_ZERO && b->m_color[0] != CC_ZERO)
                {
                    a->m_color[3] = b->m_color[0];
                    --m_tevCount;
                    return TraceResult(a);
                }
                else
                    b->m_color[3] = CC_CPREV;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::Type::TEVStage &&
                     bTrace.type == TraceResult::Type::TEVColorArg)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_color[3] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for add combine");
                a->m_color[3] = bTrace.tevColorArg;
                a->m_kColor = newKColor;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::Type::TEVColorArg &&
                     bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* b = bTrace.tevStage;
                if (b->m_color[3] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for add combine");
                b->m_color[3] = aTrace.tevColorArg;
                b->m_kColor = newKColor;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::Type::TEVColorArg &&
                     bTrace.type == TraceResult::Type::TEVColorArg)
            {
                TEVStage& stage = addTEVStage(diag, inst.m_loc);
                stage.m_color[0] = aTrace.tevColorArg;
                stage.m_color[3] = bTrace.tevColorArg;
                stage.m_kColor = newKColor;
                return TraceResult(&stage);
            }
            break;
        }
        case ArithmeticOp::Subtract:
        {
            if (aTrace.type == TraceResult::Type::TEVStage &&
                bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                {
                    a->m_cRegOut = TEVLAZY;
                    b->m_color[3] = CC_LAZY;
                    b->m_lazyCInIdx = m_cRegLazy;
                    a->m_lazyCOutIdx = m_cRegLazy++;
                }
                else
                    b->m_color[3] = CC_CPREV;
                b->m_cop = TEV_SUB;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::Type::TEVStage &&
                     bTrace.type == TraceResult::Type::TEVColorArg)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_color[3] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for subtract combine");
                a->m_color[3] = bTrace.tevColorArg;
                a->m_kColor = newKColor;
                a->m_cop = TEV_SUB;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::Type::TEVColorArg &&
                     bTrace.type == TraceResult::Type::TEVColorArg)
            {
                TEVStage& stage = addTEVStage(diag, inst.m_loc);
                stage.m_color[0] = aTrace.tevColorArg;
                stage.m_color[3] = bTrace.tevColorArg;
                stage.m_kColor = newKColor;
                stage.m_cop = TEV_SUB;
                return TraceResult(&stage);
            }
            break;
        }
        case ArithmeticOp::Multiply:
        {
            if (aTrace.type == TraceResult::Type::TEVStage &&
                bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_color[2] != CC_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                if (b->m_prev != a)
                {
                    a->m_cRegOut = TEVLAZY;
                    b->m_color[2] = CC_LAZY;
                    b->m_lazyCInIdx = m_cRegLazy;
                    a->m_lazyCOutIdx = m_cRegLazy++;
                }
                else
                    b->m_color[2] = CC_CPREV;
                b->m_color[1] = b->m_color[0];
                b->m_color[0] = CC_ZERO;
                b->m_color[3] = CC_ZERO;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::Type::TEVColorArg &&
                     bTrace.type == TraceResult::Type::TEVColorArg)
            {
                TEVStage& stage = addTEVStage(diag, inst.m_loc);
                stage.m_color[1] = aTrace.tevColorArg;
                stage.m_color[2] = bTrace.tevColorArg;
                stage.m_kColor = newKColor;
                return TraceResult(&stage);
            }
            else if (aTrace.type == TraceResult::Type::TEVStage &&
                     bTrace.type == TraceResult::Type::TEVColorArg)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_color[1] != CC_ZERO)
                {
                    if (a->m_cRegOut != TEVPREV)
                        diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                    TEVStage& stage = addTEVStage(diag, inst.m_loc);
                    stage.m_color[1] = CC_CPREV;
                    stage.m_color[2] = bTrace.tevColorArg;
                    stage.m_kColor = newKColor;
                    return TraceResult(&stage);
                }
                a->m_color[1] = a->m_color[0];
                a->m_color[0] = CC_ZERO;
                a->m_color[2] = bTrace.tevColorArg;
                a->m_kColor = newKColor;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::Type::TEVColorArg &&
                     bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* b = bTrace.tevStage;
                if (b->m_color[1] != CC_ZERO)
                {
                    if (b->m_cRegOut != TEVPREV)
                        diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                    TEVStage& stage = addTEVStage(diag, inst.m_loc);
                    stage.m_color[1] = aTrace.tevColorArg;
                    stage.m_color[2] = CC_CPREV;
                    stage.m_kColor = newKColor;
                    return TraceResult(&stage);
                }
                b->m_color[1] = b->m_color[0];
                b->m_color[0] = CC_ZERO;
                b->m_color[2] = bTrace.tevColorArg;
                b->m_kColor = newKColor;
                return TraceResult(b);
            }
            break;
        }
        default:
            diag.reportBackendErr(inst.m_loc, "invalid arithmetic op");
        }

        diag.reportBackendErr(inst.m_loc, "unable to convert arithmetic to TEV stage");
    }
    case IR::OpType::Swizzle:
    {
        if (inst.m_swizzle.m_idxs[0] == 3 && inst.m_swizzle.m_idxs[1] == 3 &&
            inst.m_swizzle.m_idxs[2] == 3 && inst.m_swizzle.m_idxs[3] == -1)
        {
            const IR::Instruction& cInst = inst.getChildInst(ir, 0);
            if (cInst.m_op != IR::OpType::Call)
                diag.reportBackendErr(inst.m_loc, "only functions accepted for alpha swizzle");
            return RecursiveTraceColor(ir, diag, cInst, true);
        }
        else
            diag.reportBackendErr(inst.m_loc, "only alpha extract may be performed with swizzle operation");
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
    case IR::OpType::Call:
    {
        const std::string& name = inst.m_call.m_name;
        bool normalize = false;
        if (!name.compare("Texture") || (normalize = true && !name.compare("TextureN")))
        {
            if (inst.getChildCount() < 2)
                diag.reportBackendErr(inst.m_loc, "Texture(map, texgen) requires 2 arguments");

            const IR::Instruction& mapInst = inst.getChildInst(ir, 0);
            const atVec4f& mapImm = mapInst.getImmVec();
            unsigned mapIdx = unsigned(mapImm.vec[0]);

            int foundStage = -1;
            for (int i=m_alphaTraceStage+1 ; i<int(m_tevCount) ; ++i)
            {
                TEVStage& testStage = m_tevs[i];
                if (testStage.m_texMapIdx == mapIdx)
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

            TEVStage& newStage = addAlphaTEVStage(diag, inst.m_loc);
            newStage.m_color[3] = CC_CPREV;

            newStage.m_texMapIdx = mapIdx;
            newStage.m_alpha[0] = CA_TEXA;

            const IR::Instruction& tcgInst = inst.getChildInst(ir, 1);
            newStage.m_texGenIdx = RecursiveTraceTexGen(ir, diag, tcgInst, IDENTITY, normalize, PTIDENTITY);

            return TraceResult(&newStage);
        }
        else if (!name.compare("ColorReg"))
        {
            const IR::Instruction& idxInst = inst.getChildInst(ir, 0);
            unsigned idx = unsigned(idxInst.getImmVec().vec[0]);
            m_aRegMask |= 1 << idx;
            return TraceResult(TevAlphaArg(CA_A0 + idx));
        }
        else if (!name.compare("Lighting"))
        {
            return TraceResult(CA_RASA);
        }
        else
            diag.reportBackendErr(inst.m_loc, "GX backend unable to interpret '%s'", name.c_str());
        break;
    }
    case IR::OpType::LoadImm:
    {
        const atVec4f& vec = inst.m_loadImm.m_immVec;
        if (vec.vec[0] == 0.f)
            return TraceResult(CA_ZERO);
        else if (vec.vec[0] == 1.f)
            return TraceResult(TEV_KASEL_1);
        unsigned idx = addKAlpha(diag, inst.m_loc, vec.vec[0]);
        return TraceResult(TevKAlphaSel(TEV_KASEL_K0_A + idx));
    }
    case IR::OpType::Arithmetic:
    {
        ArithmeticOp op = inst.m_arithmetic.m_op;
        const IR::Instruction& aInst = inst.getChildInst(ir, 0);
        const IR::Instruction& bInst = inst.getChildInst(ir, 1);
        TraceResult aTrace;
        TraceResult bTrace;
        if (aInst.m_op != IR::OpType::Arithmetic && bInst.m_op == IR::OpType::Arithmetic)
        {
            bTrace = RecursiveTraceAlpha(ir, diag, bInst);
            aTrace = RecursiveTraceAlpha(ir, diag, aInst);
        }
        else
        {
            aTrace = RecursiveTraceAlpha(ir, diag, aInst);
            bTrace = RecursiveTraceAlpha(ir, diag, bInst);
        }
        if (aTrace.type == TraceResult::Type::TEVStage &&
            bTrace.type == TraceResult::Type::TEVStage &&
            getStageIdx(aTrace.tevStage) > getStageIdx(bTrace.tevStage))
            std::swap(aTrace, bTrace);

        TevKAlphaSel newKAlpha = TEV_KASEL_1;
        if (aTrace.type == TraceResult::Type::TEVKAlphaSel &&
            bTrace.type == TraceResult::Type::TEVKAlphaSel)
            diag.reportBackendErr(inst.m_loc, "unable to handle 2 KAlphas in one stage");
        else if (aTrace.type == TraceResult::Type::TEVKAlphaSel)
        {
            newKAlpha = aTrace.tevKAlphaSel;
            aTrace.type = TraceResult::Type::TEVAlphaArg;
            aTrace.tevAlphaArg = CA_KONST;
        }
        else if (bTrace.type == TraceResult::Type::TEVKAlphaSel)
        {
            newKAlpha = bTrace.tevKAlphaSel;
            bTrace.type = TraceResult::Type::TEVAlphaArg;
            bTrace.tevAlphaArg = CA_KONST;
        }

        switch (op)
        {
        case ArithmeticOp::Add:
        {
            if (aTrace.type == TraceResult::Type::TEVStage &&
                bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_prev != a)
                {
                    a->m_aRegOut = TEVLAZY;
                    b->m_alpha[3] = CA_LAZY;
                    if (a->m_lazyAOutIdx != -1)
                        b->m_lazyAInIdx = a->m_lazyAOutIdx;
                    else
                    {
                        b->m_lazyAInIdx = m_aRegLazy;
                        a->m_lazyAOutIdx = m_aRegLazy++;
                    }
                }
                else
                    b->m_alpha[3] = CA_APREV;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::Type::TEVStage &&
                     bTrace.type == TraceResult::Type::TEVAlphaArg)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_alpha[3] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for add combine");
                a->m_alpha[3] = bTrace.tevAlphaArg;
                a->m_kAlpha = newKAlpha;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::Type::TEVAlphaArg &&
                     bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* b = bTrace.tevStage;
                if (b->m_alpha[3] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for add combine");
                b->m_alpha[3] = aTrace.tevAlphaArg;
                b->m_kAlpha = newKAlpha;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::Type::TEVAlphaArg &&
                     bTrace.type == TraceResult::Type::TEVAlphaArg)
            {
                TEVStage& stage = addAlphaTEVStage(diag, inst.m_loc);
                stage.m_alpha[0] = aTrace.tevAlphaArg;
                stage.m_alpha[3] = bTrace.tevAlphaArg;
                stage.m_kAlpha = newKAlpha;
                return TraceResult(&stage);
            }
            break;
        }
        case ArithmeticOp::Subtract:
        {
            if (aTrace.type == TraceResult::Type::TEVStage &&
                bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_aop != TEV_SUB)
                    diag.reportBackendErr(inst.m_loc, "unable to integrate alpha subtraction into stage chain");
                if (b->m_prev != a)
                {
                    a->m_aRegOut = TEVLAZY;
                    b->m_alpha[3] = CA_LAZY;
                    if (a->m_lazyAOutIdx != -1)
                        b->m_lazyAInIdx = a->m_lazyAOutIdx;
                    else
                    {
                        b->m_lazyAInIdx = m_aRegLazy;
                        a->m_lazyAOutIdx = m_aRegLazy++;
                    }
                }
                else
                    b->m_alpha[3] = CA_APREV;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::Type::TEVStage &&
                     bTrace.type == TraceResult::Type::TEVAlphaArg)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_aop != TEV_SUB)
                    diag.reportBackendErr(inst.m_loc, "unable to integrate alpha subtraction into stage chain");
                if (a->m_alpha[3] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for add combine");
                a->m_alpha[3] = bTrace.tevAlphaArg;
                a->m_kAlpha = newKAlpha;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::Type::TEVAlphaArg &&
                     bTrace.type == TraceResult::Type::TEVAlphaArg)
            {
                TEVStage& stage = addAlphaTEVStage(diag, inst.m_loc);
                stage.m_alpha[0] = aTrace.tevAlphaArg;
                stage.m_alpha[3] = bTrace.tevAlphaArg;
                stage.m_kAlpha = newKAlpha;
                stage.m_aop = TEV_SUB;
                return TraceResult(&stage);
            }
            break;
        }
        case ArithmeticOp::Multiply:
        {
            if (aTrace.type == TraceResult::Type::TEVStage &&
                bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* a = aTrace.tevStage;
                TEVStage* b = bTrace.tevStage;
                if (b->m_alpha[2] != CA_ZERO)
                    diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                if (b->m_prev != a)
                {
                    a->m_aRegOut = TEVLAZY;
                    b->m_alpha[2] = CA_LAZY;
                    b->m_lazyAInIdx = m_aRegLazy;
                    a->m_lazyAOutIdx = m_aRegLazy++;
                }
                else
                    b->m_alpha[2] = CA_APREV;
                b->m_alpha[1] = b->m_alpha[0];
                b->m_alpha[0] = CA_ZERO;
                b->m_alpha[3] = CA_ZERO;
                return TraceResult(b);
            }
            else if (aTrace.type == TraceResult::Type::TEVAlphaArg &&
                     bTrace.type == TraceResult::Type::TEVAlphaArg)
            {
                TEVStage& stage = addAlphaTEVStage(diag, inst.m_loc);
                stage.m_color[3] = CC_CPREV;
                stage.m_alpha[1] = aTrace.tevAlphaArg;
                stage.m_alpha[2] = bTrace.tevAlphaArg;
                stage.m_kAlpha = newKAlpha;
                return TraceResult(&stage);
            }
            else if (aTrace.type == TraceResult::Type::TEVStage &&
                     bTrace.type == TraceResult::Type::TEVAlphaArg)
            {
                TEVStage* a = aTrace.tevStage;
                if (a->m_alpha[1] != CA_ZERO)
                {
                    if (a->m_aRegOut != TEVPREV)
                        diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                    TEVStage& stage = addAlphaTEVStage(diag, inst.m_loc);
                    stage.m_alpha[1] = CA_APREV;
                    stage.m_alpha[2] = bTrace.tevAlphaArg;
                    stage.m_kAlpha = newKAlpha;
                    return TraceResult(&stage);
                }
                a->m_alpha[1] = a->m_alpha[0];
                a->m_alpha[0] = CA_ZERO;
                a->m_alpha[2] = bTrace.tevAlphaArg;
                a->m_kAlpha = newKAlpha;
                return TraceResult(a);
            }
            else if (aTrace.type == TraceResult::Type::TEVAlphaArg &&
                     bTrace.type == TraceResult::Type::TEVStage)
            {
                TEVStage* b = bTrace.tevStage;
                if (b->m_alpha[1] != CA_ZERO)
                {
                    if (b->m_aRegOut != TEVPREV)
                        diag.reportBackendErr(inst.m_loc, "unable to modify TEV stage for multiply combine");
                    TEVStage& stage = addAlphaTEVStage(diag, inst.m_loc);
                    stage.m_alpha[1] = aTrace.tevAlphaArg;
                    stage.m_alpha[2] = CA_APREV;
                    stage.m_kAlpha = newKAlpha;
                    return TraceResult(&stage);
                }
                b->m_alpha[1] = b->m_alpha[0];
                b->m_alpha[0] = CA_ZERO;
                b->m_alpha[2] = bTrace.tevAlphaArg;
                b->m_kAlpha = newKAlpha;
                return TraceResult(b);
            }
            break;
        }
        default:
            diag.reportBackendErr(inst.m_loc, "invalid arithmetic op");
        }

        diag.reportBackendErr(inst.m_loc, "unable to convert arithmetic to TEV stage");
    }
    case IR::OpType::Swizzle:
    {
        if (inst.m_swizzle.m_idxs[0] == 3 && inst.m_swizzle.m_idxs[1] == 3 &&
            inst.m_swizzle.m_idxs[2] == 3 && inst.m_swizzle.m_idxs[3] == -1)
        {
            const IR::Instruction& cInst = inst.getChildInst(ir, 0);
            if (cInst.m_op != IR::OpType::Call)
                diag.reportBackendErr(inst.m_loc, "only functions accepted for alpha swizzle");
            return RecursiveTraceAlpha(ir, diag, cInst);
        }
        else
            diag.reportBackendErr(inst.m_loc, "only alpha extract may be performed with swizzle operation");
    }
    default:
        diag.reportBackendErr(inst.m_loc, "invalid alpha op");
    }

    return TraceResult();
}

void GX::reset(const IR& ir, Diagnostics& diag)
{
    diag.setBackend("GX");

    m_tevCount = 0;
    m_tcgCount = 0;
    m_texMtxCount = 0;
    m_kcolorCount = 0;
    m_cRegMask = 0;
    m_cRegLazy = 0;
    m_aRegMask = 0;
    m_aRegLazy = 0;
    m_alphaTraceStage = -1;

    /* Final instruction is the root call by hecl convention */
    const IR::Instruction& rootCall = ir.m_instructions.back();
    if (!rootCall.m_call.m_name.compare("HECLOpaque"))
    {
        m_blendSrc = BL_ONE;
        m_blendDst = BL_ZERO;
    }
    else if (!rootCall.m_call.m_name.compare("HECLAlpha"))
    {
        m_blendSrc = BL_SRCALPHA;
        m_blendDst = BL_INVSRCALPHA;
    }
    else if (!rootCall.m_call.m_name.compare("HECLAdditive"))
    {
        m_blendSrc = BL_SRCALPHA;
        m_blendDst = BL_ONE;
    }
    else
    {
        diag.reportBackendErr(rootCall.m_loc, "GX backend doesn't handle '%s' root",
                              rootCall.m_call.m_name.c_str());
        return;
    }

    /* Follow Color Chain */
    const IR::Instruction& colorRoot =
    ir.m_instructions.at(rootCall.m_call.m_argInstIdxs.at(0));
    TraceResult result = RecursiveTraceColor(ir, diag, colorRoot);
    switch (result.type)
    {
    case TraceResult::Type::TEVColorArg:
    {
        TEVStage& stage = addTEVStage(diag, colorRoot.m_loc);
        stage.m_color[3] = result.tevColorArg;
        break;
    }
    case TraceResult::Type::TEVKColorSel:
    {
        TEVStage& stage = addTEVStage(diag, colorRoot.m_loc);
        stage.m_color[3] = CC_KONST;
        stage.m_kColor = result.tevKColorSel;
        break;
    }
    default: break;
    }

    /* Follow Alpha Chain */
    if (rootCall.m_call.m_argInstIdxs.size() > 1)
    {
        const IR::Instruction& alphaRoot =
        ir.m_instructions.at(rootCall.m_call.m_argInstIdxs.at(1));
        TraceResult result = RecursiveTraceAlpha(ir, diag, alphaRoot);
        switch (result.type)
        {
        case TraceResult::Type::TEVAlphaArg:
        {
            TEVStage& stage = addAlphaTEVStage(diag, alphaRoot.m_loc);
            stage.m_alpha[3] = result.tevAlphaArg;
            break;
        }
        case TraceResult::Type::TEVKAlphaSel:
        {
            TEVStage& stage = addAlphaTEVStage(diag, alphaRoot.m_loc);
            stage.m_alpha[3] = CA_KONST;
            stage.m_kAlpha = result.tevKAlphaSel;
            break;
        }
        default: break;
        }

        /* Ensure Alpha reaches end of chain */
        if (m_alphaTraceStage >= 0)
            for (unsigned i=m_alphaTraceStage+1 ; i<m_tevCount ; ++i)
                m_tevs[i].m_alpha[3] = CA_APREV;
    }

    /* Resolve lazy color/alpha regs */
    if (m_cRegLazy)
    {
        for (int i=0 ; i<int(m_tevCount) ; ++i)
        {
            TEVStage& stage = m_tevs[i];
            if (stage.m_cRegOut == TEVLAZY)
            {
                int picked = pickCLazy(diag, stage.m_loc, i);
                stage.m_cRegOut = TevRegID(TEVREG0 + picked);
                for (int j=i+1 ; j<int(m_tevCount) ; ++j)
                {
                    TEVStage& nstage = m_tevs[j];
                    if (nstage.m_lazyCInIdx == stage.m_lazyCOutIdx)
                        for (int c=0 ; c<4 ; ++c)
                            if (nstage.m_color[c] == CC_LAZY)
                                nstage.m_color[c] = TevColorArg(CC_C0 + picked * 2);
                }
            }

            if (stage.m_aRegOut == TEVLAZY)
            {
                int picked = pickALazy(diag, stage.m_loc, i);
                stage.m_aRegOut = TevRegID(TEVREG0 + picked);
                for (int j=i+1 ; j<int(m_tevCount) ; ++j)
                {
                    TEVStage& nstage = m_tevs[j];
                    if (nstage.m_lazyAInIdx == stage.m_lazyAOutIdx)
                        for (int c=0 ; c<4 ; ++c)
                            if (nstage.m_alpha[c] == CA_LAZY)
                                nstage.m_alpha[c] = TevAlphaArg(CA_A0 + picked);
                }
            }
        }
    }
}

}
