#pragma once

class Chunk
{
	Octree * m_root;

	int m_flag;
	int m_voxelSize;

	glm::vec3 m_chunkSize;
	glm::vec3 m_position;
	glm::ivec3 m_chunkIndices;

	//proc mesh
	GLuint m_vao, m_vbo, m_ebo;
public:
	vector<VoxelVertex> m_vertices;
	vector<GLuint> m_triIndices;

	Chunk();

	bool Init(glm::ivec3 chunkIndices, glm::vec3 chunkSize, int voxelSize);

	glm::vec3 GetPosition();

	void GenerateMesh();

	vector<float> CalculateSimplexNoise();
	void BindMesh();
	
	void Render();
};