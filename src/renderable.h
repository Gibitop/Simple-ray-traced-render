#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "vec3.h"
#include "material.h"

class Renderable
{
  public:
    Renderable(Material* mat)
    {
        this->mat = mat;
    }

    virtual float intersect(Vec3 origin, Vec3 ray) = 0;
    virtual Vec3 getNormal(Vec3) = 0;
    virtual ~Renderable(){};

    Material* mat;
};

#endif