#pragma once

enum LightType : Uint8
{
	Directional,
	Point,
	Spotlight
};

class LightSource
{
	glm::vec3 position;
	glm::vec3 color;
	LightType type;
public:
	LightSource(LightType type, glm::vec3 color, glm::vec3 position);

};