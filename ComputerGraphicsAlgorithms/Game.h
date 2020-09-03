#pragma once

#include <string>

#include "Camera.h"
#include "Obj.h"

namespace cga
{

const int AngleStep = 1;

typedef unsigned long long (*GetTickCountCallback)();
typedef void (*Callback)();

class Game
{
public:
	const int height = 1080;
	const int width = 1920;

	Obj initialObj, obj;

	Game(GetTickCountCallback aGetTickCountCallback, Callback aInvalidateCallback);

	// event processors
	void OnKeyDown(unsigned int virtualKeyCode);
	void OnKeyUp(unsigned int virtualKeyCode);
	void OnMouseMove(int newX, int newY);
	void OnWheelScroll(int delta);

	void ReloadScene(std::string pathToObject);

	void GameCycle();

protected:
	Camera camera;
	unsigned long long lastTick, deltaTime = 0;

	bool w = false, a = false, s = false, d = false, right = false, left = false, up = false, down = false;

	float horAngle = 0, vertAngle = 0;

	int lastX = width / 2, lastY = height / 2;
	bool firstMouse = true;

	// callbacks
	GetTickCountCallback getTickCountCallback;
	Callback invalidatedCallback;

	void DoMovement();

	void UpdateScene();
};

}
