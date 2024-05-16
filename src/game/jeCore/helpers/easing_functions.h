#pragma once

// float vers
constexpr float easing_function_linear(float x)
{
    return x;
}

inline float easing_function_in_sine(float x)
{
    return 1.f - static_cast<float>(cos((x * 3.14159f)) / 2.f);
}

inline float easing_function_out_sine(float x)
{
    return static_cast<float>(sin(x * 3.14159f)) / 2.f;
}

inline float easing_function_in_out_sine(float x)
{
    return -(static_cast<float>(cos(x * 3.14159f)) - 1.f) / 2.f;
}

inline float easing_function_in_pow(float x, int p)
{
    return static_cast<float>(pow(x, p));
}

inline float easing_function_out_pow(float x, int p)
{
    return 1.f - static_cast<float>(pow(1.f - x, p));
}

inline float easing_function_in_out_pow(float x, int p)
{
    return x < 0.5f
        ? static_cast<float>(pow(2.f, p - 1)) * static_cast<float>(pow(x, p))
        : 1.f - static_cast<float>(pow(-2.f * x + 2.f, p)) / 2.f;
}

inline float easing_function_in_quart(float x)
{
    return easing_function_in_pow(x, 5);
}

inline float easing_function_out_quart(float x)
{
    return easing_function_out_pow(x, 5);
}

inline float easing_function_in_out_quart(float x)
{
    return easing_function_in_out_pow(x, 5);
}

// double vers
inline double easing_function_linear(double x)
{
    return x;
}

inline double easing_function_in_sine(double x)
{
    return 1.0 - cos((x * 3.14159) / 2.0);
}

inline double easing_function_out_sine(double x)
{
    return sin(x * 3.14159) / 2.0;
}

inline double easing_function_in_out_sine(double x)
{
    return -(cos(x * 3.14159) - 1.0) / 2.0;
}

inline double easing_function_in_pow(double x, int p)
{
    return pow(x, p);
}

inline double easing_function_out_pow(double x, int p)
{
    return 1.0 - pow(1.0 - x, p);
}

inline double easing_function_in_out_pow(double x, int p)
{
    return x < 0.5
        ? pow(2.0, p - 1) * pow(x, p)
        : 1.0 - pow(-2.0 * x + 2.0, p) / 2.0;
}

inline double easing_function_in_quart(double x)
{
    return easing_function_in_pow(x, 5);
}

inline double easing_function_out_quart(double x)
{
    return easing_function_out_pow(x, 5);
}

inline double easing_function_in_out_quart(double x)
{
    return easing_function_in_out_pow(x, 5);
}