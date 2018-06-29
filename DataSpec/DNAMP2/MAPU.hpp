#ifndef __DNAMP2_MAPU_HPP__
#define __DNAMP2_MAPU_HPP__

#include <vector>

#include "DataSpec/DNACommon/PAK.hpp"
#include "DataSpec/DNACommon/MAPU.hpp"
#include "DNAMP2.hpp"

namespace DataSpec::DNAMP2
{

struct MAPU : DNAMAPU::MAPU
{
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const DNAMP1::PAK::Entry& entry,
                        bool force,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        MAPU mapu;
        mapu.read(rs);
        hecl::blender::Connection& conn = btok.getBlenderConnection();
        return DNAMAPU::ReadMAPUToBlender(conn, mapu, outPath, pakRouter, entry, force);
    }
};
}

#endif
