#include "game.hpp"
#define CHUNK_ACTIVE 1

Chunk::Chunk() : m_flag(0)
{
}

bool Chunk::Init(glm::ivec3 chunkIndices, glm::vec3 chunkSize, int voxelSize)
{
	m_chunkIndices = chunkIndices;
	m_chunkSize = chunkSize;
	m_voxelSize = voxelSize;

	m_position = chunkIndices * voxelSize;
	m_position *= chunkSize;

	GenerateMesh();
	
	if (m_root == nullptr) return false;

	return true;
}

glm::vec3 Chunk::GetPosition()
{
	return m_position;
}

void Chunk::GenerateMesh()
{
	vector<float> noiseValues = CalculateSimplexNoise();

	Octree node;
	m_root = node.BuildTree(m_position, m_chunkSize, m_voxelSize, noiseValues);
	
	if (m_root == nullptr)
	{
		m_flag = ~CHUNK_ACTIVE;
		return;
	}

	m_flag |= CHUNK_ACTIVE;
	m_root->GenerateVertexIndices(m_vertices);
	m_root->ContourCellProc(m_triIndices);
	//BindMesh();
}

vector<float> Chunk::CalculateSimplexNoise()
{
	vector<float> noiseValues((float)(m_chunkSize.x * m_chunkSize.z));

	for(int x = 0; x < m_chunkSize.x; x++)
	for(int z = 0; z < m_chunkSize.z; z++)
	{
		//flat plane in y axis
		noiseValues[GETINDEXXZ(x,z)] = 150;
	}

	return noiseValues;
}

void Chunk::BindMesh()
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ebo);

	if ((m_vertices.size() == 0) || m_triIndices.size() == 0)
	{
		m_flag = ~CHUNK_ACTIVE;
		return;
	}

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VoxelVertex) * m_vertices.size(), &m_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_triIndices.size(), &m_triIndices[0], GL_STATIC_DRAW);

	//location 0 should be verts
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (GLvoid*)0);
	//now normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (GLvoid*)offsetof(VoxelVertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(VoxelVertex), (GLvoid*)offsetof(VoxelVertex, textureID));

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}



void Chunk::Render()
{
	if (~m_flag & CHUNK_ACTIVE) return;

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_triIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}