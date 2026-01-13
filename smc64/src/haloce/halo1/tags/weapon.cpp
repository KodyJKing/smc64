#include "weapon.hpp"
#include "../tag.hpp"

namespace Halo1 {
    WeaponProjectileData* getProjectileData( Tag* weaponTag, uint32_t projectileIndex) {
        auto data = getTagDataAs<WeaponTagData>(weaponTag);
        if ( !data ) return nullptr;
        return data->projectileData.get<WeaponProjectileData>(projectileIndex);
    }
}
