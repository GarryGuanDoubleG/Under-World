#pragma once

class Chunk
{
	Octree * m_root;

	int m_flag;
	int m_voxelSize;
	Density::DensityType m_densityType;

	glm::vec3 m_chunkSize;
	glm::vec3 m_position;
	glm::ivec3 m_chunkIndices;

	//proc mesh
	GLuint m_vao, m_vbo, m_ebo;
public:
	bool m_init;
	unordered_map<glm::vec3, EdgeInfo> m_hermiteMap;
	vector<int> m_materialIndices;
	unordered_map<glm::vec3, Octree*> m_nodeMap;

	vector<VoxelVertex> m_vertices;
	vector<GLuint> m_triIndices;

	Chunk();

	bool Init(glm::ivec3 chunkIndices, glm::vec3 chunkSize, int voxelSize);
	bool Close();
	glm::vec3 GetPosition();

	void GenerateMesh();
	void GenerateHermiteField();
	void GenerateCaves();
	void BindMesh();
	
	void Render();
};