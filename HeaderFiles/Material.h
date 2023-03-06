#pragma once

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include"Shader2.h"

class Material
{
private:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	GLint diffuseTex;
	GLint shadowMap;
	GLint SSAOMap;
	GLint normalMap;
	GLint displacementMap;
	GLint glossMap;
	GLint roughnessMap;
	float alpha;
	glm::vec2 textureScale;

public:
	Material(
	glm::vec3 ambient,
	glm::vec3 diffuse,
	glm::vec3 specular,
	GLint diffuseTex,
	GLint shadowMap, 
	float alpha,
	glm::vec2 textureScale
	)
	{
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->diffuseTex = diffuseTex;
		this->shadowMap = shadowMap;
		this->textureScale = textureScale;
		this->alpha = alpha;
	}

	Material(
	glm::vec3 ambient,
	glm::vec3 diffuse,
	glm::vec3 specular,
	GLint diffuseTex,
	GLint shadowMap, 
	GLint normalMap,
	GLint displacementMap,
	GLint glossMap,
	GLint roughnessMap,
	GLint SSAOMap,
	float alpha,
	glm::vec2 textureScale
	)
	{
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->diffuseTex = diffuseTex;
		this->shadowMap = shadowMap;
		this->textureScale = textureScale;
		this->normalMap = normalMap;
		this->displacementMap = displacementMap;
		this->roughnessMap = roughnessMap;
		this->glossMap = glossMap;
		this->SSAOMap = SSAOMap;
		this->alpha = alpha;
	}

	~Material(){}


	GLint getDiffuseID()
	{
		return diffuseTex;
	}

	GLint getShadowMapID()
	{
		return shadowMap;
	}

	GLint getNormalMapID()
	{
		return normalMap;
	}

	GLint getDisplacementMapID()
	{
		return displacementMap;
	}

	GLint getGlossMapID()
	{
		return glossMap;
	}

	GLint getRoughnessMapID()
	{
		return roughnessMap;
	}

	GLint getSSAOMapID()
	{
		return SSAOMap;
	}

	//Function
	void sendToShader(Shader2 &ourShader)
	{
		ourShader.setVec3f(this->ambient, "material.ambient");
		ourShader.setVec3f(this->diffuse, "material.diffuse");
		ourShader.setVec3f(this->specular, "material.specular");
		ourShader.set1i(this->shadowMap, "material.shadowMap");
		ourShader.set1i(this->diffuseTex, "material.diffuseTex");
		ourShader.set1f(this->alpha, "material.alpha");
		ourShader.setVec2f(this->textureScale, "material.textureScale");
	}

	void sendToShaderNormal(Shader2 &ourShader)
	{
		ourShader.setVec3f(this->ambient, "material.ambient");
		ourShader.setVec3f(this->diffuse, "material.diffuse");
		ourShader.setVec3f(this->specular, "material.specular");
		ourShader.set1i(this->shadowMap, "material.shadowMap");
		ourShader.set1i(this->diffuseTex, "material.diffuseTex");
		ourShader.set1i(this->normalMap, "material.normalMap");
		ourShader.set1i(this->displacementMap, "material.displacementMap");
		ourShader.set1i(this->glossMap, "material.glossMap");
		ourShader.set1i(this->roughnessMap, "material.roughnessMap");
		ourShader.set1i(this->SSAOMap, "material.SSAOMap");
		ourShader.set1f(this->alpha, "material.alpha");
		ourShader.setVec2f(this->textureScale, "material.textureScale");
	}
};
