#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) out vec4 fragPosLightSpace;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 lightSpaceMatrix;
    vec3 viewPosition;
    vec4 ambientLightColor;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normaMatrix;
} push;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragUV;

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(position, 1.0f);
    gl_Position = ubo.projection * ubo.view * positionWorld;
    fragPosLightSpace = ubo.lightSpaceMatrix * positionWorld;
    fragNormalWorld = normalize(mat3(push.modelMatrix) * normal);
    fragPosWorld = positionWorld.xyz;
    fragUV = uv;
}