#version 330 core

in vec3 vs_normal;
in vec3 vs_position;
in vec2 vs_texcoord;

out vec4 FragColor;

uniform sampler2D depthRenderBuffer;
uniform mat4 projection;
uniform float screenwidth = 1024;
uniform float screenheight = 1024;
uniform float aoRadius = 0.3;
uniform float aoBias = 0.025;

const int NUM_SAMPLES = 16;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec3 N = normalize(vs_normal);
    vec3 P = vs_position.xyz;

    // Generate random sample directions
    vec3 sampleDirs[NUM_SAMPLES];
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec3 randVec = vec3(rand(vec2(i, gl_FragCoord.y * screenheight)), 
                            rand(vec2(i, gl_FragCoord.x * screenwidth)), 
                            0.0) * 2.0 - 1.0;
        sampleDirs[i] = normalize(randVec);
    }

    float occlusion = 0.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec3 samplePos = P + sampleDirs[i] * aoRadius;
        vec4 offset = projection * vec4(samplePos, 1.0);
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
        float sampleDepth = texture(depthRenderBuffer, offset.xy).r;
        vec3 samplePosProj = samplePos / sampleDepth;
        vec3 sampleNormal = normalize(vs_normal); // use vertex normal instead of texture normal
        float rangeCheck = smoothstep(0.0, 1.0, aoRadius / abs(samplePosProj.z - P.z));
        float nDotL = dot(N, sampleNormal);
        occlusion += (nDotL * rangeCheck);
    }

    occlusion = 1.0 - (occlusion / float(NUM_SAMPLES));
    occlusion = pow(occlusion, 2.0);  // apply a power function for more contrast

    FragColor = vec4(vec3(occlusion - aoBias), 1.0);
}
