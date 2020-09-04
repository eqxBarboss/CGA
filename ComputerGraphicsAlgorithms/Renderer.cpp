#include "Renderer.h"

#include <thread>
#include <algorithm>

#include "Math.h"

namespace cga
{

int Renderer::workingThreads;
std::mutex Renderer::mutex;
std::condition_variable Renderer::cv;

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

	auto x = a.x;
	auto y = a.y;

	for (int i = 0; i <= steps; i++)
	{
		buffer.SetPixel(x, y, RGB(255, 255, 255));
		x += Xinc;
		y += Yinc;
	}
}

void Renderer::Render(std::unique_ptr<Scene> &scene)
{
	Obj renderTarget = scene->obj;
	Camera &camera = scene->camera;

	const auto model = glm::mat4(1.0f);
	const auto view = camera.GetViewMatrix();
	const auto projection = GetPerspectiveProjectionMatrix(width, height, 0.1f, 1000.0f, camera.FOV);
	const auto viewPort = GetViewPortMatrix(width, height);

	const auto pvm = projection * view * model;

	int step = (std::max)(renderTarget.vertices.size() / threadCount, renderTarget.vertices.size());
	workingThreads = step == renderTarget.vertices.size() ? 1 : threadCount;
	int tasksToStart = workingThreads;

	for (int i = 0; i < tasksToStart; i++)
	{
		threadPool.push(CalculateVertices
						, std::ref<Obj>(renderTarget)
						, i * step
						, i == (tasksToStart - 1) ? renderTarget.vertices.size() : (i + 1) * step
						, std::ref<const glm::mat4>(pvm)
						, std::ref<const glm::mat4>(viewPort));
	}

	// Some stuff until waiting
	backBuffer.ClearWithColor(0);

	WaitForThreads();

	step = (std::max)(renderTarget.polygons.size() / threadCount, renderTarget.polygons.size());
	workingThreads = step == renderTarget.polygons.size() ? 1 : threadCount;
	tasksToStart = workingThreads;

	for (int i = 0; i < tasksToStart; i++)
	{
		threadPool.push(DrawPolygons
						, std::ref<Buffer>(backBuffer)
						, std::ref<Obj>(renderTarget)
						, i * step
						, i == (tasksToStart - 1) ? renderTarget.polygons.size() : (i + 1) * step);
	}
	
	WaitForThreads();

	auto temp = buffer.data;
	buffer.data = backBuffer.data;
	backBuffer.data = temp;

	aInvalidateCallback();
}

void Renderer::CalculateVertices(int id, Obj &renderTarget, int first, int last, const glm::mat4 &pvm, const glm::mat4 &viewPort)
{
	auto &vertices = renderTarget.vertices;

	for (int i = first; i < last; i++)
	{
		vertices[i] = pvm * vertices[i];

		vertices[i].x = vertices[i].x / vertices[i].w;
		vertices[i].y = vertices[i].y / vertices[i].w;
		vertices[i].z = vertices[i].z / vertices[i].w;
		vertices[i].w = 1.0f;

		vertices[i] = viewPort * vertices[i];
	}

	FinishThreadWork();
}

void Renderer::DrawPolygons(int id, Buffer &buffer, Obj &renderTarget, int first, int last)
{
	auto &vertices = renderTarget.vertices;
	auto &polygons = renderTarget.polygons;

	for (int j = first; j < last; j++)
	{
		for (int i = 0; i < polygons[j].verticesIndices.size() - 1; i++)
		{
			RasterizeLine(buffer, vertices[polygons[j].verticesIndices[i] - 1], vertices[polygons[j].verticesIndices[i + 1] - 1]);
		}

		RasterizeLine(buffer, vertices[polygons[j].verticesIndices[polygons[j].verticesIndices.size() - 1] - 1], vertices[polygons[j].verticesIndices[0] - 1]);
	}

	FinishThreadWork();
}

void Renderer::WaitForThreads()
{
	std::unique_lock<std::mutex> lock(mutex);
	if (workingThreads != 0)
	{
		cv.wait(lock);
	}
}

void Renderer::FinishThreadWork()
{
	std::unique_lock<std::mutex> lock(mutex);
	workingThreads--;
	if (workingThreads == 0)
	{
		lock.unlock();
		cv.notify_one();
	}
}

}