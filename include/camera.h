#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <iostream>

enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -100.0f;
const float PITCH = 0.0f;
const float SPEED = 50.0f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 60.0f;

class Camera {
public:
	explicit Camera(glm::vec3 position = glm::vec3(2.0f, 2.0f, 7.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	glm::vec3 pos() const{
		return m_pos;
	}
	void transfrom(CameraMovement direction, float deltaTime);
	void deflect(float xoffset, float yoffset, bool constrainPitch = true);
	void zoom(float offset);
	glm::mat4 lookAt();
	float fov() const{
		return glm::radians(m_zoom);
	}
public:
	static Camera* getCamera(const std::string & name);
	static void registry(const std::string& name, Camera* camera);
	void updateCameraVectors();
	void debug() {
		std::cout << "{"<<m_pos.x <<","<<m_pos.y<<","<<m_pos.z<<"}" << std::endl;
		std::cout << "{"<<m_up.x <<","<<m_up.y<<","<<m_up.z<<"}" << std::endl;
		std::cout << "{"<<m_front.x <<","<<m_front.y<<","<<m_front.z<<"}" << std::endl;
	}

private:
	std::string name;
	glm::vec3 m_pos;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_front;
	glm::vec3 m_worldUp;
	//euler Angles
	float Yaw;
	float Pitch;
	//camera options
	float m_movementSpeed;
	float m_mouseSensitivity;
	float m_zoom;
private:
	static std::unordered_map<std::string,Camera*> camera_instances;
};

