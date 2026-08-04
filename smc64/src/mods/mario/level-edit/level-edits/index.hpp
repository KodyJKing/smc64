#pragma once
#include "../MarioLevelEdit.hpp"
#include <unordered_map>

namespace Mod::Mario::LevelEdit::Index {

    // All level edits, keyed by BSP signature (Engine::getBSPSignature()).
    // To add a new level: run the game on that level, read the signature from
    // Engine::getBSPSignature(), and add an entry here.
    inline std::unordered_map<uint64_t, LevelEdits> s_allEdits = {
        // Truth and Reconciliation
        { 0xBE7D47F74FA95FA9ULL, LevelEdits {
            .orientedBoundingBoxes = {
                { { -16.0604f, 10.1049f, -20.2722f }, { 0.48826f, 0.450034f, 2.02474f }, { 0.f, 1.24471e-08f, 90.f } },
                { { -7.05784f, 10.1049f, -20.2722f }, { 0.48826f, 0.450034f, 2.02474f }, { 0.f, 0.f, 90.f } },
            }
        }},
        { 0xBC69F749A8E8752EULL, LevelEdits {
            .orientedBoundingBoxes = {
                { { -0.713277f, -70.9869f, -17.7928f }, { 0.4345f, 0.436288f, 2.00411f }, { -0.f, 0.f, 90.f } },
                { { 6.18326f, -70.9683f, -17.7928f }, { 0.4345f, 0.436288f, 2.00411f }, { -0.f, 0.f, 90.f } },
                { { -0.68158f, -57.5593f, -17.7928f }, { 0.4345f, 0.436288f, 2.00411f }, { -0.f, 0.f, 90.f } },
                { { -0.713277f, -60.5718f, -17.7928f }, { 0.4345f, 0.436288f, 2.00411f }, { -0.f, 0.f, 90.f } },
                { { 6.18326f, -57.5861f, -17.7928f }, { 0.4345f, 0.436288f, 2.00411f }, { -0.f, 0.f, 90.f } },
                { { 6.18326f, -60.5789f, -17.7928f }, { 0.4345f, 0.436288f, 2.00411f }, { -0.f, 0.f, 90.f } },
            }
        }},

        // Keyes:
        { 0x5B4FEA6A0330002AULL, LevelEdits {
            .orientedBoundingBoxes = {
                // Platform under coolant leak drop so Mario doesn't get blocked by OOB protection.
                { { 53.3375f, -90.9193f, -84.5287f }, { 7.16046f, 5.90961f, 0.5f }, { -0.f, -0.f, 90.f } },
            }
        }},
        { 0x6C8D75E7AE85FFA4ULL, LevelEdits {
            .orientedBoundingBoxes = {
                { { -7.07616f, 10.0843f, -70.0366f }, { 0.438821f, 0.458264f, 1.76945f }, { 0.f, 0.f, 90.f } },
                { { -16.0532f, 10.0843f, -70.0366f }, { 0.438821f, 0.458264f, 1.76945f }, { 0.f, 0.f, 90.f } },
                { { -21.5476f, -10.9504f, -70.2896f }, { 0.479384f, 0.464243f, 3.50738f }, { -0.f, 0.f, 90.f } },
                { { -21.5123f, -14.7834f, -70.2896f }, { 0.479384f, 0.464243f, 3.50738f }, { -0.f, 0.f, 90.f } },
                { { -29.64f, -14.7834f, -70.2896f }, { 0.479384f, 0.464243f, 3.50738f }, { -0.f, 0.f, 90.f } },
                { { -29.64f, -10.9481f, -70.2896f }, { 0.479384f, 0.464243f, 3.50738f }, { -0.f, 0.f, 90.f } },
                { { -29.64f, -7.07572f, -70.2896f }, { 0.479384f, 0.464243f, 3.50738f }, { -0.f, 0.f, 90.f } },
                { { -24.8054f, -17.3952f, -70.2896f }, { 0.479384f, 0.464243f, 3.50738f }, { -0.f, 0.f, 90.f } },
                { { -21.5476f, -7.07572f, -70.2896f }, { 0.479384f, 0.464243f, 3.50738f }, { -0.f, 0.f, 90.f } },
                { { -24.8054f, -4.43223f, -70.2896f }, { 0.479384f, 0.464243f, 3.50738f }, { -0.f, 0.f, 90.f } },
            }
        }},
        { 0xB1ED63741BEDD972ULL, LevelEdits {
            .orientedBoundingBoxes = {
                { { -7.56704f, -77.2342f, -77.4396f }, { 0.489415f, 0.483158f, 5.3573f }, { -0.f, 0.f, 0.f } },
                { { -11.5974f, -79.5614f, -77.4396f }, { 0.410551f, 0.438722f, 5.3573f }, { -0.f, 0.f, 90.f } },
                { { -11.5875f, -56.6736f, -77.4396f }, { 0.410551f, 0.438722f, 5.3573f }, { -0.f, 0.f, 90.f } },
                { { -7.56704f, -59.0425f, -77.4396f }, { 0.489415f, 0.501049f, 5.3573f }, { -0.f, 0.f, 0.f } },
                { { -7.56704f, -63.6387f, -77.4396f }, { 0.489415f, 0.483158f, 5.3573f }, { -0.f, 0.f, 0.f } },
                { { -7.56704f, -68.1164f, -77.4396f }, { 0.489415f, 0.483158f, 5.3573f }, { -0.f, 0.f, 0.f } },
                { { -7.56704f, -72.6166f, -77.4396f }, { 0.489415f, 0.483158f, 5.3573f }, { -0.f, 0.f, 0.f } },
                { { -16.0867f, -56.6736f, -77.4396f }, { 0.410551f, 0.438722f, 5.3573f }, { -0.f, 0.f, 90.f } },
                { { -16.0867f, -79.5614f, -77.4396f }, { 0.410551f, 0.438722f, 5.3573f }, { -0.f, 0.f, 90.f } },
            }
        }},
        { 0x3C40AA6F51DF3165ULL, LevelEdits {
            .orientedBoundingBoxes = {
                { { 20.1448f, -98.7405f, -71.6442f }, { 0.461006f, 0.474463f, 2.65274f }, { -0.f, 0.f, 90.f } },
                { { 20.0693f, -112.121f, -71.6442f }, { 0.461006f, 0.474463f, 2.65274f }, { -0.f, 0.f, 90.f } },
                { { 26.9888f, -112.121f, -71.6442f }, { 0.461006f, 0.474463f, 2.65274f }, { -0.f, 0.f, 90.f } },
                { { 20.1355f, -101.714f, -71.6442f }, { 0.461006f, 0.474463f, 2.65274f }, { -0.f, 0.f, 90.f } },
                { { 26.9888f, -98.7405f, -71.6442f }, { 0.461006f, 0.474463f, 2.65274f }, { -0.f, 0.f, 90.f } },
                { { 26.9888f, -101.714f, -71.6442f }, { 0.461006f, 0.474463f, 2.65274f }, { -0.f, 0.f, 90.f } },
            }
        }},
    };

    // Returns a mutable pointer to the LevelEdits for the given BSP signature,
    // or nullptr if no edits are registered for that level.
    inline LevelEdits* lookup(uint64_t bspSignature) {
        auto it = s_allEdits.find(bspSignature);
        return (it != s_allEdits.end()) ? &it->second : nullptr;
    }

    // Returns a mutable pointer to the LevelEdits for the given BSP signature,
    // inserting an empty entry if none exists yet.
    inline LevelEdits* openOrCreate(uint64_t bspSignature) {
        return &s_allEdits[bspSignature];
    }

}
