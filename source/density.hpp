#pragma once

#define MATERIAL_SOLID 1
#define MATERIAL_AIR 0

extern float g_time;

float MetaBall(const glm::vec3& worldPosition, const glm::vec3& origin);
float Sphere(const glm::vec3& worldPosition, const glm::vec3& origin, float radius);
float Density_Func(const glm::vec3 & worldPosition);

void GenerateMaterialIndices(const glm::vec3 &chunkPos, const glm::vec3 &chunkSize, vector<int> &materialIndices);
void DensitySetVoxelSize(const float & voxelSize);

float GetCaveNoise(glm::vec3 worldPosition);
