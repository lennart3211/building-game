#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec2 inPosition[];
layout(location = 1) in vec4 inColor[];
layout(location = 2) in float inSize[];

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;

void main() {
    vec2 position = inPosition[0];
    float size = inSize[0];
    vec4 color = inColor[0];

    // Top-left
    gl_Position = vec4(position.x - size, position.y - size, 0.0, 1.0);
    outColor = color;
    outTexCoord = vec2(0.0, 0.0);
    EmitVertex();

    // Top-right
    gl_Position = vec4(position.x + size, position.y - size, 0.0, 1.0);
    outColor = color;
    outTexCoord = vec2(1.0, 0.0);
    EmitVertex();

    // Bottom-left
    gl_Position = vec4(position.x - size, position.y + size, 0.0, 1.0);
    outColor = color;
    outTexCoord = vec2(0.0, 1.0);
    EmitVertex();

    // Bottom-right
    gl_Position = vec4(position.x + size, position.y + size, 0.0, 1.0);
    outColor = color;
    outTexCoord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}