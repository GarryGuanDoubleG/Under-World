#pragma once

#define MATERIAL_SOLID 1
#define MATERIAL_AIR 0

class Density
{
	static FastNoise terrainFN;

	static FastNoiseSIMD *terrainFNSIMD;
	static FastNoiseSIMD *caveFNSIMD;

public:
	static float densityGenTime;

	enum DensityType {Terrain, Cave};
public:
	static void SetVoxelSize(const float & voxelSize);

	static float GetSphere(const glm::vec3& worldPosition, const glm::vec3& origin, float radius);
	static float GetTerrainDensity(const glm::vec3& worldPosition, float noise3D, float noise2D = 0);
	static float GetTerrain(const glm::vec3 & worldPosition);
	
	static float GetDensity(DensityType type, const glm::vec3 &worldPosition);
	static float *GetDensitySet(DensityType type, const vector<glm::vec3> &positionSet);

	static void FreeSet(float * set);

	static void GenerateTerrainIndices(const glm::vec3 &chunkPos, const glm::vec3 &chunkSize, vector<int> &materialIndices);
	static void GenerateCaveIndices(const glm::vec3 & chunkPos, const glm::vec3 & chunkSize, vector<int>& materialIndices);
	static void GenerateMaterialIndices(DensityType type, const glm::vec3 & chunkPos, const glm::vec3 & chunkSize, vector<int>& materialIndices);

	static float GetCaveNoise(glm::vec3 worldPosition);

};
