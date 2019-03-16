#include <fstream>
#include <iostream>
#include <math.h>
#include <random>
#include <time.h>
#include <vector>

#ifdef __MINGW32__
// Apparently, mingw doesn't have strerror_r :(
#define strerror_r(errnum, buf, buflen) ""
#endif

#include <png++/png.hpp>

#include "vec3.h"

#include "plane.h"
#include "sphere.h"

const char *filename = "render.png";
const int w = 400;
const int h = 400;
const float fov = 1;
const float imageGamma = 1;
const float sensitivity = 255;
const int bounces = 2;
const int spp = 100;
const float sampleVariation = 0.001;
const float farClipping = 100;
const float nearClipping = 0.01;
const float specularClipping = .1;

Vec3 camPos(0, 0, 0);

inline Vec3 lerp(Vec3 val1, Vec3 val2, float alpha)
{
    return val1 * (1 - alpha) + val2 * alpha;
}

inline float clamp(float val, float min, float max)
{
    return val < min ? min : (val > max ? max : val);
}

inline int floatToChannel(float input)
{
    return clamp(floor(pow(input, imageGamma) * sensitivity), 0, 255);
}

int main()
{
    png::image<png::rgb_pixel> image(w, h);

    std::cout << "Render started\n";
    time_t start = time(0);
    srand(start);

    int sensorSide = w > h ? w : h;

    Material *redLight = new Material(
        Vec3(1, 0, 0),
        Vec3(0, 0, 0),
        0);

    Material *blueLight = new Material(
        Vec3(0, 0, 1),
        Vec3(0, 0, 0),
        0);

    Material *diffuse = new Material(
        Vec3(0, 0, 0),
        Vec3(0.9, 0.9, 0.9),
        0.5);

    std::vector<Renderable *> objects;

    objects.push_back(new Plane(
        Vec3(0, -1, 0),
        Vec3(1, -1, 0),
        Vec3(0, -1, 1),
        diffuse));

    objects.push_back(new Sphere(
        Vec3(-1.2, 0, 5),
        1, redLight));

    objects.push_back(new Sphere(
        Vec3(1.2, 0, 5),
        1, blueLight));


    bool progress = true;
    for (int y = h / 2 - 1; y >= -h / 2; y--)
    {
        if (progress)
        {
            progress = false;
            std::cout << "\rProgress: " << 100 * (y - h / 2) / -h << '%' << std::flush;
        }
#pragma omp parallel for
        for (int x = -w / 2; x < w / 2; x++)
        {
            Vec3 sampleResult(0, 0, 0);
            for (unsigned int sample = 0; sample < spp; sample++)
            {
                Vec3 result(0, 0, 0);
                Vec3 reflection(1, 1, 1);
                Vec3 origin = camPos;
                Vec3 direction(
                    (float)x / sensorSide + ((rand() % 20) / 10.f - 1) * sampleVariation,
                    (float)y / sensorSide + ((rand() % 20) / 10.f - 1) * sampleVariation,
                    1 / fov);
                direction = direction.normalized();
                for (int bounce = 0; bounce < bounces; bounce++)
                {
                    int closest = -1;
                    float lowestDistance = farClipping;

                    for (unsigned int i = 0; i < objects.size(); i++)
                    {
                        float distance = objects.at(i)->intersect(origin, direction);

                        if (distance <= nearClipping)
                            continue;
                        else if (distance < lowestDistance)
                        {
                            lowestDistance = distance;
                            closest = i;
                        }
                    }

                    if (closest == -1)
                        break;

                    result += objects.at(closest)->mat->emission * reflection;
                    reflection *= objects.at(closest)->mat->specular;

                    if (reflection.highest() <= specularClipping)
                        break;

                    origin = origin + direction * lowestDistance;
                    Vec3 hitNormal = objects.at(closest)->getNormal(origin);
                    direction = direction -
                                (hitNormal * 2) * (direction.dot(hitNormal));
                    direction = lerp(direction,
                                     Vec3(((rand() % 20) / 10.f - 1),
                                          ((rand() % 20) / 10.f - 1),
                                          ((rand() % 20) / 10.f - 1))
                                         .normalized(),
                                     objects.at(closest)->mat->roughness);
                    direction = direction.normalized();
                }
                sampleResult += result;
            }
            image[(-y + h / 2 - 1)][x + w / 2] = png::rgb_pixel(
                floatToChannel(sampleResult.x / spp),
                floatToChannel(sampleResult.y / spp),
                floatToChannel(sampleResult.z / spp));
        }
        progress = true;
    }
    std::cout << "\nRender finished in " << time(0) - start << " seconds\n";

    image.write(filename);

    for (unsigned int i = 0; i < objects.size(); i++)
        delete objects.at(i);

    delete redLight;
    delete diffuse;

    return 0;
}
