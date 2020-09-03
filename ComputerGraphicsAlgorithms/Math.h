#pragma once

#include <glm/glm.hpp>

namespace cga
{

inline glm::mat4 GetTranslationMatrix(glm::vec3 translation)
{
	return glm::mat4 {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		translation.x, translation.y, translation.z, 1
	};
}

inline glm::mat4 GetScaleMatrix(glm::vec3 scale)
{
	return glm::mat4 {
		scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1
	};
}

inline glm::mat4 GetRotationAroundXMatrix(float angle)
{
	angle = glm::radians(angle);

	const float cos = glm::cos(angle);
	const float sin = glm::sin(angle);

	return glm::mat4 {
		1, 0, 0, 0,
		0, cos, sin, 0,
		0, -sin, cos, 0,
		0, 0, 0, 1
	};
}

inline glm::mat4 GetRotationAroundYMatrix(float angle)
{
	angle = glm::radians(angle);

	const float cos = glm::cos(angle);
	const float sin = glm::sin(angle);

	return glm::mat4 {
		cos, 0, -sin, 0,
		0, 1, 0, 0,
		sin, 0, cos, 0,
		0, 0, 0, 1
	};
}

inline glm::mat4 GetRotationAroundZMatrix(float angle)
{
	angle = glm::radians(angle);

	const float cos = glm::cos(angle);
	const float sin = glm::sin(angle);

	return glm::mat4 {
		cos, sin, 0, 0,
		-sin, cos, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

inline glm::mat4 GetLookAtMatrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp)
{
	glm::vec3 zaxis = glm::normalize(position - target);
	glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
	glm::vec3 yaxis = glm::cross(zaxis, xaxis);

	glm::mat4 translation {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-position.x, -position.y, -position.z, 1
	};

	auto a = translation * glm::vec4(1, 1, 1, 1);

	glm::mat4 rotation {
		xaxis.x, yaxis.x, zaxis.x, 0,
		xaxis.y, yaxis.y, zaxis.y, 0,
		xaxis.z, yaxis.z, zaxis.z, 0,
		0, 0, 0, 1
	};

	return rotation * translation;
}

inline glm::mat4 GetPerspectiveProjectionMatrix(float width, float height, float zNear, float zFar, float FOV)
{
	float tanFOVHalved = glm::tan(glm::radians(FOV / 2));
	float deltaZ = zNear - zFar;
	float aspect = width / height;

	return glm::mat4 {
		1 / (aspect * tanFOVHalved), 0, 0, 0,
		0, 1 / tanFOVHalved, 0, 0,
		0, 0, zFar / deltaZ, -1,
		0, 0, zNear * zFar / deltaZ, 0
	};
}

inline glm::mat4 GetOrthographicProjectionMatrix(float width, float height, float zNear, float zFar)
{
	float deltaZ = zNear - zFar;

	return glm::mat4 {
		2 / width, 0, 0, 0,
		0, 2 / height, 0, 0,
		0, 0, 1 / deltaZ, 0,
		0, 0, zNear / deltaZ, 1
	};
}

inline glm::mat4 GetViewPortMatrix(float width, float height)
{
	float widthHalved = width / 2;
	float heightHalved = height / 2;

	return glm::mat4 {
		widthHalved, 0, 0, 0,
		0, -heightHalved, 0, 0,
		0, 0, 1, 0,
		widthHalved, heightHalved, 0, 1
	};
}

}
