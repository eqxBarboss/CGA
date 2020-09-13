#pragma once

#define NOMINMAX

#include <memory>
#include <functional>
#include <mutex>
#include <algorithm>

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
			a.z < 0 || a.z > 1 || b.z < 0 || b.z > 1) return;

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

	static inline void MyCoolDrawHorLine(Buffer& buffer, int y, int x1, int x2, COLORREF color)
	{
		memset(buffer.data + y * width + x1, color, sizeof(COLORREF) * (x2 - x1 + 1));
	}

	static inline void RasterizeTriangle(Buffer& buffer, Polygon& polygon, const std::vector<glm::vec4>& vertices)
	{
		const int iv0 = polygon.verticesIndices[0] - 1;
		const int iv1 = polygon.verticesIndices[1] - 1;
		const int iv2 = polygon.verticesIndices[2] - 1;
		const auto& v0 = vertices[iv0];
		const auto& v1 = vertices[iv1];
		const auto& v2 = vertices[iv2];
		const int v0x = v0.x;
		const int v0y = v0.y;
		const int v1x = v1.x;
		const int v1y = v1.y;
		const int v2x = v2.x;
		const int v2y = v2.y;

		if (v0x < 0 || v0x >= width || v0y < 0 || v0y >= height ||
			v1x < 0 || v1x >= width || v1y < 0 || v1y >= height ||
			v2x < 0 || v2x >= width || v2y < 0 || v2y >= height) return;

		int maxY = std::max(std::max(v0y, v1y), v2y);
		int minY = std::min(std::min(v0y, v1y), v2y);

		COLORREF color = RGB(255, 255, 255);

		bool a = v0y != v1y;
		bool b = v1y != v2y;
		bool c = v2y != v0y;

		float k1 = a ? (float)(v1x - v0x) / (v1y - v0y) : 0;
		float k2 = b ? (float)(v2x - v1x) / (v2y - v1y) : 0;
		float k3 = c ? (float)(v0x - v2x) / (v0y - v2y) : 0;

		for (int y = minY; y != maxY; y++)
		{
			{
				if ((y == v0y) && (v0y == v1y))
				{
					MyCoolDrawHorLine(buffer, y, std::min(v0x, v1x), std::max(v0x, v1x), color);
					continue;
				}

				if ((y == v1y) && (v1y == v2y))
				{
					MyCoolDrawHorLine(buffer, y, std::min(v1x, v2x), std::max(v1x, v2x), color);
					continue;
				}

				if ((y == v2y) && (v2y == v0y))
				{
					MyCoolDrawHorLine(buffer, y, std::min(v2x, v0x), std::max(v2x, v0x), color);
					continue;
				}
			}

			int x1 = v0x + k1 * (y - v0y);
			int x2 = v1x + k2 * (y - v1y);
			int x3 = v2x + k3 * (y - v2y);

			if (x1 < 0 || x1 >= width || x1 < 0 || x1 >= height ||
				x2 < 0 || x2 >= width || x2 < 0 || x2 >= height ||
				x3 < 0 || x3 >= width || x3 < 0 || x3 >= height) continue;

			if (!a)
			{
				if (x2 > x3) std::swap(x2, x3);
				MyCoolDrawHorLine(buffer, y, x2, x3, color);
				continue;
			}

			if (!b)
			{
				if (x1 > x3) std::swap(x1, x3);
				MyCoolDrawHorLine(buffer, y, x1, x3, color);
				continue;
			}

			if (!c)
			{
				if (x1 > x2) std::swap(x1, x2);
				MyCoolDrawHorLine(buffer, y, x1, x2, color);
				continue;
			}

			if (x1 > x2)
			{
				if (x1 > x3) std::swap(x1, x3);
				MyCoolDrawHorLine(buffer, y, x1, x3, color);
			}
			else
			{
				if (x2 > x3) std::swap(x2, x3);
				MyCoolDrawHorLine(buffer, y, x2, x3, color);
			}
		}
	}
};

}