#define FLT_EPSILON 1.192092896e-07 // Smallest positive number, such that 1.0 + FLT_EPSILON != 1.0

float PositivePow(float base, float power)
{
    return pow(max(abs(base), float(FLT_EPSILON)), power);
}

vec2 PositivePow(vec2 base, vec2 power)
{
    return pow(max(abs(base), vec2(FLT_EPSILON, FLT_EPSILON)), power);
}

vec3 PositivePow(vec3 base, vec3 power)
{
    return pow(max(abs(base), vec3(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON)), power);
}

vec4 PositivePow(vec4 base, vec4 power)
{
    return pow(max(abs(base), vec4(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON)), power);
}

vec3 LinearToSRGB(vec3 c) {
    vec3 sRGBLo = c * 12.92;
    vec3 sRGBHi = (PositivePow(c, vec3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
    vec3 sRGB = vec3(
    (c.x <= 0.0031308)
        ? sRGBLo.x
        : sRGBHi.x,
    (c.y <= 0.0031308)
        ? sRGBLo.y
        : sRGBHi.y,
    (c.z <= 0.0031308)
        ? sRGBLo.z
        : sRGBHi.z);
    return sRGB;
}

vec3 SRGBToLinear(vec3 c) {
    vec3 linearRGBLo = c / 12.92;
    vec3 linearRGBHi = PositivePow((c + 0.055) / 1.055, vec3(2.4, 2.4, 2.4));
    vec3 linearRGB = vec3(
    (c.x <= 0.04045)
        ? linearRGBLo.x
        : linearRGBHi.x,
    (c.y <= 0.04045)
        ? linearRGBLo.y
        : linearRGBHi.y,
    (c.z <= 0.04045)
        ? linearRGBLo.z
        : linearRGBHi.z);
    return linearRGB;
}