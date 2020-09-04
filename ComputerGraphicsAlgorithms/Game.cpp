#include "Game.h"

#include "ObjParser.h"
#include "Math.h"
#include "Camera.h"

namespace cga
{

Game::Game(std::function<int()> aGetTickCountCallback, std::function<void()> aInvalidateCallback, int aWidth, int aHeight)
	: getTickCountCallback(aGetTickCountCallback),
	renderer(aWidth, aHeight, aInvalidateCallback),
	width(aWidth),
	height(aHeight),
	lastX((float)width / 2),
	lastY((float)height / 2),
	keyStates(1024, false)
{
	lastTick = getTickCountCallback();
}

Buffer& Game::GetCurrentBuffer()
{
	return renderer.GetCurrentBuffer();
}

void Game::GameCycle()
{
	if (scene == nullptr) return;

	auto currentTick = getTickCountCallback();
	deltaTime = currentTick - lastTick;
	lastTick = currentTick;
	
	DoMovement();
}

void Game::OnUpdated()
{
	renderer.Render(scene);
}

void Game::DoMovement()
{
	if (scene == nullptr) return;

	Camera &camera = scene->camera;
	bool updated = false;

	if (keyStates[0x57])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
		updated = true;
	}
	if (keyStates[0x53])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		updated = true;
	}
	if (keyStates[0x41])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
		updated = true;
	}
	if (keyStates[0x44])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
		updated = true;
	}

	if (updated)
		OnUpdated();
}

void Game::OnKeyDown(unsigned int virtualKeyCode)
{
	keyStates[virtualKeyCode] = true;
}

void Game::OnKeyUp(unsigned int virtualKeyCode)
{
	keyStates[virtualKeyCode] = false;
}

void Game::OnMouseMove(int newX, int newY)
{
	if (scene == nullptr) return;

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

	scene->camera.ProcessMouseMovement(xoffset, yoffset);
	OnUpdated();
}

void Game::OnWheelScroll(int delta)
{
	if (scene == nullptr) return;

	if (delta == 0)
		return;

	scene->camera.ProcessMouseScroll(delta);
	OnUpdated();
}

void Game::LoadScene(std::string pathToObject)
{
	ObjParser parser;
	auto loadedObj = parser.Parse(pathToObject);
	if (loadedObj)
	{
		scene = std::make_unique<Scene>(Camera(glm::vec3(0.0f, 0.0f, 2.5f)), loadedObj.value());
	}

	OnUpdated();
}

}