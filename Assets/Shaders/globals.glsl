layout (std140) uniform Globals
{                   // base alignment  // aligned offset
    mat4 proj;      // 16              // 0
                    // 16              // 16
                    // 16              // 32
                    // 16              // 48
    mat4 view;      // 16              // 64
                    // 16              // 80
                    // 16              // 96
                    // 16              // 112
    vec4 eyePos;    // 16              // 128
    vec4 eyeForward;// 16              // 144

    float time;     // 4               // 148
    int renderMode; // 4               // 152
};

float saturate(float val) {
    return clamp(val, 0.0, 1.0);
}