#include "Metaforce/Settings.hpp"

#include <borealis/config_codec.hpp>

namespace metaforce {
namespace {
Settings sSettings{
    .video =
        {
            .fullscreen{"video.fullscreen", false},
            .lockAspectRatio{"video.lockAspectRatio", false},
        },
    .ui =
        {
            .scale{"ui.scale", 100, {.min = 50, .max = 200}},
            .sounds{"ui.sounds", true},
        },
    .demo =
        {
            .scanVisor{"demo.scanVisor", true, {.persist = false}},
            .hintSystem{"demo.hintSystem", true, {.persist = false}},
            .hardMode{"demo.hardMode", false, {.persist = false}},
            .energyTanks{"demo.energyTanks", 6, {.min = 0, .max = 14, .persist = false}},
            .visorOpacity{"demo.visorOpacity", 100, {.min = 0, .max = 100, .persist = false}},
            .saveName{"demo.saveName", "Samus", {.persist = false}},
            .suit{"demo.suit", 0, {.min = 0, .max = 3, .persist = false}},
            .beam{"demo.beam", 0, {.min = 0, .max = 3, .persist = false}},
            .upgrades{"demo.upgrades", 0b0011, {.persist = false}},
            .logbookEntry{"demo.logbookEntry", -1, {.min = -1, .max = 13, .persist = false}},
        },
};
} // namespace

Settings& GetSettings() { return sSettings; }

} // namespace metaforce
