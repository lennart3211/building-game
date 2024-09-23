#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fragUV;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    vec4 ambientLightColor;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D  uTexture;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normaMatrix;
    uint textureIndex;
} push;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(uTexture, fragUV);
}
