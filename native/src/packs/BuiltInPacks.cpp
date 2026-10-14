#include "BuiltInPacks.hpp"

#include <array>

#include "critter/CritterPack.hpp"
#include "vines/VinePack.hpp"
#include "accents/AccentPack.hpp"

namespace OmaFrames::Packs {
std::span<const SBuiltInPack> builtInPacks() {
    static constexpr std::array PACKS{
        SBuiltInPack{
            .id             = Vines::ID,
            .registerConfig = Vines::registerConfig,
            .start          = Vines::start,
            .attach         = Vines::attach,
            .stop           = Vines::stop,
        },
        SBuiltInPack{
            .id             = Critter::ID,
            .registerConfig = Critter::registerConfig,
            .start          = Critter::start,
            .attach         = Critter::attach,
            .stop           = Critter::stop,
        },
        SBuiltInPack{
            .id = "fireflies",
            .registerConfig = [] { Accents::registerConfig(Accents::Kind::Fireflies); },
            .start = [] {},
            .attach = [](PHLWINDOW window) { Accents::attach(window, Accents::Kind::Fireflies); },
            .stop = Accents::stop,
        },
        SBuiltInPack{
            .id = "frost",
            .registerConfig = [] { Accents::registerConfig(Accents::Kind::Frost); },
            .start = [] {},
            .attach = [](PHLWINDOW window) { Accents::attach(window, Accents::Kind::Frost); },
            .stop = Accents::stop,
        },
        SBuiltInPack{
            .id = "railway",
            .registerConfig = [] { Accents::registerConfig(Accents::Kind::Railway); },
            .start = [] {},
            .attach = [](PHLWINDOW window) { Accents::attach(window, Accents::Kind::Railway); },
            .stop = Accents::stop,
        },
    };

    return PACKS;
}
}
