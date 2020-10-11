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
#include "LightSource.h"

//#define DISCARD_VERTICES

namespace cga
{

class Renderer
{
public:
	Renderer(int aWidth, int aHeight, std::function<void()> aInvalidateCallback);
	~Renderer();

	Buffer& GetCurrentBuffer();

	void Render(std::unique_ptr<Scene> &scene);

private:
	static int workingThreads;
	static std::mutex mutex;
	static std::condition_variable cv;
	static int width, height;

	ctpl::thread_pool threadPool;
	int threadCount;

	Obj renderTarget;
	LightSource lightSource;
	std::vector<glm::vec4> cameraSpaceVertices;
	std::vector<bool> drawPolygon;
	Buffer buffer, backBuffer;
	float* zBuffer;
	float* zBufferInitial;

	std::function<void()> aInvalidateCallback;

	void ClearZBuffer();

	static void CalculateVertices(int id
		, Obj &renderTarget
		, std::vector<glm::vec4>& cameraSpaceVertices
		, int first
		, int last
		, const glm::mat4 &pvm
		, const glm::mat4& vm
		, const glm::mat4 &viewPort);
	static void CalculateNormals(int id
		, Obj& renderTarget
		, int first
		, int last
		, const glm::mat3& TIvm);
	static void CalculateLighting(int id
		, Obj& renderTarget
		, const std::vector<glm::vec4>& cameraSpaceVertices
		, std::vector<bool>& drawPolygon
		, int first
		, int last
		, const LightSource& lightSource);
	void DrawPolygons(Buffer &buffer, float* zBuffer, Obj &renderTarget, int first, int last);
	static void WaitForThreads();
	static void FinishThreadWork();

	static inline void RasterizeLine(Buffer& buffer, float* zBuffer, const glm::vec4& a, const glm::vec4& b, COLORREF color)
	{
		if (a.x < 0 || a.x >= width || a.y < 0 || a.y >= height ||
			b.x < 0 || b.x >= width || b.y < 0 || b.y >= height ||
			a.z < 0 || a.z > 1 || b.z < 0 || b.z > 1) return;

		auto dx = b.x - a.x;
		auto dy = b.y - a.y;
		auto dz = b.z - a.z;

		auto steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

		auto Xinc = dx / (float)steps;
		auto Yinc = dy / (float)steps;
		auto Zinc = dz / (float)steps;

		auto x = a.x;
		auto y = a.y;
		auto z = a.z;

		for (int i = 0; i <= steps; i++)
		{
			if (zBuffer[(int)y * width + (int)x] > z)
			{
				zBuffer[(int)y * width + (int)x] = z;
				buffer.SetPixel(x, y, color);
			}
			x += Xinc;
			y += Yinc;
			z += Zinc;
		}
	}

	static inline void MyCoolDrawHorLine(Buffer& buffer, float* zBuffer, int y, int x1, int x2, float z1, float z2, COLORREF color)
	{
		const int yMulWidth = y * width;
		auto Zinc = (z2 - z1) / (float)(x2 - x1 + 1);
		float z = z1;

		for (int x = x1; x != x2; x++)
		{
			if (zBuffer[yMulWidth + x] > z)
			{
				zBuffer[yMulWidth + x] = z;
				buffer.SetPixel(x, y, color);
			}
			z += Zinc;
		}
	}

	static inline void RasterizeTriangle(Buffer& buffer, float* zBuffer, Obj& renderTarget, int polygonIndex)
	{
		const auto& vertices = renderTarget.vertices;
		const auto& polygon = renderTarget.polygons[polygonIndex];
		const auto& normals = renderTarget.normals;

		const COLORREF inner = RGB(0xff, 0xbf, 0x00);
		const COLORREF outer = RGB(0, 0, 255);
		const int iv0 = polygon.verticesIndices[0];
		const int iv1 = polygon.verticesIndices[1];
		const int iv2 = polygon.verticesIndices[2];
		const auto& v0 = vertices[iv0];
		const auto& v1 = vertices[iv1];
		const auto& v2 = vertices[iv2];
		int v0x = v0.x;
		int v0y = v0.y;
		float v0z = v0.z;
		int v1x = v1.x;
		int v1y = v1.y;
		float v1z = v1.z;
		int v2x = v2.x;
		int v2y = v2.y;
		float v2z = v2.z;

		if (v0x < 0 || v0x >= width || v0y < 0 || v0y >= height ||
			v1x < 0 || v1x >= width || v1y < 0 || v1y >= height ||
			v2x < 0 || v2x >= width || v2y < 0 || v2y >= height ||
			v0z < 0 || v0z > 1 ||
			v1z < 0 || v1z > 1 ||
			v2z < 0 || v2z > 1) return;

		if (v0y == v1y && v0y == v2y) return; // don't care about degenerate triangles

		{
			if (v0y > v1y)
			{
				std::swap(v0x, v1x);
				std::swap(v0y, v1y);
				std::swap(v0z, v1z);
			};
			if (v0y > v2y)
			{
				std::swap(v0x, v2x);
				std::swap(v0y, v2y);
				std::swap(v0z, v2z);
			};
			if (v1y > v2y)
			{
				std::swap(v1x, v2x);
				std::swap(v1y, v2y);
				std::swap(v1z, v2z);
			};
		}

		int total_height = v2y - v0y;
		for (int i = 0; i < total_height; i++) {
			bool second_half = i > v1y - v0y || v1y == v0y;
			int segment_height = second_half ? v2y - v1y : v1y - v0y;
			float alpha = (float)i / total_height;
			float beta = (float)(i - (second_half ? v1y - v0y : 0)) / segment_height;
			int x1 = v0x + (v2x - v0x) * alpha;
			int x2 = second_half ? v1x + (v2x - v1x) * beta : v0x + (v1x - v0x) * beta;
			float z1 = v0z + (v2z - v0z) * alpha;
			float z2 = second_half ? v1z + (v2z - v1z) * beta : v0z + (v1z - v0z) * beta;
			if (x1 > x2)
			{
				std::swap(x1, x2);
				std::swap(z1, z2);
			}
			MyCoolDrawHorLine(buffer, zBuffer, v0y + i, x1, x2, z1, z2, polygon.color);
		}

		//RasterizeLine(buffer, zBuffer, v0, v1, outer);
		//RasterizeLine(buffer, zBuffer, v1, v2, outer);
		//RasterizeLine(buffer, zBuffer, v2, v0, outer);
	}
};

}