#include "CPASParmInfo.hpp"

namespace urde
{

CPASParmInfo::CPASParmInfo(CInputStream& in)
{
    xc_val1.m_int = 0;
    x10_val2.m_int = 0;
    x0_type = CPASAnimParm::EParmType(in.readUint32Big());
    x4_unk1 = in.readUint32Big();
    x8_unk2 = in.readFloatBig();

    switch (x0_type)
    {
    case CPASAnimParm::EParmType::Int32:
        xc_val1.m_int = in.readInt32Big();
        x10_val2.m_int = in.readInt32Big();
        break;
    case CPASAnimParm::EParmType::UInt32:
        xc_val1.m_uint = in.readUint32Big();
        x10_val2.m_uint = in.readUint32Big();
        break;
    case CPASAnimParm::EParmType::Float:
        xc_val1.m_float = in.readFloatBig();
        x10_val2.m_float = in.readFloatBig();
        break;
    case CPASAnimParm::EParmType::Bool:
        xc_val1.m_bool = in.readBool();
        x10_val2.m_bool = in.readBool();
        break;
    case CPASAnimParm::EParmType::Enum:
        xc_val1.m_int = in.readInt32Big();
        x10_val2.m_int = in.readInt32Big();
        break;
    default: break;
    }
}

}
