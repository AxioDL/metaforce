#ifndef __DNAMP3_MAPA_HPP__
#define __DNAMP3_MAPA_HPP__

#include "../DNACommon/PAK.hpp"
#include "../DNACommon/MAPA.hpp"
#include "DNAMP3.hpp"

namespace DataSpec
{
namespace DNAMP3
{
struct MAPA : DNAMAPA::MAPA
{
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        hecl::BlenderToken& btok,
                        std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        MAPA mapa;
        mapa.read(rs);
        hecl::BlenderConnection& conn = btok.getBlenderConnection();
        return DNAMAPA::ReadMAPAToBlender(conn, mapa, outPath, pakRouter, entry, force);
    }

    static bool Cook(const hecl::BlenderConnection::DataStream::MapArea& mapa, const hecl::ProjectPath& out)
    {
        return DNAMAPA::Cook<MAPA>(mapa, out);
    }

    static uint32_t Version() { return 5; }
    using Header = DNAMAPA::MAPA::HeaderMP3;
    using MappableObject = DNAMAPA::MAPA::MappableObjectMP3;
};

}
}

#endif
