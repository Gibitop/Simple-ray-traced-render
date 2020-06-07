#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <random>
#include <time.h>
#include <vector>

// Apparently, mingw doesn't have strerror_r :(
#ifdef __MINGW32__
#define strerror_r(errnum, buf, buflen) ""
#endif

#include <png++/png.hpp>

#include "vec3.h"

#include "disk.h"
#include "plane.h"
#include "sphere.h"

// #define NO_LOG
// #define NO_INTERMEDIATE_RESULTS

const char *filename = "render.png";
const int saveEveryNSamples = 10;
const int w = 400;
const int h = 400;
const float fov = 1.5;
const float imageGamma = 1 / 2.2;
const float sensitivity = 255;
const int bounces = 5;
const int spp = 1000;
const int maxTime = 600;
const float sampleVariation = 1e-4;
const float farClipping = 1e+6;
const float nearClipping = 1e-6;
const float specularClipping = 1e-6;

Vec3 camPos(0, 0, 0);

// Linear interpolation
inline Vec3 lerp(Vec3 val1, Vec3 val2, float alpha)
{
    return val1 * (1 - alpha) + val2 * alpha;
}

inline float clamp(float val, float min, float max)
{
    return val < min ? min : (val > max ? max : val);
}

// Converts a float value in range 0..1 to an int value in range 0..255
inline unsigned char floatToChannel(float input)
{
    return (unsigned char)clamp(floor(pow(input, imageGamma) * sensitivity), 0, 255);
}

inline void _saveImage(Vec3 floatImage[][h], png::image<png::rgb_pixel> pngImage, unsigned int sample)
{
#pragma omp parallel for
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            pngImage[y][x].red = floatToChannel(floatImage[x][y].x / (sample - 1));
            pngImage[y][x].green = floatToChannel(floatImage[x][y].y / (sample - 1));
            pngImage[y][x].blue = floatToChannel(floatImage[x][y].z / (sample - 1));
        }
    }
    pngImage.write(filename);
#ifndef NO_LOG
    std::cout << "Image saved\n";
#endif
}

// Macro for easy access
#define saveImage() _saveImage(floatImage, pngImage, sample)

int main()
{
    Vec3 floatImage[w][h];
    for (unsigned int x = 0; x < w; x++)
    {
        for (unsigned int y = 0; y < h; y++)
            floatImage[x][y] = Vec3();
    }
    png::image<png::rgb_pixel> pngImage(w, h);

    int sensorSide = w > h ? w : h;
    std::vector<Renderable *> objects;

    Material *light = new Material(
        Vec3(3, 3, 3),
        Vec3(0, 0, 0),
        0);

    Material *diffuseGrey = new Material(
        Vec3(0, 0, 0),
        Vec3(0.9, 0.9, 0.9),
        0.5);

    Material *diffuseRed = new Material(
        Vec3(0, 0, 0),
        Vec3(0.9, 0, 0),
        0.9);

    Material *diffuseGreen = new Material(
        Vec3(0, 0, 0),
        Vec3(0, 0.9, 0),
        0.9);

    Material *specular = new Material(
        Vec3(0, 0, 0),
        Vec3(0.9, 0.9, 0.9),
        0.1);

    // Floor
    objects.push_back(new Plane(
        Vec3(0, -0.5, 0),
        Vec3(0, 1, 0),
        diffuseGrey));

    // Roof
    objects.push_back(new Plane(
        Vec3(0, 2, 0),
        Vec3(0, -1, 0),
        diffuseGrey));

    // Wall in front
    objects.push_back(new Plane(
        Vec3(0, 0, 5),
        Vec3(0, 0, -1),
        diffuseGrey));

    // Wall in back
    objects.push_back(new Plane(
        Vec3(0, 0, -2),
        Vec3(0, 0, 1),
        diffuseGrey));

    // Right wall (green)
    objects.push_back(new Plane(
        Vec3(2, 0, 0),
        Vec3(-1, 0, 0),
        diffuseGreen));

    // Left wall (red)
    objects.push_back(new Plane(
        Vec3(-2, 0, 0),
        Vec3(1, 0, 0),
        diffuseRed));

    // Light
    objects.push_back(new Disk(
        Vec3(0, 1.999, 2.5),
        Vec3(0, -1, 0),
        1, light));

    // Right sphere (shiny)
    objects.push_back(new Sphere(
        Vec3(0.75, 0, 2.5),
        0.5, specular));

    // Left sphere (diffuse)
    objects.push_back(new Sphere(
        Vec3(-0.5, 0.05, 2),
        0.6, diffuseGrey));

    time_t start = time(0);
    time_t lastPass = start;
    time_t allPassed = 0;
    srand(start);
#ifndef NO_LOG
    std::cout << "Render started\n";
#endif
    unsigned int sample = 1;
    for (; (spp == 0 || sample <= spp) && (maxTime == 0 || allPassed < maxTime); sample++)
    {
#ifndef NO_LOG
        std::cout << "\r"
                  << "Progress: " << (spp == 0 ? round((float)(allPassed + 1) / maxTime * 100) : round((float)sample / spp * 100)) << '%' << std::setw(4) << '\0'
                  << "Time: " << allPassed + 1 << " sec" << std::setw(4) << '\0'
                  << "ETA: " << (spp == 0 ? maxTime - allPassed - 1 : round((spp - sample) * ((float)allPassed / sample)))
                  << " sec" << std::setw(4) << '\0';
        std::cout << std::flush;
#endif

#ifndef NO_INTERMEDIATE_RESULTS
        if (sample % saveEveryNSamples == 0)
            saveImage();
#endif

// Go for every pixel in the image (IN PARALLEL)
#pragma omp parallel for
        for (int x = -w / 2; x < w / 2; x++)
        {
            for (int y = h / 2; y > -h / 2; y--)
            {

                // Initial ray values
                Vec3 result(0, 0, 0);
                Vec3 reflection(1, 1, 1);
                Vec3 origin = camPos;
                Vec3 direction(
                    (float)x / sensorSide + ((rand() % RAND_MAX) / (RAND_MAX * 0.5f) - 1) * sampleVariation,
                    (float)y / sensorSide + ((rand() % RAND_MAX) / (RAND_MAX * 0.5f) - 1) * sampleVariation,
                    1 / fov);

                direction = direction.normalized();
                for (unsigned int bounce = 0; bounce < bounces; bounce++)
                {
                    int closest = -1;
                    float lowestDistance = farClipping;

                    // Find the closest intersection
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

                    // No intersections found
                    if (closest == -1)
                        break;

                    // Save to the pixel buffer
                    result += objects.at(closest)->mat->emission * reflection;

                    // Calculate light leak
                    reflection *= objects.at(closest)->mat->specular;

                    // The reflection is too dark
                    if (reflection.highest() <= specularClipping)
                    {
                        // std::cout << "Dark object ID: " << closest << std::endl;
                        break;
                    }

                    // New ray origin
                    origin += direction * lowestDistance;

                    // New ray direction
                    Vec3 hitNormal = objects.at(closest)->getNormal(origin);
                    direction = direction - (hitNormal * 2) * (direction.dot(hitNormal));
                    Vec3 randVec(
                        ((float)rand() / RAND_MAX * 2 - 1),
                        ((float)rand() / RAND_MAX * 2 - 1),
                        ((float)rand() / RAND_MAX * 2 - 1));
                    randVec = randVec.normalized();

                    randVec = randVec.dot(hitNormal) > 0 ? randVec : -randVec;

                    direction = lerp(direction, randVec, objects.at(closest)->mat->roughness);
                    direction = direction.normalized();
                }
                // Write the result of this sample
                floatImage[w / 2 + x][h / 2 - y] += result;
            }
        }
        // Update time
        time_t currentTime = time(0);
        allPassed += currentTime - lastPass;
        lastPass = currentTime;
    }
#ifndef NO_LOG
    std::cout << "\r"
              << "Progress: "
              << "100%" << std::setw(4) << '\0'
              << "Time: " << allPassed + 1 << " sec" << std::setw(4) << '\0';
    std::cout << std::flush;
    std::cout << "\nRender finished\n";
#endif

    saveImage();

    // Cleanup
    for (unsigned int i = 0; i < objects.size(); i++)
        delete objects.at(i);
    delete diffuseGrey;
    delete diffuseGreen;
    delete diffuseRed;
    delete specular;
    delete light;

    return 0;
}
