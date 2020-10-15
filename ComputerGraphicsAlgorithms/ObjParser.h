#pragma once

#include <string>
#include <optional>
#include <sstream>
#include <stdio.h>

#include "Obj.h"

namespace cga
{

class ObjParser
{
public:
	std::optional<Obj> Parse(std::string fileName);

protected:
	inline bool ExtractVertex(Obj& targetObj, std::string string)
	{
		std::istringstream vertexStringStream(string.substr(2));
		glm::vec4 vertex(0, 0, 0, 1);

		vertexStringStream >> vertex.x;
		vertexStringStream >> vertex.y;
		vertexStringStream >> vertex.z;

		if (!vertexStringStream.eof())
		{
			vertexStringStream >> vertex.w;
		}

		if (!vertexStringStream.fail() && vertexStringStream.eof())
		{
			targetObj.vertices.push_back(vertex);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool ExtractTextureCoords(Obj& targetObj, std::string string)
	{
		std::istringstream textureCoordsStringStream(string.substr(2));
		glm::vec3 textureCords(0, 0, 0);

		textureCoordsStringStream >> textureCords.x;

		if (!textureCoordsStringStream.eof())
		{
			textureCoordsStringStream >> textureCords.y;
		}

		if (!textureCoordsStringStream.eof())
		{
			textureCoordsStringStream >> textureCords.z;
		}

		if (!textureCoordsStringStream.fail() && textureCoordsStringStream.eof())
		{
			targetObj.textureCoords.push_back(textureCords);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool ExtractNormal(Obj& targetObj, std::string string)
	{
		std::istringstream normalStringStream(string.substr(2));
		glm::vec3 normal;

		normalStringStream >> normal.x;
		normalStringStream >> normal.y;
		normalStringStream >> normal.z;

		if (!normalStringStream.fail() && normalStringStream.eof())
		{
			targetObj.normals.push_back(normal);
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool ExtractFace(Obj& targetObj, std::string string)
	{
		std::istringstream faceStringStream(string.substr(2));
		Polygon polygon;
		std::string tuple;

		// TODO: Improve parsing (include '//' cases etc.)
		while (std::getline(faceStringStream, tuple, ' '))
		{	
			int vertexIndex, textureCoordIndex, normalIndex;
			if (sscanf_s(tuple.c_str(), "%d/%d/%d", &vertexIndex, &textureCoordIndex, &normalIndex) != 3)
			{
				return false;
			}
			polygon.verticesIndices.push_back(vertexIndex - 1);
			polygon.textureIndices.push_back(textureCoordIndex - 1);
			polygon.normalsIndices.push_back(normalIndex - 1);
		}

		for (int i = 0; i < polygon.verticesIndices.size(); i += 3)
		{
			Polygon p;
			int start = (i + 2 < polygon.verticesIndices.size()) ? i : i - 2;
			for (int j = start; j <= start + 2; j++)
			{
				p.verticesIndices.push_back(polygon.verticesIndices[j]);
				p.textureIndices.push_back(polygon.textureIndices[j]);
				p.normalsIndices.push_back(polygon.normalsIndices[j]);
			}
			targetObj.polygons.push_back(p);
		}

		//targetObj.polygons.push_back(polygon);
			
		return true;
	}
};

}
