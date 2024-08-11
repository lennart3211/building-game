#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 5) in vec3 instancePosition;
layout(location = 6) in vec4 instanceColor;
layout(location = 7) in float instanceSize;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 projection;
    mat4 view;
    vec4 ambientLightColor;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragWorldPos;

void main() {
    vec3 worldPos = (inPosition * instanceSize) + instancePosition;
    gl_Position = ubo.projection * ubo.view * vec4(worldPos, 1.0);
    fragColor = instanceColor;
    fragNormal = mat3(ubo.view) * inNormal;
    fragWorldPos = worldPos;
}