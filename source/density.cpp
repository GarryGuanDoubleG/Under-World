#include "game.hpp"
static float g_voxelSize = 32.0f;

float Density::densityGenTime;
FastNoise Density::terrainFN;
FastNoiseSIMD *Density::terrainFNSIMD;
FastNoiseSIMD *Density::caveFNSIMD;

omp_lock_t g_thread_lock;

const int g_maxHeight = 32.0f;


void Density::SetVoxelSize(const float & voxelSize)
{
	g_voxelSize = voxelSize;
	omp_init_lock(&g_thread_lock);


	terrainFN.SetFractalOctaves(8);
	terrainFN.SetFrequency(0.02f);
	terrainFN.SetFractalLacunarity(2.0f);
	terrainFN.SetFractalType(terrainFN.FBM);
	terrainFN.SetFractalGain(0.5);
	terrainFN.SetNoiseType(terrainFN.SimplexFractal);

	terrainFNSIMD = FastNoiseSIMD::NewFastNoiseSIMD();
	terrainFNSIMD->SetFractalOctaves(8);
	terrainFNSIMD->SetFrequency(0.02f);
	terrainFNSIMD->SetFractalLacunarity(2.0f);
	terrainFNSIMD->SetFractalType(terrainFNSIMD->FBM);
	terrainFNSIMD->SetFractalGain(0.5);
	terrainFNSIMD->SetNoiseType(terrainFNSIMD->SimplexFractal);

	caveFNSIMD = FastNoiseSIMD::NewFastNoiseSIMD();
	caveFNSIMD->SetFrequency(0.03);
	caveFNSIMD->SetSeed(12345);
	caveFNSIMD->SetNoiseType(caveFNSIMD->Cellular);
	caveFNSIMD->SetCellularReturnType(caveFNSIMD->Distance2Cave);
	caveFNSIMD->SetCellularDistanceFunction(caveFNSIMD->Natural);
	caveFNSIMD->SetCellularJitter(0.4f);
	caveFNSIMD->SetPerturbAmp(caveFNSIMD->Gradient);
	caveFNSIMD->SetPerturbFrequency(0.6f);
}

float Density::GetSphere(const glm::vec3& worldPosition, const glm::vec3& origin, float radius)
{
	return glm::length(worldPosition - origin) - radius;
}

float Density::GetTerrainDensity(const glm::vec3& worldPosition, float noise3D, float noise2D)
{
	float terrain = 0.0f;
	
	if (worldPosition.y <= 1)
		terrain = worldPosition.y - 1.f;
	else if (worldPosition.y > (noise2D + noise3D) * g_voxelSize * g_maxHeight)
		terrain = worldPosition.y - noise2D * g_voxelSize * g_maxHeight;
	else if (worldPosition.y > 1.0f)
		terrain = worldPosition.y - (noise2D + noise3D) * g_maxHeight * g_voxelSize;

	return terrain;
}

float Density::GetTerrain(const glm::vec3 & worldPosition)
{
	float time = SDL_GetTicks();

	float noise3D, noise2D;
	float noise = 0.0f;

	glm::vec3 voxelPosition = worldPosition / g_voxelSize;

	noise3D = terrainFN.GetSimplexFractal(voxelPosition.x, voxelPosition.y, voxelPosition.z);
	noise2D = noise3D * .7989;
	//noise2D = terrainFN.GetSimplex(voxelPosition.x, voxelPosition.z);
	noise = GetTerrainDensity(worldPosition, noise3D, noise2D);

	omp_set_lock(&g_thread_lock);
	densityGenTime += SDL_GetTicks() - time;
	omp_unset_lock(&g_thread_lock);

	return noise;
}

float Density::GetDensity(DensityType type, const glm::vec3 & worldPosition)
{
	switch(type)
	{
	case Terrain:
		return GetTerrain(worldPosition);
		break;
	case Cave:
		return GetCaveNoise(worldPosition);
		break;
	default:
		break;
	}
}

float *Density::GetDensitySet(DensityType type, const vector<glm::vec3> & positions)
{
	float *set = FastNoiseSIMD::GetEmptySet(positions.size());
	FastNoiseVectorSet positionSet(positions.size());

	for (int i = 0; i < positions.size(); i++)
	{
		positionSet.xSet[i] = positions[i].x / g_voxelSize;
		positionSet.ySet[i] = positions[i].y / g_voxelSize;
		positionSet.zSet[i] = positions[i].z / g_voxelSize;
	}

	switch (type)
	{
	case Terrain:
	{
		terrainFNSIMD->FillNoiseSet(set, &positionSet);
		for (int i = 0; i < positions.size(); i++)
		{
			set[i] = GetTerrainDensity(positions[i], set[i], set[i] * .7989);
		}
		break;
	}
	case Cave:
		caveFNSIMD->FillNoiseSet(set, &positionSet);
		break;
	default:
		break;
	}

	positionSet.Free();
	return set;
}

void Density::FreeSet(float *set)
{
	FastNoiseSIMD::FreeNoiseSet(set);
}

float Density::GetCaveNoise(glm::vec3 worldPosition)
{
	glm::vec3 voxelPosition = worldPosition / g_voxelSize;

	float noise = 0.0f;

	float *noiseSet = caveFNSIMD->GetNoiseSet(voxelPosition.x, voxelPosition.y, voxelPosition.z, 1, 1, 1, 1.0f);
	noise = noiseSet[0];
	FastNoiseSIMD::FreeNoiseSet(noiseSet);

	return noise;
}

void Density::GenerateTerrainIndices(const glm::vec3 &chunkPos, const glm::vec3 &chunkSize, vector<int> &materialIndices)
{
	float time = SDL_GetTicks();

	materialIndices.resize((chunkSize.x + 1) *  (chunkSize.y + 1) * (chunkSize.z + 1), -1);
	
	glm::vec3 voxelPos = chunkPos / g_voxelSize;
	glm::vec3 setSize = chunkSize + glm::vec3(1.0f);
	
	float *noiseSet = terrainFNSIMD->GetNoiseSet(voxelPos.x, voxelPos.y, voxelPos.z, setSize.x, setSize.y, setSize.z);

	for (int x = 0; x <= chunkSize.x; x++)
	{
		for (int y = 0; y <= chunkSize.y; y++)
		{
			for (int z = 0; z <= chunkSize.z; z++)
			{
				int index = GETINDEXCHUNK(glm::ivec3(chunkSize + glm::vec3(1.0f)), x, y, z);
				glm::vec3 worldPosition = chunkPos + glm::vec3(x, y, z) * g_voxelSize;
				
				float density = GetTerrainDensity(worldPosition, noiseSet[index], noiseSet[index] * .7989);
				materialIndices[index] = density > 0 ? MATERIAL_AIR : MATERIAL_SOLID;
			}
		}
	}

	FastNoiseSIMD::FreeNoiseSet(noiseSet);


	omp_set_lock(&g_thread_lock);
	densityGenTime += SDL_GetTicks() - time;
	omp_unset_lock(&g_thread_lock);
}

void Density::GenerateCaveIndices(const glm::vec3 &chunkPos, const glm::vec3 &chunkSize, vector<int> &materialIndices)
{
	float time = SDL_GetTicks();

	materialIndices.resize((chunkSize.x + 1) *  (chunkSize.y + 1) * (chunkSize.z + 1), -1);

	glm::vec3 voxelPos = chunkPos / g_voxelSize;
	glm::vec3 setSize = chunkSize + glm::vec3(1.0f);

	float *noiseSet = caveFNSIMD->GetNoiseSet(voxelPos.x, voxelPos.y, voxelPos.z, setSize.x, setSize.y, setSize.z);
	float caveThreshold = 0.75f;

	for (int x = 0; x <= chunkSize.x; x++)
	{
		for (int y = 0; y <= chunkSize.y; y++)
		{
			for (int z = 0; z <= chunkSize.z; z++)
			{
				int index = GETINDEXCHUNK(glm::ivec3(chunkSize + glm::vec3(1.0f)), x, y, z);				
				materialIndices[index] = noiseSet[index] > caveThreshold ? MATERIAL_AIR : MATERIAL_SOLID;
			}
		}
	}

	FastNoiseSIMD::FreeNoiseSet(noiseSet);

	omp_set_lock(&g_thread_lock);
	densityGenTime += SDL_GetTicks() - time;
	omp_unset_lock(&g_thread_lock);
}

void Density::GenerateMaterialIndices(DensityType type, const glm::vec3 &chunkPos, const glm::vec3 &chunkSize, vector<int> &materialIndices)
{
	switch (type)
	{
	case DensityType::Terrain:
		GenerateTerrainIndices(chunkPos, chunkSize, materialIndices);
		break;
	case Cave:
		GenerateCaveIndices(chunkPos, chunkSize, materialIndices);
		break;
	default:
		break;
	}
}