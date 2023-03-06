#version 330 core

in vec2 TexCoord;

uniform sampler2D DepthTexture;
uniform sampler2D sceneTexture;

uniform mat4 ProjectionMatrix;
uniform float Near = 0.1;
uniform float Far = 100.0;

uniform float Radius = 0.3;
uniform float Bias = 0.1;
uniform float Intensity = 1.0;

const int SampleCount = 16;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec2 texelSize = 1.0 / textureSize(DepthTexture, 0);

    float depth = texture(DepthTexture, TexCoord).r;

    vec3 position = vec3((TexCoord * 2.0 - 1.0) * vec2(ProjectionMatrix[0][0], ProjectionMatrix[1][1]), depth);

    float occlusion = 0.0;

    for (int i = 0; i < SampleCount; i++)
    {
        vec3 sample = vec3(rand(TexCoord + vec2(i)) * 2.0 - 1.0, rand(TexCoord - vec2(i)) * 2.0 - 1.0, rand(TexCoord));
        sample = normalize(sample) * Radius;

        vec3 offset = vec3(sample.xy, sample.z * texelSize.x);

        vec3 samplePosition = position + offset;

        vec4 sampleClip = ProjectionMatrix * vec4(samplePosition, 1.0);
        vec3 sampleNDC = sampleClip.xyz / sampleClip.w;
        vec2 sampleTexCoord = 0.5 * (sampleNDC.xy + 1.0);

        float sampleDepth = texture(DepthTexture, sampleTexCoord).r;
        float rangeCheck = smoothstep(Near, Far, sampleDepth);

        float occlusionSample = (sampleDepth - samplePosition.z) < Bias ? 1.0 : 0.0;
        occlusion += (1.0 - occlusionSample) * rangeCheck;
    }

    occlusion /= float(SampleCount);

    occlusion = 1.0 - occlusion;

    occlusion = pow(occlusion, Intensity);

    gl_FragColor = vec4(vec3(occlusion), 1.0);



    vec4 sceneColor = texture(sceneTexture, TexCoord);
    //float mydepth = texture(DepthTexture, TexCoord).z;
    //gl_FragColor = vec4(1,1,1,1) * mydepth;
    gl_FragColor = sceneColor;
}
