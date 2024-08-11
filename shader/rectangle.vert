#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inHalfExtent;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outHalfExtent;

void main() {
    outPosition = inPosition;
    outHalfExtent = inHalfExtent;
}