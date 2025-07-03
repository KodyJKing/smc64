// This module is responsible for interpolating entity states after an update for the purpose of time scaling.

#include "Halo1.hpp"
#include <iostream>
#include "Rewind.hpp"

using namespace Halo1;

#define REWIND_WITH_TIMESCALE(field, type, timescale) \
    { auto delta_##field = entity->field - snap.##field; \
    entity->field = snap.##field + (type)(delta_##field * timescale); }

#define REWIND_INCREASES_WITH_TIMESCALE(field, type, timescale) \
    { auto delta_##field = entity->field - snap.##field; \
    if (delta_##field > (type) 0) \
        entity->field = snap.##field + (type)(delta_##field * timescale); }

#define REWIND_DECREASES_WITH_TIMESCALE(field, type, timescale) \
    { auto delta_##field = entity->field - snap.##field; \
    if (delta_##field < (type) 0) \
        entity->field = snap.##field + (type)(delta_##field * timescale); }

#define REWIND(field, type) REWIND_WITH_TIMESCALE(field, type, timescale)
#define REWIND_INCREASES(field, type) REWIND_INCREASES_WITH_TIMESCALE(field, type, timescale)
#define REWIND_DECREASES(field, type) REWIND_DECREASES_WITH_TIMESCALE(field, type, timescale)

namespace Rewind {

    void rewindAnimFrame( Entity* entity, float timescale, Entity& snap );
    void rewindRotation( Entity* entity, float timescale, Entity& snap );
    void rewindFireCooldown( Entity* entity, float timescale, Entity& snap );

    void rewind( EntityRecord* rec, float timescale, float globalTimescale, Entity& snap ) {
            
        Entity* entity = getEntityPointer( rec );
        if ( !entity )
            return;
        
        // Position isn't updated when an entity is mounted, so don't rewind position on unmount.
        if ( entity->parentHandle == snap.parentHandle )
            REWIND( pos, Vec3 );

        REWIND( vel, Vec3 );

        // rewindAnimFrame( entity, timescale, snap );
        REWIND( ageMilis, uint32_t );

        switch ( (EntityCategory) entity->entityCategory ) {
            case EntityCategory_Biped: {
                // Shields shouldn't update when player isn't moving.
                REWIND_INCREASES_WITH_TIMESCALE( shield, float, globalTimescale );
                rewindRotation( entity, timescale, snap ); // Experimental!!!
                break;
            }
            case EntityCategory_Weapon: {
                // Weapon heat shouldn't update when player isn't moving.
                REWIND_DECREASES_WITH_TIMESCALE( heat, float, globalTimescale );
                rewindFireCooldown( entity, globalTimescale, snap );
                if (entity->parentHandle == NULL_HANDLE)
                    rewindRotation( entity, timescale, snap );
                break;
            }
            case EntityCategory_Equipment: {
                rewindRotation( entity, timescale, snap );
                break;
            }
            case EntityCategory_Projectile: {
                REWIND( projectileAge, float );
                REWIND( fuse, float );
                rewindRotation( entity, timescale, snap );

                // Do not allow these projectiles to accelerate / decelerate.
                if (
                    entity->fromResourcePath( "vehicles\\warthog\\bullet" ) ||
                    entity->fromResourcePath( "weapons\\pistol\\bullet" ) ||
                    entity->fromResourcePath( "weapons\\plasma rifle\\bolt" ) ||
                    entity->fromResourcePath( "vehicles\\scorpion\\bullet" )
                )
                    entity->vel = snap.vel;

                break;
            }
            case EntityCategory_Vehicle: {
                REWIND( angularVelocity, Vec3 );
                rewindRotation( entity, timescale, snap );
                break;
            }
            default: {
                break;
            }
        }

    }

    void rewindFireCooldown( Entity* entity, float timescale, Entity& snap ) {
        static const float timescaleThreshold = 0.6f;
        if ( timescale > timescaleThreshold )
            return;
        auto diff = entity->ticksSinceLastFired - snap.ticksSinceLastFired;
        if ( diff == 1 ) {
            entity->ticksSinceLastFired = snap.ticksSinceLastFired;
            float u = (float) rand() / RAND_MAX;
            if ( u < timescale )
                entity->ticksSinceLastFired++;
        }
    }

    void rewindAnimFrame( Entity* entity, float timescale, Entity& snap ) {
        auto dAnimFrame = entity->animFrame - snap.animFrame;
        if ( dAnimFrame > 0 ) {
            float framesToAdd = floorf( dAnimFrame * timescale );
            float lostFrames = dAnimFrame * timescale - framesToAdd;
            float u = (float) rand() / RAND_MAX;
            if ( u < lostFrames )
                framesToAdd++;
            entity->animFrame = snap.animFrame + (uint16_t) framesToAdd;
        }
    }

    void rewindRotation( Entity* entity, float timescale, Entity& snap) {
        entity->fwd = Vec3::lerp( snap.fwd, entity->fwd, timescale ).normalize();
        entity->up = Vec3::lerp( snap.up, entity->up, timescale ).rejection(entity->fwd).normalize();
    }

}
