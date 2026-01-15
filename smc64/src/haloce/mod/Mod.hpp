#pragma once

namespace HaloCE::Mod {

    inline struct Settings {
        bool freezeTime = false;

        float timeScale = 0.1f;
        bool enableTimeScale = false;
        bool overrideTimeScale = true;
        bool poseInterpolation = true;
        bool timescaleDeadzoning = false;
        
        float playerDamageScale = 3.0f;
        float npcDamageScale = 2.0f;
    } settings = {};

    void init();
    void free();
    void modThreadUpdate();
    float getGlobalTimeScale();
}
