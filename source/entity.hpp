#pragma once

#define MAX_ENTITIES 100

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 velocity;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 forward;
};

class Entity
{
	int m_flag;

	Transform m_transform;
	AABB m_boundBox;

	int m_health;
	int m_maxHealth;

	int m_nextThink;
	int m_thinkRate;

public:
	int m_id;
	GLuint m_modelID;
	Entity *m_nextFree;

	Entity();

	void Init(GLuint modelID);

	int GetID();
	int GetModelID();
	int GetHealth();
	int GetMaxHealth();

	glm::vec3 GetPosition();
};