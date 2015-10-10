#include <LogVisor/LogVisor.hpp>
#include "HECL/Backend/GX.hpp"
#include "HECL/Frontend.hpp"

static LogVisor::LogModule Log("HECL::GX");

namespace HECL
{
namespace Backend
{

unsigned GX::addKColor(const Color& color)
{
    for (unsigned i=0 ; i<m_kcolorCount ; ++i)
        if (m_kcolors[i] == color)
            return i;
    if (m_kcolorCount >= 4)
        Log.report(LogVisor::FatalError, "GX KColor overflow");
    m_kcolors[m_kcolorCount] = color;
    return m_kcolorCount++;
}

void GX::reset(const Frontend::IR& ir)
{
    m_tevCount = 0;
    m_tcgCount = 0;
    m_kcolorCount = 0;

    /* Final instruction is the root call by hecl convention */
    const Frontend::IR::Instruction& rootCall = ir.m_instructions.back();
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

    for (const Frontend::IR::Instruction& inst : ir.m_instructions)
    {
        switch (inst.m_op)
        {
        case Frontend::IR::OpCall:
        {
            const std::string& name = inst.m_call.m_name;
            if (!name.compare("ColorReg"))
            {
            }
            break;
        }
        case Frontend::IR::OpLoadImm:
        {
            addKColor(inst.m_loadImm.m_immVec);
            break;
        }
        default:
            Log.report(LogVisor::FatalError, "invalid inst op");
        }
    }
}

}
}
