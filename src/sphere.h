#ifndef SPHERE_H
#define SPHERE_H

#include "renderable.h"

class Sphere : public Renderable
{
  public:
    Sphere(Vec3 center, float radius, Material *mat) : Renderable(mat)
    {
        this->center = center;
        this->radius = radius;
    }

    Vec3 center;
    float radius;

    float intersect(Vec3 origin, Vec3 ray) override
    {
        ray = ray.normalized();

        float originMinCenterLengthSq = (origin - this->center).getLengthSquared();
        float rayDotOriginMinCenter = ray.dot(origin - this->center);
        float root = rayDotOriginMinCenter * rayDotOriginMinCenter -
                     (originMinCenterLengthSq - this->radius * this->radius);

        if (root < 0)
            return -1;
        else
        {
            float left = rayDotOriginMinCenter * -1;
            if (root == 0)
                return left;
            else
            {
                float res1 = left + sqrt(root);
                float res2 = left - sqrt(root);
                return res1 < res2 ? res1 : res2;
            }
        }
    }

    Vec3 getNormal(Vec3 point) override
    {
        return (point - this->center).normalized();
    }
};

#endif
