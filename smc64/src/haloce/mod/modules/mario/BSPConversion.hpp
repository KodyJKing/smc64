#include <vector>

namespace HaloCE::Mod::BSPConversion {

    // Convert a Halo CE CollisionBSP to an array of SM64Surface
    std::vector<SM64Surface> convertBSP(Halo1::CollisionBSP* bsp, uint16_t defaultSurfaceType);

    // Convert Halo CE static geometry to Super Mario 64 format
    std::vector<SM64Surface> haloGeometryToMario();

}
