#version 330 core

in vec4 clipSpace;

uniform mat4 invProj;

out vec4 color;

void main()
{
    vec3 ndc = clipSpace.xyz / clipSpace.w;
    vec4 viewSpace = invProj * vec4(ndc, 1.0);
    vec3 viewPos = viewSpace.xyz / viewSpace.w;
    vec2 screenPos = vec2(viewPos.x / 1400.0, viewPos.y / 800.0);

    float depth = (viewPos.z - 0.1) / (100.0 - 0.1);

    color = vec4(screenPos, depth, 1.0);
}
