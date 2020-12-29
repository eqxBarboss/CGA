#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Math.h"

namespace cga
{

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 0.001f;
const float SENSITIVTY = 0.25f;
const float DEFAULT_FOV = 45.0f;
const float MAX_FOV = 89.0f;

enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;

	float MovementSpeed;
	float MouseSensitivity;
	float FOV;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	inline glm::mat4 GetViewMatrix()
	{
		return GetLookAtMatrix(Position, Position + Front, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void ProcessKeyboard(CameraMovement direction, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yoffset);

protected:
	void UpdateCameraVectors();
};

}
