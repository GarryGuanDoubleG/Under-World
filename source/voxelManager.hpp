#pragma once

class VoxelManager
{
	int m_voxelSize, 
		m_renderRange,
		m_chunkSize;
	glm::ivec3 m_playerPos;

	unordered_map<glm::ivec3, Chunk*> m_chunkMap;

public:
	VoxelManager();

	void Init();
	void AssignChunkNeighbors(Chunk *chunk);
	void Close();
	void Update();
	void Render();
};