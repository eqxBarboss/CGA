#include "Game.h"

#include "ObjParser.h"
#include "Math.h"

namespace cga
{

Game::Game(GetTickCountCallback aGetTickCountCallback, Callback aInvalidateCallback)
	: getTickCountCallback(aGetTickCountCallback),
	invalidatedCallback(aInvalidateCallback),
	camera(glm::vec3(0.0f, 0.0f, 2.5f))
{
	lastTick = getTickCountCallback();
}

void Game::GameCycle()
{
	auto currentTick = getTickCountCallback();
	deltaTime = currentTick - lastTick;
	lastTick = currentTick;
	
	DoMovement();
}

void Game::DoMovement()
{
	bool updated = false;

	if (w)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
		updated = true;
	}
	if (s)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		updated = true;
	}
	if (a)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
		updated = true;
	}
	if (d)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
		updated = true;
	}	
	if (up)
	{
		vertAngle -= AngleStep * deltaTime;
		updated = true;
	}
	if (down)
	{
		vertAngle += AngleStep * deltaTime;
		updated = true;

	}
	if (left)
	{
		horAngle += AngleStep * deltaTime;
		updated = true;
	}
	if (right)
	{
		horAngle -= AngleStep * deltaTime;
		updated = true;
	}

	if (updated)
		UpdateScene();
}

void Game::OnKeyDown(unsigned int virtualKeyCode)
{
	if (virtualKeyCode == 0x57) w = true;
	if (virtualKeyCode == 0x41) a = true;
	if (virtualKeyCode == 0x53) s = true;
	if (virtualKeyCode == 0x44) d = true;
	if (virtualKeyCode == 0x26) up = true;
	if (virtualKeyCode == 0x28) down = true;
	if (virtualKeyCode == 0x25) left = true;
	if (virtualKeyCode == 0x27) right = true;
}

void Game::OnKeyUp(unsigned int virtualKeyCode)
{
	if (virtualKeyCode == 0x57) w = false;
	if (virtualKeyCode == 0x41) a = false;
	if (virtualKeyCode == 0x53) s = false;
	if (virtualKeyCode == 0x44) d = false;
	if (virtualKeyCode == 0x26) up = false;
	if (virtualKeyCode == 0x28) down = false;
	if (virtualKeyCode == 0x25) left = false;
	if (virtualKeyCode == 0x27) right = false;
}

void Game::OnMouseMove(int newX, int newY)
{
	if (firstMouse)
	{
		lastX = newX;
		lastY = newY;
		firstMouse = false;
	}

	float xoffset = newX - lastX;
	float yoffset = lastY - newY;

	if (xoffset == 0 && yoffset == 0)
		return;

	lastX = newX;
	lastY = newY;

	camera.ProcessMouseMovement(xoffset, yoffset);
	UpdateScene();
}

void Game::OnWheelScroll(int delta)
{
	if (delta == 0)
		return;

	camera.ProcessMouseScroll(delta);
	UpdateScene();
}

void Game::UpdateScene()
{
	auto model = glm::mat4(1.0f);
	model = model * GetRotationAroundYMatrix(glm::radians(horAngle));
	model = model * GetRotationAroundXMatrix(glm::radians(vertAngle));

	const auto view = camera.GetViewMatrix();
	const auto projection = GetPerspectiveProjectionMatrix(width, height, 0.1f, 1000.0f, camera.FOV);
	//const auto projection = GetOrthographicProjectionMatrix(10, 10, 0.1f, 1000.0f);
	const auto viewPort = GetViewPortMatrix(width, height);

	const auto pvm = projection * view * model;

	for (int i = 0; i < initialObj.vertices.size(); i++)
	{
		obj.vertices[i] = pvm * initialObj.vertices[i];
		//obj.vertices[i] = model * initialObj.vertices[i];
		//obj.vertices[i] = view * obj.vertices[i];
		//obj.vertices[i] = projection * obj.vertices[i];

		obj.vertices[i].x = obj.vertices[i].x / obj.vertices[i].w;
		obj.vertices[i].y = obj.vertices[i].y / obj.vertices[i].w;
		obj.vertices[i].z = obj.vertices[i].z / obj.vertices[i].w;
		obj.vertices[i].w = 1.0f;

		//if (obj.vertices[i].x > 1 || obj.vertices[i].x < -1 || obj.vertices[i].y > 1 || obj.vertices[i].y < -1 ||
		//	obj.vertices[i].z > 1 || obj.vertices[i].z < -1)
		//{
		//	obj.vertices[i] = glm::vec4(0, 0, 0, 0);
		//	continue;
		//}

		obj.vertices[i] = viewPort * obj.vertices[i];
	}

	invalidatedCallback();
}

void Game::ReloadScene(std::string pathToObject)
{
	ObjParser parser;
	auto loadedObj = parser.Parse(pathToObject);
	if (loadedObj)
	{
		initialObj = loadedObj.value();
		obj = initialObj;
	}

	UpdateScene();
}

}