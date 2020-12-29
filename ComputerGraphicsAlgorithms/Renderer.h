#pragma once

#define NOMINMAX

#include <memory>
#include <functional>
#include <mutex>
#include <vector>
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
    void SetMaps(std::string path);

private:
	static int workingThreads;
	static std::mutex mutex;
	static std::condition_variable cv;
	static int width, height;

	static std::vector<unsigned char> diffuseMap;
	static unsigned diffuseMapWidth, diffuseMapHeight;

	static std::vector<unsigned char> specularMap;
	static unsigned specularMapWidth, specularMapHeight;


	static std::vector<unsigned char> normalMapLoaded;
	static std::vector<glm::vec3> normalMap;
	static std::vector<glm::vec3> normalMapTransformed;
	static unsigned normalMapWidth, normalMapHeight;

	ctpl::thread_pool threadPool;
	int threadCount;

	Obj renderTarget;
	LightSource lightSource;
	std::vector<glm::vec4> cameraSpaceVertices;
	Buffer buffer, backBuffer;
	float* zBuffer;
	float* zBufferInitial;

	std::string scenePath;

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
	static void CalculateNormalsMap(int id
		, Obj& renderTarget
		, int first
		, int last
		, const glm::mat3& TIvm);
	static void CalculateLighting(int id
		, Obj& renderTarget
		, const std::vector<glm::vec4>& cameraSpaceVertices
		, int first
		, int last
		, const LightSource& lightSource);
	void DrawPolygons(Buffer &buffer, float* zBuffer, Obj &renderTarget, const std::vector<glm::vec4>& cameraSpaceVertices, const LightSource& lightSource, int first, int last);
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

	static COLORREF inline GetRgbFromMap(const std::vector<unsigned char>& map, float u, float v)
	{
		int i = std::min((int)(u * (diffuseMapWidth - 1)), (int)diffuseMapWidth - 1);
		int j = std::min((int)((1 - v) * (diffuseMapHeight - 1)), (int)diffuseMapHeight - 1);
		int textelIndex = (j * diffuseMapWidth + i) * 4;
		return RGB(map[textelIndex + 0], map[textelIndex + 1], map[textelIndex + 1]);
	}


	static glm::vec3 inline GetSpecularFromMap(const std::vector<unsigned char>& map, float u, float v)
	{
		int i = std::min((int)(u * (diffuseMapWidth - 1)), (int)diffuseMapWidth - 1);
		int j = std::min((int)((1 - v) * (diffuseMapHeight - 1)), (int)diffuseMapHeight - 1);
		int textelIndex = (j * diffuseMapWidth + i) * 4;
		return glm::vec3(map[textelIndex + 0] / 255.0f, map[textelIndex + 1] / 255.0f, map[textelIndex + 1] / 255.0f);
	}

	static const glm::vec3 inline &GetNormalFromMap(float u, float v)
	{
		int i = std::min((int)(u * (normalMapWidth - 1)), (int)normalMapWidth - 1);
		int j = std::min((int)((1 - v) * (normalMapHeight - 1)), (int)normalMapHeight - 1);
		int textelIndex = (j * normalMapWidth + i);
		return normalMapTransformed[textelIndex];
	}

	static inline void RasterizeTriangle(Buffer& buffer, float* zBuffer, Obj& renderTarget, const std::vector<glm::vec4>& cameraSpaceVertices, const LightSource& lightSource, int polygonIndex)
	{
		const auto& vertices = renderTarget.vertices;
		const auto& polygon = renderTarget.polygons[polygonIndex];
		auto& normals = renderTarget.normals;

		auto txc0 = renderTarget.textureCoords[polygon.textureIndices[0]];
		auto txc1 = renderTarget.textureCoords[polygon.textureIndices[1]];
		auto txc2 = renderTarget.textureCoords[polygon.textureIndices[2]];

		const glm::vec4* a = &cameraSpaceVertices[polygon.verticesIndices[0]];
		const glm::vec4* b = &cameraSpaceVertices[polygon.verticesIndices[1]];
		const glm::vec4* c = &cameraSpaceVertices[polygon.verticesIndices[2]];

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

		glm::vec3* nA = &normals[polygon.normalsIndices[0]];
		glm::vec3* nB = &normals[polygon.normalsIndices[1]];
		glm::vec3* nC = &normals[polygon.normalsIndices[2]];

		auto m = (v1x - v0x) * (v2y - v1y) - (v2x - v1x) * (v1y - v0y);
		if (m >= 0) return;

		if (v0z < 0 || v0z > 1 ||
			v1z < 0 || v1z > 1 ||
			v2z < 0 || v2z > 1) return;

		if (v0y == v1y && v0y == v2y) return; // don't care about degenerate triangles

		{
			if (v0y > v1y)
			{
				std::swap(v0x, v1x);
				std::swap(v0y, v1y);
				std::swap(v0z, v1z);
				auto temp = nA;
				nA = nB;
				nB = temp;
				auto temp1 = a;
				a = b;
				b = temp1;
				auto temp3 = txc0;
				txc0 = txc1;
				txc1 = temp3;
			};
			if (v0y > v2y)
			{
				std::swap(v0x, v2x);
				std::swap(v0y, v2y);
				std::swap(v0z, v2z);
				auto temp = nA;
				nA = nC;
				nC = temp;
				auto temp1 = a;
				a = c;
				c = temp1;
				auto temp3 = txc0;
				txc0 = txc2;
				txc2 = temp3;
			};
			if (v1y > v2y)
			{
				std::swap(v1x, v2x);
				std::swap(v1y, v2y);
				std::swap(v1z, v2z);
				auto temp = nB;
				nB = nC;
				nC = temp;
				auto temp1 = b;
				b = c;
				c = temp1;
				auto temp3 = txc1;
				txc1 = txc2;
				txc2 = temp3;
			};
		}

		// v0 = A
		// v1 = B
		// v2 = C
		const int ABx = v1x - v0x;
		const int ACx = v2x - v0x;
		const int ABy = v1y - v0y;
		const int ACy = v2y - v0y;

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
			if (x1 < 0) x1 = 0;
			if (x2 < 0) x2 = 0;
			if (x1 >= width) x1 = width - 1;
			if (x2 >= width) x2 = width - 1;
			if (((x1 == x2) && (x1 == 0)) || ((x1 == x2) && (x1 == width - 1))) continue;
			if (((v0y + i) < 0) || ((v0y + i) >= height)) continue;

			const int y = v0y + i;
			const int yMulWidth = y * width;
			const int PAy = v0y - y;
			auto Zinc = (z2 - z1) / (float)(x2 - x1 + 1);
			float z = z1;

			COLORREF color;

			for (int x = x1; x != x2; x++)
			{
				if (zBuffer[yMulWidth + x] > z)
				{
					zBuffer[yMulWidth + x] = z;

					{					
						glm::vec3 crs = glm::cross(glm::vec3(ACx, ABx, v0x - x), glm::vec3(ACy, ABy, PAy));					
						glm::vec3 barycentric(1.0f - (crs.x + crs.y) / crs.z, crs.y / crs.z, crs.x / crs.z);

						//if (barycentric.y < 0 || barycentric.z < 0) continue;

						glm::vec3 normal = glm::normalize(barycentric.x * *nA + barycentric.y * *nB + barycentric.z * *nC);
						glm::vec4 v = barycentric.x * *a + barycentric.y * *b + barycentric.z * *c;						

						glm::vec3 barycentricCorrected = glm::vec3(barycentric.x / v0z, barycentric.y / v1z, barycentric.z / v2z);
						float sum = barycentricCorrected.x + barycentricCorrected.y + barycentricCorrected.z;
						barycentricCorrected.x /= sum;
						barycentricCorrected.y /= sum;
						barycentricCorrected.z /= sum;

						glm::vec3 uv = barycentricCorrected.x * txc0 + barycentricCorrected.y * txc1 + barycentricCorrected.z * txc2;
						COLORREF clr = GetRgbFromMap(diffuseMap, std::clamp(uv.x, 0.0f, 1.0f) , std::clamp(uv.y, 0.0f, 1.0f));
						normal = GetNormalFromMap(std::clamp(uv.x, 0.0f, 1.0f), std::clamp(uv.y, 0.0f, 1.0f));
						//clr = RGB(255, 255, 255);

						{
						//COLORREF c0 = GetRgbFromMap(diffuseMap, txc0.x, txc0.y);
						//COLORREF c1 = GetRgbFromMap(diffuseMap, txc1.x, txc1.y);
						//COLORREF c2 = GetRgbFromMap(diffuseMap, txc2.x, txc2.y);
						//COLORREF avg = RGB(
						//	(GetRValue(c0) + GetRValue(c1) + GetRValue(c2)) / 3.0f,
						//	(GetGValue(c0) + GetGValue(c1) + GetGValue(c2)) / 3.0f,
						//	(GetBValue(c0) + GetBValue(c1) + GetBValue(c2)) / 3.0f
						//);
						}

						color = GetPhongColor(v, normal, lightSource, clr, GetSpecularFromMap(specularMap, std::clamp(uv.x, 0.0f, 1.0f), std::clamp(uv.y, 0.0f, 1.0f)));
					}

					buffer.SetPixel(x, y, color);
				}
				z += Zinc;
			}
		}
	}

	static inline COLORREF GetPhongColor(const glm::vec4& v, const glm::vec3& normal, const LightSource& lightSource, const COLORREF& color, glm::vec3 specular)
	{
		float ambientStrength = 0.1;

		float r, g, b;

		// Ambient
		r = lightSource.color.x * ambientStrength;
		g = lightSource.color.y * ambientStrength;
		b = lightSource.color.z * ambientStrength;

		glm::vec3 lightDir = glm::normalize(lightSource.position - (glm::vec3)v);
		const auto diff = std::max(glm::dot(lightDir, normal), 0.0f);

		// Diffuse
		r += lightSource.color.x * diff;
		g += lightSource.color.y * diff;
		b += lightSource.color.z * diff;

		glm::vec3 viewDir = glm::normalize((glm::vec3)-v);
		glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
		float spec = std::pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), 128);

		// Specular
		r += lightSource.color.x * spec * specular.x;
		g += lightSource.color.y * spec * specular.y;
		b += lightSource.color.z * spec * specular.z;

		r = std::min(r * GetRValue(color), 255.0f);
		g = std::min(g * GetGValue(color), 255.0f);
		b = std::min(b * GetBValue(color), 255.0f);

		return RGB(b, g, r);
	}
};

}