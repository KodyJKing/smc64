// This module handles tracking timescale based on player activity.
// It should probably should be renamed to something like ActivityTracker.

#include "Halo1.hpp"
#include "math/Math.hpp"
#include "math/Vectors.hpp"
#include <iostream>
#include <Windows.h> // Todo: Replace windows APIs and types.
#include <stdint.h>

using namespace Halo1;
using namespace Math;

namespace TimeScale {

    static const float walkingSpeed = 0.07f;

    // minTimeScale is a bit of a misnomer since it goes through a smoothstep.
    // static float minTimescale = 0.05f;
    static float minTimescale = 0.1f; 
    static float activityDecayRate = 0.2f;

    static float rotationActivityCoefficient = 100.0f;
    static float rotationSpeedSmoothing = 0.75f;
    static float maxTimescaleDueToTurning = 0.25f;

    static uint32_t unpauseAfterFireMilis = 40;
    static uint32_t unpauseReloadMilis = 1500;
    static uint32_t unpauseMeleeMilis = 400; // 250;
    static uint32_t unpauseWeaponSwapMilis = 250;

    float timescale;
    static float activityLevel;
    static uint64_t playerIsActingUntil = 0;
    static Vec3 previousLook;
    static float lookSpeed, lookSpeedSmoothed;

    // Todo: Switch from using GetTickCount64 to using in-game ticks.
    void unpauseForNMilis( uint64_t milis ) {
        auto t = GetTickCount64() + milis;
        if ( t > playerIsActingUntil )
            playerIsActingUntil = t;
    }

    void init() {
        // Todo: Load settings from json.
        timescale = minTimescale;
    }

    void updateLookSpeed() {
        auto pCam = getPlayerCameraPointer();
        float sinRot = previousLook.cross( pCam->fwd ).length();
        lookSpeed = asinf( sinRot );
        lookSpeedSmoothed = lerp( lookSpeedSmoothed, lookSpeed, 1.0f - rotationSpeedSmoothing );
        previousLook = pCam->fwd;
    }

    // How many times longer than default to unpause when player fires this weapon.
    float weaponUnpauseMultiplier(uint32_t weaponHandle) {
        auto weaponRec = getEntityRecord( weaponHandle );
        auto weapon = getEntityPointer( weaponRec );
        if ( !weapon ) return 1.0f;
        if ( weapon->fromResourcePath( "weapons\\sniper rifle\\sniper rifle" ) ) return 7.5f;
        return 1.0f;
    }

    bool isActing( Entity* pPlayer ) {

        static uint8_t old_weaponAim;
        static uint16_t old_clipAmmo;
        static uint32_t old_weaponHandle;
        static float old_plasmaUsed, old_plasmaCharge;

        bool isThrowingGrenade = pPlayer->animId == 0xBC && pPlayer->animFrame < 18;

        auto weaponHandle = pPlayer->childHandle;
        auto weaponRec = getEntityRecord( weaponHandle );
        auto weapon = getEntityPointer( weaponRec );

        // Check if firing.
        if ( weapon ) {
            auto clipAmmo = weapon->clipAmmo;
            auto plasmaUsed = weapon->plasmaUsed;
            if ( weaponHandle != old_weaponHandle ) {
                unpauseForNMilis( unpauseWeaponSwapMilis );
            } else if ( plasmaUsed > old_plasmaUsed || clipAmmo < old_clipAmmo ) {
                uint64_t milis = (uint64_t) ((float) unpauseAfterFireMilis * weaponUnpauseMultiplier( weaponHandle ));
                // std::cout << "Weapon fired. Unpausing for " << milis << " milis." << std::endl;
                unpauseForNMilis( milis );
            }
            old_clipAmmo = clipAmmo;
            old_plasmaUsed = plasmaUsed;
            old_weaponHandle = weaponHandle;
        }

        auto weaponAnim = pPlayer->weaponAnim;
        if ( weaponAnim != old_weaponAim ) {
            if ( isReloading( pPlayer ) )
                unpauseForNMilis( unpauseReloadMilis );
            // if ( isDoingMelee( pPlayer ) ) // This doesn't work, player can spam melee and only trigger this once.
            //     unpauseForNMilis( unpauseMeleeMilis );
        }
        old_weaponAim = weaponAnim;

        uint64_t now = GetTickCount64();
        bool isActing = playerIsActingUntil > now || isThrowingGrenade;

        auto playerController = Halo1::getPlayerControllerPointer();
        if ( playerController ) {
            if (playerController->actions & PlayerActionFlags::melee) {
                unpauseForNMilis( unpauseMeleeMilis );
            }
            if (pPlayer->parentHandle != NULL_HANDLE) {
                bool walkX = fabs( playerController->walkX ) > 0.01f;
                bool walkY = fabs( playerController->walkY ) > 0.01f;
                bool shooting = playerController->actions & PlayerActionFlags::shoot;
                bool altShooting = playerController->actions & PlayerActionFlags::grenade1;
                isActing = isActing || walkX || walkY || shooting || altShooting;
            }
        }

        // Check if charging plasma bolt.
        if ( weapon ) {
            float plasmaCharge = weapon->plasmaCharge;
            if ( plasmaCharge > old_plasmaCharge )
                isActing = true;
            old_plasmaCharge = plasmaCharge;
        }

        return isActing;

    }

    void update() {
        EntityRecord* playerRec = getEntityRecord( getPlayerHandle() );
        Entity* pPlayer = getEntityPointer( playerRec );
        if ( !pPlayer )
            return;

        float targetActivityLevel = isActing( pPlayer ) ? 1.0f : 0.0f;

        activityLevel = lerp( activityLevel, targetActivityLevel, activityDecayRate );

        float playerSpeed = pPlayer->vel.length();
        float speedLevel = playerSpeed / walkingSpeed;

        updateLookSpeed();
        float lookSpeedLevel = smoothstep( 0.0f, 1.0f, lookSpeedSmoothed * rotationActivityCoefficient ) * maxTimescaleDueToTurning;

        float netLevel = activityLevel + minTimescale;
        if ( pPlayer->parentHandle == NULL_HANDLE ) {
            netLevel += speedLevel + lookSpeedLevel;
        } else {
            auto vehicle = getEntityPointer( pPlayer->parentHandle );
            if ( !vehicle->fromResourcePath( "vehicles\\warthog\\warthog" ) )
                netLevel += lookSpeedLevel;
        }

        if ( isRidingTransport( pPlayer ) )
            timescale = 1.0f;
        else
            timescale = smoothstep( 0.0f, 1.0f, netLevel );
    }

}