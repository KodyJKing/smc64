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

    // Returns a normalized *copy* of this vector.
    Vec3 normalize();

    // Keeps the component of this vector that is perpendicular to the given axis.
    Vec3 rejection(Vec3 axis);

    static Vec3 lerp( Vec3& a, Vec3& b, float t );
    Vec3 lerp( Vec3& other, float t );
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
