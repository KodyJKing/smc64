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

float Vec3::lengthSquared() {
    return x * x + y * y + z * z;
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
// Vec3i

Vec3i Vec3i::operator+( const Vec3i& b ) { return Vec3i{ x + b.x, y + b.y, z + b.z }; }
Vec3i Vec3i::operator-( const Vec3i& b ) { return Vec3i{ x - b.x, y - b.y, z - b.z }; }
Vec3i Vec3i::operator*( int32_t scalar ) { return Vec3i{ x * scalar, y * scalar, z * scalar }; }
Vec3i Vec3i::operator/( int32_t scalar ) { return Vec3i{ x / scalar, y / scalar, z / scalar }; }
Vec3i& Vec3i::operator+=( const Vec3i& b ) { x += b.x; y += b.y; z += b.z; return *this; }
Vec3i& Vec3i::operator-=( const Vec3i& b ) { x -= b.x; y -= b.y; z -= b.z; return *this; }
Vec3i& Vec3i::operator*=( int32_t scalar ) { x *= scalar; y *= scalar; z *= scalar; return *this; }
Vec3i& Vec3i::operator/=( int32_t scalar ) { x /= scalar; y /= scalar; z /= scalar; return *this; }
float Vec3i::length() {
    return sqrt( static_cast<float>( x * x + y * y + z * z ) );
}
Vec3 Vec3i::toVec3() {
    return Vec3{ static_cast<float>( x ), static_cast<float>( y ), static_cast<float>( z ) };
}

////////////////////////////////////////
// Vec4

Vec4 Vec4::operator+( const Vec4& b ) { return Vec4{ x + b.x, y + b.y, z + b.z, w + b.w }; }
Vec4 Vec4::operator-( const Vec4& b ) { return Vec4{ x - b.x, y - b.y, z - b.z, w - b.w }; }
Vec4 Vec4::operator*( float scalar ) { return Vec4{ x * scalar, y * scalar, z * scalar, w * scalar }; }
Vec4 Vec4::operator/( float scalar ) { return Vec4{ x / scalar, y / scalar, z / scalar, w / scalar }; }
Vec4& Vec4::operator+=( const Vec4& b ) { x += b.x; y += b.y; z += b.z; w += b.w; return *this; }
Vec4& Vec4::operator-=( const Vec4& b ) { x -= b.x; y -= b.y; z -= b.z; w -= b.w; return *this; }
Vec4& Vec4::operator*=( float scalar ) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
Vec4& Vec4::operator/=( float scalar ) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

float Vec4::dot( const Vec4& b ) {
    return x * b.x + y * b.y + z * b.z + w * b.w;
}

////////////////////////////////////////
// Matrix4

Matrix4 Matrix4::operator*( const Matrix4& b ) {
    Matrix4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i * 4 + j] = m[i * 4 + 0] * b.m[0 * 4 + j] +
                                  m[i * 4 + 1] * b.m[1 * 4 + j] +
                                  m[i * 4 + 2] * b.m[2 * 4 + j] +
                                  m[i * 4 + 3] * b.m[3 * 4 + j];
        }
    }
    return result;
}

Matrix4 Matrix4::operator*( float scalar ) {
    Matrix4 result;
    for (int i = 0; i < 16; i++) {
        result.m[i] = m[i] * scalar;
    }
    return result;
}

Matrix4 Matrix4::operator/( float scalar ) {
    Matrix4 result;
    for (int i = 0; i < 16; i++) {
        result.m[i] = m[i] / scalar;
    }
    return result;
}

Matrix4& Matrix4::operator*=( const Matrix4& b ) {
    *this = *this * b;
    return *this;
}

Matrix4& Matrix4::operator*=( float scalar ) {
    for (int i = 0; i < 16; i++) {
        m[i] *= scalar;
    }
    return *this;
}

Matrix4& Matrix4::operator/=( float scalar ) {
    for (int i = 0; i < 16; i++) {
        m[i] /= scalar;
    }
    return *this;
}

Vec3 Matrix4::transform( Vec3 v, float w ) {
    return Vec3{
        m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * w,
        m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * w,
        m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * w
    };
}

Vec4 Matrix4::transform( Vec4 v ) {
    return Vec4{
        m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w,
        m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w,
        m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
        m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w
    };
}

Matrix4 Matrix4::identity() {
    return Matrix4{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

Matrix4 Matrix4::translation( Vec3 translation ) {
    const float x = translation.x, y = translation.y, z = translation.z;
    return Matrix4{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
    };
}

Matrix4 Matrix4::scale( Vec3 scale ) {
    const float x = scale.x, y = scale.y, z = scale.z;
    return Matrix4{
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };
}

Matrix4 Matrix4::inverse() {
    // Helper function to calculate 3x3 determinant
    auto det3x3 = [](float a11, float a12, float a13,
                     float a21, float a22, float a23,
                     float a31, float a32, float a33) -> float {
        return a11 * (a22 * a33 - a23 * a32) - 
               a12 * (a21 * a33 - a23 * a31) + 
               a13 * (a21 * a32 - a22 * a31);
    };

    // Calculate cofactors for the adjugate matrix
    float c00 = det3x3(m[5], m[6], m[7], m[9], m[10], m[11], m[13], m[14], m[15]);
    float c01 = -det3x3(m[4], m[6], m[7], m[8], m[10], m[11], m[12], m[14], m[15]);
    float c02 = det3x3(m[4], m[5], m[7], m[8], m[9], m[11], m[12], m[13], m[15]);
    float c03 = -det3x3(m[4], m[5], m[6], m[8], m[9], m[10], m[12], m[13], m[14]);

    // Calculate determinant using first row cofactors
    float det = m[0] * c00 + m[1] * c01 + m[2] * c02 + m[3] * c03;
    
    if (fabs(det) < 1e-6f) {
        // Matrix is singular, return identity matrix
        return Matrix4::identity();
    }

    float invDet = 1.0f / det;

    Matrix4 inv;
    // First row
    inv.m[0] = c00 * invDet;
    inv.m[1] = c01 * invDet;
    inv.m[2] = c02 * invDet;
    inv.m[3] = c03 * invDet;

    // Second row
    inv.m[4] = -det3x3(m[1], m[2], m[3], m[9], m[10], m[11], m[13], m[14], m[15]) * invDet;
    inv.m[5] = det3x3(m[0], m[2], m[3], m[8], m[10], m[11], m[12], m[14], m[15]) * invDet;
    inv.m[6] = -det3x3(m[0], m[1], m[3], m[8], m[9], m[11], m[12], m[13], m[15]) * invDet;
    inv.m[7] = det3x3(m[0], m[1], m[2], m[8], m[9], m[10], m[12], m[13], m[14]) * invDet;

    // Third row
    inv.m[8] = det3x3(m[1], m[2], m[3], m[5], m[6], m[7], m[13], m[14], m[15]) * invDet;
    inv.m[9] = -det3x3(m[0], m[2], m[3], m[4], m[6], m[7], m[12], m[14], m[15]) * invDet;
    inv.m[10] = det3x3(m[0], m[1], m[3], m[4], m[5], m[7], m[12], m[13], m[15]) * invDet;
    inv.m[11] = -det3x3(m[0], m[1], m[2], m[4], m[5], m[6], m[12], m[13], m[14]) * invDet;

    // Fourth row
    inv.m[12] = -det3x3(m[1], m[2], m[3], m[5], m[6], m[7], m[9], m[10], m[11]) * invDet;
    inv.m[13] = det3x3(m[0], m[2], m[3], m[4], m[6], m[7], m[8], m[10], m[11]) * invDet;
    inv.m[14] = -det3x3(m[0], m[1], m[3], m[4], m[5], m[7], m[8], m[9], m[11]) * invDet;
    inv.m[15] = det3x3(m[0], m[1], m[2], m[4], m[5], m[6], m[8], m[9], m[10]) * invDet;

    return inv;
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
