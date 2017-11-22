#pragma once

/**
* Manages frustum by managing projection and view matrice. Sets up what the player can see.
* Currently creates a perspective projection but will eventually change to orthographic projection
*/
class Camera
{
	float m_farPlane;
	float m_nearPlane;
	float m_fieldOfView;

	glm::vec3 m_pos; /**< position of camera */
	glm::vec3 m_target; /**<the position the camera looks at */
	glm::vec3 m_forward; /**< the direction the camera loks at */
	glm::vec3 m_right; /**< the direction right of the camera */
	glm::vec3 m_up; /**<the direction above the camera  */

	GLfloat m_yaw; /**< x rotation of camera */
	GLfloat m_pitch; /**< y rotation of camera  */
	GLfloat m_roll; /**<  z rotation of camera*/

					//projection and view matrices 			
	glm::mat4 m_view_mat;
	glm::mat4 m_perspect_proj; /**< projection matrix*/
	glm::mat4 m_ortho_proj;

	glm::mat4 m_invView_mat;//inverse view matrix

	GLfloat m_zoom;
	GLfloat m_speed;

	glm::vec3 m_ray;

public:
	/**
	* A constructor
	* Sets up projection and view matrices. Starts at position 0,0,3 and looks towards origin
	*/
	Camera(glm::vec3 position, glm::vec3 target);
	/**
	* Empty Destructor
	*/
	~Camera();
	/**
	* @brief returns the view matrix
	* @return the view matrix
	*/
	glm::mat4 GetViewMat();

	glm::mat4 GetInverseViewMat();
	/**
	* @brief returns the projection matrix
	* @return the projection matrix
	*/
	glm::mat4 GetProj();
	glm::vec3 GetRight();
	glm::vec3 GetUp();
	glm::vec3 GetForward();
	float GetFOV();
	glm::vec3 GetRotation();
	glm::vec3 GetPosition();
	float GetFarPlane();

	float GetNearPlane();

	/**
	* @brief adjusts camera on user input
	* @param event the user input
	*/
	void HandleInput(SDL_Event event);


};