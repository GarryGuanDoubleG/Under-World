#include "game.hpp"

#define ENTITY_INUSE 1

Entity::Entity() : m_flag (0)
{
}

void Entity::Init(GLuint modelID)
{
	m_transform.position = glm::vec3(500.0f);
	m_transform.rotation = glm::vec3(0.0f);
	m_transform.up = glm::vec3(0.0, 1.0f, 0.0f);
	m_transform.right = glm::vec3(1.0f, 0.f, 0.f);
	m_transform.forward = glm::cross(m_transform.up, m_transform.right);

	m_boundBox.min = glm::vec3(0.0f);
	m_boundBox.max = glm::vec3(20, 100, 20);

	m_flag |= ENTITY_INUSE;

	m_modelID = modelID;
}

int Entity::GetID()
{
	return m_id;
}

int Entity::GetModelID()
{
	return m_modelID;
}

int Entity::GetHealth()
{
	return m_health;
}

int Entity::GetMaxHealth()
{
	return m_maxHealth;
}

glm::vec3 Entity::GetPosition()
{
	return m_transform.position;
}
