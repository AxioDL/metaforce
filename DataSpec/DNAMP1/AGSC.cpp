#include "AGSC.hpp"
#include "amuse/AudioGroup.hpp"
#include "amuse/AudioGroupData.hpp"

extern "C" const uint8_t Atomic_H[];
extern "C" const uint8_t BetaBeetle_H[];
extern "C" const uint8_t Bird_H[];
extern "C" const uint8_t BloodFlower_H[];
extern "C" const uint8_t Burrower_H[];
extern "C" const uint8_t ChozoGhost_H[];
extern "C" const uint8_t ChubbWeed_H[];
extern "C" const uint8_t CineBoots_H[];
extern "C" const uint8_t CineGeneral_H[];
extern "C" const uint8_t CineGun_H[];
extern "C" const uint8_t CineMorphball_H[];
extern "C" const uint8_t CineSuit_H[];
extern "C" const uint8_t CineVisor_H[];
extern "C" const uint8_t Crater_H[];
extern "C" const uint8_t Crystallite_H[];
extern "C" const uint8_t Drones_H[];
extern "C" const uint8_t EliteSpacePirate_H[];
extern "C" const uint8_t FireFlea_H[];
extern "C" const uint8_t Flaaghra_H[];
extern "C" const uint8_t FlickerBat_H[];
extern "C" const uint8_t FlyingPirate_H[];
extern "C" const uint8_t FrontEnd_H[];
extern "C" const uint8_t GagantuanBeatle_H[];
extern "C" const uint8_t Gnats_H[];
extern "C" const uint8_t Gryzbee_H[];
extern "C" const uint8_t IceCrack_H[];
extern "C" const uint8_t IceWorld_H[];
extern "C" const uint8_t InjuredPirates_H[];
extern "C" const uint8_t IntroBoss_H[];
extern "C" const uint8_t IntroWorld_H[];
extern "C" const uint8_t JellyZap_H[];
extern "C" const uint8_t LavaWorld_H[];
extern "C" const uint8_t Magdolite_H[];
extern "C" const uint8_t Metaree_H[];
extern "C" const uint8_t MetroidPrime_H[];
extern "C" const uint8_t Metroid_H[];
extern "C" const uint8_t MinesWorld_H[];
extern "C" const uint8_t MiscSamus_H[];
extern "C" const uint8_t Misc_H[];
extern "C" const uint8_t OmegaPirate_H[];
extern "C" const uint8_t OverWorld_H[];
extern "C" const uint8_t Parasite_H[];
extern "C" const uint8_t PhazonGun_H[];
extern "C" const uint8_t Phazon_H[];
extern "C" const uint8_t PuddleSpore_H[];
extern "C" const uint8_t PuddleToad_H[];
extern "C" const uint8_t Puffer_H[];
extern "C" const uint8_t ReactorDoor_H[];
extern "C" const uint8_t Ridley_H[];
extern "C" const uint8_t Ripper_H[];
extern "C" const uint8_t RuinsWorld_H[];
extern "C" const uint8_t SamusShip_H[];
extern "C" const uint8_t Scarab_H[];
extern "C" const uint8_t Seedling_H[];
extern "C" const uint8_t SheeGoth_H[];
extern "C" const uint8_t SnakeWeed_H[];
extern "C" const uint8_t Sova_H[];
extern "C" const uint8_t SpacePirate_H[];
extern "C" const uint8_t SpankWeed_H[];
extern "C" const uint8_t Thardus_H[];
extern "C" const uint8_t TheEnd_H[];
extern "C" const uint8_t Torobyte_H[];
extern "C" const uint8_t Triclops_H[];
extern "C" const uint8_t Turret_H[];
extern "C" const uint8_t UI_H[];
extern "C" const uint8_t WarWasp_H[];
extern "C" const uint8_t Weapons_H[];
extern "C" const uint8_t ZZZ_H[];
extern "C" const uint8_t Zoomer_H[];
extern "C" const uint8_t lumigek_H[];
extern "C" const uint8_t test_H[];

namespace DataSpec::DNAMP1
{

using namespace std::literals;

static const std::pair<std::string_view, const uint8_t*> Headers[] =
{
    {"Atomic"sv, Atomic_H},
    {"BetaBeetle"sv, BetaBeetle_H},
    {"Bird"sv, Bird_H},
    {"BloodFlower"sv, BloodFlower_H},
    {"Burrower"sv, Burrower_H},
    {"ChozoGhost"sv, ChozoGhost_H},
    {"ChubbWeed"sv, ChubbWeed_H},
    {"CineBoots"sv, CineBoots_H},
    {"CineGeneral"sv, CineGeneral_H},
    {"CineGun"sv, CineGun_H},
    {"CineMorphball"sv, CineMorphball_H},
    {"CineSuit"sv, CineSuit_H},
    {"CineVisor"sv, CineVisor_H},
    {"Crater"sv, Crater_H},
    {"Crystallite"sv, Crystallite_H},
    {"Drones"sv, Drones_H},
    {"EliteSpacePirate"sv, EliteSpacePirate_H},
    {"FireFlea"sv, FireFlea_H},
    {"Flaaghra"sv, Flaaghra_H},
    {"FlickerBat"sv, FlickerBat_H},
    {"FlyingPirate"sv, FlyingPirate_H},
    {"FrontEnd"sv, FrontEnd_H},
    {"GagantuanBeatle"sv, GagantuanBeatle_H},
    {"Gnats"sv, Gnats_H},
    {"Gryzbee"sv, Gryzbee_H},
    {"IceCrack"sv, IceCrack_H},
    {"IceWorld"sv, IceWorld_H},
    {"InjuredPirates"sv, InjuredPirates_H},
    {"IntroBoss"sv, IntroBoss_H},
    {"IntroWorld"sv, IntroWorld_H},
    {"JellyZap"sv, JellyZap_H},
    {"LavaWorld"sv, LavaWorld_H},
    {"Magdolite"sv, Magdolite_H},
    {"Metaree"sv, Metaree_H},
    {"MetroidPrime"sv, MetroidPrime_H},
    {"Metroid"sv, Metroid_H},
    {"MinesWorld"sv, MinesWorld_H},
    {"MiscSamus"sv, MiscSamus_H},
    {"Misc"sv, Misc_H},
    {"OmegaPirate"sv, OmegaPirate_H},
    {"OverWorld"sv, OverWorld_H},
    {"Parasite"sv, Parasite_H},
    {"PhazonGun"sv, PhazonGun_H},
    {"Phazon"sv, Phazon_H},
    {"PuddleSpore"sv, PuddleSpore_H},
    {"PuddleToad"sv, PuddleToad_H},
    {"Puffer"sv, Puffer_H},
    {"ReactorDoor"sv, ReactorDoor_H},
    {"Ridley"sv, Ridley_H},
    {"Ripper"sv, Ripper_H},
    {"RuinsWorld"sv, RuinsWorld_H},
    {"SamusShip"sv, SamusShip_H},
    {"Scarab"sv, Scarab_H},
    {"Seedling"sv, Seedling_H},
    {"SheeGoth"sv, SheeGoth_H},
    {"SnakeWeed"sv, SnakeWeed_H},
    {"Sova"sv, Sova_H},
    {"SpacePirate"sv, SpacePirate_H},
    {"SpankWeed"sv, SpankWeed_H},
    {"Thardus"sv, Thardus_H},
    {"TheEnd"sv, TheEnd_H},
    {"Torobyte"sv, Torobyte_H},
    {"Triclops"sv, Triclops_H},
    {"Turret"sv, Turret_H},
    {"UI"sv, UI_H},
    {"WarWasp"sv, WarWasp_H},
    {"Weapons"sv, Weapons_H},
    {"ZZZ"sv, ZZZ_H},
    {"Zoomer"sv, Zoomer_H},
    {"lumigek"sv, lumigek_H},
    {"test"sv, test_H}
};

bool AGSC::Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& dir)
{
    dir.makeDirChain(true);

    Header head;
    head.read(rs);

    uint32_t poolLen = rs.readUint32Big();
    auto pool = rs.readUBytes(poolLen);

    uint32_t projLen = rs.readUint32Big();
    auto proj = rs.readUBytes(projLen);

    uint32_t sampLen = rs.readUint32Big();
    auto samp = rs.readUBytes(sampLen);

    uint32_t sdirLen = rs.readUint32Big();
    auto sdir = rs.readUBytes(sdirLen);

    amuse::AudioGroupData data(proj.get(), projLen, pool.get(), poolLen,
        sdir.get(), sdirLen, samp.get(), sampLen, amuse::GCNDataTag{});

    /* Load into amuse representation */
    amuse::ProjectDatabase projDb;
    projDb.setIdDatabases();
    amuse::AudioGroupDatabase group(data);
    group.setGroupPath(dir.getAbsolutePath());

    /* Extract samples */
    group.getSdir().extractAllCompressed(dir.getAbsolutePath(), data.getSamp());

    /* Import C headers */
    auto lastComp = dir.getLastComponentUTF8();
    auto search = std::lower_bound(std::cbegin(Headers), std::cend(Headers), lastComp,
                  [](const auto& a, const auto& b) { return a.first < b; });
    if (search != std::cend(Headers) && search->first == lastComp)
        group.importCHeader((char*)search->second);

    /* Write out project/pool */
    {
        auto projd = group.getProj().toYAML();
        athena::io::FileWriter fo(hecl::ProjectPath(dir, _S("!project.yaml")).getAbsolutePath());
        if (fo.hasError())
            return false;
        fo.writeUBytes(projd.data(), projd.size());
    }

    {
        auto poold = group.getPool().toYAML();
        athena::io::FileWriter fo(hecl::ProjectPath(dir, _S("!pool.yaml")).getAbsolutePath());
        if (fo.hasError())
            return false;
        fo.writeUBytes(poold.data(), poold.size());
    }

    return true;
}

bool AGSC::Cook(const hecl::ProjectPath& dir, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath());
    if (w.hasError())
        return false;

    Header head;
    head.audioDir = "Audio/"sv;
    head.groupName = dir.getLastComponentUTF8();
    head.write(w);

    amuse::ProjectDatabase projDb;
    projDb.setIdDatabases();
    amuse::AudioGroupDatabase group(dir.getAbsolutePath());

    auto proj = group.getProj().toGCNData(group.getPool(), group.getSdir());
    auto pool = group.getPool().toData<athena::Big>();
    auto sdirSamp = group.getSdir().toGCNData(group);

    w.writeUint32Big(pool.size());
    w.writeUBytes(pool.data(), pool.size());

    w.writeUint32Big(proj.size());
    w.writeUBytes(proj.data(), proj.size());

    w.writeUint32Big(sdirSamp.second.size());
    w.writeUBytes(sdirSamp.second.data(), sdirSamp.second.size());

    w.writeUint32Big(sdirSamp.first.size());
    w.writeUBytes(sdirSamp.first.data(), sdirSamp.first.size());

    return true;
}

}
