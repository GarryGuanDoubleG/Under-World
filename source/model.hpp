#pragma once

/**
*3D model class. Supports most file formats. Imports and parses model file and stores mesh information
*to use for rendering
*/
class Model
{
private:

	AnimController *m_animController;
	Assimp::Importer m_importer;
	const aiScene *m_scene;
	glm::mat4 m_invTransform;

	vector<Mesh> m_meshes; /**<container for all meshes in model */

	bool rigged;

public:

	Model(string &filepath);

	Model(const string &filepath, const vector<string> &animPaths, const vector<string> &animNames);

	/** Destructor
	*
	*/
	~Model();

	bool IsRigged();

	/**
	* @brief goes through meshes and updates them to use instanced rendering
	* @param buffer the buffer with model matrix data
	* @param amount number of times to instance
	*/
	void SetInstanceRendering(GLuint buffer, GLuint amount);

	/**
	* @brief returns the meshes that this model contains
	* @return vector container of this model's meshes
	*/
	vector<Mesh> *GetMesh();

	/**
	* @brief renders all the meshes of this model
	* @param shader compiled shader id to use to render
	*/
	void Draw(Shader *shader);

	/**
	* @brief renders all the meshes of this model
	* @param shader compiled shader id to use to render
	* @param time in seconds used to interpolate animation vertices
	*/
	void Draw(Shader *shader, string animation, float timeInSeconds);
private:
	/**
	*@brief Load textures that are embedded into the model (.fbx)
	*@param embeded Index starting with *
	*@return texture uncompressed and loaded
	*/
	Texture LoadEmbeddedTexture(aiString embededIndex);

	void LoadAnimations(const vector<string> &animations, const vector<string> &animNames);
	/**
	*@brief Reads model file using assimp to get root node of model.
	*@param path filepath of model
	*/
	void LoadModel(string path);

	void LoadModel(string model_path, const vector<string> &animations, const vector<string> &animNames);

	/**
	*@brief Loads bones
	*@param scene, root assimp container
	*/
	void LoadBones(aiMesh*mesh, vector<glm::ivec4> &boneIds, vector<glm::vec4> &weights);
	/**
	* @brief recursively traverses assimp nodes and loads meshes attached to each node
	* @param node node to traverse and load meshes
	* @param scene root assimp node
	* @param directory directory of model that's being loaded
	*/
	void ProcessNode(aiNode * node, string directory);
	/**
	* @brief initializes new mesh and stores textures in model class
	* @param mesh assimp class that stores mesh verts, normals, textures, and uv coordinates
	* @oaram scene root assimp node
	*/
	Mesh ProcessMesh(aiMesh * mesh, string directory);
	/**
	* @brief loads texture data from file
	* @param mat assimp material type with texture data
	* @param type assimp texture type (diffuse / specular)
	* @param type_name string name of texture type
	*/
	vector<Texture> LoadMaterials(aiMaterial *mat, aiTextureType type, string directory);

};