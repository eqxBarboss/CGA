#pragma once

#include <memory>
#include <functional>
#include <mutex>

#include <ctpl/ctpl_stl.h>
#include <glm/glm.hpp>

#include "Buffer.h"
#include "Scene.h"
#include "Obj.h"

namespace cga
{

class Renderer
{
public:
	int width, height;

	Renderer(int aWidth, int aHeight, std::function<void()> aInvalidateCallback);

	Buffer& GetCurrentBuffer();

	void Render(std::unique_ptr<Scene> &scene);

private:
	ctpl::thread_pool threadPool;
	std::mutex mutex;
	int threadCount;

	Buffer buffer, backBuffer;

	std::function<void()> aInvalidateCallback;

	static void CalculateVertices(int id, Obj &renderTarget, int first, int last, const glm::mat4 &pvm, const glm::mat4 &viewPort);
	static void DrawPolygons(int id, Buffer &buffer, Obj &renderTarget, int first, int last);
};

}