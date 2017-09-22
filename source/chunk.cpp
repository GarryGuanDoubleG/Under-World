#include "game.hpp"

#define CHUNK_ACTIVE 1


static const glm::vec3 AXIS_OFFSET[3] =
{
	glm::vec3(1.f, 0.f, 0.f),
	glm::vec3(0.f, 1.f, 0.f),
	glm::vec3(0.f, 0.f, 1.f)
};


Chunk::Chunk() : m_flag(0), m_init(false)
{
}

bool Chunk::Init(glm::ivec3 chunkIndices, glm::vec3 chunkSize, int voxelSize)
{
	m_chunkIndices = chunkIndices;
	m_chunkSize = chunkSize;
	m_voxelSize = voxelSize;

	m_position = chunkIndices * voxelSize;
	m_position *= chunkSize;

	m_init = true;
	
	m_densityType = m_position.y >= 0 ? Density::Terrain : Density::Cave;
	Density::GenerateMaterialIndices(m_densityType, m_position, m_chunkSize, m_materialIndices);
	GenerateHermiteField();
	GenerateMesh();
	
	if(m_root == nullptr) return false;

	return true;
}

bool Chunk::Close()
{
	
}

glm::vec3 Chunk::GetPosition()
{
	return m_position;
}

void Chunk::GenerateMesh()
{
	//find active nodes & their edge crossings
	for (int x = 0; x < m_chunkSize.x; x++)
	{
		for (int y = 0; y < m_chunkSize.y; y++)
		{
			for (int z = 0; z < m_chunkSize.z; z++)
			{
				int corners = 0;

				for (int i = 0; i < 8; i++)
				{
					glm::ivec3 cornerIndex = glm::vec3(x, y, z) + CHILD_MIN_OFFSETS[i];
					corners |= (m_materialIndices[GETINDEXCHUNK(glm::ivec3(m_chunkSize + glm::vec3(1.0f)), cornerIndex.x, cornerIndex.y, cornerIndex.z)] << i);
				}

				//no material change. Isosurface not in voxel
				if(corners == 0 || corners == 255) continue;

				Octree *node = new Octree;

				glm::vec3 leafPos = m_position + glm::vec3(x, y, z) * (float)m_voxelSize;
				node->InitNode(leafPos, m_voxelSize, corners);
				FindEdgeCrossing(node, m_hermiteMap);

				if(node->m_flag & OCTREE_ACTIVE)
					m_nodeMap.insert(std::pair<glm::vec3, Octree*>(leafPos, node));
				else
					delete node;
			}
		}
	}

	m_root = BottomUpTreeGen(m_nodeMap, m_position);

	if(m_root == nullptr)
	{
		m_flag = ~CHUNK_ACTIVE;
		return;
	}

	m_flag |= CHUNK_ACTIVE;

	
	m_root->ClusterCellBase(2000000.f);
	m_root->GenerateVertexBuffer(m_vertices);
	m_root->ProcessCell(m_triIndices, 2000000.f);


	slog("Finished Chunk Index: %i, %i, %i\n", m_chunkIndices.x, m_chunkIndices.y, m_chunkIndices.z);
}

void Chunk::GenerateHermiteField()
{
	//loop through each voxel and calculate edge crossings
	for (int x = 0; x <= m_chunkSize.x; x++)
	{
		for (int y = 0; y <= m_chunkSize.y; y++)
		{
			for (int z = 0; z <= m_chunkSize.z; z++)
			{
				for (int axis = 0; axis < 3; axis++)
				{
					glm::ivec3 index2 = glm::vec3(x, y, z) + AXIS_OFFSET[axis];
					
					if(GETINDEXCHUNK(glm::ivec3(m_chunkSize + glm::vec3(1.0f)), index2.x, index2.y, index2.z) > m_materialIndices.size())
						continue;

					int m1 = m_materialIndices[GETINDEXCHUNK(glm::ivec3(m_chunkSize + glm::vec3(1.0f)), x, y, z)];
					int m2 = m_materialIndices[GETINDEXCHUNK(glm::ivec3(m_chunkSize + glm::vec3(1.0f)), index2.x, index2.y, index2.z)];

					//no material change, no edge
					if(m1 == m2) 
						continue;

					glm::vec3 p1 = m_position + glm::vec3(x, y, z) * (float)m_voxelSize;
					glm::vec3 p2 = m_position + (glm::vec3(x, y, z) + AXIS_OFFSET[axis]) * (float)m_voxelSize;

					//get hermite data
					glm::vec3 p = Density::FindIntersection(m_densityType, p1, p2);
					glm::vec3 n = Density::CalculateNormals(m_densityType, p);

					//store in map
					EdgeInfo edge(p, n);
					m_hermiteMap[(p1 + p2) *.5f] = edge;
				}
			}
		}
	}

}

void Chunk::BindMesh()
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ebo);

	if((m_vertices.size() == 0) || m_triIndices.size() == 0)
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (GLvoid*)offsetof(VoxelVertex, position));
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
	if(~m_flag & CHUNK_ACTIVE) return;

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_triIndices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}