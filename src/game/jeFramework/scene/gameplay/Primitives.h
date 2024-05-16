#pragma once

#include <functional>
#include "Mesh.h"

inline static const std::vector<Vertex> s_verticesUnitCube
{
    //      Positions        Normals   Colours
    { { -0.5f, -0.5f, -0.5f }, { }, { 0.f, 0.f, 0.f } },
    { { 0.5f,  -0.5f, -0.5f }, { }, { 1.f, 0.f, 0.f } },
    { { -0.5f, 0.5f,  -0.5f }, { }, { 0.f, 1.f, 0.f } },
    { { 0.5f,  0.5f,  -0.5f }, { }, { 1.f, 1.f, 0.f } },
    { { -0.5f, -0.5f, 0.5f },  { }, { 0.f, 0.f, 1.f } },
    { { 0.5f,  -0.5f, 0.5f },  { }, { 1.f, 0.f, 1.f } },
    { { -0.5f, 0.5f,  0.5f },  { }, { 0.f, 1.f, 1.f } },
    { { 0.5f,  0.5f,  0.5f },  { }, { 1.f, 1.f, 1.f } }
};

inline static const std::vector<uint16_t> s_indicesUnitCube
{
    //back
    0, 2, 1,
    3, 1, 2,

    //front
    5, 7, 4,
    6, 4, 7,

    //left
    4, 6, 0,
    2, 0, 6,

    //right
    1, 3, 5,
    7, 5, 3,

    //top
    7, 3, 6,
    2, 6, 3,

    //bottom
    1, 5, 0,
    4, 0, 5,
};

inline static const std::vector<Vertex> s_verticesUnitPlane
{
    { { -0.5f, 0.f, -0.5f }, { }, { 0.f, 0.f, 0.f } },
    { { 0.5f,  0.f, -0.5f }, { }, { 1.f, 0.f, 0.f } },
    { { -0.5f, 0.f, 0.5f },  { }, { 0.f, 1.f, 0.f } },
    { { 0.5f,  0.f, 0.5f },  { }, { 1.f, 1.f, 0.f } }
};

inline static const std::vector<uint16_t> s_indicesUnitPlane
{
    0, 2, 1,
    3, 1, 2,

    1, 2, 0,
    2, 1, 3
};

inline static std::function<std::vector<Vertex>(void)> s_createVerticesUnitSphere = [](){
    std::vector<Vertex> result;



    return result;
};