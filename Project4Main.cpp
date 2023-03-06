
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<algorithm>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL/SOIL.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include<glm/vec3.hpp>
#include<glm/vec4.hpp>
#include<glm/mat4x4.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

// Other includes
#include "HeaderFiles/Shader2.h"
#include "HeaderFiles/OBJloader.h"
#include "HeaderFiles/Vertex.h"
#include "HeaderFiles/Texture.h"
#include "HeaderFiles/MyModelClass.h"
#include "HeaderFiles/Camera.h"
#include "HeaderFiles/Material.h"
#include "HeaderFiles/Light.h"
#include "HeaderFiles/ShadowPassClass.h"
#include "HeaderFiles/SSAOPassClass.h"

using namespace std;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 1400, HEIGHT = 800;
GLFWwindow* window;

Camera* camera;

float dt = 0.0f;
float curTime = 0.0f;
float lastTime = 0.0f;

double lastMouseX = 0.0;
double lastMouseY = 0.0;
double mouseX = 0.0;
double mouseY = 0.0;
double mouseOffsetX = 0.0;
double mouseOffsetY = 0.0;
bool firstMouse = true;

glm::mat4 ViewMatrix;

SSAOPassClass* ssaoPass;
ShadowPassClass* shadowPass;


// Function to init Window
void initWindow()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow(WIDTH, HEIGHT, "More 3D example", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Function to init Camera (view matrix)
void initCamera()
{	
	camera = new Camera(glm::vec3(0.f, 6.f, 9.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));
	
	ViewMatrix = glm::mat4(1.f);
	ViewMatrix = camera->getViewMatrix();
}

// Keyboard inputs
void updateKeyboardInput()
{
	//Camera
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera->move(dt, FORWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera->move(dt, BACKWARD);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera->move(dt, LEFT);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera->move(dt, RIGHT);
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
	{
		camera->move(dt,DOWN);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		camera->move(dt,UP);
	}
}

// Time management
void updateDt()
{
	curTime = static_cast<float>(glfwGetTime());
	dt = curTime - lastTime;
	lastTime = curTime;
}

// Mouse Input
void updateMouseInput()
{
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (firstMouse)
	{
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		firstMouse = false;
	}

	//Calc offset
	mouseOffsetX = mouseX - lastMouseX;
	mouseOffsetY = lastMouseY - mouseY;

	//Set last X and Y
	lastMouseX = mouseX;
	lastMouseY = mouseY;

	//Move light
	/*if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		//pointLights[0]->setPosition(camera.getPosition());
	}*/
}

// Update all inputs
void updateInput()
{
	glfwPollEvents();

	updateKeyboardInput();
	updateMouseInput();
	camera->updateInput(dt, -1, mouseOffsetX, mouseOffsetY);
}

// Update ViewMatrix, pointlight
void updateUniforms(MyModelClass* model, vector<PointLight*> &pointLights, Shader2 &ourShader)
{

	shadowPass->setShaderUnifroms(ourShader);
	//View Matrix
	ViewMatrix = camera->getViewMatrix();
	ourShader.setVec3f(camera->getPosition(), "cameraPos");

	//model->setViewMatrix(ViewMatrix);

	
	
	//Point Lights
	for (unsigned int i = 0; i < pointLights.size(); i++)
	{
		pointLights[i]->sendToShader(ourShader);
		ourShader.setVec3f(pointLights[i]->getPosition(), "lightPos0");
	}

}

// Function to render screen-sized quad
void renderScreenQuad(Shader2* shader, unsigned int sceneTexture, unsigned int ssaoTexture)
{
	
    // Create vertices for the screen-sized quad
    GLfloat quadVertices[] = {
        // Positions        // Texture Coords
        -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    };

	GLuint indices[] = { 0, 1, 2, 1, 3, 2 };

    // Create VBO, VAO and EBO
    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);

	glActiveTexture(GL_TEXTURE0 + 1); // activate texture unit 1
	glBindTexture(GL_TEXTURE_2D, ssaoTexture); // bind the depth map texture to unit 1
	
	GLint projLoc = glGetUniformLocation(shader->Program, "ProjectionMatrix");
	glm::mat4 projection;
    projection = glm::perspective(45.0f, (GLfloat)1400 / (GLfloat)800, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


    // Draw quad
    shader->Use();
    glUniform1i(glGetUniformLocation(shader->Program, "sceneTexture"), 0);
	glUniform1i(glGetUniformLocation(shader->Program, "DepthTexture"), 1);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Delete VBO, VAO and EBO
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}


// Main Function
int main()
{
    //Shaders
    vector<Shader2*> shaders;
    
    //Point Lights
    vector<PointLight*> pointLights;
    
    //Materials
    vector<Material*> materials; 

    //Textures
    vector<Texture*> textures; 
    
    //Models
    vector<MyModelClass*> myModels;
    
    //initwindow
    initWindow();

    // Build and compile our shader program
    shaders.push_back(new Shader2("Shaders/BaseVertexShader.vs", "Shaders/BaseFragmentShader.frag"));
    shaders.push_back(new Shader2("Shaders/BaseVertexShader.vs", "Shaders/AdvancedFragmentShader.frag"));
	shaders.push_back(new Shader2("Shaders/renderScreenToQuad.vs", "Shaders/simpleSSAOShader.frag"));


    initCamera();
    
	
    textures.push_back(new Texture("Textures/Cup1ColorFinal.png", GL_TEXTURE_2D));
    textures.push_back(new Texture("Textures/Cup2BaseColor.png", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/MakeupColor.png", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/Carpet/Rug_001_COLOR.png", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/NailPolish.png", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/Wood/WoodFineDark004_COL_4K.jpg", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/Wood/WoodFineDark004_NRM_4K.jpg", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/Wood/WoodFineDark004_DISP_4K.jpg", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/Wood/WoodFineDark004_GLOSS_4K.jpg", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/Fingerprints004_OVERLAY_VAR2_6K.jpg", GL_TEXTURE_2D));
	textures.push_back(new Texture("Textures/Carpet/Rug_001_NRM.jpg", GL_TEXTURE_2D));
	

    materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(1.f), glm::vec3(.8f), 0, 1, 0.85, glm::vec2(1.0f, 1.0f))); 
    materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(1.f), glm::vec3(.8f), 2, 3, 0.85, glm::vec2(1.0f, 1.0f))); 
    materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(2.f), glm::vec3(.6f), 4, 5, 12, 13, 14, 15, 20, 1.0, glm::vec2(1.5f, 1.5f))); 
	materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(1.f), glm::vec3(1.f), 6, 7, 0.8, glm::vec2(1.0f, 1.0f))); 
	materials.push_back(new Material(glm::vec3(1.4f), glm::vec3(2.f), glm::vec3(.6f), 8, 9, 16, 17, 18, 19, 21, 1.0, glm::vec2(4.0f, 4.0f))); 
	materials.push_back(new Material(glm::vec3(0.9f), glm::vec3(1.f), glm::vec3(1.f), 10, 11, 1.0, glm::vec2(1.0f, 1.0f)));
	materials.push_back(new Material(glm::vec3(1.2f), glm::vec3(1.f), glm::vec3(1.f), 16, 17, 1.0, glm::vec2(1.0f, 1.0f)));  
 
    pointLights.push_back(new PointLight(glm::vec3(12.f,10.0f,12.0f), 1.0f, glm::vec3(255.0f/256.0f,245.f/256.0f,182.f/256.0f)));
    

    myModels.push_back(new MyModelClass(materials[0], textures[0], "Models/Cup1Model.txt", ViewMatrix));
    myModels.push_back(new MyModelClass(materials[1], textures[1],  "Models/Cup1Model.txt", ViewMatrix));
    myModels.push_back(new MyModelClass(materials[2], textures[5],textures[6], textures[7],textures[8],textures[9],  "Models/table top.txt", ViewMatrix));
	myModels.push_back(new MyModelClass(materials[3], textures[2],  "Models/makeupremover2.txt", ViewMatrix));
	myModels.push_back(new MyModelClass(materials[4], textures[3], textures[10], textures[3],textures[3], textures[3], "Models/GroundPlane2.txt", ViewMatrix));
	myModels.push_back(new MyModelClass(materials[5], textures[4],  "Models/NailPolish.txt", ViewMatrix));
	myModels.push_back(new MyModelClass(materials[6], textures[0], "Models/Pal1.txt", ViewMatrix));
	myModels.push_back(new MyModelClass(materials[6], textures[0], "Models/Plush1.txt", ViewMatrix));
	

	//Transformations:
    
	//Cup 1
    myModels[0]->setOrigin(glm::vec3(-2.0f, 1.1f, -2.0f));

	//Cup 2
    myModels[1]->setOrigin(glm::vec3(3.0f, 1.1f, 0.f));

	//Table Top 
    myModels[2]->setOrigin(glm::vec3(0.0f, 0.0f, -4.0f));
	myModels[2]->setScale(glm::vec3(0.8f, 0.8f, 0.8f));

	//Makeup Remover Jar
	myModels[3]->setOrigin(glm::vec3(-0.2f, 0.15f, -4.8f));

	//Carpet Plane
	myModels[4]->setOrigin(glm::vec3(0.0f, 0.0f, -4.0f));
	myModels[4]->setScale(glm::vec3(2.0f, 1.0f, 2.0f));

	//Nail Polish Remover Bottle
	myModels[5]->setOrigin(glm::vec3(-1.5f, 0.0f, -4.0f));
	myModels[5]->setScale(glm::vec3(0.8f, 0.8f, 0.8f));

	//Makeup Pal
	myModels[6]->setOrigin(glm::vec3(-2.0f, -0.45f, 1.5f));
	myModels[6]->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
	myModels[6]->setRotation(glm::vec3(0.0f, 90.f, 0.0f));

	//Owl Plush
	myModels[7]->setOrigin(glm::vec3(-1.0f, 0.0f, -1.f));
	myModels[7]->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
	myModels[7]->setRotation(glm::vec3(0.0f, -45.f, -90.0f));


	//Shadow Pass
	shadowPass = new ShadowPassClass(pointLights[0], myModels);

	//SSAO Pass
	ssaoPass = new SSAOPassClass(ViewMatrix, myModels);
	
	//for each model pass a refernece to the shadowPass depthmap and SSAO Pass map
	for (int i = 0; i < myModels.size(); i++)
	{
		myModels[i]->setShadowTex(shadowPass->getDepthMap());
		myModels[i]->setSSAOTex(ssaoPass->getDepthMap());
	}

	
	unsigned int mainFrameBuffer;
	unsigned int textureID;
    unsigned int depthBuffer;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &mainFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
	
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);

    // Attach the depth buffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
	
        // Update the time and input
        updateDt();
       	updateInput();

        // Clear Buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		 // Pass 1: Render the shadow map
		shadowPass->render();

		// Pass 2: Render the SSAO texture
		ssaoPass->render();

		updateUniforms(myModels[0],pointLights, *shaders[0]);

		// Pass 3: Render everything to a texture
        glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer);
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myModels[0]->render(*shaders[0], false);

		updateUniforms(myModels[1],pointLights, *shaders[0]);
		myModels[1]->render(*shaders[0], false);
		
		updateUniforms(myModels[2],pointLights, *shaders[1]);
		myModels[2]->render(*shaders[1], true);

		updateUniforms(myModels[3],pointLights, *shaders[0]);
		myModels[3]->render(*shaders[0], false);

		updateUniforms(myModels[4],pointLights, *shaders[1]);
		myModels[4]->render(*shaders[1], true);

		updateUniforms(myModels[5],pointLights, *shaders[0]);
		myModels[5]->render(*shaders[0], false);

		updateUniforms(myModels[6],pointLights, *shaders[0]);
		myModels[6]->render(*shaders[0], false);

		updateUniforms(myModels[7],pointLights, *shaders[0]);
		myModels[7]->render(*shaders[0], false);
		
		// Unbind the FBO and texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);


		// Pass 4: Render everything to a quad on the screen
		renderScreenQuad(shaders[2], textureID, ssaoPass->getDepthMap2());

		// Swap the screen buffers
		glfwSwapBuffers(window);

		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
        
    }
    
    
    // Properly de-allocate all resources once they've outlived their purpose
    glfwTerminate();
    
    for (size_t i = 0; i < shaders.size(); i++)
		delete shaders[i];
	
	for (size_t i = 0; i < textures.size(); i++)
		delete textures[i];

	for (size_t i = 0; i < materials.size(); i++)
		delete materials[i];

	for (size_t i = 0; i < myModels.size(); i++)
		delete myModels[i];

	for (size_t i = 0; i < pointLights.size(); i++)
		delete pointLights[i];
    
    
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}


