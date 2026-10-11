#include <vector>
#include <unordered_set>
#include "engine/halo1.hpp"
#include "Coordinates.hpp"
#include "decomp/surface_terrains.h"
#include "libsm64.h"
#include "functions/ClipTriangle.hpp"

#define BAN_PLANES 1

#ifdef BAN_PLANES
// Experimental collision improvement by hand banning certain problem plane-indices.
namespace {
    #define KEY(index, signature)  ((int64_t)(index) ^ static_cast<int64_t>(signature))

    std::unordered_set<int64_t> bannedPlanes = {
        // Keyes:
        KEY(0x089D, 0x5B4FEA6A0330002A),
        KEY(0x0BAE, 0x5B4FEA6A0330002A),
        KEY(0x0BAD, 0x5B4FEA6A0330002A),
        KEY(0x089F, 0x5B4FEA6A0330002A),

        KEY(0x16CF, 0x6C8D75E7AE85FFA4),
        KEY(0x0FDC, 0x6C8D75E7AE85FFA4),
        KEY(0x16C5, 0x6C8D75E7AE85FFA4),

        KEY(0x170A, 0xB1ED63741BEDD972),
        KEY(0x1890, 0xB1ED63741BEDD972),
        KEY(0x0E9C, 0xB1ED63741BEDD972),
        KEY(0x0F7A, 0xB1ED63741BEDD972),
        KEY(0x0F5F, 0xB1ED63741BEDD972),
        KEY(0x0F6B, 0xB1ED63741BEDD972),
        KEY(0x0F68, 0xB1ED63741BEDD972),
        KEY(0x0F5E, 0xB1ED63741BEDD972),
        KEY(0x0F84, 0xB1ED63741BEDD972),

        KEY(0x05A1, 0x3C40AA6F51DF3165),
    };

    bool isPlaneBanned(int32_t planeIndex, int64_t signature) {
        int64_t key = KEY(planeIndex, signature);
        return bannedPlanes.find(key) != bannedPlanes.end();
    }
}
#endif // BAN_PLANES

#define TWO_SIDED_MARGIN 10.0f

namespace Mod::Mario::BSPConversion {

    std::vector<SM64Surface> convertBSP(Engine::CollisionBSP* bsp, uint16_t defaultSurfaceType) {
        std::vector<SM64Surface> result;

        uint32_t bspVertexCount = bsp->vertices.count;
        Engine::BSPVertex* bspVertices = bsp->vertices.get<Engine::BSPVertex>(0);
        if (bspVertices == nullptr || bspVertexCount == 0)
            return result;

        uint32_t bspEdgeCount = bsp->edges.count;
        Engine::BSPEdge* bspEdges = bsp->edges.get<Engine::BSPEdge>(0);
        if (bspEdges == nullptr || bspEdgeCount == 0)
            return result;

        uint32_t bspSurfaceCount = bsp->surfaces.count;
        Engine::BSPSurface* bspSurfaces = bsp->surfaces.get<Engine::BSPSurface>(0);
        if (bspSurfaces == nullptr || bspSurfaceCount == 0)
            return result;

        uint32_t planeCount = bsp->planes.count;
        Engine::BSPPlane* planes = bsp->planes.get<Engine::BSPPlane>(0);
        if (planes == nullptr || planeCount == 0)
            return result;

        // For each edge, for each surface adjacent to the edge,
        // create a triangle formed by the edge and the first vertex of the surface.
        for (uint32_t i = 0; i < bspEdgeCount; i++) {
            auto edge = &bspEdges[i];
            
            uint32_t surfaces[2] = { edge->leftSurface, edge->rightSurface };
            for (uint32_t j = 0; j < 2; j++) {
                if (surfaces[j] >= bspSurfaceCount)
                    continue; // Invalid surface, skip it.
                auto surface = &bspSurfaces[surfaces[j]];
                // Get the first vertex of the first edge of the surface.
                if (surface->firstEdgeIndex >= bspEdgeCount)
                    continue; // Invalid surface, skip it.
                
                auto firstEdge = &bspEdges[surface->firstEdgeIndex];
                if (firstEdge->startVertex >= bspVertexCount)
                    continue; // Invalid edge, skip it.
                auto firstVertex = &bspVertices[firstEdge->startVertex];

                // Add triangle formed between edge and firstVertex, if it's not degenerate.
                if (edge->startVertex == firstEdge->startVertex ||
                    edge->endVertex == firstEdge->startVertex) {
                    // Degenerate, skip.
                    continue;
                }
                
                auto p0 = &bspVertices[edge->startVertex];
                auto p1 = &bspVertices[edge->endVertex];
                auto p2 = firstVertex;

                // Todo: Implement chunking to limit the max coordinate needed to be loaded into Mario's engine.
                if (!Coordinates::isHaloVectorSafe(p0->pos) ||
                    !Coordinates::isHaloVectorSafe(p1->pos) ||
                    !Coordinates::isHaloVectorSafe(p2->pos)) {
                    // Skip triangles with unsafe coordinates to avoid Mario getting launched into space.
                    continue;
                }

                auto v01 = p1->pos - p0->pos;
                auto v02 = p2->pos - p0->pos;
                auto cross = v01.cross(v02);
                auto crossMagSq = cross.lengthSquared();
                if (crossMagSq < 0.0001f) {
                    // Degenerate triangle, skip.
                    continue;
                }

                // Create a SM64Surface from the triangle.
                SM64Surface sm64Surface;
                sm64Surface.type    = defaultSurfaceType;
                sm64Surface.force   = 0;
                sm64Surface.terrain = 0x0000;

                if (surface->planeIndex >= bsp->planes.count) {
                    // Invalid plane index, skip this surface.
                    continue;
                }

                Engine::BSPPlane* plane = &planes[surface->planeIndex];
                Engine::BSPVertex* p[3] = { p0, p2, p1 };
                Vec3 normal = plane->normal * (surface->isFlipped ? -1.0f : 1.0f);
                if (cross.dot(normal) < 0) {
                    std::swap(p[1], p[2]);
                }

                Vec3 verts[3];
                for (int k = 0; k < 3; k++)
                    verts[k] = Coordinates::haloToMario(p[k]->pos);

                for (int k = 0; k < 3; k++) {
                    sm64Surface.vertices[k][0] = (int32_t) verts[k].x;
                    sm64Surface.vertices[k][1] = (int32_t) verts[k].y;
                    sm64Surface.vertices[k][2] = (int32_t) verts[k].z;
                }

                result.push_back(sm64Surface);
            }
        }

        return result;
    }

    // Convert Halo CE static world geometry into mario-space surfaces for the chunk
    // neighborhood centered on `loadedChunk` (radius=1 -> 3x3x3 chunks).
    std::vector<SM64Surface> haloGeometryToMarioForChunk(Vec3i loadedChunk, int radius) {

        // Todo: Implement LRU-cache for previously converted chunks to avoid redundant conversions.
        // Todo: Traverse Halo's BSP structure to avoid iterating over irrelevant geometry.

        #ifdef BAN_PLANES
        int64_t bspSignature = Engine::getBSPSignature();
        #endif // BAN_PLANES
        
        std::vector<SM64Surface> result;

        auto bsp = (Engine::CollisionBSP*) Engine::getBSPPointer();
        if (bsp == nullptr) return result;

        uint32_t bspVertexCount  = bsp->vertices.count;
        Engine::BSPVertex* bspVertices = bsp->vertices.get<Engine::BSPVertex>(0);
        if (bspVertices == nullptr || bspVertexCount == 0) return result;

        uint32_t bspEdgeCount = bsp->edges.count;
        Engine::BSPEdge* bspEdges = bsp->edges.get<Engine::BSPEdge>(0);
        if (bspEdges == nullptr || bspEdgeCount == 0) return result;

        uint32_t bspSurfaceCount = bsp->surfaces.count;
        Engine::BSPSurface* bspSurfaces = bsp->surfaces.get<Engine::BSPSurface>(0);
        if (bspSurfaces == nullptr || bspSurfaceCount == 0) return result;

        uint32_t planeCount = bsp->planes.count;
        Engine::BSPPlane* planes = bsp->planes.get<Engine::BSPPlane>(0);
        if (planes == nullptr || planeCount == 0) return result;

        // Neighborhood AABB in mario world space.
        Vec3 chunkOriginMario = Coordinates::marioChunkOrigin(loadedChunk);
        float halfExtent      = Coordinates::chunkHalfExtent();
        float reach           = (float) radius * Coordinates::chunkExtent + halfExtent;
        Vec3 aabbMin = { chunkOriginMario.x - reach, chunkOriginMario.y - reach, chunkOriginMario.z - reach };
        Vec3 aabbMax = { chunkOriginMario.x + reach, chunkOriginMario.y + reach, chunkOriginMario.z + reach };

        for (uint32_t i = 0; i < bspEdgeCount; i++) {
            auto edge = &bspEdges[i];

            uint32_t surfaces[2] = { edge->leftSurface, edge->rightSurface };
            for (uint32_t j = 0; j < 2; j++) {
                if (surfaces[j] >= bspSurfaceCount) continue;
                auto surface = &bspSurfaces[surfaces[j]];
                if (surface->firstEdgeIndex >= bspEdgeCount) continue;

                auto firstEdge = &bspEdges[surface->firstEdgeIndex];
                if (firstEdge->startVertex >= bspVertexCount) continue;
                auto firstVertex = &bspVertices[firstEdge->startVertex];

                if (edge->startVertex == firstEdge->startVertex ||
                    edge->endVertex   == firstEdge->startVertex) {
                    continue;
                }

                auto p0 = &bspVertices[edge->startVertex];
                auto p1 = &bspVertices[edge->endVertex];
                auto p2 = firstVertex;

                auto addTriangle = [&](Vec3 m0, Vec3 m1, Vec3 m2) {
                    Vec3 p0 = Coordinates::marioToHalo(m0);
                    Vec3 p1 = Coordinates::marioToHalo(m1);
                    Vec3 p2 = Coordinates::marioToHalo(m2);

                    auto v01 = p1 - p0;
                    auto v02 = p2 - p0;
                    auto cross = v01.cross(v02);
                    if (cross.lengthSquared() < 0.0001f) return;

                    if (surface->planeIndex >= bsp->planes.count) return;

                    #ifdef BAN_PLANES
                    if (isPlaneBanned(surface->planeIndex, bspSignature)) return;
                    #endif // BAN_PLANES

                    Engine::BSPPlane* plane = &planes[surface->planeIndex];

                    Vec3* mPtrs[3] = { &m0, &m2, &m1 };
                    Vec3 normal = plane->normal * (surface->isFlipped ? -1.0f : 1.0f);
                    if (cross.dot(normal) < 0) {
                        std::swap(mPtrs[1], mPtrs[2]);
                    }

                    uint16_t surfaceType = SURFACE_WALL_MISC;

                    SM64Surface sm64Surface;
                    sm64Surface.type    = surfaceType;
                    sm64Surface.force   = 0;
                    sm64Surface.terrain = 0x0000;

                    for (int k = 0; k < 3; k++) {
                        Vec3 local = { mPtrs[k]->x - chunkOriginMario.x,
                                    mPtrs[k]->y - chunkOriginMario.y,
                                    mPtrs[k]->z - chunkOriginMario.z };
                        sm64Surface.vertices[k][0] = (int32_t) local.x;
                        sm64Surface.vertices[k][1] = (int32_t) local.y;
                        sm64Surface.vertices[k][2] = (int32_t) local.z;
                    }

                    result.push_back(sm64Surface);

                    if (surface->bits.twoSided) {
                        // Create a second triangle with the opposite winding order.
                        SM64Surface sm64Surface2;
                        sm64Surface2.type    = surfaceType;
                        sm64Surface2.force   = 0;
                        sm64Surface2.terrain = 0x0000;

                        Vec3 marioNormal = Coordinates::haloToMario(normal).normalize();

                        for (int k = 0; k < 3; k++) {
                            Vec3 local = { mPtrs[2-k]->x - chunkOriginMario.x - marioNormal.x * TWO_SIDED_MARGIN,
                                        mPtrs[2-k]->y - chunkOriginMario.y - marioNormal.y * TWO_SIDED_MARGIN,
                                        mPtrs[2-k]->z - chunkOriginMario.z - marioNormal.z * TWO_SIDED_MARGIN };
                            sm64Surface2.vertices[k][0] = (int32_t) local.x;
                            sm64Surface2.vertices[k][1] = (int32_t) local.y;
                            sm64Surface2.vertices[k][2] = (int32_t) local.z;
                        }

                        result.push_back(sm64Surface2);
                    }
                };

                auto addTriangleClipped = [&](Vec3 p0, Vec3 p1, Vec3 p2) {
                    auto m0 = Coordinates::haloToMario(p0);
                    auto m1 = Coordinates::haloToMario(p1);
                    auto m2 = Coordinates::haloToMario(p2);

                    ClippedTriangle clipped;
                    Triangle tri = { m0, m1, m2 };
                    BoundingBox bbox = { aabbMin, aabbMax };
                    clipTriangle(tri, bbox, clipped);
                    
                    for (unsigned int i = 0; i < clipped.vertexCount; i += 3) {
                        addTriangle(
                            clipped.vertices[i],
                            clipped.vertices[i + 1],
                            clipped.vertices[i + 2]
                        );
                    }
                };

                addTriangleClipped(p0->pos, p1->pos, p2->pos);
            }
        }

        return result;
    }

}
