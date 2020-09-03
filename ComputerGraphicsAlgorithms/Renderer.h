#pragma once

#include <memory>
#include <functional>

#include "framework.h"

#include "Scene.h"

namespace cga
{

class Renderer
{
public:
	int width, height;

	Renderer(int aWidth, int aHeight, HDC aDeviceContext, std::function<void()> aInvalidateCallback);

	void Render(std::unique_ptr<Scene> &scene);

private:
	HDC deviceContext;
	std::function<void()> aInvalidateCallback;
};

}