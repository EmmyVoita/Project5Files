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
#include "Vertex.h"
#include "OBJloader.h"
#include "Shader2.h"


class MyMeshClass
{
private:
	Vertex* vertexArray;
	unsigned nrOfVertices;
	GLuint* indexArray;
	unsigned nrOfIndices;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	
	glm::vec3 position;
	glm::vec3 origin;
	glm::vec3 rotation;
	glm::vec3 scale;
	
	glm::mat4 ModelMatrix;
	glm::mat4* ViewMatrix;


	void initVAO()
	{
		//Create VAO
		glCreateVertexArrays(1, &this->VAO);
		glBindVertexArray(this->VAO);

		//GEN VBO AND BIND AND SEND DATA
		glGenBuffers(1, &this->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, this->nrOfVertices * sizeof(Vertex), this->vertexArray, GL_STATIC_DRAW);

		//GEN EBO AND BIND AND SEND DATA
		if (this->nrOfIndices > 0)
		{
			glGenBuffers(1, &this->EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->nrOfIndices * sizeof(GLuint), this->indexArray, GL_STATIC_DRAW);
		}
		

		//SET VERTEXATTRIBPOINTERS AND ENABLE (INPUT ASSEMBLY)
		//Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		//Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
		glEnableVertexAttribArray(1);
		//Texcoord
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texcoord));
		glEnableVertexAttribArray(2);
		//Normal
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(3);

		//BIND VAO 0
		glBindVertexArray(0);
		
	}
	
	
	void updateModelMatrix()
	{
		this->ModelMatrix = glm::mat4(1.f);
		this->ModelMatrix = glm::translate(this->ModelMatrix, this->origin);
		this->ModelMatrix = glm::translate(this->ModelMatrix, this->position);
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.x), glm::vec3(1.f, 0.f, 0.f));
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.y), glm::vec3(0.f, 1.f, 0.f));
		this->ModelMatrix = glm::rotate(this->ModelMatrix, glm::radians(this->rotation.z), glm::vec3(0.f, 0.f, 1.f));
		this->ModelMatrix = glm::scale(this->ModelMatrix, this->scale);
	}

public:
	MyMeshClass(Vertex* vertexArray, const unsigned& nrOfVertices, GLuint* indexArray, const unsigned& nrOfIndices, glm::mat4* ViewMatrix, glm::vec3 position = glm::vec3(0.f), glm::vec3 origin = glm::vec3(0.f), glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f))
	{
		this->position = position;
		this->origin = origin;
		this->rotation = rotation;
		this->scale = scale;

		this->nrOfVertices = nrOfVertices;
		this->nrOfIndices = nrOfIndices;

		this->ViewMatrix = ViewMatrix;

		this->vertexArray = new Vertex[this->nrOfVertices];
		for (int i = 0; i < nrOfVertices; i++)
		{
			this->vertexArray[i] = vertexArray[i];
		}

		this->indexArray = new GLuint[this->nrOfIndices];
		for (int i = 0; i < nrOfIndices; i++)
		{
			this->indexArray[i] = indexArray[i];
		}

		this->initVAO();
		this->updateModelMatrix();
	}


	~MyMeshClass()
	{
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);

		if (this->nrOfIndices > 0)
		{
			glDeleteBuffers(1, &this->EBO);
		}

		delete[] this->vertexArray;
		delete[] this->indexArray;
	}


	//Modifiers
	void setPosition(const glm::vec3 position)
	{
		this->position = position;
	}

	void setOrigin(const glm::vec3 origin)
	{
		this->origin = origin;
	}

	void setRotation(const glm::vec3 rotation)
	{
		this->rotation = rotation;
	}

	void setScale(const glm::vec3 scale)
	{
		this->scale = scale;
	}
	
	//Functions

	void move(const glm::vec3 position)
	{
		this->position += position;
	}

	void rotate(const glm::vec3 rotation)
	{
		this->rotation += rotation;
	}

	void scaleUp(const glm::vec3 scale)
	{
		this->scale += scale;
	}

	void renderForShadow(Shader2 &ourShader)
	{
		//Update uniforms
		this->updateModelMatrix();

		ourShader.Use();

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		
		// Pass them to the shaders
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(this->ModelMatrix));

		//Bind VAO
		glBindVertexArray(this->VAO);

		//RENDER
		if (this->nrOfIndices == 0)
		glDrawArrays(GL_TRIANGLES, 0, this->nrOfVertices);
		else
		glDrawElements(GL_TRIANGLES, this->nrOfIndices, GL_UNSIGNED_INT, 0);

		//Cleanup
		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void renderForAmbientOcclusion(Shader2 &ourShader)
	{
		//Update uniforms
		this->updateModelMatrix();

		ourShader.Use();

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		

		glm::mat4 projection;
		
       	projection = glm::perspective(45.0f, (GLfloat)1400 / (GLfloat)800, 0.1f, 100.0f);

		// Pass them to the shaders
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(this->ModelMatrix));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*this->ViewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		

		//Bind VAO
		glBindVertexArray(this->VAO);

		//RENDER
		if (this->nrOfIndices == 0)
		glDrawArrays(GL_TRIANGLES, 0, this->nrOfVertices);
		else
		glDrawElements(GL_TRIANGLES, this->nrOfIndices, GL_UNSIGNED_INT, 0);

		//Cleanup
		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void render(Shader2 &ourShader)
	{
		//Update uniforms
		this->updateModelMatrix();

		ourShader.Use();

       	glm::mat4 projection;
		
       	projection = glm::perspective(45.0f, (GLfloat)1400 / (GLfloat)800, 0.1f, 100.0f);
		
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        	
        // Pass them to the shaders
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(this->ModelMatrix));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(*this->ViewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		//Bind VAO
		glBindVertexArray(this->VAO);

		//RENDER
		if (this->nrOfIndices == 0)
			glDrawArrays(GL_TRIANGLES, 0, this->nrOfVertices);
		else
			glDrawElements(GL_TRIANGLES, this->nrOfIndices, GL_UNSIGNED_INT, 0);

		//Cleanup
		glBindVertexArray(0);
		glUseProgram(0);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
};
