#pragma once
#define BONES_PER_VERTEX 4

typedef struct Vertex_S
{
	Vertex_S() {};
	Vertex_S(glm::vec3 pos, glm::vec3 norm, glm::vec2 texel)
		: position(pos), normal(norm), uv(texel) {};
	glm::vec3 position; /**< vertex postion in ndc */
	glm::vec3 normal; /**< normal value of vertex */
	glm::vec2 uv; /**< texture coordinates */
}Vertex;

struct Weight
{
	GLuint vertexID;
	float weight;
};

struct BoneData
{
	string name;
	vector<Weight> weights;
	glm::mat4 offset;
};


struct VertexBone
{
	int indices[BONES_PER_VERTEX];
	float weights[BONES_PER_VERTEX];
};

/**
* Mesh class that contains data on components of a model
*/
class Mesh
{
	friend class Model;

	std::vector<Vertex> m_vertices; /**<container for vertices in mesh  */
	std::vector<VertexBone> m_boneVertices;
	std::vector<glm::mat4>m_boneTransforms;
	std::vector<GLuint> m_indices; /**< container for order in which to draw triangles*/
	std::vector<Texture> m_textures; /**< container for textures used in this mesh*/

	GLuint m_vao, /**< vertex array object, stores gpu state for drawing this mesh */
		m_vbo, /**<vertex buffer object, stores the vertex data of this mesh */
		m_ebo;/**<element buffer object, stores the order to draw the triangles  */

	GLboolean m_instanced;
	GLuint	m_instanceCount;
public:
	/**
	* Constructor
	* stores a reference to vertex, index, and texture containers
	*/
	Mesh(vector<Vertex> &vertices, vector<GLuint> &indices, vector<Texture> &textures);

	/**
	* @brief draws all the vertices and textures stored in this mesh
	* @param the shader to use for drawing
	*/
	void Draw(GLuint shader);

	/**
	* @brief additional buffer for instance rendering
	*/
	void MeshSetInstance(GLuint instanceBuffer, GLuint amount);

	/**
	* @brief binds all the vertex data to a vertex array object
	*/
	void MeshInit();

	void MeshInitRigged();
};
