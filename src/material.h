#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"

class Material
{
  public:
    Material(Vec3 emission,
             Vec3 specular,
             float roughness)
    {
        this->emission = emission;
        this->specular = specular;
        this->roughness = roughness;
    }
    Vec3 emission, specular;
    float roughness;
};

#endif
