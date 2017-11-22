#include "game.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 target)
{
	m_pos = position;

	//camera orientation vecs
	m_up = glm::vec3(0.0f, 1.0f, 0.0f); //set default up vector to positive y
	m_right = glm::vec3(1.0f, 0.0f, 0.0f);
	m_forward = glm::normalize(target - position);

	//default perspective mat
	m_view_mat = glm::lookAt(m_pos, m_pos + m_forward, m_up);

	//set up perspective mat4
	m_nearPlane = 1.0f;
	m_farPlane = 200000.0f;
	m_fieldOfView = 90.0f;
	m_perspect_proj = glm::perspective(glm::radians(m_fieldOfView), SCREEN_WIDTH / SCREEN_HEIGHT, m_nearPlane, m_farPlane);
	//m_perspect_proj = glm::ortho(-840.f, 840.f, -540.f, 540.f, m_nearPlane, 200000.f);

	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_roll = 0.0f;

	m_speed = 30.0f;
}

Camera::~Camera()
{

}


glm::mat4 Camera::GetProj()
{
	return m_perspect_proj;
}

glm::mat4 Camera::GetViewMat()
{
	return m_view_mat;
}

glm::mat4 Camera::GetInverseViewMat()
{
	return m_invView_mat;
}

glm::vec3 Camera::GetPosition()
{
	return m_pos;
}

float Camera::GetFarPlane()
{
	return m_farPlane;
}

float Camera::GetNearPlane()
{
	return m_nearPlane;
}

glm::vec3 Camera::GetForward()
{
	return m_forward;
}

float Camera::GetFOV()
{
	return m_fieldOfView;
}
glm::vec3 Camera::GetRotation()
{
	return glm::vec3(glm::radians(m_pitch), glm::radians(m_yaw), glm::radians(m_roll));
}


void Camera::HandleInput(SDL_Event event)
{
	GLfloat cam_speed = m_speed * g_game->GetDeltaTime();

	if(event.type == SDL_KEYDOWN)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_w:
			m_pos += cam_speed * m_forward;
			break;
		case SDLK_s:
			m_pos -= cam_speed * m_forward;
			break;
		case SDLK_a:
			m_pos -= m_right * cam_speed;
			break;
		case SDLK_d:
			m_pos += m_right * cam_speed;
			break;
		case SDLK_e:
			m_speed += 20;
			break;
		case SDLK_r:
			m_speed -= 20;
			break;
		default:
			break;
		}
	}
	else if(event.type == SDL_MOUSEMOTION && g_game->IsFPMode())
	{
		GLfloat sensitivity = 0.05f; // mouse sensitivity

		glm::vec2 motion = glm::vec2(event.motion.x, event.motion.y);
		glm::vec2 center = glm::vec2(SCREEN_WIDTH * .5f, SCREEN_HEIGHT * .5f);
		glm::vec2 offset = motion - center;

		float xoffset = (float)offset.x * sensitivity;
		float yoffset = (float)offset.y * sensitivity;

		m_yaw += xoffset;
		m_pitch -= yoffset;

		if(m_pitch > 89.0f)
			m_pitch = 89.0f;
		else if(m_pitch < -89.0f)
			m_pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		front.y = sin(glm::radians(m_pitch));
		front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

		m_forward = glm::normalize(front);
		m_right = glm::normalize(glm::cross(m_forward, m_up));
	}

	m_view_mat = glm::lookAt(m_pos, m_pos + m_forward, m_up);
	m_invView_mat = glm::inverse(m_view_mat);
}
