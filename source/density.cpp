#include "game.hpp"

static float g_voxelSize = 32.0f;
static FastNoise fn;
static FastNoise caveGen;

float g_time = 0.0f;
omp_lock_t g_thread_lock;

const int g_maxHeight = 31.0f;

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

	noise2D = fn.GetSimplex(voxelPosition.x, voxelPosition.z);
	noise3D = fn.GetSimplexFractal(voxelPosition.x, voxelPosition.y, voxelPosition.z);

	if (worldPosition.y > noise2D * g_voxelSize * g_maxHeight)
		noise = worldPosition.y - noise2D * g_voxelSize * g_maxHeight;
	else if (worldPosition.y <= g_voxelSize)
		noise = worldPosition.y - 1.0f;
	else
		noise = worldPosition.y - (noise2D + noise3D) * g_maxHeight * g_voxelSize;

	//float noise = worldPosition.y - 10;

	omp_set_lock(&g_thread_lock);
	g_time += SDL_GetTicks() - time;
	omp_unset_lock(&g_thread_lock);

	return noise;
}

void DensitySetVoxelSize(const float & voxelSize)
{
	g_voxelSize = voxelSize;
	omp_init_lock(&g_thread_lock);

	fn.SetFractalOctaves(8);
	fn.SetFrequency(0.02f);
	fn.SetFractalLacunarity(2.0f);
	fn.SetFractalType(fn.FBM);
	fn.SetFractalGain(0.5);
	fn.SetNoiseType(fn.SimplexFractal);

	caveGen.SetFrequency(0.01);
	caveGen.SetFractalOctaves(4);
	caveGen.SetFractalLacunarity(2);
	caveGen.SetFractalGain(0.5);
	caveGen.SetFractalType(caveGen.Billow);
	caveGen.SetSeed(0);
	caveGen.SetNoiseType(caveGen.SimplexFractal);
}

vector<int> GetMaterialIndices(const glm::vec3 &chunkPos, const glm::vec3 &chunkSize)
{
	vector<int> materialIndices;
	materialIndices.reserve((chunkSize.x + 1) *  (chunkSize.y + 1) * (chunkSize.z + 1));
	
	for (int i = 0; i < (chunkSize.x + 1) * (chunkSize.y + 1) * (chunkSize.z + 1); i++)
		materialIndices.push_back(-1);


	for (int x = 0; x <= chunkSize.x; x++)
	{
		for (int y = 0; y <= chunkSize.y; y++)
		{
			for (int z = 0; z <= chunkSize.z; z++)
			{
				glm::vec3 worldPosition = chunkPos + glm::vec3(x, y, z) * g_voxelSize;

				float noise = Density_Func(worldPosition);
				materialIndices[GETINDEXCHUNK(glm::ivec3(chunkSize + glm::vec3(1.0f)), x, y, z)] = noise >= 0 ? MATERIAL_AIR : MATERIAL_SOLID;
			}
		}
	}

 	return materialIndices;
}