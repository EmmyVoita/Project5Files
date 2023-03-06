


#include"Texture.h"
#include"OBJloader.h"
#include"MyMeshClass.h"
#include"Shader2.h"
#include"Material.h"

using namespace std;

class MyModelClass
{
private:
	Material* material;
	Texture* overrideTextureDiffuse;
	Texture* overrideNormal;
	Texture* overrideDisplacment;	
	Texture* overrideGloss;
	Texture* overrideRoughness;
	//Texture* overrideTextureDiffusePointer;
	vector<MyMeshClass*> meshes;
	unsigned int *depthMap;
	unsigned int *SSAOMap;


public:

	//OBJ file loaded model
	MyModelClass(Material* material, Texture* overrideTextureDiffuse,  const char* fileName, glm::mat4 &ViewMatrix)
	{
		this->material = material;
		this->overrideTextureDiffuse = overrideTextureDiffuse;

	    //Load the model from the file
	    vector<Vertex> mesh = loadOBJ(fileName);
	    Vertex* vertexArray = mesh.data();
	    unsigned nrOfVertices =  mesh.size();
	    GLuint* indexArray = NULL;
	    unsigned nrOfIndices = 0;
    
	        this->meshes.push_back(new MyMeshClass(vertexArray, nrOfVertices, indexArray, nrOfIndices, &ViewMatrix, glm::vec3(1.f, 0.f, 0.0f),
			glm::vec3(0.f),
			glm::vec3(0.f),
			glm::vec3(1.f)));
	}

	
	MyModelClass(Material* material, Texture* overrideTextureDiffuse, Texture* overrideNormal, Texture* overrideDisplacment, Texture* overrideGloss, Texture* overrideRoughness, const char* fileName, glm::mat4 &ViewMatrix)
	{
		this->material = material;
		this->overrideTextureDiffuse = overrideTextureDiffuse;
		this->overrideNormal = overrideNormal;
		this->overrideDisplacment = overrideDisplacment;
		this->overrideGloss = overrideGloss;
		this->overrideRoughness = overrideRoughness;

	    //Load the model from the file
	    vector<Vertex> mesh = loadOBJ(fileName);
	    Vertex* vertexArray = mesh.data();
	    unsigned nrOfVertices =  mesh.size();
	    GLuint* indexArray = NULL;
	    unsigned nrOfIndices = 0;
    
	    this->meshes.push_back(new MyMeshClass(vertexArray, nrOfVertices, indexArray, nrOfIndices, &ViewMatrix, glm::vec3(1.f, 0.f, 0.0f),
			glm::vec3(0.f),
			glm::vec3(0.f),
			glm::vec3(1.f)));
	}

	
	/*void setViewMatrix(glm::mat4 &ViewMatrix)
	{
		this->ViewMatrix = ViewMatrix;
	}*/

	~MyModelClass()
	{
		for (auto*& i : this->meshes)
			delete i;
	}
	
	//Functions
	
	void setOrigin(const glm::vec3 origin)
	{
		for (auto& i : this->meshes)
			i->setOrigin(origin);
	}
	
	void setRotation(const glm::vec3 rotation)
	{
		for (auto& i : this->meshes)
			i->setRotation(rotation);
	}
	
	void setScale(const glm::vec3 scaled)
	{
		for (auto& i : this->meshes)
			i->setScale(scaled);
	}

	void setPosition(const glm::vec3 position)
	{
		for (auto& i : this->meshes)
			i->setPosition(position);
	}
	
	void move(const glm::vec3 position)
	{
		for (auto& i : this->meshes)
			i->move(position);
	}

	void setShadowTex(unsigned int *depthMap)
	{
		this->depthMap = depthMap;
	}

	void setSSAOTex(unsigned int *depthMap)
	{
		this->SSAOMap = depthMap;
	}

	void renderForShadow(Shader2 &ourShader)
	{
		ourShader.Use();
		
		for(unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->renderForShadow(ourShader);
		}
	}

	void renderForAmbientOcclusion(Shader2 &ourShader)
	{
		ourShader.Use();
		
		for(unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->renderForAmbientOcclusion(ourShader);
		}
	}

	void render(Shader2 &ourShader, bool passNormal)
	{
	
		

		ourShader.Use();
	

		if(!passNormal)
			this->material->sendToShader(ourShader);
		else
			this->material->sendToShaderNormal(ourShader);
		
		ourShader.Use();
		
		for (auto& i : this->meshes)
		{   



			this->overrideTextureDiffuse->bind(material->getDiffuseID());

			//for wood shader bind normal and displacment map
			if(passNormal)
			{
				this->overrideNormal->bind(material->getNormalMapID());
				this->overrideDisplacment->bind(material->getDisplacementMapID());
				this->overrideGloss->bind(material->getGlossMapID());
				this->overrideRoughness->bind(material->getRoughnessMapID());
			}
				

			ourShader.set1i( material->getShadowMapID(), "material.shadowMap");
			glActiveTexture(GL_TEXTURE0 + material->getShadowMapID()); // activate texture unit 1
			glBindTexture(GL_TEXTURE_2D, *depthMap); // bind the depth map texture to unit 1

			ourShader.set1i( material->getSSAOMapID(), "material.ambientOcclusion");
			glActiveTexture(GL_TEXTURE0 + material->getSSAOMapID()); // activate texture unit 1
			glBindTexture(GL_TEXTURE_2D, *SSAOMap); // bind the depth map texture to unit 1

		}

		for(unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i]->render(ourShader);
		}
		

	}
};
