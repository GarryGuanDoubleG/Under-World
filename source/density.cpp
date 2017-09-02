#include "game.hpp"

float Density_Func(const glm::vec3 & worldPosition)
{
	return worldPosition.y - 150.0f;
}
