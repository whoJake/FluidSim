#pragma once
#include "Ray.h"

struct Sphere
{
    glm::vec3 position;
    float radius;
    glm::vec4 color;
};

struct HitInfo
{
    glm::vec3 normal;
    float distance;
    glm::vec4 color;
};

class Blas
{
public:
    Blas();

    bool traverse(const Ray& ray, HitInfo* hit) const;
private:
    void add_random_sphere();
private:
    std::vector<Sphere> m_data;

    glm::vec3 m_min;
    glm::vec3 m_max;
};