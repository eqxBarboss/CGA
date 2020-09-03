#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "framework.h"

#include "Scene.h"
#include "Renderer.h"

namespace cga
{

const int AngleStep = 1;

class Game
{
public:
	Game(std::function<int()> aGetTickCountCallback, std::function<void()> aInvalidateCallback, HDC aDeviceContext, int aWidth, int aHeight);

	// event processors
	void OnKeyDown(unsigned int virtualKeyCode);
	void OnKeyUp(unsigned int virtualKeyCode);
	void OnMouseMove(int newX, int newY);
	void OnWheelScroll(int delta);

	void LoadScene(std::string pathToObject);

	void GameCycle();

private:
	std::unique_ptr<Scene> scene;
	Renderer renderer;

	unsigned long long lastTick, deltaTime = 0;

	std::vector<bool> keyStates;

	int width, height;
	int lastX, lastY;
	bool firstMouse = true;

	// callbacks
	std::function<int()> getTickCountCallback;

	void DoMovement();

	void OnUpdated();
};

}
