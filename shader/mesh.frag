#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fragUV;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientLightColor;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normaMatrix;
} push;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 normalColor = fragNormalWorld * 0.5 + 0.5;  // Transform from [-1,1] to [0,1]
    outColor = vec4(normalColor, 1.0);
}
