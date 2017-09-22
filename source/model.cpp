#include "game.hpp"

/** Constructor
* Loads model data using Assimp
*/
Model::Model(string &filepath)
{
	m_animController = new AnimController;
	this->LoadModel(filepath);
}

Model::Model(const string &filepath, const vector<string> &animPaths, const vector<string> &animNames)
{
	this->LoadModel(filepath, animPaths, animNames);
}

/** Destructor
*
*/
Model::~Model()
{

}

bool Model::IsRigged()
{
	return rigged;
}

/**
* @brief goes through m_meshes and updates them to use instanced rendering
* @param buffer the buffer with model matrix data
* @param amount number of times to instance
*/
void Model::SetInstanceRendering(GLuint buffer, GLuint amount)
{
	for (GLuint i = 0; i < this->m_meshes.size(); i++)
		this->m_meshes[i].MeshSetInstance(buffer, amount);
}

vector<Mesh> *Model::GetMesh()
{
	return &this->m_meshes;
}

Texture Model::LoadEmbeddedTexture(aiString embededIndex)
{
	GLint texIndex;
	string index = embededIndex.data;
	index.erase(0, 1);
	texIndex = stoi(index);

	aiTexture *aiTex = m_scene->mTextures[texIndex];

	return Texture(aiTex);;
}

/**
*@brief Reads model file using assimp to get root node of model.
*@param filepath of model
*/
void Model::LoadModel(string model_path)
{
	m_scene = m_importer.ReadFile(model_path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if(!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
	{
		cout << "ASSIMP ERROR: " << m_importer.GetErrorString() << endl;
		return;
	}

	string directory = model_path.substr(0, model_path.find_last_of('\\') + 1);

	this->ProcessNode(m_scene->mRootNode, directory);

	//LoadEmbeddedTextures();
}

void Model::LoadAnimations(const vector<string> &animations, const vector<string> &animNames)
{
	for (int i = 0; i < animations.size(); i++)
	{
		m_animController->LoadAnimation(animations[i], animNames[i]);
	}
}

void Model::LoadModel(string model_path, const vector<string> &animations, const vector<string> &animNames)
{
	m_scene = m_importer.ReadFile(model_path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if(!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
	{
		cout << "ASSIMP ERROR: %s" << m_importer.GetErrorString() << endl;
		return;
	}

	string directory = model_path.substr(0, model_path.find_last_of('\\') + 1);

	aiMatrix4x4 globalTransform = m_scene->mRootNode->mTransformation;
	globalTransform.Inverse();

	MathUtil::AssimpToGLMMat4(&globalTransform, m_invTransform);

	this->ProcessNode(m_scene->mRootNode, directory);

	LoadAnimations(animations, animNames);
}

/**
* @brief renders all the m_meshes of this model
* @param shader compiled shader id to use to render
*/
void Model::Draw(Shader *shader)
{
	for (GLuint i = 0; i < this->m_meshes.size(); i++)
		this->m_meshes[i].Draw(shader);
}

void Model::DrawVertices(Shader *shader)
{
	for (GLuint i = 0; i < this->m_meshes.size(); i++)
		this->m_meshes[i].DrawVertices(shader);
}

void Model::Draw(Shader *shader, string animation, float timeInSeconds)
{
	m_animController->SetAnimation(timeInSeconds, animation);
	m_animController->BoneTransform(timeInSeconds);

	glUniformMatrix4fv(shader->Uniform("gBones"), 
						m_animController->m_finalTransforms.size(), GL_FALSE, 
						glm::value_ptr(m_animController->m_finalTransforms[0]));

	for (GLuint i = 0; i < this->m_meshes.size(); i++)
		this->m_meshes[i].Draw(shader);
}

/**
* @brief loads texture data from file
* @param mat assimp material type with texture data
* @param type assimp texture type (diffuse / specular)
* @param type_name string name of texture type
*/
vector<Texture> Model::LoadMaterials(aiMaterial *mat, aiTextureType type, string directory)
{
	vector<Texture> textures;

	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		Texture tex;
		aiString path;
		mat->GetTexture(type, i, &path);

		//texture is embedded
		if(path.data[0] == '*')
			tex = LoadEmbeddedTexture(path);
		else
			tex = Texture(path.C_Str(), directory.c_str());

		//check if load failed
		if(tex.GetTexID() != 0)
		{
			tex.SetTexType(type);
			textures.push_back(tex);
		}
	}

	return textures;
}
/**
* @brief recursively traverses assimp nodes and loads m_meshes attached to each node
* @param node node to traverse and load m_meshes
* @oaram m_scene root assimp node
*/
void Model::ProcessNode(aiNode *node, string directory)
{
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = m_scene->mMeshes[node->mMeshes[i]];
		this->m_meshes.push_back(this->ProcessMesh(mesh, directory));
	}

	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], directory);
	}
}

void Model::LoadBones(aiMesh*mesh, vector<glm::ivec4> &boneIds, vector<glm::vec4> &weights)
{
	//m_animController->m_boneTransforms.resize(m_animController->m_boneTransforms.size() + mesh->mNumBones);
	//weights.resize(mesh->mNumVertices);
	//boneIds.resize(mesh->mNumVertices);

	//fill(boneIds.begin(), boneIds.end(), glm::ivec4(0));
	//fill(weights.begin(), weights.end(), glm::vec4(0));

	//for (int i = 0; i < mesh->mNumBones; i++)
	//{
	//	int boneIndex = 0;
	//	string boneName(mesh->mBones[i]->mName.data);

	//	if(m_animController->m_boneMap.find(boneName) == m_animController->m_boneMap.end())
	//	{
	//		boneIndex = m_animController->m_boneCount;
	//		++m_animController->m_boneCount;
	//	}
	//	else
	//	{
	//		boneIndex = m_animController->m_boneMap[boneName];
	//	}

	//	m_animController->m_boneMap[boneName] = boneIndex;

	//	glm::mat4 offsetMat(0);
	//	MathUtil::AssimpToGLMMat4(&mesh->mBones[i]->mOffsetMatrix, offsetMat);

	//	m_animController->m_boneTransforms[boneIndex] = offsetMat;

	//	for (int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
	//	{
	//		int vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
	//		float weight = mesh->mBones[i]->mWeights[j].mWeight;

	//		//find unused weight and assign
	//		for (int k = 0; k < 4; k++)
	//		{
	//			if(weights[vertexID][k] == 0)
	//			{
	//				boneIds[vertexID][k] = boneIndex;
	//				weights[vertexID][k] = weight;
	//				break;
	//			}
	//		}

	//	}

	//}
}

vector<Vertex> LoadVertices(aiMesh*mesh)
{
	vector<Vertex> vertices;
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		glm::vec3 vert, normal;
		glm::vec2 uv;

		vert.x = mesh->mVertices[i].x;
		vert.y = mesh->mVertices[i].y;
		vert.z = mesh->mVertices[i].z;

		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;

		if(mesh->mTextureCoords[0])
		{
			uv.x = mesh->mTextureCoords[0][i].x;
			uv.y = mesh->mTextureCoords[0][i].y;
		}
		else
			uv = glm::vec2(0.0f, 0.0f);

		vertex.position = vert;
		vertex.normal = normal;
		vertex.uv = uv;

		vertices.push_back(vertex);
	}

	return vertices;
}

//vector<BoneVertex> LoadBoneVertices(aiMesh *mesh, vector<glm::ivec4> boneIds,
//	vector<glm::vec4> weights)
//{
//	vector<BoneVertex> vertices;
//	for (GLuint i = 0; i < mesh->mNumVertices; i++)
//	{
//		BoneVertex vertex;
//
//		glm::vec3 vert, normal;
//		glm::vec2 uv;
//
//		vert.x = mesh->mVertices[i].x;
//		vert.y = mesh->mVertices[i].y;
//		vert.z = mesh->mVertices[i].z;
//
//		normal.x = mesh->mNormals[i].x;
//		normal.y = mesh->mNormals[i].y;
//		normal.z = mesh->mNormals[i].z;
//
//		if(mesh->mTextureCoords[0])
//		{
//			uv.x = mesh->mTextureCoords[0][i].x;
//			uv.y = mesh->mTextureCoords[0][i].y;
//		}
//		else
//			uv = glm::vec2(0.0f, 0.0f);
//
//		vertex.position = vert;
//		vertex.normal = normal;
//		vertex.uv = uv;
//		vertex.boneIds = boneIds[i];
//		vertex.weights = weights[i];
//
//		vertices.push_back(vertex);
//	}
//
//	return vertices;
//}

/**
* @brief initializes new mesh and stores textures in model class
* @param mesh assimp class that stores mesh verts, normals, textures, and uv coordinates
* @oaram m_scene root assimp node
*/
Mesh Model::ProcessMesh(aiMesh *mesh, string directory)
{
	//vertex
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	//bones
	vector<glm::ivec4> boneIds;
	vector<glm::vec4> weights;


	//store the indices which define the order we draw the triangles
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	if(mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = m_scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture>diffuse_maps = LoadMaterials(material, aiTextureType_DIFFUSE, directory);
		textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

		vector<Texture> specularMaps = LoadMaterials(material, aiTextureType_SPECULAR, directory);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	//load skeletal data
	/*if(mesh->HasBones())
	{
		rigged = true;
		LoadBones(mesh, boneIds, weights);

		vector<BoneVertex> boneVertices = LoadBoneVertices(mesh, boneIds, weights);
		return Mesh(boneVertices, indices, textures);
	}
	else
	{
		vertices = LoadVertices(mesh);
	}*/

	vertices = LoadVertices(mesh);

	return Mesh(vertices, indices, textures);
}
