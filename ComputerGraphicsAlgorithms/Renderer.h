#pragma once

#include <memory>
#include <functional>
#include <mutex>

#include <ctpl/ctpl_stl.h>
#include <glm/glm.hpp>

#include "Buffer.h"
#include "Scene.h"
#include "Obj.h"

//#define DISCARD_VERTICES

namespace cga
{

class Renderer
{
public:
	Renderer(int aWidth, int aHeight, std::function<void()> aInvalidateCallback);

	Buffer& GetCurrentBuffer();

	void Render(std::unique_ptr<Scene> &scene);

private:
	static int workingThreads;
	static std::mutex mutex;
	static std::condition_variable cv;
	static int width, height;

	ctpl::thread_pool threadPool;
	int threadCount;

	Buffer buffer, backBuffer;

	std::function<void()> aInvalidateCallback;

	static void CalculateVertices(int id, Obj &renderTarget, int first, int last, const glm::mat4 &pvm, const glm::mat4 &viewPort);
	static void DrawPolygons(int id, Buffer &buffer, Obj &renderTarget, int first, int last);
	static void WaitForThreads();
	static void FinishThreadWork();

	static inline void RasterizeLine(Buffer& buffer, glm::vec4 a, glm::vec4 b)
	{
		if (a.x < 0 || a.x >= width || a.y < 0 || a.y >= height ||
			b.x < 0 || b.x >= width || b.y < 0 || b.y >= height ||
			a.z < -1 || a.z > 1 || b.z < -1 || b.z > 1) return;

		auto dx = b.x - a.x;
		auto dy = b.y - a.y;

		auto steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

		auto Xinc = dx / (float)steps;
		auto Yinc = dy / (float)steps;

		auto x = a.x;
		auto y = a.y;

		for (int i = 0; i <= steps; i++)
		{
			buffer.SetPixel(x, y, RGB(255, 255, 255));
			x += Xinc;
			y += Yinc;
		}
	}
};

}