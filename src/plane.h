#ifndef PLANE_H
#define PLANE_H
#ifndef NO_LOG
#include <iostream>
#endif
#include "renderable.h"

class Plane : public Renderable
{
public:
    Vec3 point, normal;

    Plane(Vec3 point, Vec3 normal, Material *mat) : Renderable(mat)
    {
        this->point = point;
        this->normal = normal.normalized();
    }

    Plane(Vec3 p0, Vec3 p1, Vec3 p2, Material *mat) : Renderable(mat)
    {
        this->point = p0;
        this->normal = (p1 - p0).cross(p2 - p0).normalized();
    }

    float intersect(Vec3 origin, Vec3 ray) override
    {
        // https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
        float denominator = ray.normalized().dot(-this->normal);
        if (denominator > 1e-6)
        {
            float distance = (this->point - origin).dot(-this->normal) / denominator;
            // if (distance <= 0)
                // std::cout << "Distance <= 0" << std::endl;
            // else
                // std::cout << "intersect" << std::endl;
            return distance > 0 ? distance : -1;
        }
        // std::cout << "Denominator < 0" << std::endl;
        return -1;
    }

    Vec3 getNormal(Vec3) override
    {
        return this->normal;
    }
};
#endif
