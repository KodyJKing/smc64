#pragma once

#define POWDER_KEG_RESOURCE_PATH "smc64\\powder_keg\\powder_keg"

namespace Mod::Mario::PowderKeg {
    void update();
    bool isKegEquiped();
    void setKegEquiped(bool active);
}
