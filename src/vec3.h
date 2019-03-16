#ifndef VEC3_H
#define VEC3_H

#include <math.h>

class Vec3
{
  public:
    Vec3() {}
    Vec3(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    float x = 1, y = 1, z = 1;

    inline float getLength()
    {
        return sqrt(getLengthSquared());
    }

    inline float getLengthSquared()
    {
        return x * x + y * y + z * z;
    }

    inline bool isNull()
    {
        return this->x == 0 &&
               this->y == 0 &&
               this->z == 0;
    }

    inline Vec3 normalized()
    {
        if (isNull())
            return *this;
        float len = this->getLength();
        return Vec3(
            this->x / len,
            this->y / len,
            this->z / len);
    }

    inline Vec3 operator-(Vec3 other)
    {
        return Vec3(
            this->x - other.x,
            this->y - other.y,
            this->z - other.z);
    }

    inline Vec3 operator+(Vec3 other)
    {
        return Vec3(
            this->x + other.x,
            this->y + other.y,
            this->z + other.z);
    }

    inline void operator+=(Vec3 other)
    {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;
    }

    Vec3 operator*(float other)
    {
        return Vec3(
            this->x * other,
            this->y * other,
            this->z * other);
    }

    Vec3 operator*(Vec3 other)
    {
        return Vec3(
            this->x * other.x,
            this->y * other.y,
            this->z * other.z);
    }

    void operator*=(Vec3 other)
    {
        this->x *= other.x;
        this->y *= other.y;
        this->z *= other.z;
    }

    inline float dot(Vec3 other)
    {
        return this->x * other.x +
               this->y * other.y +
               this->z * other.z;
    }

    inline Vec3 cross(Vec3 other)
    {
        return Vec3(
            this->y * other.z - this->z * other.y,
            this->z * other.x - this->x * other.z,
            this->x * other.y - this->y * other.x);
    }

    inline float highest()
    {
        if (this->x > this->y)
            return x > z ? x : z;
        else
            return y > z ? y : z;
    }
};

#endif