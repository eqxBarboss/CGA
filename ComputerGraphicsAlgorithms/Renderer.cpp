#include "Renderer.h"

#include "Math.h"

namespace cga
{

Renderer::Renderer(int aWidth, int aHeight, HDC aDeviceContext, std::function<void()> aInvalidateCallback)
	: width(aWidth),
	height(aHeight),
	deviceContext(aDeviceContext),
	aInvalidateCallback(aInvalidateCallback)
{

}

inline void RasterizeLine(HDC dc, glm::vec4 a, glm::vec4 b)
{
	MoveToEx(dc, a.x, a.y, NULL);
	LineTo(dc, b.x, b.y);

	//auto dx = b.x - a.x;
	//auto dy = b.y - a.y;

	//auto steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	//auto Xinc = dx / (float)steps;
	//auto Yinc = dy / (float)steps;

	//auto X = a.x;
	//auto Y = a.y;

	//for (int i = 0; i <= steps; i++)
	//{
	//	SetPixel(dc, X, Y, RGB(0, 0, 0));
	//	X += Xinc;
	//	Y += Yinc;
	//}
}

void Renderer::Render(std::unique_ptr<Scene> &scene)
{
	Obj obj = scene->obj;
	Camera &camera = scene->camera;

	auto model = glm::mat4(1.0f);
	const auto view = camera.GetViewMatrix();
	const auto projection = GetPerspectiveProjectionMatrix(width, height, 0.1f, 1000.0f, camera.FOV);
	const auto viewPort = GetViewPortMatrix(width, height);

	const auto pvm = projection * view * model;

	for (int i = 0; i < obj.vertices.size(); i++)
	{
		obj.vertices[i] = pvm * obj.vertices[i];

		obj.vertices[i].x = obj.vertices[i].x / obj.vertices[i].w;
		obj.vertices[i].y = obj.vertices[i].y / obj.vertices[i].w;
		obj.vertices[i].z = obj.vertices[i].z / obj.vertices[i].w;
		obj.vertices[i].w = 1.0f;

		obj.vertices[i] = viewPort * obj.vertices[i];
	}

	SelectObject(deviceContext, GetStockObject(WHITE_BRUSH));
	Rectangle(deviceContext, 0, 0, width, height);

	for (const auto& polygon : obj.polygons)
	{
		for (int i = 0; i < polygon.verticesIndices.size() - 1; i++)
		{
			RasterizeLine(deviceContext, obj.vertices[polygon.verticesIndices[i] - 1], obj.vertices[polygon.verticesIndices[i + 1] - 1]);
		}

		RasterizeLine(deviceContext, obj.vertices[polygon.verticesIndices[polygon.verticesIndices.size() - 1] - 1], obj.vertices[polygon.verticesIndices[0] - 1]);
	}

	aInvalidateCallback();
}

}