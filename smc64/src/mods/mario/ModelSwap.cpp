#include "DynamicGeometry.hpp"
#include "ModelSwap.hpp"

#include "engine/halo1.hpp"
#include "./Coordinates.hpp"
#include "BSPConversion.hpp"
#include "MarioBSPChunk.hpp"
#include "Mario.hpp"
#include "functions/MoveElevatorSurfaceObject.hpp"

#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <mutex>

#include "decomp/surface_terrains.h"

namespace Mod::Mario::DynamicGeometry::ModelSwap {

    enum ModelSwapType {
        Empty,
        AABB,
        Surfaces
    };

    struct ModelSwapAABB {
        Vec3 center;
        Vec3 halfExtents;
    };

    struct ModelSwapEntry {
        ModelSwapType type;
        ModelSwapAABB aabb;
        std::vector<SM64Surface> surfaces;
    };

    struct ModelSwapKey {
        std::string tagPath;
        uint16_t boneIndex;
        bool operator==(const ModelSwapKey& other) const {
            return tagPath == other.tagPath && boneIndex == other.boneIndex;
        }
    };

    struct ModelSwapKeyHash {
        size_t operator()(const ModelSwapKey& key) const {
            size_t h = std::hash<std::string>{}(key.tagPath);
            h ^= std::hash<uint16_t>{}(key.boneIndex) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    typedef std::unordered_map<ModelSwapKey, ModelSwapEntry, ModelSwapKeyHash> ModelSwapMap;

    #define MARIO_HEIGHT 160.0f
    #define H MARIO_HEIGHT

    ModelSwapMap modelSwapMap = {
        {
            { "scenery\\c_storage\\c_storage", 0 },
            {
                ModelSwapType::AABB,
                { { 0.0f, 0.0f, 0.0f }, { H * 0.5f, H, H } }
            }
        },
        {
            { "scenery\\c_uplink\\c_uplink", 0 },
            {
                ModelSwapType::AABB,
                { { 0.0f, H * -0.7f, 0.0f }, { H * 0.5f, H, H } }
            }
        },
        {
            { "scenery\\c_field_generator\\c_field_generator", 0 },
            {
                ModelSwapType::AABB,
                { { 0.0f, H, 0.0f }, { H * 0.75f, H * 0.8f, H * 0.25F } }
            }
        },
        
        { { "vehicles\\warthog\\warthog", 8 }, { ModelSwapType::Empty, {} } },
        { { "vehicles\\warthog\\warthog", 13 }, { ModelSwapType::Empty, {} } },

        // Have not found the state that controls when this is tangible. For now, just make it completely intangible to Mario.
        { { "levels\\a50\\devices\\prison door\\prison door", 0 }, { ModelSwapType::Empty, {} } },

        // Make door of cryotube intangible so Mario doesn't get stuck in it.
        { { "levels\\a10\\devices\\cryotube\\cryotube", 0 }, { ModelSwapType::Empty, {} } },
        { { "levels\\a10\\devices\\cryotube\\cryotube", 1 }, { ModelSwapType::Empty, {} } },

        // // characters\flood_infection\flood_infection
        // { { "characters\\flood_infection\\flood_infection", 0 }, { ModelSwapType::Empty, {} } },
    };

    // Build a closed box (12 triangles) centered at `center` with the given half extents.
    // Vertices are emitted in bone-local mario space with outward-facing normals.
    static std::vector<SM64Surface> convertAABBToSM64Surfaces(const Vec3& center, const Vec3& halfExtents) {
        std::vector<SM64Surface> surfaces;
        surfaces.reserve(12);

        const float hx = halfExtents.x;
        const float hy = halfExtents.y;
        const float hz = halfExtents.z;

        // 8 corners of the box.
        auto corner = [&](float sx, float sy, float sz) -> Vec3 {
            return { center.x + sx * hx, center.y + sy * hy, center.z + sz * hz };
        };

        auto addTriangle = [&](const Vec3& a, const Vec3& b, const Vec3& c) {
            SM64Surface surface;
            surface.type    = SURFACE_DEFAULT;
            surface.force   = 0;
            surface.terrain = 0x0000;
            const Vec3 verts[3] = { a, b, c };
            for (int k = 0; k < 3; k++) {
                surface.vertices[k][0] = (int32_t) verts[k].x;
                surface.vertices[k][1] = (int32_t) verts[k].y;
                surface.vertices[k][2] = (int32_t) verts[k].z;
            }
            surfaces.push_back(surface);
        };

        // Each quad is wound CCW relative to its outward normal.
        auto addQuad = [&](const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d) {
            addTriangle(a, b, c);
            addTriangle(a, c, d);
        };

        // +X face
        addQuad(corner(+1, -1, -1), corner(+1, +1, -1), corner(+1, +1, +1), corner(+1, -1, +1));
        // -X face
        addQuad(corner(-1, -1, -1), corner(-1, -1, +1), corner(-1, +1, +1), corner(-1, +1, -1));
        // +Y face
        addQuad(corner(-1, +1, -1), corner(-1, +1, +1), corner(+1, +1, +1), corner(+1, +1, -1));
        // -Y face
        addQuad(corner(-1, -1, -1), corner(+1, -1, -1), corner(+1, -1, +1), corner(-1, -1, +1));
        // +Z face
        addQuad(corner(-1, -1, +1), corner(+1, -1, +1), corner(+1, +1, +1), corner(-1, +1, +1));
        // -Z face
        addQuad(corner(-1, -1, -1), corner(-1, +1, -1), corner(+1, +1, -1), corner(+1, -1, -1));

        return surfaces;
    }

    // Public API:

    bool hasModelSwapFor(Engine::Entity* entity, size_t boneIndex) {
        auto tag = entity->tag();
        if (!tag) return false;
        auto path = tag->getResourcePath();
        if (!path) return false;
        ModelSwapKey key{ path, (uint16_t)boneIndex };
        return modelSwapMap.find(key) != modelSwapMap.end();
    }

    void getModelSwapSurfacesFor(Engine::Entity* entity, size_t boneIndex, std::vector<SM64Surface>& outSurfaces) {
        auto tag = entity->tag();
        if (!tag) return;
        auto path = tag->getResourcePath();
        if (!path) return;
        ModelSwapKey key{ path, (uint16_t)boneIndex };
        auto it = modelSwapMap.find(key);
        if (it == modelSwapMap.end()) return;

        const ModelSwapEntry& entry = it->second;
        if (entry.type == ModelSwapType::AABB) {
            // Convert AABB to SM64 surfaces
            const ModelSwapAABB& aabb = entry.aabb;
            std::vector<SM64Surface> surfaces = convertAABBToSM64Surfaces(aabb.center, aabb.halfExtents);
            outSurfaces.insert(outSurfaces.end(), surfaces.begin(), surfaces.end());
        } else if (entry.type == ModelSwapType::Surfaces) {
            outSurfaces.insert(outSurfaces.end(), entry.surfaces.begin(), entry.surfaces.end());
        } else if (entry.type == ModelSwapType::Empty) {
            // Do nothing, empty model swap
        }
    }

};
