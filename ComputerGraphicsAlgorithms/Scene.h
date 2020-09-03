#pragma once

#include "Camera.h"
#include "Obj.h"

namespace cga
{

class Scene
{
public:
	Camera camera;
	Obj obj;

	Scene(Camera aCamera, Obj aObj);
};

}
