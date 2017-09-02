#include "game.hpp"

VoxelManager::VoxelManager()
{

}

void VoxelManager::Init()
{
	m_voxelSize = 64.f;
	m_renderRange = 3;
	m_chunkSize = 16;
	 
	std::cout << "Voxel Init\n";
	float time = g_game->GetElapsedTime();

	//draw chunks within render range of player
	glm::ivec3 playerChunkIndex = g_game->GetPlayerPosition();
	playerChunkIndex /= (m_chunkSize * m_voxelSize);

	int x_range = 2 * m_renderRange + 1;
	int z_range = x_range;
	int y_range = 1;

	m_chunks = vector<Chunk>(x_range * y_range * z_range);
	
	int i = 0;
	std::vector<std::thread> threads;
	for (int x = -m_renderRange; x <= m_renderRange; x++)
	for (int z = -m_renderRange; z <= m_renderRange; z++)
	for (int y = 0; y <= 0; y++)
	{
		threads.push_back(thread([&](Chunk *chunk) { chunk->Init(playerChunkIndex + glm::ivec3(x, y, z), glm::vec3(m_chunkSize), m_voxelSize); }, &m_chunks[i++]));
		if (threads.size() >= 4)
		{
			for (auto &t : threads)
				t.join();
			threads.clear();
		}
		//m_chunks[i++].Init(playerChunkIndex + glm::ivec3(x, y, z), glm::vec3(m_chunkSize), m_voxelSize);
	}

	for (auto &t : threads)
		t.join();


	int verts = 0, triangles = 0;
	for (auto &chunk : m_chunks) {
		chunk.BindMesh();
		verts += chunk.m_vertices.size();
		triangles += chunk.m_triIndices.size();
	}

	std::cout << "Time to Generate (milisecond): " << g_game->GetElapsedTime() - time << endl;
	std::cout << "Vertices: " << verts << endl << "Triangles: " << triangles << endl;
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