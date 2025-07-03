#include "Vectors.hpp"

////////////////////////////////////////
// Vec3

std::string Vec3::toString() {
    return "Vec( " + std::to_string( x ) + ", " + std::to_string( y ) + ", " + std::to_string( z ) + " )";
}

Vec3 Vec3::operator+( const Vec3& b ) { return Vec3{ x + b.x, y + b.y, z + b.z }; }
Vec3 Vec3::operator-( const Vec3& b ) { return Vec3{ x - b.x, y - b.y, z - b.z }; }
Vec3 Vec3::operator*( float scalar ) { return Vec3{ x * scalar, y * scalar, z * scalar }; }
Vec3 Vec3::operator/( float scalar ) { return Vec3{ x / scalar, y / scalar, z / scalar }; }

Vec3& Vec3::operator+=( const Vec3& b ) { x += b.x; y += b.y; z += b.z; return *this; }
Vec3& Vec3::operator-=( const Vec3& b ) { x -= b.x; y -= b.y; z -= b.z; return *this; }
Vec3& Vec3::operator*=( float scalar ) { x *= scalar; y *= scalar; z *= scalar; return *this; }
Vec3& Vec3::operator/=( float scalar ) { x /= scalar; y /= scalar; z /= scalar; return *this; }

Vec3 Vec3::cross( const Vec3& b ) {
    return Vec3{
        y * b.z - z * b.y,
        z * b.x - x * b.z,
        x * b.y - y * b.x
    };
}

float Vec3::dot( const Vec3& b ) {
    return x * b.x + y * b.y + z * b.z;
}

float Vec3::length() {
    return sqrt( x * x + y * y + z * z );
}

Vec3 Vec3::normalize() {
    float len = length();
    return Vec3{ x / len, y / len, z / len };
}

Vec3 Vec3::rejection(Vec3 axis) {
    return *this - axis * (this->dot(axis) / axis.dot(axis));
}

Vec3 Vec3::lerp( Vec3& a, Vec3& b, float t ) {
    return a + (b - a) * t;
}

Vec3 Vec3::lerp( Vec3& b, float t ) {
    return *this + (b - *this) * t;
}

////////////////////////////////////////
// Quaternion

Quaternion Quaternion::operator+( Quaternion b ) { return Quaternion{ x + b.x, y + b.y, z + b.z, w + b.w }; }
Quaternion Quaternion::operator-( Quaternion b ) { return Quaternion{ x - b.x, y - b.y, z - b.z, w - b.w }; }
Quaternion Quaternion::operator*( Quaternion b ) {
    return Quaternion{
        w * b.x + x * b.w + y * b.z - z * b.y, 
        w * b.y - x * b.z + y * b.w + z * b.x, 
        w * b.z + x * b.y - y * b.x + z * b.w,
        w * b.w - x * b.x - y * b.y - z * b.z
    };
}
Quaternion Quaternion::operator/( Quaternion b ) {
    return *this * b.conjugate() / b.lengthSquared();
}
Quaternion Quaternion::operator*( float scalar ) { return Quaternion{ x * scalar, y * scalar, z * scalar, w * scalar }; }
Quaternion Quaternion::operator/( float scalar ) { return Quaternion{ x / scalar, y / scalar, z / scalar, w / scalar }; }
Quaternion& Quaternion::operator+=( Quaternion b ) { x += b.x; y += b.y; z += b.z; w += b.w; return *this; }
Quaternion& Quaternion::operator-=( Quaternion b ) { x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this; }
Quaternion& Quaternion::operator*=( Quaternion b ) { *this = *this * b; return *this; }
Quaternion& Quaternion::operator/=( Quaternion b ) { *this = *this / b; return *this; }
Quaternion& Quaternion::operator*=( float scalar ) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
Quaternion& Quaternion::operator/=( float scalar ) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

float Quaternion::lengthSquared() { return x * x + y * y + z * z + w * w; }
float Quaternion::length() { return sqrt( x * x + y * y + z * z + w * w ); }
float Quaternion::dot( Quaternion b ) { return x * b.x + y * b.y + z * b.z + w * b.w; }
Quaternion Quaternion::normalize() { return *this / length(); }
Quaternion Quaternion::conjugate() { return Quaternion{ -x, -y, -z, w }; }

Quaternion Quaternion::nlerp( Quaternion& b, float t, bool shortestPath) {
    Quaternion c = *this;
    if (shortestPath && b.dot(c) < 0.0f)
        c *= -1.0f;
    Quaternion result = (c * (1.0f - t) + b * t);
    float len = result.length();
    if (fabs(len) < 0.0001f)
        return b;
    return result / len;
}

Quaternion Quaternion::pow( float exponent ) {
    float angle = acosf(w) * 2.0f;
    float newAngle = angle * exponent;
    float mult = sinf(newAngle / 2.0f) / sinf(angle / 2.0f);
    return Quaternion{ x * mult, y * mult, z * mult, cosf(newAngle / 2.0f) };
}

////////////////////////////////////////
// Camera

Vec3 Camera::left() {
    return up.cross( fwd ).normalize();
}

Vec3 Camera::project(Vec3 p) {
    Vec3 toP = p - pos;
    Vec3 left = this->left();

    float x = toP.dot( left );
    float y = toP.dot( up );
    float z = toP.dot( fwd );

    float scale = 1.0f / z / tanf(fov / 2);

    x *= scale;
    y *= scale;

    if (verticalFov)
        x *= height / width;
    else
        y *= width / height;

    x = (1 - x) * width / 2;
    y = (1 - y) * height / 2;
    
    return Vec3{ x, y, z };
}
