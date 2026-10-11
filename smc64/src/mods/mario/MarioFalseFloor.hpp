#pragma once

namespace Mod::Mario {
    // Keep a fake floor under Mario at all times so he never thinks unloaded parts of the world are OOB.
    void updateFalseFloor();
    void freeFalseFloor();
}
