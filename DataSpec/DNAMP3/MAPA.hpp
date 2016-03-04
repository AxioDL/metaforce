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
                        std::function<void(const hecl::SystemChar*)> fileChanged)
    {
        MAPA mapa;
        mapa.read(rs);
        hecl::BlenderConnection& conn = hecl::BlenderConnection::SharedConnection();
        return DNAMAPA::ReadMAPAToBlender(conn, mapa, outPath, pakRouter, entry, force);
    }
};

}
}

#endif
