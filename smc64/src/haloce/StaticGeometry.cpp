#include "libsm64.h"
#include "Halo1.hpp"

#include <vector>

namespace HaloCE::Mod::StaticGeometry {

    float const scaleFactor = 100.0f;

    // Convert Halo CE static geometry to Super Mario 64 format
    std::vector<SM64Surface> haloGeometryToMario() {

        std::vector<SM64Surface> result;

        uint32_t bspVertexCount = Halo1::getBSPVertexCount();
        Halo1::BSPVertex* bspVertices = Halo1::getBSPVertexArray();
        if (bspVertices == nullptr || bspVertexCount == 0)
            return result;

        uint32_t bspEdgeCount = Halo1::getBSPEdgeCount();
        Halo1::BSPEdge* bspEdges = Halo1::getBSPEdgeArray();
        if (bspEdges == nullptr || bspEdgeCount == 0)
            return result;

        uint32_t bspSurfaceCount = Halo1::getBSPSurfaceCount();
        Halo1::BSPSurface* bspSurfaces = Halo1::getBSPSurfaceArray();
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
                auto crossMagSq = v01.cross(v02).lengthSquared();
                if (crossMagSq < 0.01f) {
                    // Degenerate triangle, skip.
                    continue;
                }

                // Create a SM64Surface from the triangle.
                SM64Surface sm64Surface;
                sm64Surface.type = 0; // Default type
                sm64Surface.force = 0; // Default force
                sm64Surface.terrain = 0; // Default terrain

                Halo1::BSPVertex* p[3] = { p0, p1, p2 };
                for (int k = 0; k < 3; k++) {
                    sm64Surface.vertices[k][0] = (int32_t) (p[k]->pos.x * scaleFactor);
                    sm64Surface.vertices[k][1] = (int32_t) (p[k]->pos.y * scaleFactor);
                    sm64Surface.vertices[k][2] = (int32_t) (p[k]->pos.z * scaleFactor);
                }

                result.push_back(sm64Surface);
            }
        }

        return result;
    }

}