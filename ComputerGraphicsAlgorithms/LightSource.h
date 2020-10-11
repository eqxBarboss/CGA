#pragma once

#include <glm/glm.hpp>
#include <windows.h>

namespace cga
{

class LightSource
{
public:
	LightSource(glm::vec3 aPosition, COLORREF aColor) : position(aPosition), color(aColor) {}

	glm::vec3 position;
	COLORREF color;
};

}