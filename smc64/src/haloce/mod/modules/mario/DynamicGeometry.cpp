#include "DynamicGeometry.hpp"

#include "haloce/halo1/halo1.hpp"
#include "./Coordinates.hpp"
#include "BSPConversion.hpp"

#include <unordered_map>

namespace HaloCE::Mod::Mario::DynamicGeometry {

    float allocateRange = 10.0f;
    float deallocateRange = 20.0f;

    struct ObjectEntry {
        uint32_t surfaceObjectId;
        std::vector<SM64Surface> surfaces;
    };

    std::unordered_map<Halo1::Entity*, ObjectEntry> objectMap;

    void allocateDynamicGeometryForEntity(Halo1::Entity* entity) {
        // Check if already allocated
        if (objectMap.find(entity) != objectMap.end()) return;
        
        SM64SurfaceObject surfaceObject = {};

        auto entityTag = entity->tag();
        if (entityTag == nullptr) return;
        auto collisionTag = getCollisionGeometryTag(entityTag);
        if (collisionTag == nullptr) return;
        auto collisionData = (Halo1::CollisionTagData*) collisionTag->getData();
        if (collisionData == nullptr) return;

        auto nodeCount = collisionData->collisionNodes.count;
        if (nodeCount != 1) return;
        auto node = collisionData->collisionNodes.get<Halo1::CollisionNode>(0);
        if (node == nullptr) return;
        auto bsp = node->collisionBsps.get<Halo1::CollisionBSP>(0);
        if (bsp == nullptr) return;
        
        if (entity->worldBones.count() == 0) return;
        Halo1::WorldTransform* bone = entity->worldBones.get(entity, 0);

        // We have everything we need now, create the surface object.

        auto surfaces = HaloCE::Mod::BSPConversion::convertBSP(bsp);
        surfaceObject.surfaceCount = static_cast<uint32_t>(surfaces.size());
        surfaceObject.surfaces = surfaces.data();
        
        
        // Set position and orientation
        Vec3 marioSpacePos = Coordinates::haloToMario(bone->pos);
        surfaceObject.transform.position[0] = marioSpacePos.x;
        surfaceObject.transform.position[1] = marioSpacePos.y;
        surfaceObject.transform.position[2] = marioSpacePos.z;
        //
        Vec3 eulerRotation = orientationToEulerAngles(bone->x, bone->z) * (180.0f / 3.14159265f);
        // Note: libsm64 uses the order: pitch, yaw, roll
        surfaceObject.transform.eulerRotation[0] = eulerRotation.y;
        surfaceObject.transform.eulerRotation[1] = eulerRotation.x;
        surfaceObject.transform.eulerRotation[2] = eulerRotation.z;

        uint32_t surfaceObjectId = sm64_surface_object_create(&surfaceObject);
        
        ObjectEntry entry;
        entry.surfaceObjectId = surfaceObjectId;
        entry.surfaces = std::move(surfaces);
        objectMap[entity] = std::move(entry);
    }

    void deallocateDynamicGeometryForEntity(Halo1::Entity* entity) {
        // Placeholder for deallocation logic
        auto it = objectMap.find(entity);
        if (it != objectMap.end()) {
            sm64_surface_object_delete(it->second.surfaceObjectId);
            objectMap.erase(it);
        }
    }
    
    void update(SM64MarioState& marioState) {
        
        Vec3 marioPos = Vec3{ marioState.position[0], marioState.position[1], marioState.position[2] };
        Vec3 marioWorldPos = Coordinates::marioToHalo(marioPos);
        
        Halo1::foreachEntityRecord([&](Halo1::EntityRecord* entityRecord) {
            if (!entityRecord) return;
            auto entity = entityRecord->entity();
            if (!entity) return;

            auto category = entity->entityCategory;
            if (category != Halo1::EntityCategory_Scenery) return;
            
            Vec3 entityPos = entity->pos;
            float distance = (entityPos - marioWorldPos).lengthSquared();
            if (distance < allocateRange * allocateRange) {
                allocateDynamicGeometryForEntity(entity);
            } else if (distance > deallocateRange * deallocateRange) {
                deallocateDynamicGeometryForEntity(entity);
            }
        });
    }
    
    void free() {
        // Clear all allocated dynamic geometry
        objectMap.clear();
    }

}
