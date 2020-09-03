#include "ObjParser.h"

#include <iostream>
#include <fstream>

namespace cga
{

std::optional<Obj> ObjParser::Parse(std::string fileName)
{
	std::ifstream targetFile(fileName);
	if (targetFile.is_open())
	{
		Obj obj;
		std::string line;

		while (std::getline(targetFile, line))
		{
			if (line.substr(0, 2) == "v ")
			{
				if (!ExtractVertex(obj, line)) return {};
			}
			else if (line.substr(0, 2) == "vt")
			{
				if (!ExtractTextureCoords(obj, line)) return {};
			}
			else if (line.substr(0, 2) == "vn")
			{
				if (!ExtractNormal(obj, line)) return {};
			}
			else if (line.substr(0, 2) == "f ")
			{
				if (!ExtractFace(obj, line)) return {};
			}
			// TODO: Possibly add check for the same number of values read on each category with overall number (for category) in the file
		}

		return obj;
	}

	return {};
}

}