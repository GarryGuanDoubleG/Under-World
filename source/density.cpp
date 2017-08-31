#include "game.hpp"

float Density_Func(const glm::vec3 & worldPosition)
{
	return 150.0f - worldPosition.y;
}
