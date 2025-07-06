#pragma once

#include <string>

struct Vec3 {
    float x, y, z;

    std::string toString();

    Vec3 operator+( const Vec3& other );
    Vec3 operator-( const Vec3& other );
    Vec3 operator*( float scalar );
    Vec3 operator/( float scalar );

    Vec3& operator+=( const Vec3& other );
    Vec3& operator-=( const Vec3& other );
    Vec3& operator*=( float scalar );
    Vec3& operator/=( float scalar );

    Vec3 cross( const Vec3& other );
    float dot( const Vec3& other );

    float length();
    float lengthSquared();

    // Returns a normalized *copy* of this vector.
    Vec3 normalize();

    // Keeps the component of this vector that is perpendicular to the given axis.
    Vec3 rejection(Vec3 axis);

    static Vec3 lerp( Vec3& a, Vec3& b, float t );
    Vec3 lerp( Vec3& other, float t );
};

struct Vec3i {
    int32_t x, y, z;

    Vec3i operator+( const Vec3i& other );
    Vec3i operator-( const Vec3i& other );
    Vec3i operator*( int32_t scalar );
    Vec3i operator/( int32_t scalar );

    Vec3i& operator+=( const Vec3i& other );
    Vec3i& operator-=( const Vec3i& other );
    Vec3i& operator*=( int32_t scalar );
    Vec3i& operator/=( int32_t scalar );

    float length();

    Vec3 toVec3();
};

struct Vec4 {
    float x, y, z, w;

    Vec4 operator+( const Vec4& other );
    Vec4 operator-( const Vec4& other );
    Vec4 operator*( float scalar );
    Vec4 operator/( float scalar );

    Vec4& operator+=( const Vec4& other );
    Vec4& operator-=( const Vec4& other );
    Vec4& operator*=( float scalar );
    Vec4& operator/=( float scalar );

    float dot( const Vec4& other );
};

struct Matrix4 {
    union {
        float m[16]; // Column-major order
        struct {
            Vec4 x, y, z, w; 
        } columns;
    };

    Matrix4 operator*( const Matrix4& other );
    Matrix4 operator*( float scalar );
    Matrix4 operator/( float scalar );
    Matrix4& operator*=( const Matrix4& other );
    Matrix4& operator*=( float scalar );
    Matrix4& operator/=( float scalar );

    Vec3 transform( Vec3 v, float w = 0.0f );
    Vec4 transform( Vec4 v );

    static Matrix4 identity();
    static Matrix4 translation( Vec3 translation );
    static Matrix4 scale( Vec3 scale );

    Matrix4 inverse();
};

struct Quaternion {
    float x, y, z, w;

    Quaternion operator+( Quaternion other );
    Quaternion operator-( Quaternion other );
    Quaternion operator*( Quaternion other );
    Quaternion operator/( Quaternion other );
    Quaternion operator*( float scalar );
    Quaternion operator/( float scalar );
    Quaternion& operator+=( Quaternion other );
    Quaternion& operator-=( Quaternion other );
    Quaternion& operator*=( Quaternion other );
    Quaternion& operator/=( Quaternion other );
    Quaternion& operator*=( float scalar );
    Quaternion& operator/=( float scalar );

    float lengthSquared();
    float length();
    float dot( Quaternion other );
    Quaternion conjugate();
    Quaternion normalize();
    Quaternion nlerp( Quaternion& other, float t, bool shortestPath = true );
    Quaternion pow( float exponent );
};

struct Camera {
    Vec3 pos, fwd, up;
    float fov, width, height;
    bool verticalFov;
    Vec3 left();
    Vec3 project(Vec3 p);
};
