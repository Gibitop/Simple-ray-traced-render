#ifndef PLANE_H
#define PLANE_H
#include "renderable.h"

class Plane : public Renderable
{
  public:
    Plane(Vec3 p0, Vec3 p1, Vec3 p2, Material* mat) : Renderable(mat)
    {
        this->p0 = p0;
        this->p1 = p1;
        this->p2 = p2;
    }

    Vec3 p0, p1, p2;

    float intersect(Vec3 origin, Vec3 ray) override
    {
        ray = ray.normalized();
        Vec3 cross = (this->p1 - this->p0).cross(this->p2 - this->p0);
        return cross.dot(origin - this->p0) / (cross.dot((ray - origin) * -1));
    }

    Vec3 getNormal(Vec3) override
    {
        return (this->p1 - this->p0).cross(this->p2 - this->p0).normalized();
    }
};
#endif
