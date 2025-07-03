namespace Math {
    float lerp( float start, float end, float t ) {
        return start + (end - start) * t;
    }

    float unlerp( float start, float end, float x ) {
        return (x - start) / (end - start);
    }

    float clamp( float x, float min, float max ) {
        return x < min ? min : x > max ? max : x;
    }

    float smoothstep( float edge0, float edge1, float x ) {
        x = clamp( (x - edge0) / (edge1 - edge0), 0.0f, 1.0f );
        return x * x * (3 - 2 * x);
    }
}