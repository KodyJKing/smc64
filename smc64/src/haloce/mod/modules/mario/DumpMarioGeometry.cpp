#include <cstdio>
#include <string>
#include <stdint.h>
#include "libsm64.h"
#include "math/Vectors.hpp"
#include "MarioSkeleton.hpp"

#include "haloce/halo1/Halo1.hpp"
#include "Coordinates.hpp"

namespace HaloCE::Mod::Mario {

    void dumpMarioGeometry() {
        struct SM64MarioInputs marioInputs = {0};
        struct SM64MarioState marioState = {0};
        struct SM64MarioGeometryBuffers marioGeometry = {0};
        
        uint32_t marioId = sm64_mario_create(99999.0f, 99999.0f, 99999.0f);
        if (marioId < 0) {
            printf("Failed to create Mario instance for dumping geometry.\n");
            return;
        }

        // Initialize Mario geometry buffers.
        { 
            marioGeometry.position = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
            marioGeometry.color = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
            marioGeometry.normal = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
            marioGeometry.uv = (float*)malloc(sizeof(float) * 6 * SM64_GEO_MAX_TRIANGLES);
            marioGeometry.numTrianglesUsed = 0;
        }

        auto playerPos = Halo1::getPlayerPosition();
        if (playerPos.has_value()) {
            auto pos = playerPos.value();
            // Convert Halo CE coordinates to Super Mario 64 coordinates
            auto marioPos = Coordinates::haloToMario(pos);
            sm64_set_mario_position(marioId, marioPos.x, marioPos.y, marioPos.z);
            std::cout << "Set Mario position to (" << marioPos.x << ", " << marioPos.y << ", " << marioPos.z << ")\n";
        }

        sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
        sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
        sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);

        Vec3 marioPos = Vec3{
            marioState.position[0],
            marioState.position[1],
            marioState.position[2]
        };

        std::string folderPath = "C:\\code\\projects\\hacking\\smc64\\data\\";
        // Write position CSV
        {
            std::string filePath = folderPath + "mario.csv";
            FILE* f = fopen(filePath.c_str(), "w");
            if (f) {
                for (uint16_t i = 0; i < marioGeometry.numTrianglesUsed * 3; i++) {
                    float x = marioGeometry.position[i * 3 + 0] - marioPos.x;
                    float y = marioGeometry.position[i * 3 + 1] - marioPos.y;
                    float z = marioGeometry.position[i * 3 + 2] - marioPos.z;

                    float r = marioGeometry.color[i * 3 + 0];
                    float g = marioGeometry.color[i * 3 + 1];
                    float b = marioGeometry.color[i * 3 + 2];

                    float nx = marioGeometry.normal[i * 3 + 0];
                    float ny = marioGeometry.normal[i * 3 + 1];
                    float nz = marioGeometry.normal[i * 3 + 2];

                    float u = marioGeometry.uv[i * 2 + 0];
                    float v = marioGeometry.uv[i * 2 + 1];

                    fprintf(f, "%.6f,%.6f,%.6f, %.6f,%.6f,%.6f, %.6f,%.6f,%.6f, %.6f,%.6f\n",
                        x, y, z, r, g, b, nx, ny, nz, u, v
                    );
                }
                fclose(f);
            }
        }

        // Write skeleton CSV
        {
            std::string filePath = folderPath + "mario_skeleton.csv";
            FILE* f = fopen(filePath.c_str(), "w");
            if (f) {
                dumpSkeleton(marioGeometry, marioPos, f);
                fclose(f);
            }
        }

        // Free resources
        {
            free(marioGeometry.position);
            free(marioGeometry.color);
            free(marioGeometry.normal);
            free(marioGeometry.uv);
            sm64_mario_delete(marioId);
        }
    }
}
