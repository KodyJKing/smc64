#include "math.hpp"
namespace Halo1 {
    Halo1::WorldTransform inverseWorldTransform(Halo1::WorldTransform& wt) {
        // Invert rotation (transpose)
        Vec3 x = Vec3{wt.x.x, wt.y.x, wt.z.x};
        Vec3 y = Vec3{wt.x.y, wt.y.y, wt.z.y};
        Vec3 z = Vec3{wt.x.z, wt.y.z, wt.z.z};

        // Invert scale
        float invScale = 1.0f / wt.w;

        // Invert translation
        Vec3 invTranslation = (x * wt.pos.x + y * wt.pos.y + z * wt.pos.z) * -invScale;

        return WorldTransform{
            invScale,
            x, y, z,
            invTranslation
        };
    }

    Halo1::WorldTransform multiplyWorldTransforms(Halo1::WorldTransform& a, Halo1::WorldTransform& b) {
        // Combine rotations
        Vec3 x = a.x * a.w * b.x.x + a.y * b.w * b.x.y + a.z * b.w * b.x.z;
        Vec3 y = a.x * b.w * b.y.x + a.y * b.w * b.y.y + a.z * b.w * b.y.z;
        Vec3 z = a.x * b.w * b.z.x + a.y * b.w * b.z.y + a.z * b.w * b.z.z;

        // Combine scales
        float w = a.w * b.w;

        // Combine translations
        Vec3 pos = a.x * (b.pos.x * w) + a.y * (b.pos.y * w) + a.z * (b.pos.z * w) + a.pos;

        return WorldTransform{
            w,
            x, y, z,
            pos
        };
    }
}
