#pragma once
#include "FastNoise.h"

#define MATERIAL_SOLID 1
#define MATERIAL_AIR 0

extern float g_time;

float Density_Func(const glm::vec3 & worldPosition);
vector<int> GetMaterialIndices(const glm::vec3 &chunkPos, const glm::vec3 &chunkSize);

void DensitySetVoxelSize(const float & voxelSize);