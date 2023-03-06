#version 330

#define SAMPLE_RATE 0.025

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	sampler2D shadowMap;
	sampler2D diffuseTex;
	sampler2D normalMap;
	sampler2D displacementMap;
	sampler2D glossMap; // New gloss texture
	sampler2D roughnessMap;
	sampler2D ambientOcclusion;
	float alpha;
	vec2 textureScale;

};

struct PointLight
{
	vec3 position;
	float intensity;
	vec3 color;
	float constant;
	float linear;
	float quadratic;
};

//input from vertex shader
in vec3 vs_position;
in vec3 vs_color;
in vec2 vs_texcoord;
in vec3 vs_normal;
in vec4 vs_fragPosLightSpace;

//output of fragment shader
out vec4 color;

//Uniforms
uniform Material material;
uniform PointLight pointLight;
uniform vec3 lightPos0;
uniform vec3 cameraPos;

//Functions

float ShadowCalculation(vec4 fragPosLightSpace)
{
	//return 1.0;

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

	if(projCoords.z > 1.0)
	{
		 float shadow = 0.0;
		 return shadow;
	}
    

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(material.shadowMap, projCoords.xy).r;  
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;


	float bias = max(0.05 * (1.0 - dot(vs_normal, normalize(lightPos0 - vs_position))), 0.005);  

    // check whether current frag pos is in shadow
    //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;


	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(material.shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
	for(int y = -1; y <= 1; ++y)
	{
		float pcfDepth = texture(material.shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
		shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
	}    
	}
	shadow /= 9.0;

    return shadow;
}  


vec3 calculateAmbient(Material material)
{
	float ao = texture(material.ambientOcclusion, vs_texcoord * material.textureScale).r;
	vec3 AmbientColor = vec3(pointLight.color * material.ambient * ao);
	return AmbientColor;
}
 
vec3 calculateDiffuse(Material material, vec3 vs_position, vec3 vs_normal, vec3 lightPos0)
{
	vec3 posToLightDirVec = normalize(lightPos0 - vs_position);
	float diffuse = clamp(dot(posToLightDirVec, normalize(vs_normal)), 0, 1);
	vec3 diffuseFinal = material.diffuse * diffuse;

	return diffuseFinal;
}

vec3 calculateSpecular(Material material, vec3 vs_position, vec3 vs_normal, vec3 lightPos0, vec3 cameraPos)
{
	float roughness = texture(material.roughnessMap, vec2(vs_texcoord.y * material.textureScale.y * 8,vs_texcoord.x * material.textureScale.x * 8)).r;
	roughness += .2f;
	roughness = clamp(roughness,0,1);
	//roughness = pow(roughness, 10);
	//roughness * 0.1;
	vec3 lightToPosDirVec = normalize(vs_position - lightPos0);
	vec3 reflectDirVec = normalize(reflect(lightToPosDirVec, normalize(vs_normal)));
	vec3 posToViewDirVec = normalize(cameraPos - vs_position);
	float specularConstant = pow(max(dot(posToViewDirVec, reflectDirVec), 0), 8 / (roughness * roughness));//
	vec3 specularFinal = material.specular * specularConstant * pointLight.color;

	return specularFinal;
}


//--------------------------------------------------------------------------------------------------------------------------------
//Noise
//--------------------------------------------------------------------------------------------------------------------------------



vec2 random2(vec2 st)
{
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}


float noise(vec2 st) 
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}



//--------------------------------------------------------------------------------------------------------------------------------
//Bump map 
//--------------------------------------------------------------------------------------------------------------------------------



vec3 bump_map(vec2 TexCoord)
{	

	 // Sample the height map texture and multiply it by the height scale factor
    float height = texture(material.displacementMap, TexCoord).r * 3.0;

    // Sample the normal map texture and convert it from tangent space to view space
    vec3 normal_map = normalize(texture(material.normalMap, TexCoord).rgb * 2.0 - 1.0);
	normal_map.x = normal_map.x + vs_normal.x * 4.;
	normal_map.y = normal_map.y + vs_normal.y * 4.;
	normal_map.z = normal_map.z + vs_normal.z * 4.;
    vec3 T = normalize(vec3(dFdx(TexCoord), 0.0));
    vec3 B = cross(normal_map, T);
    mat3 TBN = mat3(T, B, normal_map);

    // Apply the height offset to the vertex position and sample the perturbed normal
    vec3 pos = vs_position + height * normal_map;
    vec3 preturb = normalize(texture(material.normalMap, TexCoord + dFdx(TexCoord)).rgb * 2.0 - 1.0);
    preturb = TBN * preturb;

    // Interpolate between the perturbed normal and the original normal based on the height offset
    float h = 0.1;
    vec3 normal = mix(normal_map, preturb, height * h * 10.0);

    return normal;
}



//--------------------------------------------------------------------------------------------------------------------------------
//Main
//--------------------------------------------------------------------------------------------------------------------------------




void main()
{
	
	//Bump Map to return final normals
	vec2 scaled_uv = vec2(vs_texcoord.y * material.textureScale.y,vs_texcoord.x * material.textureScale.x);
	vec3 normals = bump_map(scaled_uv);

	color = vec4(normals, 1.);

	//ShadowCalculation
	float shadow = ShadowCalculation(vs_fragPosLightSpace);

	//Ambient light
	vec3 ambientFinal = calculateAmbient(material);

	//Diffuse light
	vec3 diffuseFinal = calculateDiffuse(material, vs_position, normals, pointLight.position);

	//Specular light
	vec3 specularFinal = calculateSpecular(material, vs_position, normals, pointLight.position, cameraPos);

	//Attenuation
	float distance = length(pointLight.position - vs_position);
	//constant linear quadratic
	float attenuation = pointLight.constant / (1.f + pointLight.linear * distance + pointLight.quadratic * (distance * distance));

	//Final light
	ambientFinal *= attenuation;
	diffuseFinal *= attenuation;
	specularFinal *= attenuation;

	vec3 glossColor = texture(material.glossMap, scaled_uv).rgb;
	specularFinal *= glossColor * 15;
	
	
	color = texture(material.diffuseTex, scaled_uv) * ((vec4(ambientFinal, 1.0) + (1.0-shadow) * (vec4(diffuseFinal.xyz, 1.0) + vec4(specularFinal.xyz, 1.0))) * pointLight.intensity); 
	//color = vec4(color.x,color.y,color.z,material.alpha);

	
	color = vec4(1,1,1,1) * texture(material.ambientOcclusion, scaled_uv);

}

