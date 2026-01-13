
#include "halo1.hpp"
namespace Halo1 {
    bool isGameLoaded() { return dllBase() && isMapLoaded() && Halo1::isCameraLoaded() && areEntitiesLoaded(); }
}
