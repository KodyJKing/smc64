#include "libsm64.h"
#include "haloce/halo1/halo1.hpp"
#include "Coordinates.hpp"

#include <vector>

namespace HaloCE::Mod::BSPConversion {

    std::vector<SM64Surface> convertBSP(Halo1::CollisionBSP* bsp) {

        std::vector<SM64Surface> result;

        uint32_t bspVertexCount = bsp->vertices.count;
        Halo1::BSPVertex* bspVertices = bsp->vertices.get<Halo1::BSPVertex>(0);
        if (bspVertices == nullptr || bspVertexCount == 0)
            return result;

        uint32_t bspEdgeCount = bsp->edges.count;
        Halo1::BSPEdge* bspEdges = bsp->edges.get<Halo1::BSPEdge>(0);
        if (bspEdges == nullptr || bspEdgeCount == 0)
            return result;

        uint32_t bspSurfaceCount = bsp->surfaces.count;
        Halo1::BSPSurface* bspSurfaces = bsp->surfaces.get<Halo1::BSPSurface>(0);
        if (bspSurfaces == nullptr || bspSurfaceCount == 0)
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

                auto v01 = p1->pos - p0->pos;
                auto v02 = p2->pos - p0->pos;
                auto cross = v01.cross(v02);
                auto crossMagSq = cross.lengthSquared();
                if (crossMagSq < 0.01f) {
                    // Degenerate triangle, skip.
                    continue;
                }

                // Create a SM64Surface from the triangle.
                SM64Surface sm64Surface;
                sm64Surface.type    = 0x0015;
                sm64Surface.force   = 0;
                sm64Surface.terrain = 0x0000;

                if (surface->planeIndex >= Halo1::getBSPPlaneCount()) {
                    // Invalid plane index, skip this surface.
                    continue;
                }
                Halo1::BSPPlane* plane = &Halo1::getBSPPlaneArray()[surface->planeIndex];

                Halo1::BSPVertex* p[3] = { p0, p2, p1 };
                if (cross.dot(plane->normal) < 0) {
                    std::swap(p[1], p[2]);
                }

                for (int k = 0; k < 3; k++) {
                    Vec3 marioPos = Coordinates::haloToMario(p[k]->pos);
                    sm64Surface.vertices[k][0] = (int32_t) (marioPos.x);
                    sm64Surface.vertices[k][1] = (int32_t) (marioPos.y);
                    sm64Surface.vertices[k][2] = (int32_t) (marioPos.z);
                }

                result.push_back(sm64Surface);
            }
        }

        return result;
    }

    // Convert Halo CE static geometry to Super Mario 64 format
    std::vector<SM64Surface> haloGeometryToMario() {
        auto bsp = (Halo1::CollisionBSP*) Halo1::getBSPPointer();
        if (bsp == nullptr) return std::vector<SM64Surface>{};
        return convertBSP(bsp);
    }

}
