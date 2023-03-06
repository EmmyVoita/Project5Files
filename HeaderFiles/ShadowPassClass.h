// GLM Mathematics
#include <glm/glm.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>


#include "Light.h"
#include "Shader2.h"

using namespace std;

class ShadowPassClass
{
private:
	PointLight* pointLight;
	Shader2* simpleDepthShader;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glm::mat4 lightSpaceMatrix;
	unsigned int depthMap;
	vector<MyModelClass*> myModels;
	

	void initDepthMap()
	{
		//Create a framebuffer object for rendering the depth map
		glGenFramebuffers(1, &depthMapFBO);  
		
		//Create a 2D texture to use as the ramebuffer's depth buffer
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		// check for errors
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			printf("Error: Shadow map framebuffer is not complete!\n");
			exit(1);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
	}
	
	
	void renderDepthMap()
	{

		// 1. render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        float near_plane = 1.0f, far_plane = 40.5f;
        lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
        //lightView = glm::lookAt(pointLight->getPosition(), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightView = glm::lookAt(pointLight->getPosition(), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        simpleDepthShader->Use();
        simpleDepthShader->setMat4fv(lightSpaceMatrix, "lightSpaceMatrix");
		//glm::mat4 model = pointLight->getModelMatrix();
		//simpleDepthShader->setMat4fv(model, "model");

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
			//render the scene for the depth texture
            renderScene();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


		
        // reset viewport
        glViewport(0, 0, 1400, 800);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void renderScene()
	{

		//doesnt seem like the fix for peter panning is working
		//cups are not solid objects
		glCullFace(GL_FRONT);
		
		for(int i = 0; i < myModels.size(); i++)
		{
			myModels[i]->renderForShadow(*simpleDepthShader);
		}

		glCullFace(GL_BACK);
	}	

public:

	ShadowPassClass(PointLight* pointLight, vector<MyModelClass*> myModels)
	{
		this->pointLight = pointLight;
		this->myModels = myModels;
		simpleDepthShader = new Shader2("Shaders/simpleDepthShader.vs", "Shaders/simpleDepthShader.frag");
		initDepthMap();

	}
	
	unsigned int* getDepthMap()
	{
		return &depthMap;
	}

	void render()
	{
		renderDepthMap();
	}

	void setShaderUnifroms(Shader2 &ourShader)
	{
		ourShader.setMat4fv(lightSpaceMatrix, "lightSpaceMatrix");
	}
};
