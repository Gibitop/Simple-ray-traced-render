#ifndef DISK_H
#define DISK_H

#include "plane.h"

class Disk : public Plane
{
public:
    Disk(Vec3 center, Vec3 p1, Vec3 p2, float radius, Material* mat)
        : Plane(center, p1, p2, mat)
    {
        this->center = center;
        this->radius = radius;
    }

    Vec3 center;
    float radius;

    float intersect(Vec3 origin, Vec3 ray) override
    {
        float result = Plane::intersect(origin, ray);
        if ((ray * result + origin - this->center).getLengthSquared() >= this->radius * this->radius)
            return -1;
        return result;
    }
};

#endif