#version 450

layout(points) in;
layout(line_strip, max_vertices = 24) out;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec3 inHalfExtent[];

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
    mat4 view;
} ubo;

void main() {
    vec3 center = inPosition[0];
    vec3 halfExtent = inHalfExtent[0];

    vec3 vertices[8];
    vertices[0] = center + vec3(-halfExtent.x, -halfExtent.y, -halfExtent.z);
    vertices[1] = center + vec3( halfExtent.x, -halfExtent.y, -halfExtent.z);
    vertices[2] = center + vec3( halfExtent.x,  halfExtent.y, -halfExtent.z);
    vertices[3] = center + vec3(-halfExtent.x,  halfExtent.y, -halfExtent.z);
    vertices[4] = center + vec3(-halfExtent.x, -halfExtent.y,  halfExtent.z);
    vertices[5] = center + vec3( halfExtent.x, -halfExtent.y,  halfExtent.z);
    vertices[6] = center + vec3( halfExtent.x,  halfExtent.y,  halfExtent.z);
    vertices[7] = center + vec3(-halfExtent.x,  halfExtent.y,  halfExtent.z);

    // Bottom face
    for (int i = 0; i < 4; i++) {
        gl_Position = ubo.proj * ubo.view * vec4(vertices[i], 1.0);
        EmitVertex();
    }
    gl_Position = ubo.proj * ubo.view * vec4(vertices[0], 1.0);
    EmitVertex();
    EndPrimitive();

    // Top face
    for (int i = 4; i < 8; i++) {
        gl_Position = ubo.proj * ubo.view * vec4(vertices[i], 1.0);
        EmitVertex();
    }
    gl_Position = ubo.proj * ubo.view * vec4(vertices[4], 1.0);
    EmitVertex();
    EndPrimitive();

    // Connecting edges
    for (int i = 0; i < 4; i++) {
        gl_Position = ubo.proj * ubo.view * vec4(vertices[i], 1.0);
        EmitVertex();
        gl_Position = ubo.proj * ubo.view * vec4(vertices[i+4], 1.0);
        EmitVertex();
        EndPrimitive();
    }
}