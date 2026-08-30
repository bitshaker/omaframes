#include "BuiltInPacks.hpp"

#include <array>

#include "critter/CritterPack.hpp"
#include "vines/VinePack.hpp"

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
    };

    return PACKS;
}
}
