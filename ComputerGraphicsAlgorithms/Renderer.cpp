#include "Renderer.h"

#include <thread>

#include "Math.h"

namespace cga
{

Renderer::Renderer(int aWidth, int aHeight, std::function<void()> aInvalidateCallback)
	: width(aWidth),
	height(aHeight),
	aInvalidateCallback(aInvalidateCallback),
	threadCount(std::thread::hardware_concurrency()),
	threadPool(std::thread::hardware_concurrency()),
	buffer(aWidth, aHeight, 0),
	backBuffer(aWidth, aHeight, 0)
{

}

Buffer& Renderer::GetCurrentBuffer()
{
	return buffer;
}

inline void RasterizeLine(Buffer &buffer, glm::vec4 a, glm::vec4 b)
{
	auto dx = b.x - a.x;
	auto dy = b.y - a.y;

	auto steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	auto Xinc = dx / (float)steps;
	auto Yinc = dy / (float)steps;

	auto X = a.x;
	auto Y = a.y;

	for (int i = 0; i <= steps; i++)
	{
		buffer.SetPixel(X, Y, RGB(255, 255, 255));
		X += Xinc;
		Y += Yinc;
	}
}

void Renderer::Render(std::unique_ptr<Scene> &scene)
{
	Obj obj = scene->obj;
	Camera &camera = scene->camera;

	const auto model = glm::mat4(1.0f);
	const auto view = camera.GetViewMatrix();
	const auto projection = GetPerspectiveProjectionMatrix(width, height, 0.1f, 1000.0f, camera.FOV);
	const auto viewPort = GetViewPortMatrix(width, height);

	const auto pvm = projection * view * model;

	int step = obj.vertices.size() / threadCount;
	if (step == 0) step = obj.vertices.size();
	int first = 0;

	while (1)
	{
		int last = first + step;
		if (last > obj.vertices.size()) last = obj.vertices.size();

		threadPool.push([this, &obj, first, last, pvm, viewPort](int id)
			{
				this->CalculateVertices(obj.vertices, first, last, pvm, viewPort);
			});

		if (last == obj.vertices.size()) break;
		first += step;
	}

	while (1)
	{
		if (threadPool.n_idle() == threadCount) break;
	}

	//CalculateVertices(obj.vertices, 0, obj.vertices.size(), pvm, viewPort);

	backBuffer.ClearWithColor(0);

	DrawPolygons(0, backBuffer, obj.polygons, obj.vertices, 0, obj.polygons.size());

	//step = obj.polygons.size() / threadCount;
	//if (step == 0) step = obj.polygons.size();
	//first = 0;

	//while (1)
	//{
	//	int last = first + step;
	//	if (last > obj.polygons.size()) last = obj.polygons.size();

	//	threadPool.push([this, &obj, first, last](int id)
	//		{
	//			this->DrawPolygons(this->backBuffer, obj.polygons, obj.vertices, first, last);
	//		});

	//	if (last == obj.polygons.size()) break;
	//	first += step;
	//}

	//while (1)
	//{
	//	if (threadPool.n_idle() == threadCount) break;
	//}

	auto temp = buffer.data;
	buffer.data = backBuffer.data;
	backBuffer.data = temp;

	aInvalidateCallback();
}

void Renderer::CalculateVertices(std::vector<glm::vec4> &vertices, int first, int last, glm::mat4 pvm, glm::mat4 viewPort)
{
	for (int i = first; i < last; i++)
	{
		vertices[i] = pvm * vertices[i];

		vertices[i].x = vertices[i].x / vertices[i].w;
		vertices[i].y = vertices[i].y / vertices[i].w;
		vertices[i].z = vertices[i].z / vertices[i].w;
		vertices[i].w = 1.0f;

		vertices[i] = viewPort * vertices[i];
	}
}

void Renderer::DrawPolygons(int id, Buffer &buffer, std::vector<Polygon> &polygons, std::vector<glm::vec4> &vertices, int first, int last)
{
	for (int j = first; j < last; j++)
	{
		for (int i = 0; i < polygons[j].verticesIndices.size() - 1; i++)
		{
			RasterizeLine(buffer, vertices[polygons[j].verticesIndices[i] - 1], vertices[polygons[j].verticesIndices[i + 1] - 1]);
		}

		RasterizeLine(buffer, vertices[polygons[j].verticesIndices[polygons[j].verticesIndices.size() - 1] - 1], vertices[polygons[j].verticesIndices[0] - 1]);
	}
}

}