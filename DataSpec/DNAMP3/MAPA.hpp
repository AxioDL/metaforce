#ifndef __DNAMP3_MAPA_HPP__
#define __DNAMP3_MAPA_HPP__

#include "DataSpec/DNACommon/PAK.hpp"
#include "DataSpec/DNACommon/MAPA.hpp"
#include "DNAMP3.hpp"

namespace DataSpec::DNAMP3
{
struct MAPA : DNAMAPA::MAPA
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
        MAPA mapa;
        mapa.read(rs);
        hecl::blender::Connection& conn = btok.getBlenderConnection();
        return DNAMAPA::ReadMAPAToBlender(conn, mapa, outPath, pakRouter, entry, force);
    }

    static bool Cook(const hecl::blender::MapArea& mapa, const hecl::ProjectPath& out)
    {
        return DNAMAPA::Cook<MAPA>(mapa, out);
    }

    static uint32_t Version() { return 5; }
    using Header = DNAMAPA::MAPA::HeaderMP3;
    using MappableObject = DNAMAPA::MAPA::MappableObjectMP3;
};

}

#endif
