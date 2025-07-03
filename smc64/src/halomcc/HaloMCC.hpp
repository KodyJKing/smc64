#include <Windows.h>

namespace HaloMCC {
    uintptr_t mccBase();
    // True even when menu is not open. For example, when loading a map or when in the main menu.
    bool isGamePaused();
    // True only when in a map and the pause menu is open.
    bool isPauseMenuOpen();
}