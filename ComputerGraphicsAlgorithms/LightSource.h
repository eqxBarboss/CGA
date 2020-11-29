#pragma once

#include <glm/glm.hpp>
#include <windows.h>

namespace cga
{

class LightSource
{
public:
	LightSource(glm::vec3 aPosition, glm::vec3 aColor) : position(aPosition), color(aColor) {}

	glm::vec3 position;
	glm::vec3 color;
};

}