#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace cga 
{

class Polygon
{
public:
	std::vector<int> verticesIndices;
	std::vector<int> textureIndices;
	std::vector<int> normalsIndices;
};

class Obj
{
public:
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec3> textureCoords;
	std::vector<glm::vec3> normals;
	std::vector<Polygon> polygons;
};

}
