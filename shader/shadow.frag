#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;

layout(set = 0, binding = 0) uniform GlobalUBO {
    vec2 cursorPosition;
    float innerRadius;
    float outerRadius;
    float fadeExponent;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
    float distance = length(inUV - ubo.cursorPosition);
    float t = smoothstep(ubo.innerRadius, ubo.outerRadius, distance);
    float opacity = pow(t, ubo.fadeExponent);

    outColor = vec4(inColor.rgb, opacity);
}