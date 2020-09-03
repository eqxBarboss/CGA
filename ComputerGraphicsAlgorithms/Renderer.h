#pragma once

#include <memory>
#include <functional>
#include <mutex>

#include <ctpl/ctpl_stl.h>
#include <glm/glm.hpp>

#include "framework.h"

#include "Scene.h"
#include "Obj.h"

namespace cga
{

class Renderer
{
public:
	int width, height;

	Renderer(int aWidth, int aHeight, HDC aDeviceContext, std::function<void()> aInvalidateCallback);

	void Render(std::unique_ptr<Scene> &scene);

private:
	ctpl::thread_pool threadPool;
	std::mutex mutex;
	int threadCount;

	HDC deviceContext;
	std::function<void()> aInvalidateCallback;

	void CalculateVertices(std::vector<glm::vec4> &vertices, int first, int last, glm::mat4 pvm, glm::mat4 viewPort);
	void DrawPolygons(std::vector<Polygon> &polygons, std::vector<glm::vec4> &vertices, int first, int last);
};

}