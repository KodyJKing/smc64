#pragma once

namespace HaloCE::Mod {

    inline struct Settings {
        // Scale slow motion by shields.
        bool panicMode = false;
        // Scale slow motion by "adrenaline" value, acquired by killing enemies.
        bool adrenalineMode = false;

        float timeScale = 0.1f;
        bool enableTimeScale = true;
        bool overrideTimeScale = false;
        bool poseInterpolation = true;
        bool timescaleDeadzoning = false;
        
        float playerDamageScale = 3.0f;
        float npcDamageScale = 2.0f;
    } settings = {};

    namespace Adrenaline {
        extern float adrenaline;
    }

    void init();
    void free();
    void modThreadUpdate();
    float getGlobalTimeScale();
}