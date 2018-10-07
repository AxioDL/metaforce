#pragma once

#include <vector>

#include "DataSpec/DNACommon/PAK.hpp"
#include "DataSpec/DNACommon/MAPU.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{

struct MAPU : DNAMAPU::MAPU
{
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
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

