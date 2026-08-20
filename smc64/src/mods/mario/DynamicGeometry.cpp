#include "DynamicGeometry.hpp"

#include "engine/halo1.hpp"
#include "./Coordinates.hpp"
#include "BSPConversion.hpp"
#include "ModelSwap.hpp"
#include "MarioBSPChunk.hpp"
#include "Mario.hpp"
#include "functions/MoveElevatorSurfaceObject.hpp"
#include "functions/IsElevatorEntity.hpp"

#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <mutex>

#include "decomp/surface_terrains.h"

// #define DEBUG_DYNAMIC_GEOMETRY 1

#ifdef DEBUG_DYNAMIC_GEOMETRY
    #define IMGUI_DEFINE_MATH_OPERATORS
    #include "imgui.h"
    #include "spark/overlay/ESP.hpp"
    #include <iostream>
    #define ISO_TIME_STRING std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
    #define LOG_TIME "[" << ISO_TIME_STRING << "] "
    #define LOG(x) std::cout << "[DynamicGeometry] " << LOG_TIME << x << std::endl;
#else
    #define LOG(x) ;
#endif
namespace Mod::Mario::DynamicGeometry {

    static std::mutex s_updateMutex;

    float getAllocateRange(Engine::Tag* entityTag) {
        if (isElevatorTag(entityTag)) return 100.0f;
        return 8.0f;
    }

    float getDeallocateRange(Engine::Tag* entityTag) {
        if (isElevatorTag(entityTag)) return 200.0f;
        return 10.0f;
    }


    struct EntityEntry {
        Engine::EntityHandle entityHandle;
        // ID's of -1 are a sentinel value meaning "no object allocated for this bone"
        std::vector<int64_t> surfaceObjectIds;

        #ifdef DEBUG_DYNAMIC_GEOMETRY
        char* tagPath = nullptr;
        // Raw local-space surfaces used to build each bone's surface object, kept around for debug rendering.
        std::vector<std::vector<SM64Surface>> boneSurfaces;
        #endif

        void initSurfaceObjects(size_t boneCount) {
            surfaceObjectIds.resize(boneCount, -1);
            #ifdef DEBUG_DYNAMIC_GEOMETRY
            boneSurfaces.resize(boneCount);
            #endif
        }

        bool hasSurfaceObjectForBone(size_t boneIndex) const {
            return boneIndex < surfaceObjectIds.size() && surfaceObjectIds[boneIndex] != -1;
        }

        bool setSurfaceObjectForBone(size_t boneIndex, int64_t objectId) {
            if (boneIndex >= surfaceObjectIds.size()) return false;
            surfaceObjectIds[boneIndex] = objectId;
            return true;
        }

        Engine::Entity* entity() {
            return Engine::getEntityPointer(entityHandle);
        }

        void clearSurfaceObjects() {
            for (auto& objectId : surfaceObjectIds) {
                if (objectId != -1) {
                    LOG("Deleting surface object with ID: " << objectId);
                    sm64_surface_object_delete(objectId);
                    objectId = -1;
                }
            }
            #ifdef DEBUG_DYNAMIC_GEOMETRY
            for (auto& surfaces : boneSurfaces) {
                surfaces.clear();
            }
            #endif
        }

    };

    std::unordered_map<Engine::EntityHandle, EntityEntry> objectMap;

    bool entityEntryExists(Engine::EntityHandle handle) {
        auto it = objectMap.find(handle);
        if (it != objectMap.end()) {
            return true;
        }
        return false;
    }

    EntityEntry* createEntityEntry(Engine::EntityHandle handle, size_t boneCount) {
        if (entityEntryExists(handle)) return nullptr;
        EntityEntry entry;
        entry.entityHandle = handle;
        entry.initSurfaceObjects(boneCount);

        #ifdef DEBUG_DYNAMIC_GEOMETRY
        auto entity = Engine::getEntityPointer(handle);
        if (entity) {
            auto tag = entity->tag();
            if (tag) {
                entry.tagPath = tag->getResourcePath();
            }
        }
        #endif

        objectMap[handle] = entry;
        return &objectMap[handle];
    }

    EntityEntry* getEntityEntry(Engine::EntityHandle handle) {
        auto it = objectMap.find(handle);
        if (it != objectMap.end()) {
            return &it->second;
        }
        return nullptr;
    }

    uint16_t surfaceTypeForEntity(Engine::Entity* entity) {
        auto category = entity->entityCategory;
        if (category == Engine::EntityCategory_Scenery) return SURFACE_HANGABLE;
        return SURFACE_NOT_SLIPPERY;
    }

    std::vector<SM64Surface> convertCollisionBSPToSM64Surfaces(Engine::Entity* entity, size_t boneIndex) {
        if (ModelSwap::hasModelSwapFor(entity, boneIndex)) {
            std::vector<SM64Surface> surfaces;
            ModelSwap::getModelSwapSurfacesFor(entity, boneIndex, surfaces);
            return surfaces;
        }

        auto entityTag = entity->tag();
        if (entityTag == nullptr) return {};
        auto collisionTag = getCollisionGeometryTag(entityTag);
        if (collisionTag == nullptr) return {};
        auto collisionData = (Engine::CollisionTagData*) collisionTag->getData();
        if (collisionData == nullptr) return {};

        auto nodeCount = collisionData->collisionNodes.count;
        if (nodeCount <= boneIndex) return {};
        auto node = collisionData->collisionNodes.get<Engine::CollisionNode>(boneIndex);
        if (node == nullptr) return {};

        std::vector<SM64Surface> surfaces;
        
        auto bspIndex = entity->getBspPermutation(node->permutationIndex);
        auto bspCount = node->collisionBsps.count;
        if (bspIndex >= bspCount) return surfaces;
        auto bsp = node->collisionBsps.get<Engine::CollisionBSP>(bspIndex);
        if (bsp == nullptr) return surfaces;
        uint16_t surfaceType = surfaceTypeForEntity(entity);
        auto bspSurfaces = Mod::Mario::BSPConversion::convertBSP(bsp, surfaceType);
        surfaces.insert(surfaces.end(), bspSurfaces.begin(), bspSurfaces.end());
        
        return surfaces;
    }

    void getTransform(Engine::Entity* entity, size_t boneIndex, SM64ObjectTransform& transform) {
        if (entity->worldBones.count() == 0) return;
        Engine::Transform* bone = entity->worldBones.get(entity, boneIndex);

        Vec3 marioSpacePos = Coordinates::haloToMario(bone->pos);
        Vec3 chunkOrigin = Coordinates::marioChunkOrigin(MarioBSPChunk::getLoadedChunk());
        Vec3 localPos = marioSpacePos - chunkOrigin;
        transform.position[0] = localPos.x;
        transform.position[1] = localPos.y;
        transform.position[2] = localPos.z;

        Vec3 eulerRotation = orientationToEulerAngles(bone->x, bone->z) * (180.0f / 3.14159265f);
        transform.eulerRotation[0] = eulerRotation.y;
        transform.eulerRotation[1] = eulerRotation.x;
        transform.eulerRotation[2] = eulerRotation.z;
    }

    void allocateDynamicGeometryForBone(EntityEntry& entityEntry, size_t boneIndex) {
        auto entity = entityEntry.entity();
        if (!entity) return;
        if (entityEntry.hasSurfaceObjectForBone(boneIndex)) return;
        
        SM64SurfaceObject surfaceObject = {};
        auto surfaces = convertCollisionBSPToSM64Surfaces(entity, boneIndex);
        if (surfaces.empty()) return;
        surfaceObject.surfaceCount = static_cast<uint32_t>(surfaces.size());
        surfaceObject.surfaces = surfaces.data();

        getTransform(entity, boneIndex, surfaceObject.transform);

        uint32_t surfaceObjectId = sm64_surface_object_create(&surfaceObject);
        entityEntry.setSurfaceObjectForBone(boneIndex, surfaceObjectId);

        #ifdef DEBUG_DYNAMIC_GEOMETRY
        if (boneIndex < entityEntry.boneSurfaces.size()) {
            entityEntry.boneSurfaces[boneIndex] = std::move(surfaces);
        }
        #endif
    }

    void allocateDynamicGeometryForEntity(Engine::EntityHandle entityHandle) {
        auto entity = Engine::getEntityPointer(entityHandle);
        if (!entity) return;

        auto entityEntry = createEntityEntry(entityHandle, entity->worldBones.count());
        if (!entityEntry) return;
        
        auto entityTag = entity->tag();
        if (entityTag) {
            LOG("Creating dynamic geometry for entity: " << entity << " [" << entityTag->getResourcePath() << "]");
            LOG("Entity has " << entity->worldBones.count() << " bones");
        }

        auto boneCount = entity->worldBones.count();
        for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
            allocateDynamicGeometryForBone(*entityEntry, boneIndex);
        }
    }

    void deallocateDynamicGeometryForEntity(Engine::EntityHandle entityHandle) {
        auto entry = getEntityEntry(entityHandle);
        if (!entry) return;
        LOG("Deallocating dynamic geometry for entity: " << entry->entity() << " [" << entry->tagPath << "]");
        entry->clearSurfaceObjects();
        objectMap.erase(entityHandle);
    }

    /// Should this entity be tangible this frame?
    bool entityTangible(Engine::Entity* entity) {
        if (!entity) return true;
        auto category = entity->entityCategory;
        if (category == Engine::EntityCategory_Biped) {
            if (!marioCanCollideWithBipeds()) return false;
            if (entity->health <= 0.0f) return false;
        }
        return true;
    }

    void updateObjectTransform(Engine::EntityHandle entityHandle, SM64MarioState& marioState) {
        auto entity = Engine::getEntityPointer(entityHandle);
        if (!entity) return;

        auto entry = getEntityEntry(entityHandle);
        if (!entry) return;

        bool isElevator = isElevatorEntity(entity);
        auto boneCount = entity->worldBones.count();

        for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
            auto surfaceObjectId = entry->surfaceObjectIds[boneIndex];
            if (surfaceObjectId <= -1) continue;

            SM64ObjectTransform transform;
            getTransform(entity, boneIndex, transform);

            if (!entityTangible(entity)) {
                // Ugly hack: Just teleport it really far away.
                transform.position[1] = marioState.position[1] - 10000.0f;

                // This can cause Mario to teleport with a biped as it dies. We need to change how this works.
            } 

            if (isElevator) {
                moveElevatorSurfaceObject(surfaceObjectId, transform, marioState);
            } else {
                sm64_surface_object_move(surfaceObjectId, &transform);
            }
        }
    }

    /// Should this entity ever be tangible?
    bool shouldCreateGeometryFor(Engine::Entity* entity) {
        if (!entity) return false;
        auto category = entity->entityCategory;
        if (category == Engine::EntityCategory_Scenery) return true;
        if (category == Engine::EntityCategory_Vehicle) return true;
        if (category == Engine::EntityCategory_Machine) return true;
        if (category == Engine::EntityCategory_Biped) {
            auto playerEntity = Engine::getPlayerEntity();
            if (entity == playerEntity) return false;
            return true;
        }
        return false;
    }

    Vec3 getEntityPosition(Engine::Entity* entity) {
        if (entity->worldBones.count() == 0) return entity->pos;
        Engine::Transform* bone = entity->worldBones.get(entity, 0);
        return bone ? bone->pos : entity->pos;
    }

    void cleanStale() {
        std::erase_if(objectMap, [](auto& kv) {
            if (Engine::entityHandleStale(kv.first)) {
                auto& entry = kv.second;
                entry.clearSurfaceObjects();
                return true;
            }
            return false;
        });
    }
    
    void update(SM64MarioState& marioState) {
        std::lock_guard<std::mutex> updateLock(s_updateMutex);

        Vec3 marioWorldPos = Mario::marioWorldPosition();

        cleanStale();

        Engine::foreachEntityRecordIndexed([&](Engine::EntityRecord* entityRecord, uint32_t index) {
            if (!entityRecord) return;
            auto entity = entityRecord->entity();
            if (!entity) return;

            if (!shouldCreateGeometryFor(entity)) return;

            auto entityHandle = Engine::entityHandleFromIndex(index);

            auto tag = entity->tag();
            float allocateRange = getAllocateRange(tag);
            float deallocateRange = getDeallocateRange(tag);
            
            Vec3 entityPos = getEntityPosition(entity);
            float distance = (entityPos - marioWorldPos).lengthSquared();
            if (distance < allocateRange * allocateRange) {
                bool isRegistered = entityEntryExists(entityHandle);
                if (isRegistered) {
                    updateObjectTransform(entityHandle, marioState);
                } else {
                    allocateDynamicGeometryForEntity(entityHandle);
                }
            } else if (distance > deallocateRange * deallocateRange) {
                deallocateDynamicGeometryForEntity(entityHandle);
            }
        });
    }

    void clearAll() {
        std::lock_guard<std::mutex> updateLock(s_updateMutex);
        LOG("Clearing all dynamic geometry objects (" << objectMap.size() << " entries)");
        for (auto& [key, entry] : objectMap) {
            entry.clearSurfaceObjects();
        }
        objectMap.clear();
    }

    void onLoadedChunkChanged(Vec3i /*oldChunk*/, Vec3i /*newChunk*/) {
        clearAll();
    }
    
    void free() {
        clearAll();
    }

    void debugRender() {
        #ifdef DEBUG_DYNAMIC_GEOMETRY
        namespace ESP = Spark::Overlay::ESP;

        std::lock_guard<std::mutex> updateLock(s_updateMutex);

        constexpr ImU32 kEdgeColor = IM_COL32(0, 200, 255, 20);
        constexpr ImU32 kTextColor = IM_COL32(255, 255, 255, 255);

        for (auto& [handle, entry] : objectMap) {
            auto entity = entry.entity();
            if (!entity) continue;

            auto boneCount = entry.surfaceObjectIds.size();
            for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
                if (!entry.hasSurfaceObjectForBone(boneIndex)) continue;
                if (entity->worldBones.count() <= boneIndex) continue;

                Engine::Transform* bone = entity->worldBones.get(entity, boneIndex);

                if (boneIndex < entry.boneSurfaces.size()) {
                    for (const auto& surface : entry.boneSurfaces[boneIndex]) {
                        Vec3 worldVerts[3];
                        for (int k = 0; k < 3; ++k) {
                            Vec3 boneLocal = Coordinates::marioToHalo(surface.vertices[k]);
                            worldVerts[k] = bone->transformPoint(boneLocal);
                        }
                        ESP::drawLine(worldVerts[0], worldVerts[1], kEdgeColor);
                        ESP::drawLine(worldVerts[1], worldVerts[2], kEdgeColor);
                        ESP::drawLine(worldVerts[2], worldVerts[0], kEdgeColor);
                    }
                }

                // if (entry.tagPath) {
                //     ESP::drawText(bone->pos, entry.tagPath, kTextColor);
                // }
            }
        }

        // Render a window with stats about the dynamic geometry system
        ImGui::Begin("Dynamic Geometry Debug");
        ImGui::Text("Active Entities: %d", (int) objectMap.size());

        if (ImGui::CollapsingHeader("Entities")) {
            if (ImGui::BeginTable("EntitiesTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Entity");
                ImGui::TableSetupColumn("Bone");
                ImGui::TableSetupColumn("Surface Object ID");
                ImGui::TableHeadersRow();

                for (const auto& [handle, entry] : objectMap) {
                    const char* entityName = entry.tagPath ? entry.tagPath : "<unknown>";

                    for (size_t boneIndex = 0; boneIndex < entry.surfaceObjectIds.size(); ++boneIndex) {
                        auto objectId = entry.surfaceObjectIds[boneIndex];
                        if (objectId == -1) continue;

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", entityName);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", (int) boneIndex);
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%lld", (long long) objectId);
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
        #endif
    }

}
