#include "game.hpp"
#include "FastNoiseSIMD.h"
static float g_voxelSize = 32.0f;
static FastNoiseSIMD *fn;
static FastNoiseSIMD *caveGen1;
static FastNoiseSIMD *caveGen2;

float g_time = 0.0f;
omp_lock_t g_thread_lock;

const int g_maxHeight = 64.0f;

float MetaBall(const glm::vec3& worldPosition, const glm::vec3& origin)
{
	/*float radius = glm::length(worldPosition - origin);
	float radiusSQ = radius * radius;
	if (radius == 0) return 1.0f;
	return 1.0f / radiusSQ;*/

	float radius = 256 * 2;

	return Sphere(worldPosition, origin, radius);
}

float Sphere(const glm::vec3& worldPosition, const glm::vec3& origin, float radius)
{
	return glm::length(worldPosition - origin) - radius;
}

float Density_Func(const glm::vec3 & worldPosition)
{
	float time = SDL_GetTicks();

	float noise3D, noise2D;
	float noise = 0.0f;
	float minHeight = 1.0f;
	float noiseMin3D = -0.6f;
	float noiseMax3D = -0.4f;

	glm::vec3 voxelPosition = worldPosition / g_voxelSize;

	////noise2D =  fn.GetSimplex(voxelPosition.x, voxelPosition.z);
	float *noise3DSet = fn->GetSimplexFractalSet(voxelPosition.x, voxelPosition.y, voxelPosition.z, 1, 1, 1, 1.0f);
	noise3D = noise3DSet[0];
	FastNoiseSIMD::FreeNoiseSet(noise3DSet);
	//noise3D =  fn->GetSimplexFractal(voxelPosition.x, voxelPosition.y, voxelPosition.z);
	noise2D = .7989 * noise3D;

	if (worldPosition.y <= 1)
		noise = worldPosition.y - 1.f;
	if (worldPosition.y > 0 && worldPosition.y > (noise2D + noise3D) * g_voxelSize * g_maxHeight)
		noise = worldPosition.y - noise2D * g_voxelSize * g_maxHeight;
	else if (worldPosition.y > 1.0f)
		noise = worldPosition.y - (noise2D + noise3D) * g_maxHeight * g_voxelSize;


	//noise = GetCaveNoise(voxelPosition);
	//noise = 1.0f - noise;
	//noise = noise * 2 - 1;

	//noise = worldPosition.y - noise * g_maxHeight * g_voxelSize;
	omp_set_lock(&g_thread_lock);
	g_time += SDL_GetTicks() - time;
	omp_unset_lock(&g_thread_lock);

	return noise;
}

void DensitySetVoxelSize(const float & voxelSize)
{
	g_voxelSize = voxelSize;
	omp_init_lock(&g_thread_lock);

	fn = FastNoiseSIMD::NewFastNoiseSIMD();
	fn->SetFractalOctaves(8);
	fn->SetFrequency(0.02f);
	fn->SetFractalLacunarity(2.0f);
	fn->SetFractalType(fn->FBM);
	fn->SetFractalGain(0.5);
	fn->SetNoiseType(fn->SimplexFractal);

	//caveGen1->SetFrequency(0.04);
	////caveGen1->SetFractalType(caveGen1->RigidMulti);
	//caveGen1->SetSeed(12345);
	//caveGen1->SetNoiseType(caveGen1->Cellular);
	//caveGen1->SetCellularReturnType(caveGen1->Distance2Div);
	//caveGen1->SetCellularDistanceFunction(caveGen1->Euclidean);
	//caveGen1->SetCellularJitter(0.3f);

	//caveGen2 = caveGen1;
	//caveGen2->SetSeed(54321);

}

float GetCaveNoise(glm::vec3 worldPosition)
{
	float frequency = 0.02;

	glm::vec3 voxelPosition = worldPosition / g_voxelSize;

	float noise1 = 0.0f;
	/*float noise1 = caveGen1.GetCellular(voxelPosition.x, voxelPosition.y, voxelPosition.z);

	if (noise1 < 0)
	{
		float a = 1;
	}*/

	noise1 = 1.0f - noise1;
	noise1 = noise1 * 2 - 1;
	return noise1;
	/*glm::vec3 worldPos2 = worldPosition + glm::vec3(frequency * 0.5f);
	float noise2 = caveGen2.GetCellular(worldPos2.x, worldPos2.y, worldPos2.z);

	return glm::min(noise1, noise2);*/
}

void GenerateMaterialIndices(const glm::vec3 &chunkPos, const glm::vec3 &chunkSize, vector<int> &materialIndices)
{
	materialIndices.resize((chunkSize.x + 1) *  (chunkSize.y + 1) * (chunkSize.z + 1), -1);
	
	glm::vec3 voxelPos = chunkPos / g_voxelSize;
	glm::vec3 setSize = chunkSize + glm::vec3(1.0f);
	float *noiseSet = fn->GetNoiseSet(voxelPos.x, voxelPos.y, voxelPos.z, setSize.x, setSize.y, setSize.z, 1.0f);

	for (int x = 0; x <= chunkSize.x; x++)
	{
		for (int y = 0; y <= chunkSize.y; y++)
		{
			for (int z = 0; z <= chunkSize.z; z++)
			{
				glm::vec3 worldPosition = chunkPos + glm::vec3(x, y, z) * g_voxelSize;

				int index = GETINDEXCHUNK(glm::ivec3(chunkSize + glm::vec3(1.0f)), x, y, z);
				materialIndices[index] = worldPosition.y - noiseSet[index] * g_maxHeight * g_voxelSize >= 0 ? MATERIAL_AIR : MATERIAL_SOLID;
			}
		}
	}

	FastNoiseSIMD::FreeNoiseSet(noiseSet);
}