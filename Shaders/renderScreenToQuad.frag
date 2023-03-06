#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D sceneTexture;
uniform sampler2D ssaoTexture;

void main()
{
    vec4 sceneColor = texture(sceneTexture, TexCoord);
    float ssaoValue = texture(ssaoTexture, TexCoord).r;

    // Scale the SSAO value by a factor of your choice (e.g. 0.2)
    ssaoValue *= 0.2;

    // Multiply the scene color with the SSAO value
    vec3 finalColor = sceneColor.rgb * (1.0 + ssaoValue);

    //FragColor = vec4(finalColor, sceneColor.a);
    FragColor = vec4(texture(ssaoTexture, TexCoord).xy,0,1);
}

