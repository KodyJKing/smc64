#include "haloce/ui/DrawBSP.hpp"

#include "overlay/ESP.hpp"

namespace HaloCE::Mod::UI  {

    void drawBSPPoints(Halo1::CollisionBSP* bsp, Vec3 origin, Vec3 x, Vec3 y, Vec3 z) {
        if ( !bsp ) return;

        uint64_t vertexArrayAddress = Halo1::translateMapAddress( bsp->vertices.offset );
        if ( !vertexArrayAddress ) return;
        Halo1::BSPVertex* vertices = (Halo1::BSPVertex*) vertexArrayAddress;

        for ( uint32_t i = 0; i < bsp->vertices.count; i++ ) {
            Halo1::BSPVertex& vertex = vertices[i];
            Vec3 worldPos = x * vertex.pos.x + y * vertex.pos.y + z * vertex.pos.z + origin;
            Overlay::ESP::drawPoint( worldPos, IM_COL32( 0, 255, 255, 255 ) );
        }
    }

    // This is for small BSPs, so don't use any special culling logic.
    // In all other respects, this is like the old implementation (renderESP_BSP).
    void drawBSP(Halo1::CollisionBSP* bsp, Vec3 origin, Vec3 x, Vec3 y, Vec3 z) {
        namespace ESP = Overlay::ESP;
        Camera &camera = ESP::camera;

        auto toWorld = [&]( Vec3 localPos ) -> Vec3 {
            return x * localPos.x + y * localPos.y + z * localPos.z + origin;
        };

        auto bspVertexCount = bsp->vertices.count;
        if ( bspVertexCount == 0 ) return;
        Halo1::BSPVertex* bspVertices = (Halo1::BSPVertex*) Halo1::translateMapAddress( bsp->vertices.offset );
        if (!bspVertices ) return;

        auto bspEdgeCount = bsp->edges.count;
        if ( bspEdgeCount == 0 ) return;
        auto edgeArrayAddress = Halo1::translateMapAddress( bsp->edges.offset );
        if ( !edgeArrayAddress ) return;
        Halo1::BSPEdge* bspEdges = (Halo1::BSPEdge*) edgeArrayAddress;

        auto bspSurfaceCount = bsp->surfaces.count;
        if ( bspSurfaceCount == 0 ) return;
        Halo1::BSPSurface* bspSurfaces = (Halo1::BSPSurface*) Halo1::translateMapAddress( bsp->surfaces.offset );
        if ( !bspSurfaces ) return;

        
        uint8_t alpha = 0xFF;
        auto color = IM_COL32(255, 255, 0, alpha);
        
        // //// Draw BSP vertices
        // for (uint32_t i = 0; i < bspVertexCount; i++) {
        //     auto vertex = &bspVertices[i];
        //     Vec3 pos = vertex->pos;
        //     ESP::drawPoint(toWorld(pos), color);
        // }

        //// Draw BSP surfaces

        alpha = 0x20;
        color = IM_COL32(255, 255, 255, alpha);

        for (uint32_t i = 0; i < bspEdgeCount; i++) {
            auto edge = &bspEdges[i];

            auto p0 = &bspVertices[edge->startVertex];
            auto p1 = &bspVertices[edge->endVertex];

            uint32_t surfaces[2] = {edge->leftSurface, edge->rightSurface};
            for (uint32_t j = 0; j < 2; j++)
            {
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

                // // Draw triangle formed between edge and firstVertex, if it's not degenerate.
                if (edge->startVertex == firstEdge->startVertex ||
                    edge->endVertex == firstEdge->startVertex)
                {
                    // Degenerate, skip.
                    continue;
                }

                auto p2 = firstVertex;

                // Draw the triangle.
                ESP::drawLine(toWorld(p0->pos), toWorld(p1->pos), color);
                ESP::drawLine(toWorld(p1->pos), toWorld(p2->pos), color);
                ESP::drawLine(toWorld(p2->pos), toWorld(p0->pos), color);

                // // Render text for surface material.
                // Vec3 textPos = (p0->pos + p1->pos + p2->pos) / 3.0f;
                // auto material = surface->material;
                // char materialText[255] = {0};
                // sprintf( materialText, "%X", material );
                // ESP::drawText( textPos, materialText, color );
            }
        }
    }
    
}
