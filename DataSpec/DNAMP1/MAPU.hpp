#ifndef __DNAMP1_MAPU_HPP__
#define __DNAMP1_MAPU_HPP__

#include <vector>

#include "../DNACommon/PAK.hpp"
#include "../DNACommon/MAPU.hpp"
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

#endif
