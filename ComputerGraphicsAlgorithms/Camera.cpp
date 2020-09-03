#include "Camera.h"

namespace cga
{

// position = glm::vec3(0.0f, 0.0f, 0.0f), up = glm::vec3(0.0f, 1.0f, 0.0f), yaw = YAW, pitch = PITCH
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
	: Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	MovementSpeed(SPEED),
	MouseSensitivity(SENSITIVTY),
	FOV(DEFAULT_FOV),
	Position(position),
	WorldUp(up),
	Yaw(yaw),
	Pitch(pitch)
{
	UpdateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
	: Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	MovementSpeed(SPEED),
	MouseSensitivity(SENSITIVTY),
	FOV(DEFAULT_FOV),
	Position(glm::vec3(posX, posY, posZ)),
	WorldUp(glm::vec3(upX, upY, upZ)),
	Yaw(yaw),
	Pitch(pitch)
{
	UpdateCameraVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	float velocity = MovementSpeed * deltaTime;
	if (direction == FORWARD)
		Position += Front * velocity;
	if (direction == BACKWARD)
		Position -= Front * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) // constrainPitch = true
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
	if (FOV >= 1.0f && this->FOV <= MAX_FOV)
		FOV -= yoffset;
	if (FOV <= 1.0f)
		FOV = 1.0f;
	if (FOV >= MAX_FOV)
		FOV = MAX_FOV;
}

void Camera::UpdateCameraVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}

}