#include "game.hpp"

VoxelManager::VoxelManager()
{

}

void VoxelManager::Init()
{
	m_voxelSize = 1024;
	m_renderRange = 3;
	m_chunkSize = 16;
	 
	std::cout << "Voxel Init\n";
	float time = g_game->GetElapsedTime();

	const glm::ivec3 playerChunkIndex = glm::ivec3(0, 0, 0);
	m_playerPos = playerChunkIndex;

	int x_range = 2 * m_renderRange + 1;
	int z_range = x_range;
	int y_range = 3;
	int totalChunks = x_range * y_range * z_range;

	m_chunkMap.reserve(totalChunks);
	m_chunkMap.clear();

	Chunk *chunks = new Chunk[totalChunks];

	Density::SetVoxelSize(m_voxelSize);
	Density::SetMaxVoxelHeight(m_chunkSize * (y_range / 2 + 1));
	Density::Initialize();

	glm::vec3 playerPos = glm::vec3(0);

	for (int x = -m_renderRange; x <= m_renderRange; x++)
	{ 
		omp_set_num_threads(4);
		for (int z = -m_renderRange; z <= m_renderRange; z++)
		{
			#pragma omp parallel for
			for (int y = -1; y <= 1; y++)
			{
				int chunkIndex = (y_range * x_range) * (x + m_renderRange) + y_range * (z + m_renderRange) + (y + 1);
				chunks[chunkIndex].Init(playerChunkIndex + glm::ivec3(x, y, z), glm::vec3(m_chunkSize), m_voxelSize);			
			}
		}
	}

	for (int i = 0; i < totalChunks; i++)
	{
		Chunk * chunk = &chunks[i];

		m_chunkMap[chunk->m_chunkIndex] = chunk;
		m_renderList.push_back(chunk);
	}

	int verts = 0, triangles = 0;
	float heightMapTime = 0, materialTime = 0, findVoxelTime = 0, hermiteTime = 0, meshTime = 0;
	for (auto &value : m_chunkMap) {
		if (!value.second) continue;

		Chunk *chunk = value.second;
		AssignChunkNeighbors(value.second);
		chunk->GenerateSeam();
		chunk->BindMesh();

		//measure logistics
		verts += chunk->m_vertices.size();
		triangles += chunk->m_triIndices.size();

		heightMapTime += chunk->heightMapTime;
		materialTime += chunk->materialTime;
		findVoxelTime += chunk->findVoxelTime;
		hermiteTime += chunk->hermiteTime;
		meshTime += chunk->meshTime;
	}
	SlogCheckGLError();
	std::cout << "Time to Generate (milisecond): " << g_game->GetElapsedTime() - time << endl;
	std::cout << "Average time " << (g_game->GetElapsedTime() - time) / m_chunkMap.size() << endl;
	std::cout << "HeightMap Time: " << heightMapTime << std::endl;
	std::cout << "materialTime: " << materialTime << std::endl;
	std::cout << "Find Voxel Time: " << findVoxelTime << std::endl;
	std::cout << "Hermite Time: " << hermiteTime << std::endl;
	std::cout << "mesh Time: " << meshTime << std::endl;
	std::cout << "Density Time: " << Density::densityGenTime << endl;
	std::cout << "Vertices: " << verts << endl << "Triangles: " << triangles << endl;
}

void VoxelManager::AssignChunkNeighbors(Chunk *chunk)
{
	if (!chunk) return;
	const glm::ivec3 chunkIndex = chunk->m_chunkIndex;

	chunk->AssignNeighbor(m_chunkMap[chunkIndex + glm::ivec3(0, 1, 0)], TOP);
	chunk->AssignNeighbor(m_chunkMap[chunkIndex + glm::ivec3(0, -1, 0)], BOTTOM);
	chunk->AssignNeighbor(m_chunkMap[chunkIndex + glm::ivec3(-1, 0, 0)], LEFT);
	chunk->AssignNeighbor(m_chunkMap[chunkIndex + glm::ivec3(1, 0, 0)], RIGHT);
	chunk->AssignNeighbor(m_chunkMap[chunkIndex + glm::ivec3(0, 0, -1)], BACK);
	chunk->AssignNeighbor(m_chunkMap[chunkIndex + glm::ivec3(0, 0, 1)], FRONT);
	chunk->AssignNeighbor(m_chunkMap[chunkIndex + glm::ivec3(1, 0, 1)], FRONT_RIGHT);
}

void VoxelManager::Close()
{
	m_chunkMap.empty();
}

void VoxelManager::GenerateChunksInRange()
{	
	for (int x = -m_renderRange; x <= m_renderRange; x++)
	{
		for (int z = -m_renderRange; z <= m_renderRange; z++)
		{
			//#pragma omp parallel for
			for (int y = -1; y <= 1; y++)
			{
				glm::ivec3 chunkIndex = m_playerPos + glm::ivec3(x, y, z);
				if (m_chunkMap.find(chunkIndex) != m_chunkMap.end() && m_chunkMap[chunkIndex] != nullptr)
					continue;
				Chunk *chunk = new Chunk();
				chunk->Init(chunkIndex, glm::vec3(m_chunkSize), m_voxelSize);
				m_newChunks.push_back(chunk);
				m_chunkMap[chunkIndex] = chunk;
			}
		}
	}
	
	for (auto &chunk : m_newChunks)
	{
		AssignChunkNeighbors(chunk);
		chunk->GenerateSeam();
	}

	if(m_newChunks.size() != 0)
		cout << "Number of new Chunks: " << m_newChunks.size() << endl;
}

void VoxelManager::Update()
{
	//const glm::ivec3 playerChunkIndex = glm::vec3(1, 0, 1) *g_game->GetPlayerPosition() / (float)m_voxelSize / (float)m_chunkSize;
	//if (m_playerPos != playerChunkIndex)
	//{
	//	m_playerPos = playerChunkIndex;
	//	//#pragma omp concurrent
	//	GenerateChunksInRange();
	//}

	//if (m_newChunks.size() != 0)
	//{
	//	for (auto &chunk : m_newChunks) 
	//	{
	//		chunk->BindMesh();
	//		m_renderList.push_back(chunk);
	//	}
	//	m_newChunks.clear();
	//}
}

void VoxelManager::Render()
{
	for (auto &chunk : m_renderList )
	{
		if (chunk) chunk->Render();
	}
}