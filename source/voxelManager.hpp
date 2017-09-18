#pragma once

class VoxelManager
{
	int m_voxelSize, 
		m_renderRange,
		m_chunkSize;
	glm::ivec3 m_playerPos;

	vector<Chunk> m_chunks;
public:
	VoxelManager();

	void Init();
	void Close();
	void Update();
	void Render();
};