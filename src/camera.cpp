#include "camera.h"

#include "glm/gtc/matrix_transform.hpp"
#include <algorithm>

std::unordered_map<std::string, Camera*> Camera::camera_instances;

Camera::Camera(glm::vec3 position, glm::vec3 up , float yaw , float pitch ) :m_movementSpeed(SPEED),m_mouseSensitivity(SENSITIVITY),m_zoom(ZOOM){
	m_pos = position;
	m_worldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

void Camera::transfrom(CameraMovement direction, float deltaTime) {
	float velocity = m_movementSpeed * deltaTime;

	if (direction == FORWARD)
		m_pos += m_front * velocity;
	if (direction == BACKWARD)
		m_pos -= m_front * velocity;
	if (direction == RIGHT)
		m_pos += m_right * velocity;
	if (direction == LEFT)
		m_pos -= m_right * velocity;
}

void Camera::deflect(float xoffset, float yoffset, bool constrainPitch)
{
	Yaw+= xoffset * m_mouseSensitivity;
	Pitch += yoffset * m_mouseSensitivity;

	if (constrainPitch) {
		Pitch = std::min(89.0f, std::max(Pitch, -89.0f));
	}

	updateCameraVectors();
}

void Camera::zoom(float offset)
{
	m_zoom -= (float)offset;
	//boudary constrain
	m_zoom = std::min(ZOOM, std::max(m_zoom, 1.0f));
}


glm::mat4 Camera::lookAt() {
	return glm::lookAt(m_pos, m_pos+m_front, m_up);
}

Camera* Camera::getCamera(const std::string& name)
{
	auto itr=camera_instances.find(name);
	if (itr != camera_instances.end())
		return itr->second;
	else {
		return nullptr;
	}
}

void Camera::registry(const std::string& name, Camera* camera)
{
	camera_instances.insert({name,camera});
}

void Camera::updateCameraVectors() {
	//calculate the new front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	m_front = glm::normalize(front);
	//re-calculate the right and up vector
	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_front));
}

