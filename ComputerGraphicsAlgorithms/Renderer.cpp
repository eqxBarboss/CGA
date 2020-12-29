#include "Renderer.h"

#include <thread>
#include <algorithm> 

#include "Math.h"

#include "lodepng.h"

namespace cga
{

int Renderer::width, Renderer::height;
int Renderer::workingThreads;
std::mutex Renderer::mutex;
std::condition_variable Renderer::cv;

std::vector<unsigned char> Renderer::diffuseMap;
unsigned Renderer::diffuseMapWidth, Renderer::diffuseMapHeight;

std::vector<unsigned char> Renderer::specularMap;
unsigned Renderer::specularMapWidth, Renderer::specularMapHeight;

std::vector<unsigned char> Renderer::normalMapLoaded;
std::vector<glm::vec3> Renderer::normalMap;
unsigned Renderer::normalMapWidth, Renderer::normalMapHeight;
std::vector<glm::vec3> Renderer::normalMapTransformed;

Renderer::Renderer(int aWidth, int aHeight, std::function<void()> aInvalidateCallback)
	: aInvalidateCallback(aInvalidateCallback),
	threadCount(std::thread::hardware_concurrency()),
	threadPool(std::thread::hardware_concurrency()),
	buffer(aWidth, aHeight, 0),
	backBuffer(aWidth, aHeight, 0),
	lightSource(glm::vec3(1.0f, 2.5f, 1.5f), glm::vec3(1, 1, 1))
{
	width = aWidth;
	height = aHeight;
	zBuffer = new float[width * height];
	zBufferInitial = new float[width * height];

	for (int i = 0; i < width * height; i++) zBufferInitial[i] = (float)1;
}

Renderer::~Renderer()
{
	delete [] zBuffer;
	delete [] zBufferInitial;
}

Buffer& Renderer::GetCurrentBuffer()
{
	return buffer;
}

void Renderer::Render(std::unique_ptr<Scene> &scene)
{
	renderTarget = scene->obj;
	cameraSpaceVertices = renderTarget.vertices;
	Camera &camera = scene->camera;
	LightSource lightSource = this->lightSource;

	const auto model = glm::mat4(1.0f);
	const auto view = camera.GetViewMatrix();
	const auto projection = GetPerspectiveProjectionMatrix(width, height, 0.1f, 1000.0f, camera.FOV);
	const auto viewPort = GetViewPortMatrix(width, height);

	const auto vm = view * model;
	const auto pvm = projection * vm;
	const glm::mat3 TIvm = glm::transpose(glm::inverse(vm));

	lightSource.position = vm * glm::vec4(lightSource.position, 1.0f);

	int step;
	int tasksToStart;

	// Vertices
	{
		step = (std::max)(renderTarget.vertices.size() / threadCount, renderTarget.vertices.size());
		workingThreads = step == renderTarget.vertices.size() ? 1 : threadCount;
		tasksToStart = workingThreads;

		for (int i = 0; i < tasksToStart; i++)
		{
			threadPool.push(CalculateVertices
				, std::ref<Obj>(renderTarget)
				, std::ref<std::vector<glm::vec4>>(cameraSpaceVertices)
				, i * step
				, i == (tasksToStart - 1) ? renderTarget.vertices.size() : (i + 1) * step
				, std::ref<const glm::mat4>(pvm)
				, std::ref<const glm::mat4>(vm)
				, std::ref<const glm::mat4>(viewPort));
		}

		// Some stuff until waiting
		backBuffer.ClearWithColor(RGB(50, 200, 50));
		ClearZBuffer();

		WaitForThreads();
	}

	// Normals
	{
		step = (std::max)(renderTarget.normals.size() / threadCount, renderTarget.normals.size());
		workingThreads = step == renderTarget.normals.size() ? 1 : threadCount;
		tasksToStart = workingThreads;

		for (int i = 0; i < tasksToStart; i++)
		{
			threadPool.push(CalculateNormals
				, std::ref<Obj>(renderTarget)
				, i * step
				, i == (tasksToStart - 1) ? renderTarget.normals.size() : (i + 1) * step
				, std::ref<const glm::mat3>(TIvm));
		}

		WaitForThreads();
	}

	// Normals
	{
		step = (std::max)(normalMap.size() / threadCount, normalMap.size());
		workingThreads = step == normalMap.size() ? 1 : threadCount;
		tasksToStart = workingThreads;

		for (int i = 0; i < tasksToStart; i++)
		{
			threadPool.push(CalculateNormalsMap
				, std::ref<Obj>(renderTarget)
				, i * step
				, i == (tasksToStart - 1) ? normalMap.size() : (i + 1) * step
				, std::ref<const glm::mat3>(TIvm));
		}

		WaitForThreads();
	}

	// Calculate lighting for polygons and discard by facing
	//{
	//	step = (std::max)(renderTarget.polygons.size() / threadCount, renderTarget.polygons.size());
	//	workingThreads = step == renderTarget.polygons.size() ? 1 : threadCount;
	//	tasksToStart = workingThreads;

	//	for (int i = 0; i < tasksToStart; i++)
	//	{
	//		threadPool.push(CalculateLighting
	//			, std::ref<Obj>(renderTarget)
	//			, std::ref<std::vector<glm::vec4>>(cameraSpaceVertices)
	//			, i * step
	//			, i == (tasksToStart - 1) ? renderTarget.polygons.size() : (i + 1) * step
	//			, lightSource);
	//	}

	//	WaitForThreads();
	//}

	DrawPolygons(std::ref<Buffer>(backBuffer)
		, zBuffer
		, std::ref<Obj>(renderTarget)
		, std::ref<std::vector<glm::vec4>>(cameraSpaceVertices)
		, lightSource
		, 0
		, renderTarget.polygons.size());

	std::swap(buffer.data, backBuffer.data);

	aInvalidateCallback();
}

void Renderer::SetMaps(std::string path) {
	normalMap.clear();
	normalMapLoaded.clear();
	diffuseMap.clear();
    specularMap.clear();
	lodepng::decode(diffuseMap, diffuseMapWidth, diffuseMapHeight, path + "\\Albedo Map.png");
	lodepng::decode(specularMap, specularMapWidth, specularMapHeight, path + "\\Specular Map.png");
	lodepng::decode(normalMapLoaded, normalMapWidth, normalMapHeight, path + "\\Normal Map.png");

	for (int i = 0; i < normalMapLoaded.size(); i += 4) 
		normalMap.push_back(glm::vec3(
		  normalMapLoaded[i] / 255.0f * 2 - 1
		, normalMapLoaded[i + 1] / 255.0f * 2 - 1
		, normalMapLoaded[i + 2] / 255.0f * 2 - 1
	));

	for (int i = 0; i < normalMap.size(); i++) normalMapTransformed.push_back(normalMap[i]);
}

void Renderer::CalculateVertices(int id
	, Obj &renderTarget
	, std::vector<glm::vec4>& cameraSpaceVertices
	, int first
	, int last
	, const glm::mat4 &pvm
	, const glm::mat4& vm
	, const glm::mat4 &viewPort)
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

	for (int i = first; i < last; i++)
	{
		cameraSpaceVertices[i] = vm * cameraSpaceVertices[i];
	}

	FinishThreadWork();
}

void Renderer::CalculateNormals(int id
	, Obj& renderTarget
	, int first
	, int last
	, const glm::mat3& TIvm)
{
	auto& normals = renderTarget.normals;

	for (int i = first; i < last; i++)
	{
		normals[i] = glm::normalize(TIvm * normals[i]);
	}

	FinishThreadWork();
}

void Renderer::CalculateNormalsMap(int id
	, Obj& renderTarget
	, int first
	, int last
	, const glm::mat3& TIvm)
{
	for (int i = first; i < last; i++)
	{
		normalMapTransformed[i] = glm::normalize(TIvm * normalMap[i]);
	}

	FinishThreadWork();
}

//void Renderer::CalculateLighting(int id
//	, Obj& renderTarget
//	, const std::vector<glm::vec4>& cameraSpaceVertices
//	, int first
//	, int last
//	, const LightSource& lightSource)
//{
//	auto& polygons = renderTarget.polygons;
//	const auto& vertices = cameraSpaceVertices;
//	const auto& normals = renderTarget.normals;
//
//	for (int i = first; i < last; i++)
//	{
//		auto& polygon = polygons[i];
//		{
//			int R = 0;
//			int G = 0;
//			int B = 0;
//
//			for (int j = 0; j < 3; j++)
//			{
//				glm::vec3 lightDir = glm::normalize(lightSource.position - (glm::vec3)vertices[polygon.verticesIndices[j]]);
//				const auto dot = glm::dot(lightDir, normals[polygon.normalsIndices[j]]);
//				if (dot < 0) continue;
//				R += GetRValue(lightSource.color) * dot;
//				G += GetGValue(lightSource.color) * dot;
//				B += GetBValue(lightSource.color) * dot;
//			}
//
//			//polygon.color = RGB(R / 3, G / 3, B / 3);
//		}
//	}
//
//	FinishThreadWork();
//}

void Renderer::DrawPolygons(Buffer& buffer, float* zBuffer, Obj& renderTarget, const std::vector<glm::vec4>& cameraSpaceVertices, const LightSource& lightSource, int first, int last)
{
	//int width = std::min((int)diffuseMapWidth, Renderer::width);
	//int height = std::min((int)diffuseMapHeight, Renderer::height);
	//for (int i = 0; i < width; i++)
	//	for (int j = 0; j < height; j++)
	//	{
	//		int textelNumber = (j * diffuseMapWidth + i) * 4;
	//		buffer.SetPixel(i, j, RGB(diffuseMap[textelNumber + 2], diffuseMap[textelNumber + 1], diffuseMap[textelNumber + 0]));
	//	}

	//return;

	for (int j = first; j < last; j++)
	{
		RasterizeTriangle(buffer, zBuffer, renderTarget, cameraSpaceVertices, lightSource, j);
	}
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

void Renderer::ClearZBuffer()
{
	memcpy(zBuffer, zBufferInitial, width * height * sizeof(float));
}

}