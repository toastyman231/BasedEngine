#version 410 core

out vec4 outColor;
in vec3 vpos;
uniform float iTime = 0.0;
void main()
{
    vec3 Scale1 = vec3(15, 0.4, 0.975);
    vec3 Scale2 = vec3(25, 0.8, 0.5);
    vec3 Scale3 = vec3(75, 3.2, 0.8);

    vec3 Color1 = vec3(1, 0.67256093, 0);
    vec3 Color2 = vec3(1, 0.7411765, 0);
    vec3 Color3 = vec3(1, 0.7411765, 0);

    float pos = length(vpos);
    float f1 = sin(pos * Scale1.x - iTime * Scale1.y);
    float f2 = sin(pos * Scale2.x - iTime * Scale2.y);
    float f3 = sin(pos * Scale3.x - iTime * Scale3.y);
    vec3 col = vec3(1, 0.8078432, 0);
    if (f1 > Scale1.z) {
        col = Color1;
    }              
    else if (f2 > Scale2.z) {
        col = Color2;
    }
    else if (f3 > Scale3.z) {
        col = Color3;
    }
    // Output to screen
    outColor = vec4(col,1.0);
}