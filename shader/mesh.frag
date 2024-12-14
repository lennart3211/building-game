#version 450

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragPosLightSpace;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 lightSpaceMatrix;
    vec3 viewPosition;
    vec4 ambientLightColor;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D shadowMap;

layout(set = 2, binding = 0) uniform sampler2D  uTexture;


layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normaMatrix;
    uint colorIndex;
} push;

layout(location = 0) out vec4 outColor;

vec4 colors[] = {
  vec4(0.75, 0.5, 0.75, 1),
  vec4(0.5, 0.8, 0.75, 1),
  vec4(0.9, 0.8, 0.2, 1),
  vec4(0.5, 0.2, 0.9, 1),
};

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
    return shadow;
}

void main() {
    vec4 texColor = texture(uTexture, fragUV);
    texColor *= colors[push.colorIndex];

    float shadow = ShadowCalculation(fragPosLightSpace);

    vec3 normal = normalize(fragNormalWorld);
    vec3 lightDir = normalize(ubo.lightPosition - fragPosWorld);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * ubo.lightColor.rgb;

    vec3 lighting = (ubo.ambientLightColor.rgb + (1.0 - shadow) * diffuse);

    outColor = vec4(texColor.rgb * lighting, texColor.a);
}
