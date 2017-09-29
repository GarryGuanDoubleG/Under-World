#include "game.hpp"

VoxelManager::VoxelManager()
{

}

void VoxelManager::Init()
{
	m_voxelSize = 256;
	m_renderRange = 1;
	m_chunkSize = 8;
	 
	std::cout << "Voxel Init\n";
	float time = g_game->GetElapsedTime();

	glm::ivec3 playerChunkIndex = glm::ivec3(0, 0, 0);

	int x_range = 2 * m_renderRange + 1;
	int z_range = x_range;
	int y_range = 3;

	m_chunks = vector<Chunk>(x_range * y_range * z_range);

	Density::SetVoxelSize(m_voxelSize);
	Density::SetMaxVoxelHeight(m_chunkSize * (y_range / 2 + 1));
	Density::Initialize();

	for (int x = -m_renderRange; x <= m_renderRange; x++)
	{ 
		for (int z = -m_renderRange; z <= m_renderRange; z++)
		{
			#pragma omp parallel for
			for (int y = -1; y <= 1; y++)
			{
				int chunkIndex = (y_range * x_range) * (x + m_renderRange) + y_range * (z + m_renderRange) + (y + 1);
				m_chunks[chunkIndex].Init(playerChunkIndex + glm::ivec3(x, y, z), glm::vec3(m_chunkSize), m_voxelSize);
			}
		}
	}

	int verts = 0, triangles = 0;
	for (auto &chunk : m_chunks) {
		chunk.BindMesh();
		verts += chunk.m_vertices.size();
		triangles += chunk.m_triIndices.size();
	}

	std::cout << "Time to Generate (milisecond): " << g_game->GetElapsedTime() - time << endl;
	std::cout << "Density Time: " << Density::densityGenTime << endl;
	std::cout << "Vertices: " << verts << endl << "Triangles: " << triangles << endl;
}

void VoxelManager::Close()
{
	m_chunks.empty();
}

void VoxelManager::Update()
{

}

void VoxelManager::Render()
{
	for (vector<Chunk>::iterator it = m_chunks.begin(); it < m_chunks.end(); ++it)
	{
		it->Render();
	}
}