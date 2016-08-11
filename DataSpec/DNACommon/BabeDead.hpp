#ifndef _DNACOMMON_BABEDEAD_HPP_
#define _DNACOMMON_BABEDEAD_HPP_

#include "BlenderConnection.hpp"
#include "zeus/Math.hpp"
#include <cfloat>

namespace DataSpec
{

template<class BabeDeadLight>
void ReadBabeDeadLightToBlender(hecl::BlenderConnection::PyOutStream& os,
                                const BabeDeadLight& light, unsigned s, unsigned l);

template<class BabeDeadLight>
void WriteBabeDeadLightFromBlender(BabeDeadLight& lightOut, const hecl::BlenderConnection::DataStream::Light& lightIn);

}

#endif // _DNACOMMON_BABEDEAD_HPP_
